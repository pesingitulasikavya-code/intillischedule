#include "database.hpp"
#include <sqlite3.h>
#include <iostream>
#include <sstream>

namespace intellischedule {

Database::Database(const std::string& dbPath) : dbPath_(dbPath) {}

Database::~Database() {
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

bool Database::initialize() {
    int rc = sqlite3_open(dbPath_.c_str(), &db_);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to open SQLite database: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }

    const char* schema = R"(
        CREATE TABLE IF NOT EXISTS tasks (
            id TEXT PRIMARY KEY,
            title TEXT NOT NULL,
            description TEXT,
            duration INTEGER NOT NULL DEFAULT 1,
            deadline INTEGER,
            value REAL NOT NULL DEFAULT 1.0,
            priority INTEGER NOT NULL DEFAULT 1,
            start_time INTEGER,
            finish_time INTEGER,
            prerequisite_ids TEXT
        );

        CREATE TABLE IF NOT EXISTS scenarios (
            id TEXT PRIMARY KEY,
            name TEXT NOT NULL,
            description TEXT,
            mode TEXT NOT NULL,
            time_budget INTEGER NOT NULL DEFAULT 40,
            planning_horizon INTEGER NOT NULL DEFAULT 100
        );

        CREATE TABLE IF NOT EXISTS scenario_tasks (
            scenario_id TEXT NOT NULL,
            task_id TEXT NOT NULL,
            PRIMARY KEY (scenario_id, task_id),
            FOREIGN KEY (scenario_id) REFERENCES scenarios(id) ON DELETE CASCADE,
            FOREIGN KEY (task_id) REFERENCES tasks(id) ON DELETE CASCADE
        );

        CREATE TABLE IF NOT EXISTS schedule_runs (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            scenario_id TEXT,
            mode TEXT NOT NULL,
            algorithm_name TEXT NOT NULL,
            total_value REAL NOT NULL,
            total_duration INTEGER NOT NULL,
            scheduled_count INTEGER NOT NULL,
            execution_time_ms REAL NOT NULL,
            is_valid INTEGER NOT NULL,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        );
    )";

    char* errMsg = nullptr;
    rc = sqlite3_exec(db_, schema, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to create tables: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }

    seedDefaultScenarios();
    return true;
}

static std::string joinPrereqs(const std::vector<std::string>& list) {
    std::ostringstream oss;
    for (size_t i = 0; i < list.size(); ++i) {
        oss << list[i];
        if (i + 1 < list.size()) oss << ",";
    }
    return oss.str();
}

static std::vector<std::string> splitPrereqs(const std::string& str) {
    std::vector<std::string> result;
    if (str.empty()) return result;
    std::stringstream ss(str);
    std::string item;
    while (std::getline(ss, item, ',')) {
        if (!item.empty()) result.push_back(item);
    }
    return result;
}

bool Database::saveTask(const Task& task) {
    const char* sql = R"(
        INSERT OR REPLACE INTO tasks (id, title, description, duration, deadline, value, priority, start_time, finish_time, prerequisite_ids)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?);
    )";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, task.id.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, task.title.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, task.description.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, task.duration);

    if (task.deadline.has_value()) sqlite3_bind_int(stmt, 5, *task.deadline);
    else sqlite3_bind_null(stmt, 5);

    sqlite3_bind_double(stmt, 6, task.value);
    sqlite3_bind_int(stmt, 7, task.priority);

    if (task.startTime.has_value()) sqlite3_bind_int(stmt, 8, *task.startTime);
    else sqlite3_bind_null(stmt, 8);

    if (task.finishTime.has_value()) sqlite3_bind_int(stmt, 9, *task.finishTime);
    else sqlite3_bind_null(stmt, 9);

    std::string prereqStr = joinPrereqs(task.prerequisiteIds);
    sqlite3_bind_text(stmt, 10, prereqStr.c_str(), -1, SQLITE_TRANSIENT);

    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return ok;
}

bool Database::updateTask(const Task& task) {
    return saveTask(task);
}

bool Database::deleteTask(const std::string& id) {
    const char* sql = "DELETE FROM tasks WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_STATIC);
    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return ok;
}

std::optional<Task> Database::getTask(const std::string& id) {
    const char* sql = "SELECT id, title, description, duration, deadline, value, priority, start_time, finish_time, prerequisite_ids FROM tasks WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return std::nullopt;
    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        Task t;
        t.id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        t.title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        const char* desc = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        if (desc) t.description = desc;
        t.duration = sqlite3_column_int(stmt, 3);
        if (sqlite3_column_type(stmt, 4) != SQLITE_NULL) t.deadline = sqlite3_column_int(stmt, 4);
        t.value = sqlite3_column_double(stmt, 5);
        t.priority = sqlite3_column_int(stmt, 6);
        if (sqlite3_column_type(stmt, 7) != SQLITE_NULL) t.startTime = sqlite3_column_int(stmt, 7);
        if (sqlite3_column_type(stmt, 8) != SQLITE_NULL) t.finishTime = sqlite3_column_int(stmt, 8);
        const char* prereqs = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
        if (prereqs) t.prerequisiteIds = splitPrereqs(prereqs);
        sqlite3_finalize(stmt);
        return t;
    }
    sqlite3_finalize(stmt);
    return std::nullopt;
}

std::vector<Task> Database::getAllTasks() {
    std::vector<Task> list;
    const char* sql = "SELECT id, title, description, duration, deadline, value, priority, start_time, finish_time, prerequisite_ids FROM tasks ORDER BY id;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return list;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Task t;
        t.id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        t.title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        const char* desc = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        if (desc) t.description = desc;
        t.duration = sqlite3_column_int(stmt, 3);
        if (sqlite3_column_type(stmt, 4) != SQLITE_NULL) t.deadline = sqlite3_column_int(stmt, 4);
        t.value = sqlite3_column_double(stmt, 5);
        t.priority = sqlite3_column_int(stmt, 6);
        if (sqlite3_column_type(stmt, 7) != SQLITE_NULL) t.startTime = sqlite3_column_int(stmt, 7);
        if (sqlite3_column_type(stmt, 8) != SQLITE_NULL) t.finishTime = sqlite3_column_int(stmt, 8);
        const char* prereqs = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
        if (prereqs) t.prerequisiteIds = splitPrereqs(prereqs);
        list.push_back(t);
    }
    sqlite3_finalize(stmt);
    return list;
}

bool Database::saveScenario(const Scenario& s) {
    const char* sql = R"(
        INSERT OR REPLACE INTO scenarios (id, name, description, mode, time_budget, planning_horizon)
        VALUES (?, ?, ?, ?, ?, ?);
    )";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, s.id.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, s.name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, s.description.c_str(), -1, SQLITE_STATIC);
    std::string mStr = modeToString(s.mode);
    sqlite3_bind_text(stmt, 4, mStr.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 5, s.timeBudget);
    sqlite3_bind_int(stmt, 6, s.planningHorizon);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return false;
    }
    sqlite3_finalize(stmt);

    // Save tasks and link
    for (const auto& t : s.tasks) {
        saveTask(t);
        const char* linkSql = "INSERT OR IGNORE INTO scenario_tasks (scenario_id, task_id) VALUES (?, ?);";
        sqlite3_stmt* linkStmt = nullptr;
        if (sqlite3_prepare_v2(db_, linkSql, -1, &linkStmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(linkStmt, 1, s.id.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(linkStmt, 2, t.id.c_str(), -1, SQLITE_STATIC);
            sqlite3_step(linkStmt);
            sqlite3_finalize(linkStmt);
        }
    }
    return true;
}

std::optional<Scenario> Database::getScenario(const std::string& id) {
    const char* sql = "SELECT id, name, description, mode, time_budget, planning_horizon FROM scenarios WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return std::nullopt;
    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        Scenario s;
        s.id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        s.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        const char* desc = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        if (desc) s.description = desc;
        s.mode = stringToMode(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)));
        s.timeBudget = sqlite3_column_int(stmt, 4);
        s.planningHorizon = sqlite3_column_int(stmt, 5);
        sqlite3_finalize(stmt);

        // Fetch associated tasks
        const char* tSql = R"(
            SELECT t.id, t.title, t.description, t.duration, t.deadline, t.value, t.priority, t.start_time, t.finish_time, t.prerequisite_ids
            FROM tasks t
            JOIN scenario_tasks st ON t.id = st.task_id
            WHERE st.scenario_id = ?
            ORDER BY t.id;
        )";
        sqlite3_stmt* tStmt = nullptr;
        if (sqlite3_prepare_v2(db_, tSql, -1, &tStmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(tStmt, 1, id.c_str(), -1, SQLITE_STATIC);
            while (sqlite3_step(tStmt) == SQLITE_ROW) {
                Task t;
                t.id = reinterpret_cast<const char*>(sqlite3_column_text(tStmt, 0));
                t.title = reinterpret_cast<const char*>(sqlite3_column_text(tStmt, 1));
                const char* tDesc = reinterpret_cast<const char*>(sqlite3_column_text(tStmt, 2));
                if (tDesc) t.description = tDesc;
                t.duration = sqlite3_column_int(tStmt, 3);
                if (sqlite3_column_type(tStmt, 4) != SQLITE_NULL) t.deadline = sqlite3_column_int(tStmt, 4);
                t.value = sqlite3_column_double(tStmt, 5);
                t.priority = sqlite3_column_int(tStmt, 6);
                if (sqlite3_column_type(tStmt, 7) != SQLITE_NULL) t.startTime = sqlite3_column_int(tStmt, 7);
                if (sqlite3_column_type(tStmt, 8) != SQLITE_NULL) t.finishTime = sqlite3_column_int(tStmt, 8);
                const char* prereqs = reinterpret_cast<const char*>(sqlite3_column_text(tStmt, 9));
                if (prereqs) t.prerequisiteIds = splitPrereqs(prereqs);
                s.tasks.push_back(t);
            }
            sqlite3_finalize(tStmt);
        }
        return s;
    }
    sqlite3_finalize(stmt);
    return std::nullopt;
}

std::vector<Scenario> Database::getAllScenarios() {
    std::vector<Scenario> list;
    const char* sql = "SELECT id FROM scenarios ORDER BY id;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return list;

    std::vector<std::string> ids;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        ids.push_back(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
    }
    sqlite3_finalize(stmt);

    for (const auto& id : ids) {
        auto s = getScenario(id);
        if (s.has_value()) list.push_back(*s);
    }
    return list;
}

bool Database::recordScheduleRun(const std::string& scenarioId, const ScheduleResult& r) {
    const char* sql = R"(
        INSERT INTO schedule_runs (scenario_id, mode, algorithm_name, total_value, total_duration, scheduled_count, execution_time_ms, is_valid)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?);
    )";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, scenarioId.c_str(), -1, SQLITE_STATIC);
    std::string mStr = modeToString(r.mode);
    sqlite3_bind_text(stmt, 2, mStr.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, r.algorithmName.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 4, r.totalValue);
    sqlite3_bind_int(stmt, 5, r.totalDuration);
    sqlite3_bind_int(stmt, 6, r.scheduledCount);
    sqlite3_bind_double(stmt, 7, r.executionTimeMs);
    sqlite3_bind_int(stmt, 8, r.isValidSchedule ? 1 : 0);

    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return ok;
}

void Database::seedDefaultScenarios() {
    // Check if already seeded
    auto list = getAllScenarios();
    if (!list.empty()) return;

    // Faculty Demo 1: Mode A - Sprint Planning Time Budget (DP vs Greedy counterexample)
    Scenario s1;
    s1.id = "scenario-mode-a-sprint";
    s1.name = "Sprint Planning: Maximum Value Selection (Knapsack)";
    s1.description = "Product backlog optimization under a 16-hour sprint budget. Demonstrates where the greedy ratio heuristic fails and 0/1 Knapsack DP achieves strictly higher utility.";
    s1.mode = SchedulingMode::MODE_A_MAX_VALUE_KNAPSACK;
    s1.timeBudget = 16;
    s1.planningHorizon = 24;

    s1.tasks = {
        {"TASK-A1", "Security Patch Authentication Fix", "High security vulnerability CVE-2026-901", 6, 20, 18.0, 1, std::nullopt, std::nullopt, {}},
        {"TASK-A2", "Database Index Optimization", "Optimize Postgres slow query latency", 10, 20, 25.0, 2, std::nullopt, std::nullopt, {}},
        {"TASK-A3", "Payment Gateway Webhook Retry", "Reliable automated retry on idempotency keys", 10, 20, 24.0, 1, std::nullopt, std::nullopt, {}},
        {"TASK-A4", "Redis Cache Invalidation Service", "Low latency cache refresh on product updates", 5, 20, 11.0, 2, std::nullopt, std::nullopt, {}},
        {"TASK-A5", "Analytics Dashboard Export CSV", "Self-service business metrics export", 3, 20, 5.0, 3, std::nullopt, std::nullopt, {}}
    };
    saveScenario(s1);

    // Faculty Demo 2: Mode B - Executive Conference Room Allocation (Interval Scheduling EFT)
    Scenario s2;
    s2.id = "scenario-mode-b-conference";
    s2.name = "Executive Conference Room Scheduling (Earliest Finish Time)";
    s2.description = "Selecting the maximum cardinality set of non-overlapping meeting requests for Boardroom A via Earliest Finish Time (EFT) greedy algorithm.";
    s2.mode = SchedulingMode::MODE_B_INTERVAL_SCHEDULING;
    s2.timeBudget = 12;
    s2.planningHorizon = 12;

    s2.tasks = {
        {"MEET-01", "Morning Standup & Sync", "Engineering leadership sync", 2, std::nullopt, 10.0, 1, 1, 3, {}},
        {"MEET-02", "Client Architecture Presentation", "Quarterly strategic review with enterprise partner", 4, std::nullopt, 20.0, 1, 2, 6, {}},
        {"MEET-03", "AI Infrastructure Retrospective", "Post-mortem review on cluster utilization", 2, std::nullopt, 12.0, 2, 3, 5, {}},
        {"MEET-04", "Security Audit Debrief", "SOC2 compliance examination", 3, std::nullopt, 15.0, 1, 4, 7, {}},
        {"MEET-05", "Product Roadmap Planning", "FY2027 strategic initiative ranking", 3, std::nullopt, 18.0, 1, 5, 8, {}},
        {"MEET-06", "Candidate Technical Interview", "Senior Distributed Systems role", 2, std::nullopt, 10.0, 2, 6, 8, {}},
        {"MEET-07", "All-Hands Operations Briefing", "Company quarterly milestones recap", 3, std::nullopt, 14.0, 3, 8, 11, {}},
        {"MEET-08", "Evening Systems Maintenance", "Database replica zero-downtime rolling reboot", 2, std::nullopt, 16.0, 1, 9, 11, {}}
    };
    saveScenario(s2);

    // Faculty Demo 3: Mode C - Complex CI/CD Build Pipeline (DAG & Critical Path)
    Scenario s3;
    s3.id = "scenario-mode-c-pipeline";
    s3.name = "Software Build & Release Pipeline (Prerequisite DAG)";
    s3.description = "Automated continuous delivery graph. Evaluates Kahn's topological ordering, checks for deadlock cycles, and computes the critical path.";
    s3.mode = SchedulingMode::MODE_C_PREREQUISITE_ORDERING;
    s3.timeBudget = 30;
    s3.planningHorizon = 40;

    s3.tasks = {
        {"PIPE-01", "Checkout & Linting", "Static analysis and dependency verification", 3, 10, 5.0, 1, std::nullopt, std::nullopt, {}},
        {"PIPE-02", "C++ Engine Compilation", "Clang/GCC native binary compilation", 6, 18, 15.0, 1, std::nullopt, std::nullopt, {"PIPE-01"}},
        {"PIPE-03", "Frontend React Bundle", "Vite production minification and assets", 4, 16, 10.0, 2, std::nullopt, std::nullopt, {"PIPE-01"}},
        {"PIPE-04", "Unit & Contract Tests", "GoogleTest suite and API regression suite", 5, 25, 20.0, 1, std::nullopt, std::nullopt, {"PIPE-02"}},
        {"PIPE-05", "Integration E2E Tests", "Playwright end-to-end browser automation", 8, 30, 25.0, 1, std::nullopt, std::nullopt, {"PIPE-02", "PIPE-03"}},
        {"PIPE-06", "Security Vulnerability Scan", "Trivy container security audit", 3, 28, 12.0, 2, std::nullopt, std::nullopt, {"PIPE-04"}},
        {"PIPE-07", "Docker Container Assembly", "Multi-stage distroless image build", 4, 32, 18.0, 1, std::nullopt, std::nullopt, {"PIPE-05", "PIPE-06"}},
        {"PIPE-08", "Canary Cluster Deployment", "Automated blue/green rollout on Kubernetes", 5, 38, 30.0, 1, std::nullopt, std::nullopt, {"PIPE-07"}}
    };
    saveScenario(s3);
}

} // namespace intellischedule

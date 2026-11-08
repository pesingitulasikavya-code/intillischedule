#include "httplib.h"
#include "models.hpp"
#include "algorithms/knapsack_dp.hpp"
#include "algorithms/knapsack_greedy.hpp"
#include "algorithms/interval_scheduler.hpp"
#include "algorithms/graph_scheduler.hpp"
#include "validator.hpp"
#include "database.hpp"
#include <iostream>

using namespace intellischedule;

int main(int argc, char* argv[]) {
    int port = 8080;
    if (argc >= 2) {
        port = std::stoi(argv[1]);
    }

    Database db("intellischedule.db");
    if (!db.initialize()) {
        std::cerr << "[IntelliSchedule C++] Database initialization failed!" << std::endl;
        return 1;
    }

    httplib::Server svr;

    // CORS headers
    svr.set_default_headers({
        {"Access-Control-Allow-Origin", "*"},
        {"Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS"},
        {"Access-Control-Allow-Headers", "Content-Type, Authorization"}
    });

    svr.Options(".*", [](const httplib::Request&, httplib::Response& res) {
        res.status = 204;
    });

    // 1. GET /api/health
    svr.Get("/api/health", [](const httplib::Request&, httplib::Response& res) {
        nlohmann::json body = {
            {"status", "ok"},
            {"engine", "IntelliSchedule C++20 Core Optimization Platform"},
            {"version", "1.0.0"},
            {"algorithms", {
                "0/1 Knapsack Dynamic Programming",
                "Greedy Value-to-Duration Ratio Heuristic",
                "Earliest Finish Time (EFT) Interval Scheduler",
                "Kahn's Topological Sort & Cycle Detection",
                "Critical Path Analysis"
            }},
            {"storage", "SQLite3 Embedded Persistence"},
            {"cpp_standard", "C++20"}
        };
        res.set_content(body.dump(2), "application/json");
    });

    // 2. GET /api/tasks
    svr.Get("/api/tasks", [&db](const httplib::Request&, httplib::Response& res) {
        auto tasks = db.getAllTasks();
        nlohmann::json j = tasks;
        res.set_content(j.dump(2), "application/json");
    });

    // 3. POST /api/tasks
    svr.Post("/api/tasks", [&db](const httplib::Request& req, httplib::Response& res) {
        try {
            auto j = nlohmann::json::parse(req.body);
            Task t = j.get<Task>();
            if (t.id.empty()) {
                t.id = "TASK-" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
            }
            if (t.duration <= 0) {
                res.status = 400;
                res.set_content(nlohmann::json{{"error", "Task duration must be strictly positive (> 0)"}}.dump(), "application/json");
                return;
            }
            db.saveTask(t);
            res.status = 201;
            res.set_content(nlohmann::json{{"success", true}, {"task", t}}.dump(2), "application/json");
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(nlohmann::json{{"error", e.what()}}.dump(), "application/json");
        }
    });

    // 4. PUT /api/tasks/:id
    svr.Put(R"(/api/tasks/([a-zA-Z0-9_\-]+))", [&db](const httplib::Request& req, httplib::Response& res) {
        std::string id = req.matches[1];
        try {
            auto j = nlohmann::json::parse(req.body);
            Task t = j.get<Task>();
            t.id = id;
            if (t.duration <= 0) {
                res.status = 400;
                res.set_content(nlohmann::json{{"error", "Task duration must be strictly positive (> 0)"}}.dump(), "application/json");
                return;
            }
            db.updateTask(t);
            res.set_content(nlohmann::json{{"success", true}, {"task", t}}.dump(2), "application/json");
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(nlohmann::json{{"error", e.what()}}.dump(), "application/json");
        }
    });

    // 5. DELETE /api/tasks/:id
    svr.Delete(R"(/api/tasks/([a-zA-Z0-9_\-]+))", [&db](const httplib::Request& req, httplib::Response& res) {
        std::string id = req.matches[1];
        bool ok = db.deleteTask(id);
        if (ok) {
            res.set_content(nlohmann::json{{"success", true}, {"id", id}}.dump(), "application/json");
        } else {
            res.status = 404;
            res.set_content(nlohmann::json{{"error", "Task not found"}}.dump(), "application/json");
        }
    });

    // 6. POST /api/schedule
    svr.Post("/api/schedule", [&db](const httplib::Request& req, httplib::Response& res) {
        try {
            auto j = nlohmann::json::parse(req.body);
            std::string modeStr = j.value("mode", "MODE_A_MAX_VALUE_KNAPSACK");
            SchedulingMode mode = stringToMode(modeStr);
            int timeBudget = j.value("timeBudget", 40);
            int horizonStart = j.value("planningHorizonStart", 0);
            std::string scenarioId = j.value("scenarioId", "custom");

            std::vector<Task> tasks;
            if (j.contains("tasks") && j["tasks"].is_array()) {
                tasks = j["tasks"].get<std::vector<Task>>();
            }

            ScheduleResult result;
            if (mode == SchedulingMode::MODE_A_MAX_VALUE_KNAPSACK) {
                std::string alg = j.value("algorithm", "DP");
                if (alg == "GREEDY") {
                    result = KnapsackGreedyScheduler::schedule(tasks, timeBudget);
                } else {
                    result = KnapsackDPScheduler::schedule(tasks, timeBudget);
                }
                ScheduleValidator::validate(result, timeBudget);
            } else if (mode == SchedulingMode::MODE_B_INTERVAL_SCHEDULING) {
                result = IntervalScheduler::schedule(tasks);
                ScheduleValidator::validate(result);
            } else if (mode == SchedulingMode::MODE_C_PREREQUISITE_ORDERING) {
                result = GraphScheduler::schedule(tasks, horizonStart);
                ScheduleValidator::validate(result);
            }

            db.recordScheduleRun(scenarioId, result);

            nlohmann::json out = result;
            res.set_content(out.dump(2), "application/json");
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(nlohmann::json{{"error", e.what()}}.dump(), "application/json");
        }
    });

    // 7. POST /api/compare
    svr.Post("/api/compare", [](const httplib::Request& req, httplib::Response& res) {
        try {
            auto j = nlohmann::json::parse(req.body);
            int timeBudget = j.value("timeBudget", 40);
            std::vector<Task> tasks;
            if (j.contains("tasks") && j["tasks"].is_array()) {
                tasks = j["tasks"].get<std::vector<Task>>();
            }

            auto dpResult = KnapsackDPScheduler::schedule(tasks, timeBudget);
            ScheduleValidator::validate(dpResult, timeBudget);

            auto greedyResult = KnapsackGreedyScheduler::schedule(tasks, timeBudget);
            ScheduleValidator::validate(greedyResult, timeBudget);

            ModeAComparison comp;
            comp.dpResult = dpResult;
            comp.greedyResult = greedyResult;
            comp.valueDifference = dpResult.totalValue - greedyResult.totalValue;

            if (dpResult.totalValue > 1e-9) {
                comp.efficiencyGapPercent = (comp.valueDifference / dpResult.totalValue) * 100.0;
            } else {
                comp.efficiencyGapPercent = 0.0;
            }

            comp.greedyMatchedOptimal = (std::abs(comp.valueDifference) < 1e-9);

            if (comp.greedyMatchedOptimal) {
                comp.conclusion = "The greedy heuristic reached the exact optimum for this specific instance. Note: Heuristics are not guaranteed optimal for general 0/1 Knapsack instances.";
            } else {
                std::ostringstream oss;
                oss << "Dynamic Programming found a superior schedule yielding "
                    << comp.valueDifference << " more value points ("
                    << comp.efficiencyGapPercent << "% efficiency gap). "
                    << "This empirically demonstrates that the greedy density heuristic can strand budget capacity in discrete 0/1 Knapsack optimization.";
                comp.conclusion = oss.str();
            }

            nlohmann::json out = comp;
            res.set_content(out.dump(2), "application/json");
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(nlohmann::json{{"error", e.what()}}.dump(), "application/json");
        }
    });

    // 8. GET /api/scenarios/:id
    svr.Get(R"(/api/scenarios/([a-zA-Z0-9_\-]+))", [&db](const httplib::Request& req, httplib::Response& res) {
        std::string id = req.matches[1];
        auto s = db.getScenario(id);
        if (s.has_value()) {
            nlohmann::json j = *s;
            res.set_content(j.dump(2), "application/json");
        } else {
            res.status = 404;
            res.set_content(nlohmann::json{{"error", "Scenario not found"}}.dump(), "application/json");
        }
    });

    // 9. GET /api/scenarios
    svr.Get("/api/scenarios", [&db](const httplib::Request&, httplib::Response& res) {
        auto list = db.getAllScenarios();
        nlohmann::json j = list;
        res.set_content(j.dump(2), "application/json");
    });

    std::cout << "[IntelliSchedule C++] Native HTTP Server running on http://localhost:" << port << std::endl;
    svr.listen("0.0.0.0", port);
    return 0;
}

#pragma once

#include "models.hpp"
#include <string>
#include <vector>
#include <optional>

struct sqlite3;

namespace intellischedule {

class Database {
public:
    explicit Database(const std::string& dbPath = "intellischedule.db");
    ~Database();

    bool initialize();

    // Task CRUD
    bool saveTask(const Task& task);
    bool updateTask(const Task& task);
    bool deleteTask(const std::string& id);
    std::optional<Task> getTask(const std::string& id);
    std::vector<Task> getAllTasks();

    // Scenario Management
    bool saveScenario(const Scenario& scenario);
    std::optional<Scenario> getScenario(const std::string& id);
    std::vector<Scenario> getAllScenarios();

    // Run History
    bool recordScheduleRun(const std::string& scenarioId, const ScheduleResult& result);

private:
    std::string dbPath_;
    sqlite3* db_{nullptr};

    void seedDefaultScenarios();
};

} // namespace intellischedule

#pragma once

#include <string>
#include <vector>
#include <optional>
#include "nlohmann/json.hpp"

namespace intellischedule {

enum class SchedulingMode {
    MODE_A_MAX_VALUE_KNAPSACK,
    MODE_B_INTERVAL_SCHEDULING,
    MODE_C_PREREQUISITE_ORDERING
};

inline std::string modeToString(SchedulingMode mode) {
    switch (mode) {
        case SchedulingMode::MODE_A_MAX_VALUE_KNAPSACK:
            return "MODE_A_MAX_VALUE_KNAPSACK";
        case SchedulingMode::MODE_B_INTERVAL_SCHEDULING:
            return "MODE_B_INTERVAL_SCHEDULING";
        case SchedulingMode::MODE_C_PREREQUISITE_ORDERING:
            return "MODE_C_PREREQUISITE_ORDERING";
    }
    return "UNKNOWN";
}

inline SchedulingMode stringToMode(const std::string& str) {
    if (str == "MODE_B_INTERVAL_SCHEDULING" || str == "MODE_B" || str == "INTERVAL") {
        return SchedulingMode::MODE_B_INTERVAL_SCHEDULING;
    }
    if (str == "MODE_C_PREREQUISITE_ORDERING" || str == "MODE_C" || str == "GRAPH" || str == "DAG") {
        return SchedulingMode::MODE_C_PREREQUISITE_ORDERING;
    }
    return SchedulingMode::MODE_A_MAX_VALUE_KNAPSACK;
}

struct Task {
    std::string id;
    std::string title;
    std::string description;
    int duration{1};               // positive integer duration
    std::optional<int> deadline;   // optional deadline
    double value{1.0};             // non-negative priority value
    int priority{1};               // 1 = High, 2 = Medium, 3 = Low
    std::optional<int> startTime;  // required for Mode B
    std::optional<int> finishTime; // required for Mode B
    std::vector<std::string> prerequisiteIds; // dependencies for Mode C

    // Scheduled attributes computed by the engine
    std::optional<int> scheduledStart;
    std::optional<int> scheduledFinish;
    bool isScheduled{false};
    std::string exclusionReason;
};

// JSON serialization for Task
inline void to_json(nlohmann::json& j, const Task& t) {
    j = nlohmann::json{
        {"id", t.id},
        {"title", t.title},
        {"description", t.description},
        {"duration", t.duration},
        {"value", t.value},
        {"priority", t.priority},
        {"prerequisiteIds", t.prerequisiteIds},
        {"isScheduled", t.isScheduled}
    };
    if (t.deadline.has_value()) j["deadline"] = *t.deadline;
    else j["deadline"] = nullptr;

    if (t.startTime.has_value()) j["startTime"] = *t.startTime;
    else j["startTime"] = nullptr;

    if (t.finishTime.has_value()) j["finishTime"] = *t.finishTime;
    else j["finishTime"] = nullptr;

    if (t.scheduledStart.has_value()) j["scheduledStart"] = *t.scheduledStart;
    else j["scheduledStart"] = nullptr;

    if (t.scheduledFinish.has_value()) j["scheduledFinish"] = *t.scheduledFinish;
    else j["scheduledFinish"] = nullptr;

    if (!t.exclusionReason.empty()) j["exclusionReason"] = t.exclusionReason;
    else j["exclusionReason"] = nullptr;
}

inline void from_json(const nlohmann::json& j, Task& t) {
    j.at("id").get_to(t.id);
    j.at("title").get_to(t.title);
    if (j.contains("description") && !j["description"].is_null()) {
        j.at("description").get_to(t.description);
    }
    if (j.contains("duration")) j.at("duration").get_to(t.duration);
    if (j.contains("value")) j.at("value").get_to(t.value);
    if (j.contains("priority")) j.at("priority").get_to(t.priority);
    if (j.contains("deadline") && !j["deadline"].is_null()) {
        t.deadline = j["deadline"].get<int>();
    }
    if (j.contains("startTime") && !j["startTime"].is_null()) {
        t.startTime = j["startTime"].get<int>();
    }
    if (j.contains("finishTime") && !j["finishTime"].is_null()) {
        t.finishTime = j["finishTime"].get<int>();
    }
    if (j.contains("prerequisiteIds") && j["prerequisiteIds"].is_array()) {
        t.prerequisiteIds = j["prerequisiteIds"].get<std::vector<std::string>>();
    }
    if (j.contains("scheduledStart") && !j["scheduledStart"].is_null()) {
        t.scheduledStart = j["scheduledStart"].get<int>();
    }
    if (j.contains("scheduledFinish") && !j["scheduledFinish"].is_null()) {
        t.scheduledFinish = j["scheduledFinish"].get<int>();
    }
    if (j.contains("isScheduled")) {
        j.at("isScheduled").get_to(t.isScheduled);
    }
    if (j.contains("exclusionReason") && !j["exclusionReason"].is_null()) {
        j.at("exclusionReason").get_to(t.exclusionReason);
    }
}

struct ValidationViolation {
    std::string rule;
    std::string taskId;
    std::string message;
};

inline void to_json(nlohmann::json& j, const ValidationViolation& v) {
    j = nlohmann::json{
        {"rule", v.rule},
        {"taskId", v.taskId},
        {"message", v.message}
    };
}

struct ScheduleResult {
    bool success{false};
    SchedulingMode mode{SchedulingMode::MODE_A_MAX_VALUE_KNAPSACK};
    std::string algorithmName;
    std::string assumptions;
    std::string complexityExplanation;
    double totalValue{0.0};
    int totalDuration{0};
    int scheduledCount{0};
    int unscheduledCount{0};
    double executionTimeMs{0.0};
    std::vector<Task> scheduledTasks;
    std::vector<Task> unscheduledTasks;
    std::vector<std::string> topologicalOrder;
    std::vector<std::string> detectedCycle;
    std::vector<std::string> criticalPath;
    bool isValidSchedule{false};
    std::vector<ValidationViolation> validationViolations;
    std::vector<std::string> warnings;
};

inline void to_json(nlohmann::json& j, const ScheduleResult& r) {
    j = nlohmann::json{
        {"success", r.success},
        {"mode", modeToString(r.mode)},
        {"algorithmName", r.algorithmName},
        {"assumptions", r.assumptions},
        {"complexityExplanation", r.complexityExplanation},
        {"totalValue", r.totalValue},
        {"totalDuration", r.totalDuration},
        {"scheduledCount", r.scheduledCount},
        {"unscheduledCount", r.unscheduledCount},
        {"executionTimeMs", r.executionTimeMs},
        {"scheduledTasks", r.scheduledTasks},
        {"unscheduledTasks", r.unscheduledTasks},
        {"topologicalOrder", r.topologicalOrder},
        {"detectedCycle", r.detectedCycle},
        {"criticalPath", r.criticalPath},
        {"isValidSchedule", r.isValidSchedule},
        {"validationViolations", r.validationViolations},
        {"warnings", r.warnings}
    };
}

struct ModeAComparison {
    ScheduleResult dpResult;
    ScheduleResult greedyResult;
    double valueDifference{0.0};
    double efficiencyGapPercent{0.0};
    bool greedyMatchedOptimal{false};
    std::string conclusion;
};

inline void to_json(nlohmann::json& j, const ModeAComparison& c) {
    j = nlohmann::json{
        {"dpResult", c.dpResult},
        {"greedyResult", c.greedyResult},
        {"valueDifference", c.valueDifference},
        {"efficiencyGapPercent", c.efficiencyGapPercent},
        {"greedyMatchedOptimal", c.greedyMatchedOptimal},
        {"conclusion", c.conclusion}
    };
}

struct Scenario {
    std::string id;
    std::string name;
    std::string description;
    SchedulingMode mode{SchedulingMode::MODE_A_MAX_VALUE_KNAPSACK};
    int timeBudget{40};
    int planningHorizon{100};
    std::vector<Task> tasks;
};

inline void to_json(nlohmann::json& j, const Scenario& s) {
    j = nlohmann::json{
        {"id", s.id},
        {"name", s.name},
        {"description", s.description},
        {"mode", modeToString(s.mode)},
        {"timeBudget", s.timeBudget},
        {"planningHorizon", s.planningHorizon},
        {"tasks", s.tasks}
    };
}

inline void from_json(const nlohmann::json& j, Scenario& s) {
    j.at("id").get_to(s.id);
    j.at("name").get_to(s.name);
    if (j.contains("description") && !j["description"].is_null()) {
        j.at("description").get_to(s.description);
    }
    if (j.contains("mode")) {
        s.mode = stringToMode(j["mode"].get<std::string>());
    }
    if (j.contains("timeBudget")) j.at("timeBudget").get_to(s.timeBudget);
    if (j.contains("planningHorizon")) j.at("planningHorizon").get_to(s.planningHorizon);
    if (j.contains("tasks") && j["tasks"].is_array()) {
        s.tasks = j["tasks"].get<std::vector<Task>>();
    }
}

} // namespace intellischedule

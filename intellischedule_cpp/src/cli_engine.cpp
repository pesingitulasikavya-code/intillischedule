#include "models.hpp"
#include "algorithms/knapsack_dp.hpp"
#include "algorithms/knapsack_greedy.hpp"
#include "algorithms/interval_scheduler.hpp"
#include "algorithms/graph_scheduler.hpp"
#include "validator.hpp"
#include "database.hpp"
#include <iostream>
#include <string>
#include <sstream>

using namespace intellischedule;

int main(int argc, char* argv[]) {
    Database db("intellischedule.db");
    db.initialize();

    if (argc < 2) {
        std::cerr << "Usage: intellischedule_cli <command> [json_arguments]" << std::endl;
        return 1;
    }

    std::string cmd = argv[1];

    if (cmd == "health") {
        nlohmann::json res = {
            {"status", "ok"},
            {"engine", "IntelliSchedule C++20 Core Optimization Engine"},
            {"version", "1.0.0"},
            {"algorithms", {
                "0/1 Knapsack Dynamic Programming (Exact)",
                "Greedy Value-per-Duration Heuristic",
                "Earliest Finish Time (EFT) Interval Scheduler",
                "Kahn's Topological Sort & Cycle Detection",
                "Critical Path Dynamic Programming"
            }},
            {"storage", "SQLite3 Embedded Engine"},
            {"cpp_standard", "C++20"}
        };
        std::cout << res.dump() << std::endl;
        return 0;
    }

    if (cmd == "schedule") {
        std::string jsonStr;
        if (argc >= 3) {
            jsonStr = argv[2];
        } else {
            std::string line;
            while (std::getline(std::cin, line)) jsonStr += line;
        }

        try {
            auto j = nlohmann::json::parse(jsonStr);
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
                std::string subAlgorithm = j.value("algorithm", "DP");
                if (subAlgorithm == "GREEDY") {
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
            std::cout << out.dump() << std::endl;
            return 0;
        } catch (const std::exception& e) {
            nlohmann::json err = {
                {"success", false},
                {"error", std::string("Schedule execution error: ") + e.what()}
            };
            std::cout << err.dump() << std::endl;
            return 1;
        }
    }

    if (cmd == "compare") {
        std::string jsonStr;
        if (argc >= 3) {
            jsonStr = argv[2];
        } else {
            std::string line;
            while (std::getline(std::cin, line)) jsonStr += line;
        }

        try {
            auto j = nlohmann::json::parse(jsonStr);
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
                comp.conclusion = "The greedy heuristic successfully reached the exact optimum for this specific instance. Note: This heuristic is still NOT guaranteed to be optimal in the general 0/1 Knapsack case.";
            } else {
                std::ostringstream oss;
                oss << "Dynamic Programming outperformed the Greedy heuristic by "
                    << comp.valueDifference << " value points ("
                    << comp.efficiencyGapPercent << "% efficiency gap). "
                    << "This empirically proves that the Greedy ratio choice can strand capacity in discrete 0/1 optimization.";
                comp.conclusion = oss.str();
            }

            nlohmann::json out = comp;
            std::cout << out.dump() << std::endl;
            return 0;
        } catch (const std::exception& e) {
            nlohmann::json err = {
                {"success", false},
                {"error", std::string("Comparison error: ") + e.what()}
            };
            std::cout << err.dump() << std::endl;
            return 1;
        }
    }

    if (cmd == "tasks") {
        std::string sub = (argc >= 3) ? argv[2] : "list";
        if (sub == "list") {
            auto tasks = db.getAllTasks();
            nlohmann::json j = tasks;
            std::cout << j.dump() << std::endl;
            return 0;
        }
        if (sub == "save") {
            std::string payload;
            if (argc >= 4) payload = argv[3];
            else {
                std::string line;
                while (std::getline(std::cin, line)) payload += line;
            }
            try {
                auto tJson = nlohmann::json::parse(payload);
                Task t = tJson.get<Task>();
                bool ok = db.saveTask(t);
                nlohmann::json out = {{"success", ok}, {"task", t}};
                std::cout << out.dump() << std::endl;
                return 0;
            } catch (const std::exception& e) {
                std::cout << nlohmann::json{{"success", false}, {"error", e.what()}}.dump() << std::endl;
                return 1;
            }
        }
        if (sub == "delete") {
            std::string id = (argc >= 4) ? argv[3] : "";
            bool ok = db.deleteTask(id);
            std::cout << nlohmann::json{{"success", ok}, {"id", id}}.dump() << std::endl;
            return 0;
        }
    }

    if (cmd == "scenarios") {
        std::string sub = (argc >= 3) ? argv[2] : "list";
        if (sub == "list") {
            auto scs = db.getAllScenarios();
            nlohmann::json j = scs;
            std::cout << j.dump() << std::endl;
            return 0;
        }
        if (sub == "get") {
            std::string id = (argc >= 4) ? argv[3] : "";
            auto s = db.getScenario(id);
            if (s.has_value()) {
                nlohmann::json j = *s;
                std::cout << j.dump() << std::endl;
            } else {
                std::cout << nlohmann::json{{"error", "Scenario not found"}}.dump() << std::endl;
            }
            return 0;
        }
    }

    std::cerr << "Unknown command: " << cmd << std::endl;
    return 1;
}

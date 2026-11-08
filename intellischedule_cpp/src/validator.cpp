#include "validator.hpp"
#include <unordered_set>
#include <unordered_map>
#include <algorithm>

namespace intellischedule {

bool ScheduleValidator::validate(ScheduleResult& result, int timeBudget) {
    bool passed = true;

    // Rule 1: No negative or zero duration on scheduled tasks
    for (const auto& task : result.scheduledTasks) {
        if (task.duration <= 0) {
            passed = false;
            result.validationViolations.push_back({
                "STRICTLY_POSITIVE_DURATION",
                task.id,
                "Task '" + task.id + "' has non-positive duration (" + std::to_string(task.duration) + ")"
            });
        }
    }

    // Rule 2: Each task is selected at most once
    std::unordered_set<std::string> seenIds;
    for (const auto& task : result.scheduledTasks) {
        if (seenIds.find(task.id) != seenIds.end()) {
            passed = false;
            result.validationViolations.push_back({
                "AT_MOST_ONCE_SELECTION",
                task.id,
                "Task '" + task.id + "' is duplicate-scheduled more than once"
            });
        }
        seenIds.insert(task.id);
    }

    // Mode-specific validation rules
    if (result.mode == SchedulingMode::MODE_A_MAX_VALUE_KNAPSACK) {
        // Mode A Rule: Aggregate duration must not exceed time budget
        int totalScheduledDuration = 0;
        for (const auto& task : result.scheduledTasks) {
            totalScheduledDuration += task.duration;
        }

        if (timeBudget > 0 && totalScheduledDuration > timeBudget) {
            passed = false;
            result.validationViolations.push_back({
                "TIME_BUDGET_COMPLIANCE",
                "",
                "Total scheduled duration (" + std::to_string(totalScheduledDuration) +
                ") strictly exceeds available time budget (" + std::to_string(timeBudget) + ")"
            });
        }
    } else if (result.mode == SchedulingMode::MODE_B_INTERVAL_SCHEDULING) {
        // Mode B Rule: Mutually non-overlapping intervals
        for (size_t i = 0; i < result.scheduledTasks.size(); ++i) {
            const auto& t1 = result.scheduledTasks[i];
            if (!t1.scheduledStart.has_value() || !t1.scheduledFinish.has_value()) {
                passed = false;
                result.validationViolations.push_back({
                    "INTERVAL_BOUNDS_PRESENT",
                    t1.id,
                    "Scheduled interval bounds are undefined for task '" + t1.id + "'"
                });
                continue;
            }

            int s1 = *t1.scheduledStart;
            int f1 = *t1.scheduledFinish;

            if (f1 <= s1) {
                passed = false;
                result.validationViolations.push_back({
                    "VALID_INTERVAL_SPAN",
                    t1.id,
                    "Task '" + t1.id + "' has non-positive interval span [" +
                    std::to_string(s1) + ", " + std::to_string(f1) + ")"
                });
            }

            for (size_t j = i + 1; j < result.scheduledTasks.size(); ++j) {
                const auto& t2 = result.scheduledTasks[j];
                if (!t2.scheduledStart.has_value() || !t2.scheduledFinish.has_value()) continue;

                int s2 = *t2.scheduledStart;
                int f2 = *t2.scheduledFinish;

                // Check overlap: max(s1, s2) < min(f1, f2)
                if (std::max(s1, s2) < std::min(f1, f2)) {
                    passed = false;
                    result.validationViolations.push_back({
                        "MUTUALLY_COMPATIBLE_INTERVALS",
                        t1.id + " & " + t2.id,
                        "Interval collision between task '" + t1.id + "' [" +
                        std::to_string(s1) + ", " + std::to_string(f1) + ") and task '" +
                        t2.id + "' [" + std::to_string(s2) + ", " + std::to_string(f2) + ")"
                    });
                }
            }
        }
    } else if (result.mode == SchedulingMode::MODE_C_PREREQUISITE_ORDERING) {
        // Mode C Rule: Cycle check and prerequisite finish <= dependent start
        if (!result.detectedCycle.empty()) {
            passed = false;
            // Cycle is already reported
        }

        std::unordered_map<std::string, const Task*> scheduledMap;
        for (const auto& t : result.scheduledTasks) {
            scheduledMap[t.id] = &t;
        }

        for (const auto& task : result.scheduledTasks) {
            if (!task.scheduledStart.has_value()) continue;
            int vStart = *task.scheduledStart;

            for (const auto& prereqId : task.prerequisiteIds) {
                auto it = scheduledMap.find(prereqId);
                if (it != scheduledMap.end()) {
                    const auto* prereq = it->second;
                    if (prereq->scheduledFinish.has_value()) {
                        int uFinish = *prereq->scheduledFinish;
                        if (uFinish > vStart) {
                            passed = false;
                            result.validationViolations.push_back({
                                "PREREQUISITE_PRECEDENCE_PRESERVED",
                                task.id,
                                "Precedence violation: Prerequisite '" + prereqId +
                                "' completes at t=" + std::to_string(uFinish) +
                                ", but dependent task '" + task.id +
                                "' starts prematurely at t=" + std::to_string(vStart)
                            });
                        }
                    }
                }
            }
        }
    }

    result.isValidSchedule = passed;
    if (!passed) {
        result.success = false;
    }
    return passed;
}

} // namespace intellischedule

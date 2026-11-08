#include "algorithms/interval_scheduler.hpp"
#include <chrono>
#include <algorithm>

namespace intellischedule {

ScheduleResult IntervalScheduler::schedule(const std::vector<Task>& tasks) {
    auto startTime = std::chrono::high_resolution_clock::now();

    ScheduleResult result;
    result.mode = SchedulingMode::MODE_B_INTERVAL_SCHEDULING;
    result.algorithmName = "Greedy Interval Scheduling by Earliest Finish Time (EFT)";
    result.assumptions = "Mode B assumes tasks possess fixed immutable [start, finish) time intervals on a single shared resource. "
                         "Tasks cannot be preempted or modified. Two tasks are compatible iff their intervals do not overlap.";
    result.complexityExplanation = "Time: O(n log n) sorting + O(n) greedy sweep = O(n log n), Space: O(n). "
                                   "Provably optimal for maximum cardinality subset via the mathematical 'Greedy Stays Ahead' inductive proof.";

    std::vector<Task> validCandidates;
    for (const auto& t : tasks) {
        if (!t.startTime.has_value() || !t.finishTime.has_value()) {
            Task invalid = t;
            invalid.isScheduled = false;
            invalid.exclusionReason = "Missing fixed start or finish time required for Mode B";
            result.unscheduledTasks.push_back(invalid);
            continue;
        }

        int s = *t.startTime;
        int f = *t.finishTime;

        if (s < 0 || f <= s) {
            Task invalid = t;
            invalid.isScheduled = false;
            invalid.exclusionReason = "Invalid interval range [start=" + std::to_string(s) +
                                      ", finish=" + std::to_string(f) + "). Finish must be strictly greater than start.";
            result.unscheduledTasks.push_back(invalid);
            continue;
        }

        validCandidates.push_back(t);
    }

    // Deterministic sorting:
    // 1. Earliest finish time ascending
    // 2. Earliest start time ascending
    // 3. ID ascending
    std::sort(validCandidates.begin(), validCandidates.end(), [](const Task& a, const Task& b) {
        if (*a.finishTime != *b.finishTime) {
            return *a.finishTime < *b.finishTime;
        }
        if (*a.startTime != *b.startTime) {
            return *a.startTime < *b.startTime;
        }
        return a.id < b.id;
    });

    int lastFinishTime = -1;
    std::string lastScheduledTaskId = "";
    int lastScheduledStart = -1;
    int lastScheduledFinish = -1;

    for (auto& task : validCandidates) {
        int s = *task.startTime;
        int f = *task.finishTime;

        // Compatible condition: current task starts at or after the previous task finishes
        if (s >= lastFinishTime) {
            task.isScheduled = true;
            task.scheduledStart = s;
            task.scheduledFinish = f;
            task.duration = f - s; // enforce consistency

            lastFinishTime = f;
            lastScheduledTaskId = task.id;
            lastScheduledStart = s;
            lastScheduledFinish = f;

            result.totalValue += task.value;
            result.totalDuration += task.duration;
            result.scheduledTasks.push_back(task);
        } else {
            task.isScheduled = false;
            task.scheduledStart = std::nullopt;
            task.scheduledFinish = std::nullopt;
            task.exclusionReason = "Interval collision: starts at t=" + std::to_string(s) +
                                  " which conflicts with scheduled task '" + lastScheduledTaskId +
                                  "' [" + std::to_string(lastScheduledStart) + ", " +
                                  std::to_string(lastScheduledFinish) + ")";
            result.unscheduledTasks.push_back(task);
        }
    }

    result.scheduledCount = static_cast<int>(result.scheduledTasks.size());
    result.unscheduledCount = static_cast<int>(result.unscheduledTasks.size());
    result.success = true;
    result.isValidSchedule = true; // Validator will independently confirm

    auto endTime = std::chrono::high_resolution_clock::now();
    result.executionTimeMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();
    return result;
}

} // namespace intellischedule

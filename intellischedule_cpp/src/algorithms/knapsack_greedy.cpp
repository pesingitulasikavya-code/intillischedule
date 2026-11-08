#include "algorithms/knapsack_greedy.hpp"
#include <chrono>
#include <algorithm>
#include <cmath>

namespace intellischedule {

ScheduleResult KnapsackGreedyScheduler::schedule(const std::vector<Task>& tasks, int timeBudget) {
    auto startTime = std::chrono::high_resolution_clock::now();

    ScheduleResult result;
    result.mode = SchedulingMode::MODE_A_MAX_VALUE_KNAPSACK;
    result.algorithmName = "Greedy Value-per-Duration Heuristic (Approximation)";
    result.assumptions = "Mode A heuristic assumes greedy local choice by ratio (value/duration). "
                         "DISCLOSURE: This heuristic is NOT guaranteed to find the globally optimal schedule for 0/1 Knapsack.";
    result.complexityExplanation = "Time: O(n log n) sorting + O(n) greedy linear pass = O(n log n), Space: O(n). "
                                   "Sub-optimal in pathological discrete capacity scenarios where high-density items leave unfilled budget gaps.";

    if (timeBudget <= 0) {
        result.success = true;
        result.isValidSchedule = true;
        result.totalValue = 0.0;
        result.totalDuration = 0;
        result.scheduledCount = 0;
        result.unscheduledCount = static_cast<int>(tasks.size());
        for (const auto& t : tasks) {
            Task unsched = t;
            unsched.isScheduled = false;
            unsched.exclusionReason = "Time budget is zero or negative (W=" + std::to_string(timeBudget) + ")";
            result.unscheduledTasks.push_back(unsched);
        }
        auto endTime = std::chrono::high_resolution_clock::now();
        result.executionTimeMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();
        return result;
    }

    std::vector<Task> sortedTasks = tasks;
    // Deterministic tie-breaking:
    // 1. Ratio descending
    // 2. Value descending
    // 3. Duration ascending
    // 4. ID ascending
    std::sort(sortedTasks.begin(), sortedTasks.end(), [](const Task& a, const Task& b) {
        double ratioA = (a.duration > 0) ? (a.value / a.duration) : 0.0;
        double ratioB = (b.duration > 0) ? (b.value / b.duration) : 0.0;

        if (std::abs(ratioA - ratioB) > 1e-9) {
            return ratioA > ratioB;
        }
        if (std::abs(a.value - b.value) > 1e-9) {
            return a.value > b.value;
        }
        if (a.duration != b.duration) {
            return a.duration < b.duration;
        }
        return a.id < b.id;
    });

    int remainingBudget = timeBudget;
    int currentTimelinePointer = 0;

    for (auto& task : sortedTasks) {
        if (task.duration <= 0) {
            task.isScheduled = false;
            task.exclusionReason = "Invalid task duration <= 0";
            result.unscheduledTasks.push_back(task);
            continue;
        }

        if (task.duration <= remainingBudget) {
            task.isScheduled = true;
            task.scheduledStart = currentTimelinePointer;
            task.scheduledFinish = currentTimelinePointer + task.duration;
            currentTimelinePointer += task.duration;
            remainingBudget -= task.duration;

            result.totalValue += task.value;
            result.totalDuration += task.duration;
            result.scheduledTasks.push_back(task);
        } else {
            task.isScheduled = false;
            task.scheduledStart = std::nullopt;
            task.scheduledFinish = std::nullopt;
            task.exclusionReason = "Insufficient remaining time budget (required: " +
                                  std::to_string(task.duration) + ", remaining: " +
                                  std::to_string(remainingBudget) + ")";
            result.unscheduledTasks.push_back(task);
        }
    }

    result.scheduledCount = static_cast<int>(result.scheduledTasks.size());
    result.unscheduledCount = static_cast<int>(result.unscheduledTasks.size());
    result.success = true;
    result.isValidSchedule = (result.totalDuration <= timeBudget);

    auto endTime = std::chrono::high_resolution_clock::now();
    result.executionTimeMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();
    return result;
}

} // namespace intellischedule

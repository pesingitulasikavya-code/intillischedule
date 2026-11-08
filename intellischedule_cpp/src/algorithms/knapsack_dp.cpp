#include "algorithms/knapsack_dp.hpp"
#include <chrono>
#include <algorithm>
#include <cmath>

namespace intellischedule {

ScheduleResult KnapsackDPScheduler::schedule(const std::vector<Task>& tasks, int timeBudget) {
    auto startTime = std::chrono::high_resolution_clock::now();

    ScheduleResult result;
    result.mode = SchedulingMode::MODE_A_MAX_VALUE_KNAPSACK;
    result.algorithmName = "0/1 Knapsack Dynamic Programming (Exact Optimum)";
    result.assumptions = "Mode A assumes tasks are independent, non-divisible (0/1 choice), have strictly positive durations, and non-negative values. The time budget W is a hard upper bound.";
    result.complexityExplanation = "Time: O(n * W), Space: O(n * W) table allocation for deterministic ID backtracking. Globally optimal for the mathematical 0/1 knapsack formulation.";

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

    // Filter valid positive-duration tasks with deterministic sorting by ID
    std::vector<Task> validTasks = tasks;
    std::sort(validTasks.begin(), validTasks.end(), [](const Task& a, const Task& b) {
        return a.id < b.id; // Deterministic tie-breaking
    });

    int n = static_cast<int>(validTasks.size());
    int W = timeBudget;

    // dp[i][w] stores maximum value achievable using a subset of first i tasks with capacity w
    // Using double for value, represented with precision or scaled
    std::vector<std::vector<double>> dp(n + 1, std::vector<double>(W + 1, 0.0));

    for (int i = 1; i <= n; ++i) {
        const auto& task = validTasks[i - 1];
        int d = task.duration;
        double v = task.value;

        for (int w = 0; w <= W; ++w) {
            // Option 1: Do not pick task i
            dp[i][w] = dp[i - 1][w];

            // Option 2: Pick task i if capacity allows
            if (d <= w && d > 0) {
                double pickVal = dp[i - 1][w - d] + v;
                // Deterministic comparison
                if (pickVal > dp[i][w] + 1e-9) {
                    dp[i][w] = pickVal;
                }
            }
        }
    }

    // Reconstruction phase (Backtrack)
    std::vector<bool> picked(n, false);
    int currW = W;
    for (int i = n; i >= 1; --i) {
        const auto& task = validTasks[i - 1];
        int d = task.duration;
        double v = task.value;

        if (d <= currW && d > 0) {
            double withTask = dp[i - 1][currW - d] + v;
            double withoutTask = dp[i - 1][currW];
            // If picked contributes to the maximum (deterministic tie-breaking favors inclusion)
            if (std::abs(dp[i][currW] - withTask) < 1e-9) {
                picked[i - 1] = true;
                currW -= d;
            }
        }
    }

    int currentTimelinePointer = 0;
    for (int i = 0; i < n; ++i) {
        Task t = validTasks[i];
        if (picked[i]) {
            t.isScheduled = true;
            t.scheduledStart = currentTimelinePointer;
            t.scheduledFinish = currentTimelinePointer + t.duration;
            currentTimelinePointer += t.duration;
            result.totalValue += t.value;
            result.totalDuration += t.duration;
            result.scheduledTasks.push_back(t);
        } else {
            t.isScheduled = false;
            t.scheduledStart = std::nullopt;
            t.scheduledFinish = std::nullopt;
            if (t.duration > timeBudget) {
                t.exclusionReason = "Task duration (" + std::to_string(t.duration) + ") exceeds entire budget (" + std::to_string(timeBudget) + ")";
            } else {
                t.exclusionReason = "Excluded by 0/1 Knapsack DP optimization: combination of other tasks yields higher aggregate value";
            }
            result.unscheduledTasks.push_back(t);
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

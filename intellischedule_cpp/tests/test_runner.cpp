#include "models.hpp"
#include "algorithms/knapsack_dp.hpp"
#include "algorithms/knapsack_greedy.hpp"
#include "algorithms/interval_scheduler.hpp"
#include "algorithms/graph_scheduler.hpp"
#include "validator.hpp"
#include <iostream>
#include <cassert>
#include <vector>
#include <cmath>

using namespace intellischedule;

int testsRun = 0;
int testsPassed = 0;
int testsFailed = 0;

#define ASSERT_TRUE(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << "  [FAILED] " << msg << " (" << __FILE__ << ":" << __LINE__ << ")" << std::endl; \
            testsFailed++; \
            return; \
        } \
    } while (0)

#define ASSERT_EQUAL(a, b, msg) \
    do { \
        if ((a) != (b)) { \
            std::cerr << "  [FAILED] " << msg << " [Expected " << (b) << ", got " << (a) << "] (" << __FILE__ << ":" << __LINE__ << ")" << std::endl; \
            testsFailed++; \
            return; \
        } \
    } while (0)

#define RUN_TEST(fn) \
    do { \
        testsRun++; \
        std::cout << "[TEST " << testsRun << "] Running " #fn "..." << std::endl; \
        int prevFailed = testsFailed; \
        fn(); \
        if (testsFailed == prevFailed) { \
            testsPassed++; \
            std::cout << "  [PASSED] " #fn << std::endl; \
        } \
    } while (0)

// 1. Empty Task Set Test
void test_empty_task_set() {
    std::vector<Task> emptyTasks;
    auto resA = KnapsackDPScheduler::schedule(emptyTasks, 20);
    ASSERT_TRUE(resA.success, "Mode A empty should succeed");
    ASSERT_EQUAL(resA.scheduledCount, 0, "Mode A scheduled count should be 0");
    ASSERT_TRUE(ScheduleValidator::validate(resA, 20), "Validator should pass on empty schedule");

    auto resB = IntervalScheduler::schedule(emptyTasks);
    ASSERT_TRUE(resB.success, "Mode B empty should succeed");
    ASSERT_EQUAL(resB.scheduledCount, 0, "Mode B scheduled count should be 0");

    auto resC = GraphScheduler::schedule(emptyTasks);
    ASSERT_TRUE(resC.success, "Mode C empty should succeed");
    ASSERT_EQUAL(resC.scheduledCount, 0, "Mode C scheduled count should be 0");
}

// 2. One Task Fits / One Task Does Not Fit
void test_one_task_fits_and_does_not_fit() {
    Task fits{"T1", "Fits Task", "", 5, std::nullopt, 10.0, 1};
    auto res1 = KnapsackDPScheduler::schedule({fits}, 5);
    ASSERT_EQUAL(res1.scheduledCount, 1, "T1 should fit in budget 5");
    ASSERT_EQUAL(res1.scheduledTasks.front().id, "T1", "T1 should be scheduled");

    Task tooBig{"T2", "Too Big Task", "", 12, std::nullopt, 20.0, 1};
    auto res2 = KnapsackDPScheduler::schedule({tooBig}, 10);
    ASSERT_EQUAL(res2.scheduledCount, 0, "T2 should NOT fit in budget 10");
    ASSERT_EQUAL(res2.unscheduledCount, 1, "T2 should be in unscheduled list");
}

// Helper for brute-force 0/1 knapsack on small sets
double bruteForceKnapsack(const std::vector<Task>& tasks, int budget) {
    int n = static_cast<int>(tasks.size());
    double bestVal = 0.0;
    int totalSubsets = 1 << n;
    for (int mask = 0; mask < totalSubsets; ++mask) {
        int dur = 0;
        double val = 0.0;
        for (int i = 0; i < n; ++i) {
            if (mask & (1 << i)) {
                dur += tasks[i].duration;
                val += tasks[i].value;
            }
        }
        if (dur <= budget && val > bestVal) {
            bestVal = val;
        }
    }
    return bestVal;
}

// 3. DP Compared with Brute-Force Optimum for Small Deterministic Cases
void test_dp_vs_brute_force_exhaustive() {
    std::vector<Task> tasks = {
        {"T1", "Task 1", "", 2, std::nullopt, 3.0, 1},
        {"T2", "Task 2", "", 3, std::nullopt, 4.0, 1},
        {"T3", "Task 3", "", 4, std::nullopt, 5.0, 1},
        {"T4", "Task 4", "", 5, std::nullopt, 8.0, 1},
        {"T5", "Task 5", "", 9, std::nullopt, 10.0, 1}
    };

    for (int budget = 1; budget <= 20; ++budget) {
        double expected = bruteForceKnapsack(tasks, budget);
        auto dpRes = KnapsackDPScheduler::schedule(tasks, budget);
        ASSERT_TRUE(std::abs(dpRes.totalValue - expected) < 1e-9, "DP must match brute-force optimum");
        ASSERT_TRUE(ScheduleValidator::validate(dpRes, budget), "Validator must confirm budget invariant");
    }
}

// 4. Greedy Heuristic Compared with DP (Proving Greedy Is Not Globally Optimal)
void test_greedy_vs_dp_counterexample() {
    // Classic counterexample for 0/1 Knapsack where greedy ratio fails:
    // Capacity = 50
    // Item 1: weight 30, value 60 (ratio 2.0)
    // Item 2: weight 20, value 30 (ratio 1.5)
    // Item 3: weight 30, value 45 (ratio 1.5)
    // Or simpler:
    // Budget = 10
    // T1: duration 6, value 36 (ratio 6.0)
    // T2: duration 5, value 26 (ratio 5.2)
    // T3: duration 5, value 26 (ratio 5.2)
    // Greedy picks T1 (dur 6, val 36). Remaining budget = 4. Cannot pick T2 or T3! Total val = 36.
    // Optimal DP picks T2 + T3 (dur 10, val 52)!
    std::vector<Task> tasks = {
        {"T1", "High Density Big Task", "", 6, std::nullopt, 36.0, 1},
        {"T2", "Medium Density Task A", "", 5, std::nullopt, 26.0, 1},
        {"T3", "Medium Density Task B", "", 5, std::nullopt, 26.0, 1}
    };

    int budget = 10;
    auto dpRes = KnapsackDPScheduler::schedule(tasks, budget);
    auto greedyRes = KnapsackGreedyScheduler::schedule(tasks, budget);

    ASSERT_EQUAL(dpRes.totalValue, 52.0, "DP should achieve optimum 52.0");
    ASSERT_EQUAL(greedyRes.totalValue, 36.0, "Greedy gets trapped with 36.0");
    ASSERT_TRUE(dpRes.totalValue > greedyRes.totalValue, "DP strictly outperforms Greedy on this instance");
}

// 5. Overlapping and Non-Overlapping Intervals (Mode B)
void test_interval_scheduling_overlaps() {
    std::vector<Task> tasks = {
        {"T1", "Morning", "", 2, std::nullopt, 10.0, 1, 1, 3},
        {"T2", "Overlap Mid", "", 3, std::nullopt, 15.0, 1, 2, 5},
        {"T3", "Noon", "", 2, std::nullopt, 10.0, 1, 3, 5},
        {"T4", "Afternoon", "", 3, std::nullopt, 10.0, 1, 5, 8}
    };

    auto res = IntervalScheduler::schedule(tasks);
    ASSERT_TRUE(res.success, "Interval scheduling should succeed");
    ASSERT_EQUAL(res.scheduledCount, 3, "T1, T3, T4 are mutually compatible (count = 3)");
    ASSERT_EQUAL(res.unscheduledCount, 1, "T2 collides and should be unscheduled");
    ASSERT_EQUAL(res.unscheduledTasks.front().id, "T2", "T2 was excluded due to collision");
    ASSERT_TRUE(ScheduleValidator::validate(res), "Validator must confirm non-overlapping invariant");
}

// 6. Equal Finish Times and Deterministic Tie-Breaking
void test_equal_finish_times_deterministic_tie_breaking() {
    // Two tasks with identical finish time:
    // T_B: [2, 6)
    // T_A: [1, 6)
    // Earliest finish times are equal (6).
    // Tie-breaking: earlier start time first (T_A starts at 1, T_B starts at 2)
    std::vector<Task> tasks = {
        {"T_B", "Task B", "", 4, std::nullopt, 10.0, 1, 2, 6},
        {"T_A", "Task A", "", 5, std::nullopt, 10.0, 1, 1, 6}
    };

    auto res1 = IntervalScheduler::schedule(tasks);
    auto res2 = IntervalScheduler::schedule(tasks); // idempotent stability
    ASSERT_EQUAL(res1.scheduledCount, 1, "Only one can be scheduled");
    ASSERT_EQUAL(res1.scheduledTasks.front().id, "T_A", "T_A wins tie-break due to earlier start time");
    ASSERT_EQUAL(res1.scheduledTasks.front().id, res2.scheduledTasks.front().id, "Deterministic output across runs");
}

// 7. Dependency Chain and Branching DAG (Mode C)
void test_dependency_chain_and_branching_dag() {
    // Chain: A -> B -> C
    // Branch: A -> D -> C
    std::vector<Task> tasks = {
        {"A", "Task A", "", 3, std::nullopt, 10.0, 1, std::nullopt, std::nullopt, {}},
        {"B", "Task B", "", 4, std::nullopt, 10.0, 1, std::nullopt, std::nullopt, {"A"}},
        {"D", "Task D", "", 2, std::nullopt, 10.0, 1, std::nullopt, std::nullopt, {"A"}},
        {"C", "Task C", "", 5, std::nullopt, 10.0, 1, std::nullopt, std::nullopt, {"B", "D"}}
    };

    auto res = GraphScheduler::schedule(tasks, 0);
    ASSERT_TRUE(res.success, "Valid DAG should schedule cleanly");
    ASSERT_EQUAL(res.scheduledCount, 4, "All 4 tasks scheduled");
    ASSERT_TRUE(ScheduleValidator::validate(res), "Validator must confirm DAG precedence");

    // Check precedence: A finishes at 3, B finishes at 7, D finishes at 5, C starts at max(7, 5)=7, finishes at 12
    for (const auto& t : res.scheduledTasks) {
        if (t.id == "A") ASSERT_EQUAL(*t.scheduledFinish, 3, "A finishes at 3");
        if (t.id == "B") ASSERT_EQUAL(*t.scheduledFinish, 7, "B finishes at 7");
        if (t.id == "D") ASSERT_EQUAL(*t.scheduledFinish, 5, "D finishes at 5");
        if (t.id == "C") ASSERT_EQUAL(*t.scheduledStart, 7, "C starts after max(B, D) at 7");
    }
}

// 8. Dependency Cycle Detection (Mode C)
void test_dependency_cycle_detection() {
    // Cycle: X -> Y -> Z -> X
    std::vector<Task> tasks = {
        {"X", "Task X", "", 2, std::nullopt, 10.0, 1, std::nullopt, std::nullopt, {"Z"}},
        {"Y", "Task Y", "", 3, std::nullopt, 10.0, 1, std::nullopt, std::nullopt, {"X"}},
        {"Z", "Task Z", "", 4, std::nullopt, 10.0, 1, std::nullopt, std::nullopt, {"Y"}}
    };

    auto res = GraphScheduler::schedule(tasks);
    ASSERT_TRUE(!res.success, "Cycle must cause schedule failure");
    ASSERT_TRUE(!res.detectedCycle.empty(), "Detected cycle list must not be empty");
    ASSERT_TRUE(!res.isValidSchedule, "Invalid schedule must NOT be marked successful");
    ASSERT_EQUAL(res.scheduledCount, 0, "No tasks may be scheduled under cyclic deadlock");
}

// 9. Missing Dependency ID
void test_missing_dependency_id() {
    std::vector<Task> tasks = {
        {"T_OK", "Existing Task", "", 3, std::nullopt, 10.0, 1, std::nullopt, std::nullopt, {}},
        {"T_DEP", "Dependent Task", "", 4, std::nullopt, 10.0, 1, std::nullopt, std::nullopt, {"NON_EXISTENT_ID"}}
    };

    auto res = GraphScheduler::schedule(tasks);
    ASSERT_TRUE(!res.success, "Missing dependency must cause schedule failure");
    ASSERT_TRUE(!res.validationViolations.empty(), "Validation violations should be recorded");
}

// 10. Invalid / Negative Duration
void test_invalid_negative_duration() {
    Task invalidTask{"T_BAD", "Negative Duration Task", "", -5, std::nullopt, 10.0, 1};
    auto res = KnapsackDPScheduler::schedule({invalidTask}, 20);
    ASSERT_EQUAL(res.scheduledCount, 0, "Negative duration task should never be scheduled");

    ScheduleResult forgedResult;
    forgedResult.mode = SchedulingMode::MODE_A_MAX_VALUE_KNAPSACK;
    forgedResult.scheduledTasks.push_back(invalidTask);
    bool valid = ScheduleValidator::validate(forgedResult, 20);
    ASSERT_TRUE(!valid, "Validator must reject negative duration tasks");
}

// 11. Schedule Invariant Validation (Forged Colliding Interval Rejected)
void test_schedule_invariant_validation() {
    ScheduleResult forgedInterval;
    forgedInterval.mode = SchedulingMode::MODE_B_INTERVAL_SCHEDULING;

    Task t1{"A", "Task A", "", 4, std::nullopt, 10.0, 1, 2, 6};
    t1.scheduledStart = 2;
    t1.scheduledFinish = 6;

    Task t2{"B", "Task B", "", 4, std::nullopt, 10.0, 1, 4, 8}; // Overlaps [4, 6)
    t2.scheduledStart = 4;
    t2.scheduledFinish = 8;

    forgedInterval.scheduledTasks = {t1, t2};

    bool passed = ScheduleValidator::validate(forgedInterval);
    ASSERT_TRUE(!passed, "Validator must reject overlapping intervals in Mode B");
    ASSERT_TRUE(!forgedInterval.isValidSchedule, "isValidSchedule must be set to false");
}

int main() {
    std::cout << "==========================================================" << std::endl;
    std::cout << "  INTELLISCHEDULE C++20 AUTOMATED VERIFICATION SUITE       " << std::endl;
    std::cout << "==========================================================" << std::endl;

    RUN_TEST(test_empty_task_set);
    RUN_TEST(test_one_task_fits_and_does_not_fit);
    RUN_TEST(test_dp_vs_brute_force_exhaustive);
    RUN_TEST(test_greedy_vs_dp_counterexample);
    RUN_TEST(test_interval_scheduling_overlaps);
    RUN_TEST(test_equal_finish_times_deterministic_tie_breaking);
    RUN_TEST(test_dependency_chain_and_branching_dag);
    RUN_TEST(test_dependency_cycle_detection);
    RUN_TEST(test_missing_dependency_id);
    RUN_TEST(test_invalid_negative_duration);
    RUN_TEST(test_schedule_invariant_validation);

    std::cout << "==========================================================" << std::endl;
    std::cout << "TEST SUMMARY: " << testsPassed << "/" << testsRun << " PASSED";
    if (testsFailed > 0) {
        std::cout << " (" << testsFailed << " FAILED)" << std::endl;
        return 1;
    }
    std::cout << " - ALL ASSERTS VERIFIED!" << std::endl;
    std::cout << "==========================================================" << std::endl;
    return 0;
}

#include "algorithms/graph_scheduler.hpp"
#include <chrono>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <algorithm>
#include <stack>

namespace intellischedule {

ScheduleResult GraphScheduler::schedule(const std::vector<Task>& tasks, int horizonStart) {
    auto startTime = std::chrono::high_resolution_clock::now();

    ScheduleResult result;
    result.mode = SchedulingMode::MODE_C_PREREQUISITE_ORDERING;
    result.algorithmName = "Prerequisite Graph Scheduler (Kahn's Topological Sort + Critical Path DP)";
    result.assumptions = "Mode C assumes a Directed Acyclic Graph (DAG) of task dependencies. Each edge (u, v) dictates that task u must strictly complete before task v may begin. Cycles render scheduling physically impossible and are immediately flagged.";
    result.complexityExplanation = "Time: O(V + E) for cycle detection, topological sorting, and timeline assignment; Space: O(V + E) for adjacency graph representation.";

    if (tasks.empty()) {
        result.success = true;
        result.isValidSchedule = true;
        auto endTime = std::chrono::high_resolution_clock::now();
        result.executionTimeMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();
        return result;
    }

    std::unordered_map<std::string, Task> taskMap;
    for (const auto& t : tasks) {
        taskMap[t.id] = t;
    }

    // Step 1: Validate missing dependency IDs
    bool hasMissingDependency = false;
    for (const auto& t : tasks) {
        for (const auto& prereqId : t.prerequisiteIds) {
            if (taskMap.find(prereqId) == taskMap.end()) {
                hasMissingDependency = true;
                ValidationViolation v;
                v.rule = "PREREQUISITE_EXISTS";
                v.taskId = t.id;
                v.message = "Task '" + t.id + "' references non-existent prerequisite ID '" + prereqId + "'";
                result.validationViolations.push_back(v);
            }
        }
    }

    if (hasMissingDependency) {
        result.success = false;
        result.isValidSchedule = false;
        result.warnings.push_back("Fatal validation error: One or more tasks reference missing prerequisite IDs.");
        for (const auto& t : tasks) {
            Task unsched = t;
            unsched.isScheduled = false;
            unsched.exclusionReason = "Unresolved dependency graph (missing prerequisite ID)";
            result.unscheduledTasks.push_back(unsched);
        }
        auto endTime = std::chrono::high_resolution_clock::now();
        result.executionTimeMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();
        return result;
    }

    // Build Adjacency List and In-degrees
    // Edge: u -> v means u must complete before v starts
    std::unordered_map<std::string, std::vector<std::string>> adj;
    std::unordered_map<std::string, int> inDegree;
    for (const auto& t : tasks) {
        inDegree[t.id] = 0;
        adj[t.id] = {};
    }

    for (const auto& t : tasks) {
        for (const auto& prereqId : t.prerequisiteIds) {
            adj[prereqId].push_back(t.id);
            inDegree[t.id]++;
        }
    }

    // Step 2: Cycle Detection using DFS 3-Coloring to extract the cycle loop
    std::unordered_map<std::string, int> color; // 0 = White (unvisited), 1 = Gray (visiting), 2 = Black (visited)
    std::unordered_map<std::string, std::string> parent;
    std::vector<std::string> detectedCycle;

    std::function<bool(const std::string&)> dfsCycle = [&](const std::string& u) -> bool {
        color[u] = 1; // Gray

        // Sort neighbors deterministically
        auto neighbors = adj[u];
        std::sort(neighbors.begin(), neighbors.end());

        for (const auto& v : neighbors) {
            if (color[v] == 1) {
                // Back-edge found -> cycle!
                detectedCycle.push_back(v);
                std::string curr = u;
                while (curr != v && !curr.empty()) {
                    detectedCycle.push_back(curr);
                    curr = parent[curr];
                }
                detectedCycle.push_back(v);
                std::reverse(detectedCycle.begin(), detectedCycle.end());
                return true;
            }
            if (color[v] == 0) {
                parent[v] = u;
                if (dfsCycle(v)) return true;
            }
        }

        color[u] = 2; // Black
        return false;
    };

    // Sort task list deterministically for cycle checking
    std::vector<std::string> taskIds;
    for (const auto& t : tasks) taskIds.push_back(t.id);
    std::sort(taskIds.begin(), taskIds.end());

    for (const auto& id : taskIds) {
        if (color[id] == 0) {
            if (dfsCycle(id)) {
                break;
            }
        }
    }

    if (!detectedCycle.empty()) {
        result.success = false;
        result.isValidSchedule = false;
        result.detectedCycle = detectedCycle;

        std::string cycleStr;
        for (size_t i = 0; i < detectedCycle.size(); ++i) {
            cycleStr += detectedCycle[i];
            if (i + 1 < detectedCycle.size()) cycleStr += " -> ";
        }

        ValidationViolation v;
        v.rule = "ACYCLIC_DEPENDENCY_GRAPH";
        v.taskId = detectedCycle.front();
        v.message = "Circular dependency cycle detected: " + cycleStr;
        result.validationViolations.push_back(v);

        result.warnings.push_back("FATAL GRAPH ERROR: Dependency cycle detected (" + cycleStr +
                                  "). In Mode C, tasks involved in a dependency cycle can never be scheduled without deadlock.");

        for (const auto& t : tasks) {
            Task unsched = t;
            unsched.isScheduled = false;
            unsched.exclusionReason = "Blocked by dependency cycle: " + cycleStr;
            result.unscheduledTasks.push_back(unsched);
        }

        auto endTime = std::chrono::high_resolution_clock::now();
        result.executionTimeMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();
        return result;
    }

    // Step 3: Kahn's Algorithm with Min-Heap for Deterministic Topological Order
    // Min-heap ordering: priority (asc), then title (asc), then ID (asc)
    auto cmp = [&taskMap](const std::string& a, const std::string& b) {
        const auto& ta = taskMap[a];
        const auto& tb = taskMap[b];
        if (ta.priority != tb.priority) return ta.priority > tb.priority;
        if (ta.title != tb.title) return ta.title > tb.title;
        return ta.id > tb.id;
    };
    std::priority_queue<std::string, std::vector<std::string>, decltype(cmp)> pq(cmp);

    for (const auto& [id, deg] : inDegree) {
        if (deg == 0) {
            pq.push(id);
        }
    }

    std::vector<std::string> topoOrder;
    while (!pq.empty()) {
        std::string u = pq.top();
        pq.pop();
        topoOrder.push_back(u);

        for (const auto& v : adj[u]) {
            inDegree[v]--;
            if (inDegree[v] == 0) {
                pq.push(v);
            }
        }
    }

    result.topologicalOrder = topoOrder;

    // Step 4: Schedule Assignment Policy
    // Earliest Start Time ES(v) = max(horizonStart, max_{u in preds(v)} finishTime(u))
    std::unordered_map<std::string, int> scheduledFinishes;
    std::unordered_map<std::string, int> scheduledStarts;
    std::unordered_map<std::string, Task> scheduledTaskMap;

    for (const auto& id : topoOrder) {
        Task t = taskMap[id];
        int earliestStart = horizonStart;

        for (const auto& prereqId : t.prerequisiteIds) {
            if (scheduledFinishes.find(prereqId) != scheduledFinishes.end()) {
                earliestStart = std::max(earliestStart, scheduledFinishes[prereqId]);
            }
        }

        int finish = earliestStart + std::max(1, t.duration);
        t.isScheduled = true;
        t.scheduledStart = earliestStart;
        t.scheduledFinish = finish;

        scheduledStarts[id] = earliestStart;
        scheduledFinishes[id] = finish;

        // Check deadline violation
        if (t.deadline.has_value() && finish > *t.deadline) {
            int overrun = finish - *t.deadline;
            std::string warn = "Deadline violation on task '" + t.title + "': completes at t=" +
                               std::to_string(finish) + " but deadline is t=" + std::to_string(*t.deadline) +
                               " (overrun by " + std::to_string(overrun) + " time units)";
            result.warnings.push_back(warn);
        }

        scheduledTaskMap[id] = t;
        result.scheduledTasks.push_back(t);
        result.totalValue += t.value;
        result.totalDuration = std::max(result.totalDuration, finish);
    }

    // Step 5: Critical Path Analysis (Longest path in DAG)
    // dist[v] = maximum total duration of a path ending at v
    std::unordered_map<std::string, int> dist;
    std::unordered_map<std::string, std::string> critParent;
    for (const auto& id : topoOrder) {
        dist[id] = taskMap[id].duration;
    }

    std::string critEnd = topoOrder.empty() ? "" : topoOrder.front();
    int maxPathLen = 0;

    for (const auto& u : topoOrder) {
        for (const auto& v : adj[u]) {
            int newDist = dist[u] + taskMap[v].duration;
            if (newDist > dist[v]) {
                dist[v] = newDist;
                critParent[v] = u;
                if (newDist > maxPathLen) {
                    maxPathLen = newDist;
                    critEnd = v;
                }
            }
        }
    }

    if (!critEnd.empty()) {
        std::vector<std::string> path;
        std::string curr = critEnd;
        while (!curr.empty()) {
            path.push_back(curr);
            if (critParent.find(curr) != critParent.end()) {
                curr = critParent[curr];
            } else {
                break;
            }
        }
        std::reverse(path.begin(), path.end());
        result.criticalPath = path;
    }

    result.scheduledCount = static_cast<int>(result.scheduledTasks.size());
    result.unscheduledCount = 0;
    result.success = true;
    result.isValidSchedule = true;

    auto endTime = std::chrono::high_resolution_clock::now();
    result.executionTimeMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();
    return result;
}

} // namespace intellischedule

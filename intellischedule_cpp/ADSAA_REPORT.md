# ADSAA Project Report: Intelligent Task Scheduling and Resource Optimization Platform (IntelliSchedule)

**Course:** Advanced Data Structures and Algorithms Analysis (ADSAA)  
**Project Title:** INTELLISCHEDULE – Intelligent Task Scheduling and Resource Optimization Platform  
**Language Standard:** C++20 (ISO/IEC 14882:2020)  
**Academic Focus:** Dynamic Programming, Greedy Strategy, Topological Graph Ordering, Invariant Validation  

---

## 1. Executive Summary & Problem Statement

Modern software systems, industrial manufacturing, and engineering sprint planning require allocating finite resources (developer hours, single compute processors, conference suites) among competing tasks. Many naive commercial schedulers attempt to conflate fundamentally distinct mathematical problems into a single heuristic, erroneously claiming universal optimality.

The **IntelliSchedule** platform solves this challenge by decoupling task scheduling into three mathematically rigorous, well-defined problem modes:
1. **Mode A (Maximum-Value Task Selection):** Modeled as the discrete 0/1 Knapsack Problem.
2. **Mode B (Maximum-Number Interval Scheduling):** Modeled as Earliest Finish Time (EFT) Greedy Selection.
3. **Mode C (Prerequisite-Aware Task Ordering):** Modeled as Cycle-Guarded Topological Sorting and Critical Path Analysis on a Directed Acyclic Graph (DAG).

Every generated schedule undergoes independent automated invariant validation before verification.

---

## 2. Project Objectives

1. **Exact Mathematical Modeling:** Replace ad-hoc scheduling heuristics with distinct algorithmic paradigms.
2. **Empirical Algorithm Comparison:** Implement both 0/1 Knapsack DP (exact optimum) and the Greedy Value-to-Duration ratio heuristic, empirically demonstrating that greedy heuristics can strand budget capacity in discrete optimization.
3. **Provable Correctness & Deadlock Guarding:** Ensure that circular dependencies in task graphs ($u \to v \to \dots \to u$) are detected in $O(V + E)$ time and surfaced with the exact cyclic path, preventing catastrophic system deadlock.
4. **Independent Invariant Verification:** Design a decoupled validator that independently audits non-negative durations, at-most-once selection, capacity adherence, and temporal precedence.

---

## 3. Formal Problem Definitions & Assumptions

### Mode A: Maximum-Value Task Selection (0/1 Knapsack)
* **Given:** A set of $n$ independent tasks $T = \{t_1, t_2, \dots, t_n\}$, where each task $t_i$ has a strictly positive integer duration $d_i \in \mathbb{Z}^+$ and a non-negative priority value $v_i \in \mathbb{R}_{\ge 0}$. A finite scalar time budget $W \in \mathbb{Z}^+$.
* **Objective:** Find a subset of task indices $S \subseteq \{1, \dots, n\}$ maximizing:
  $$\max_{S} \sum_{i \in S} v_i \quad \text{subject to} \quad \sum_{i \in S} d_i \le W$$
* **Assumptions:** Tasks are independent (no dependencies) and discrete (0/1 choice: a task is either fully scheduled or omitted; preemption or fractional execution is prohibited).
* **Complexity:** NP-Complete (weakly NP-hard, solvable in pseudo-polynomial time $O(nW)$ via Dynamic Programming).

### Mode B: Maximum-Number Interval Scheduling
* **Given:** A set of $n$ tasks $T = \{t_1, \dots, t_n\}$ where each task $t_i$ requires exclusive access to a single resource during a pre-assigned, immutable half-open interval $[s_i, f_i)$ with $0 \le s_i < f_i$.
* **Objective:** Find a subset $S \subseteq T$ of maximum cardinality $|S|$ such that for all distinct $t_i, t_j \in S$:
  $$[s_i, f_i) \cap [s_j, f_j) = \emptyset \iff (f_i \le s_j) \lor (f_j \le s_i)$$
* **Assumptions:** Tasks cannot be shifted or stretched; single shared resource.
* **Complexity:** Solvable in polynomial time $O(n \log n)$ via greedy selection by Earliest Finish Time.

### Mode C: Prerequisite-Aware Task Ordering (DAG Scheduling)
* **Given:** A set of tasks $V = \{t_1, \dots, t_n\}$ with positive durations $d_i > 0$ and directed edges $E = \{(u, v) \mid u, v \in V\}$ where directed edge $(u, v)$ signifies that task $u$ is a prerequisite for task $v$.
* **Objective:**
  1. Determine whether $G=(V, E)$ contains a directed cycle. If a cycle exists, abort and output the cycle loop.
  2. If $G$ is a DAG, produce a topological ordering $\pi$ such that for all $(u, v) \in E$, $\pi(u) < \pi(v)$.
  3. Assign earliest feasible start and finish times:
     $$ES(v) = \max \left( \text{horizonStart}, \max_{u \in \text{preds}(v)} (\text{FinishTime}(u)) \right)$$
     $$\text{FinishTime}(v) = ES(v) + d_v$$
  4. Compute the Critical Path (longest path through the DAG).
* **Assumptions:** Tasks can be executed in sequence or parallel as soon as all prerequisite dependencies finish.

---

## 4. Algorithms & Pseudocode

### 4.1 Mode A: 0/1 Knapsack Dynamic Programming
```text
Algorithm KnapsackDP(tasks, W):
  n = |tasks|
  Sort tasks deterministically by ID
  Initialize 2D table DP[0..n][0..W] to 0.0

  For i from 1 to n:
    (d, v) = (tasks[i-1].duration, tasks[i-1].value)
    For w from 0 to W:
      DP[i][w] = DP[i-1][w]
      If d <= w and d > 0:
        If DP[i-1][w - d] + v > DP[i][w]:
          DP[i][w] = DP[i-1][w - d] + v

  // Reconstruction
  currW = W
  selected = []
  For i from n down to 1:
    (d, v) = (tasks[i-1].duration, tasks[i-1].value)
    If d <= currW and |DP[i][currW] - (DP[i-1][currW - d] + v)| < EPS:
      selected.append(tasks[i-1])
      currW = currW - d

  Return selected
```

### 4.2 Mode A: Greedy Ratio Heuristic
```text
Algorithm KnapsackGreedy(tasks, W):
  Sort tasks in descending order of (value / duration), breaking ties by higher value, then shorter duration, then ID
  remainingBudget = W
  selected = []

  For each task in sortedTasks:
    If task.duration <= remainingBudget:
      selected.append(task)
      remainingBudget -= task.duration

  Return selected
```

### 4.3 Mode B: Earliest Finish Time (EFT) Greedy Scheduler
```text
Algorithm IntervalSchedulerEFT(tasks):
  Filter tasks where finish > start >= 0
  Sort valid tasks primarily by finish ascending, then start ascending, then ID
  lastFinish = -1
  selected = []

  For each task in sortedTasks:
    If task.start >= lastFinish:
      selected.append(task)
      lastFinish = task.finish

  Return selected
```

### 4.4 Mode C: Cycle Detection & Kahn's Topological Sort
```text
Algorithm GraphScheduler(tasks):
  Build adjacency list ADJ and in-degree map IN_DEG
  
  // Phase 1: Cycle Detection via DFS 3-Coloring (0=White, 1=Gray, 2=Black)
  For each vertex u in tasks:
    If color[u] == 0:
      If DFS_Visit(u) == CYCLE_FOUND:
        Return Infeasible("Circular Dependency Cycle Detected")

  // Phase 2: Kahn's Algorithm with Deterministic Priority Queue
  Initialize PriorityQueue PQ with all u having IN_DEG[u] == 0 (ordered by priority asc, ID asc)
  topoOrder = []

  While PQ is not empty:
    u = PQ.pop()
    topoOrder.append(u)
    For each v in ADJ[u]:
      IN_DEG[v] -= 1
      If IN_DEG[v] == 0:
        PQ.push(v)

  // Phase 3: Timeline & Critical Path DP
  For each u in topoOrder:
    start[u] = max(horizon, max_{p in preds[u]} finish[p])
    finish[u] = start[u] + duration[u]

  Return topoOrder, start, finish, CriticalPath
```

---

## 5. Correctness Proofs & Invariants

### 5.1 Mode A: Optimality of Dynamic Programming
* **Optimal Substructure:** Suppose an optimal subset $S^*$ for capacity $W$ includes task $n$. Then the subset $S^* \setminus \{n\}$ must be an optimal solution for the subproblem with capacity $W - d_n$ over tasks $\{1, \dots, n-1\}$. If a superior subset existed for that subproblem, substituting it into $S^*$ would yield a total value exceeding the presumed optimal $S^*$, a contradiction.
* **Sub-Optimality of Greedy Ratio:** The Greedy heuristic fails for discrete 0/1 Knapsack because items cannot be divided. As proven in Unit Test 4, with $W=10$ and items $t_1=(d=6, v=36, r=6.0)$ vs $t_2, t_3=(d=5, v=26, r=5.2)$, greedy picks $t_1$ and achieves $V=36$, whereas DP picks $t_2 + t_3$ and achieves $V=52$ (a $30.8\%$ efficiency gap).

### 5.2 Mode B: "Greedy Stays Ahead" Proof for EFT
* **Inductive Hypothesis:** Let $i_1, i_2, \dots, i_k$ be the greedy selections, and let $j_1, j_2, \dots, j_m$ be an arbitrary optimal schedule sorted by finish time.
* **Base Case:** By definition of EFT, $f(i_1) \le f(j_1)$.
* **Inductive Step:** Assume $f(i_r) \le f(j_r)$ for $r < k$. Since $j_{r+1}$ is compatible with $j_r$, $s(j_{r+1}) \ge f(j_r) \ge f(i_r)$. Thus $j_{r+1}$ is a valid candidate for the greedy algorithm at step $r+1$. Since greedy chooses the task with the minimum finish time among all remaining candidates, $f(i_{r+1}) \le f(j_{r+1})$.
* Hence, for all $r \le k$, the greedy finish time is at least as early as the optimal finish time. If $m > k$, candidate $j_{k+1}$ would be compatible with $i_k$, so the greedy algorithm could not have terminated at step $k$. Therefore, $k = m$, proving global optimality.

---

## 6. Time and Space Complexity Analysis

| Mode / Module | Algorithm | Time Complexity | Auxiliary Space | Deterministic Tie-Breaking Policy |
| :--- | :--- | :--- | :--- | :--- |
| **Mode A (Optimal)** | 0/1 Knapsack Dynamic Programming | $O(n \cdot W)$ | $O(n \cdot W)$ table | Lexicographical ID ascending on equal DP cell values |
| **Mode A (Heuristic)** | Value-per-Duration Density Greedy | $O(n \log n)$ | $O(n)$ | Higher value $\to$ shorter duration $\to$ ID ascending |
| **Mode B** | Earliest Finish Time (EFT) Greedy | $O(n \log n)$ | $O(n)$ | Earlier finish $\to$ earlier start $\to$ ID ascending |
| **Mode C (Order)** | Kahn's Topological Sort | $O(V + E)$ | $O(V + E)$ | Min-Priority Queue by priority $\to$ title $\to$ ID |
| **Mode C (Cycle)** | DFS 3-Coloring Cycle Extraction | $O(V + E)$ | $O(V)$ recursion | Deterministic neighbor sorting before DFS visit |
| **Mode C (Path)** | Critical Path Analysis via DAG DP | $O(V + E)$ | $O(V)$ | Longest path predecessor tracking |
| **Validator** | Independent Invariant Audit | $O(n^2)$ Mode B, $O(n + E)$ Mode A/C | $O(n)$ | Full pairwise non-overlap inspection |

---

## 7. Experimental Verification & Test Suite Matrix

The C++ unit test suite (`intellischedule_tests`) runs 11 rigorous automated verification tests:

| Test ID | Test Name | Tested Principle | Verification Result |
| :---: | :--- | :--- | :---: |
| **1** | `test_empty_task_set` | Boundary handling of empty sets across all modes | **PASSED** |
| **2** | `test_one_task_fits_and_does_not_fit` | Boundary capacity enforcement ($d \le W$ vs $d > W$) | **PASSED** |
| **3** | `test_dp_vs_brute_force_exhaustive` | DP equivalence against $O(2^n)$ exhaustive search | **PASSED** |
| **4** | `test_greedy_vs_dp_counterexample` | Empirical demonstration that Greedy is sub-optimal | **PASSED** |
| **5** | `test_interval_scheduling_overlaps` | Mutual compatibility filtering in Mode B | **PASSED** |
| **6** | `test_equal_finish_times_deterministic_tie_breaking` | Stable sorting & reproducible selection on equal EFT | **PASSED** |
| **7** | `test_dependency_chain_and_branching_dag` | Topological sort and earliest start time propagation | **PASSED** |
| **8** | `test_dependency_cycle_detection` | DFS 3-coloring cycle detection and deadlock abort | **PASSED** |
| **9** | `test_missing_dependency_id` | Validation failure when referencing nonexistent task ID | **PASSED** |
| **10** | `test_invalid_negative_duration` | Invariant rejection of tasks with duration $\le 0$ | **PASSED** |
| **11** | `test_schedule_invariant_validation` | Validator rejection of forged overlapping intervals | **PASSED** |

**Summary:** 11/11 tests passed with zero failures.

---

## 8. Limitations & Future Scope

1. **Multi-Resource Interval Scheduling:** Current Mode B schedules on a single machine/resource. Future iterations can implement the greedy interval partitioning algorithm with $d$ resources using a min-heap of finish times in $O(n \log d)$.
2. **Preemptive Scheduling with Deadlines:** Integrating Earliest Deadline First (EDF) and Least Slack Time (LST) for real-time operating systems workloads.
3. **Branch-and-Bound for Multidimensional Knapsack:** Extending Mode A to handle multiple concurrent resource budgets (e.g., CPU, RAM, GPU bandwidth).

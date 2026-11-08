# IntelliSchedule: Faculty Demonstration & Evaluation Script

This document provides a structured walkthrough for demonstrating the **IntelliSchedule** platform during faculty project evaluations, viva examinations, and lab defenses.

---

## Demo Overview Table

| Scenario | Mode | Academic Topic | Key Takeaway Demonstrated |
| :--- | :--- | :--- | :--- |
| **Demo 1** | **Mode A** | 0/1 Knapsack DP vs Greedy Ratio | Greedy is sub-optimal; DP guarantees global maximum |
| **Demo 2** | **Mode B** | Interval Scheduling (EFT) | Greedy stays ahead; optimal cardinality on 1 resource |
| **Demo 3** | **Mode C** | Prerequisite DAG & Critical Path | Kahn's topological sort & earliest precedence timeline |
| **Demo 4** | **Mode C** | Cycle Deadlock Detection | DFS 3-coloring cycle isolation; deadlock prevention |

---

## Scenario 1: Sprint Backlog Optimization (Mode A)

### Context & Problem
A software engineering team has an available sprint capacity of **$W = 16$ developer-hours**. Five candidate user stories have been estimated with positive durations and priority values.

### Step-by-Step Actions in the UI
1. Select **"Sprint Planning: Maximum Value Selection (Knapsack)"** from the scenario dropdown.
2. Observe the initial tasks:
   - `TASK-A1`: Duration = 6, Value = 18.0 (Ratio = 3.00)
   - `TASK-A2`: Duration = 10, Value = 25.0 (Ratio = 2.50)
   - `TASK-A3`: Duration = 10, Value = 24.0 (Ratio = 2.40)
   - `TASK-A4`: Duration = 5, Value = 11.0 (Ratio = 2.20)
   - `TASK-A5`: Duration = 3, Value = 5.0 (Ratio = 1.67)
3. Click the **"Compare DP vs Greedy"** button.

### What to Explain to the Faculty
- **Dynamic Programming Result:**
  - Selects `TASK-A1` (6h) and `TASK-A2` (10h).
  - Total Duration = $16 / 16$ hours ($100\%$ capacity utilization).
  - Total Utility Value = **$43.0$ points**.
- **Greedy Ratio Heuristic:**
  - Evaluates items by value density $v_i / d_i$.
  - First picks `TASK-A1` ($r = 3.00$, leaving 10h).
  - Next picks `TASK-A2` ($r = 2.50$, leaving 0h).
- Now reduce the budget to **$W = 10$** and add tasks:
  - `T1`: Duration 6, Value 36 (Ratio = 6.0)
  - `T2`: Duration 5, Value 26 (Ratio = 5.2)
  - `T3`: Duration 5, Value 26 (Ratio = 5.2)
- Re-run comparison:
  - **Greedy** picks `T1` (Ratio 6.0) and stops because 4 hours remain. Total value = **$36.0$**.
  - **DP** selects `T2` and `T3`. Total value = **$52.0$**!
  - **Theoretical Defense:** *"0/1 Knapsack is NP-complete. While the greedy density ratio is optimal for Fractional Knapsack, in discrete optimization it leaves stranded capacity. Our DP algorithm guarantees the exact optimum in $O(nW)$ time."*

---

## Scenario 2: Executive Boardroom Scheduling (Mode B)

### Context & Problem
Multiple executives and project teams have requested exclusive access to Boardroom A. The requests possess fixed, non-negotiable $[s_i, f_i)$ start and finish times.

### Step-by-Step Actions in the UI
1. Select **"Executive Conference Room Scheduling (Earliest Finish Time)"** from the scenario dropdown.
2. Click **Mode B: Interval Scheduling** in the mode bar.
3. Click **"Generate Schedule"**.

### What to Explain to the Faculty
- The engine sorts tasks deterministically by earliest finish time $f_i$ ascending ($O(n \log n)$).
- When finish times match (e.g., `MEET-07` and `MEET-08` both finish at $t=11$), deterministic tie-breaking chooses earlier start time, then alphabetical ID.
- The timeline displays the maximum number of non-overlapping meetings.
- The **Schedule Invariant Validator** displays **"ALL INVARIANTS SATISFIED"**, having independently checked all pairwise intervals for any overlap:
  $$\forall i \ne j, \quad \max(s_i, s_j) \ge \min(f_i, f_j)$$
- **Theoretical Defense:** *"Why does Earliest Finish Time maximize cardinality? By the 'Greedy Stays Ahead' inductive argument: at every step $k$, the greedy schedule finishes no later than any arbitrary optimal schedule, leaving maximum possible remaining time for subsequent tasks."*

---

## Scenario 3: CI/CD Build & Deployment Pipeline (Mode C)

### Context & Problem
An enterprise release pipeline consists of 8 interconnected stages with explicit precedence dependencies. For example, Unit Tests (`PIPE-04`) cannot run until C++ Engine Compilation (`PIPE-02`) completes.

### Step-by-Step Actions in the UI
1. Select **"Software Build & Release Pipeline (Prerequisite DAG)"** from the scenario dropdown.
2. Click **"Generate Schedule"**.

### What to Explain to the Faculty
- **Topological Sorting:** Kahn's algorithm processes vertices with in-degree 0 using a min-priority queue, ensuring reproducible, deterministic ordering.
- **Timeline Assignment Policy:**
  $$ES(v) = \max \left( \text{horizonStart}, \max_{u \in \text{preds}(v)} (\text{FinishTime}(u)) \right)$$
  Tasks with independent prerequisites run in parallel; dependent tasks start immediately after their slowest prerequisite.
- **Critical Path Analysis:** Highlighted with amber borders (`PIPE-01 ➔ PIPE-02 ➔ PIPE-05 ➔ PIPE-07 ➔ PIPE-08`). This is the longest duration path through the network ($26$ hours), governing the minimum time needed to complete the entire pipeline.

---

## Scenario 4: Circular Deadlock Detection (Fault Injection Test)

### Context & Problem
A software developer inadvertently introduces a circular dependency:
`MOD-A` requires `MOD-C`, `MOD-C` requires `MOD-B`, and `MOD-B` requires `MOD-A`.

### Step-by-Step Actions in the UI
1. Click the **"Cycle Deadlock Demo"** button on the top header.
2. Click **"Generate Schedule"**.

### What to Explain to the Faculty
- The application **does not crash, hang in an infinite loop, or silently ignore the cycle**.
- The engine runs DFS 3-coloring ($0 = \text{White}, 1 = \text{Gray}, 2 = \text{Black}$). A back-edge to a Gray vertex immediately identifies the circular loop.
- The UI displays a prominent red alert:
  **"CIRCULAR DEPENDENCY DEADLOCK DETECTED"**
  `CYCLE PATH: MOD-A ➔ MOD-B ➔ MOD-C ➔ MOD-A`
- All affected tasks are placed in the **Unscheduled Tasks** panel with the reason *"Blocked by dependency cycle"*.
- The Schedule Validator marks `isValidSchedule = false`.
- **Theoretical Defense:** *"A graph with a directed cycle cannot be topologically sorted because no linear ordering exists where all edges point forward. Our validator guarantees an invalid or deadlocked schedule is never marked successful."*

---

## Scenario 5: Automated Verification Suite

### Action
Click the **"Run C++ Test Suite"** button on the top right.

### Result
An embedded terminal displays the live execution of the compiled native binary (`intellischedule_tests`):
```text
TEST SUMMARY: 11/11 PASSED - ALL ASSERTS VERIFIED!
```
Every mathematical property (DP vs brute force, greedy ratio counterexample, interval compatibility, topological precedence, cycle detection, negative duration rejection) is verified in $< 10$ milliseconds.

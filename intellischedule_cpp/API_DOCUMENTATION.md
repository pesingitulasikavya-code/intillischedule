# IntelliSchedule REST API Documentation

This document describes all RESTful endpoints exposed by the IntelliSchedule platform (supported both by the standalone C++ `intellischedule_server` on port 8080 and the Express/Node full-stack service on port 3000).

---

## 1. System Health & Engine Status

### `GET /api/health`
Returns the operational health, C++ standard, supported algorithmic modules, and embedded database status.

**Example Request:**
```bash
curl -X GET http://localhost:8080/api/health
```

**Response (HTTP 200 OK):**
```json
{
  "algorithms": [
    "0/1 Knapsack Dynamic Programming (Exact)",
    "Greedy Value-per-Duration Heuristic",
    "Earliest Finish Time (EFT) Interval Scheduler",
    "Kahn's Topological Sort & Cycle Detection",
    "Critical Path Dynamic Programming"
  ],
  "cpp_standard": "C++20",
  "engine": "IntelliSchedule C++20 Core Optimization Engine",
  "status": "ok",
  "storage": "SQLite3 Embedded Engine",
  "version": "1.0.0"
}
```

---

## 2. Task Management Endpoints

### `GET /api/tasks`
Retrieves all persisted tasks from the SQLite database.

**Example Request:**
```bash
curl -X GET http://localhost:8080/api/tasks
```

**Response (HTTP 200 OK):**
```json
[
  {
    "id": "TASK-A1",
    "title": "Security Patch Authentication Fix",
    "description": "High security vulnerability CVE-2026-901",
    "duration": 6,
    "deadline": 20,
    "value": 18.0,
    "priority": 1,
    "startTime": null,
    "finishTime": null,
    "prerequisiteIds": [],
    "isScheduled": false,
    "scheduledStart": null,
    "scheduledFinish": null,
    "exclusionReason": null
  }
]
```

---

### `POST /api/tasks`
Creates a new task in the database.

**Example Request:**
```bash
curl -X POST http://localhost:8080/api/tasks \
  -H "Content-Type: application/json" \
  -d '{
    "id": "TASK-NEW",
    "title": "Asynchronous Event Bus Architecture",
    "description": "Decoupled pub-sub channel using Redis streams",
    "duration": 5,
    "value": 22.0,
    "priority": 1,
    "deadline": 30,
    "prerequisiteIds": []
  }'
```

**Response (HTTP 201 Created):**
```json
{
  "success": true,
  "task": {
    "id": "TASK-NEW",
    "title": "Asynchronous Event Bus Architecture",
    "duration": 5,
    "value": 22.0,
    "priority": 1,
    "deadline": 30,
    "prerequisiteIds": []
  }
}
```

---

### `PUT /api/tasks/{id}`
Updates an existing task by its unique ID.

**Example Request:**
```bash
curl -X PUT http://localhost:8080/api/tasks/TASK-NEW \
  -H "Content-Type: application/json" \
  -d '{
    "title": "Asynchronous Event Bus Architecture (v2)",
    "duration": 4,
    "value": 25.0,
    "priority": 1,
    "deadline": 28,
    "prerequisiteIds": []
  }'
```

**Response (HTTP 200 OK):**
```json
{
  "success": true,
  "task": {
    "id": "TASK-NEW",
    "title": "Asynchronous Event Bus Architecture (v2)",
    "duration": 4,
    "value": 25.0,
    "priority": 1
  }
}
```

---

### `DELETE /api/tasks/{id}`
Deletes a task by ID from the database.

**Example Request:**
```bash
curl -X DELETE http://localhost:8080/api/tasks/TASK-NEW
```

**Response (HTTP 200 OK):**
```json
{
  "success": true,
  "id": "TASK-NEW"
}
```

---

## 3. Core Optimization & Scheduling Engine

### `POST /api/schedule`
Executes task scheduling under explicit algorithmic constraints for Mode A, Mode B, or Mode C.

#### Mode A Request (0/1 Knapsack Dynamic Programming):
```bash
curl -X POST http://localhost:8080/api/schedule \
  -H "Content-Type: application/json" \
  -d '{
    "mode": "MODE_A_MAX_VALUE_KNAPSACK",
    "timeBudget": 16,
    "algorithm": "DP",
    "tasks": [
      { "id": "T1", "title": "Patch", "duration": 6, "value": 18.0, "priority": 1 },
      { "id": "T2", "title": "Index", "duration": 10, "value": 25.0, "priority": 2 },
      { "id": "T3", "title": "Webhook", "duration": 10, "value": 24.0, "priority": 1 }
    ]
  }'
```

**Response (HTTP 200 OK):**
```json
{
  "success": true,
  "mode": "MODE_A_MAX_VALUE_KNAPSACK",
  "algorithmName": "0/1 Knapsack Dynamic Programming (Exact Optimum)",
  "assumptions": "Mode A assumes tasks are independent, non-divisible (0/1 choice), have strictly positive durations, and non-negative values. The time budget W is a hard upper bound.",
  "complexityExplanation": "Time: O(n * W), Space: O(n * W) table allocation for deterministic ID backtracking. Globally optimal for the mathematical 0/1 knapsack formulation.",
  "totalValue": 43.0,
  "totalDuration": 16,
  "scheduledCount": 2,
  "unscheduledCount": 1,
  "executionTimeMs": 0.007,
  "scheduledTasks": [
    {
      "id": "T1",
      "title": "Patch",
      "duration": 6,
      "value": 18.0,
      "priority": 1,
      "isScheduled": true,
      "scheduledStart": 0,
      "scheduledFinish": 6
    },
    {
      "id": "T2",
      "title": "Index",
      "duration": 10,
      "value": 25.0,
      "priority": 2,
      "isScheduled": true,
      "scheduledStart": 6,
      "scheduledFinish": 16
    }
  ],
  "unscheduledTasks": [
    {
      "id": "T3",
      "title": "Webhook",
      "duration": 10,
      "value": 24.0,
      "isScheduled": false,
      "exclusionReason": "Excluded by 0/1 Knapsack DP optimization: combination of other tasks yields higher aggregate value"
    }
  ],
  "isValidSchedule": true,
  "validationViolations": [],
  "warnings": []
}
```

---

#### Mode B Request (Interval Scheduling via Earliest Finish Time):
```bash
curl -X POST http://localhost:8080/api/schedule \
  -H "Content-Type: application/json" \
  -d '{
    "mode": "MODE_B_INTERVAL_SCHEDULING",
    "tasks": [
      { "id": "M1", "title": "Morning Sync", "duration": 2, "value": 10.0, "startTime": 1, "finishTime": 3 },
      { "id": "M2", "title": "Client Review", "duration": 4, "value": 20.0, "startTime": 2, "finishTime": 6 },
      { "id": "M3", "title": "Retrospective", "duration": 2, "value": 12.0, "startTime": 3, "finishTime": 5 },
      { "id": "M4", "title": "Security Brief", "duration": 3, "value": 15.0, "startTime": 5, "finishTime": 8 }
    ]
  }'
```

---

#### Mode C Request (Prerequisite Ordering & Cycle Detection):
```bash
curl -X POST http://localhost:8080/api/schedule \
  -H "Content-Type: application/json" \
  -d '{
    "mode": "MODE_C_PREREQUISITE_ORDERING",
    "tasks": [
      { "id": "A", "title": "Step A", "duration": 3, "value": 10, "prerequisiteIds": [] },
      { "id": "B", "title": "Step B", "duration": 4, "value": 10, "prerequisiteIds": ["A"] },
      { "id": "C", "title": "Step C", "duration": 2, "value": 10, "prerequisiteIds": ["B"] }
    ]
  }'
```

**If a cycle is passed (e.g. A -> B -> C -> A), the API returns:**
```json
{
  "success": false,
  "isValidSchedule": false,
  "detectedCycle": ["A", "B", "C", "A"],
  "validationViolations": [
    {
      "rule": "ACYCLIC_DEPENDENCY_GRAPH",
      "taskId": "A",
      "message": "Circular dependency cycle detected: A -> B -> C -> A"
    }
  ],
  "warnings": [
    "FATAL GRAPH ERROR: Dependency cycle detected (A -> B -> C -> A). In Mode C, tasks involved in a dependency cycle can never be scheduled without deadlock."
  ]
}
```

---

### `POST /api/compare`
Executes both 0/1 Knapsack DP and the Greedy Value-per-Duration heuristic simultaneously on the same task set and time budget, computing the exact efficiency gap and empirical explanation.

**Example Request:**
```bash
curl -X POST http://localhost:8080/api/compare \
  -H "Content-Type: application/json" \
  -d '{
    "timeBudget": 10,
    "tasks": [
      { "id": "T1", "title": "High Density Task", "duration": 6, "value": 36.0 },
      { "id": "T2", "title": "Medium Task A", "duration": 5, "value": 26.0 },
      { "id": "T3", "title": "Medium Task B", "duration": 5, "value": 26.0 }
    ]
  }'
```

**Response (HTTP 200 OK):**
```json
{
  "dpResult": {
    "totalValue": 52.0,
    "totalDuration": 10,
    "scheduledCount": 2,
    "scheduledTasks": [
      { "id": "T2", "duration": 5, "value": 26.0 },
      { "id": "T3", "duration": 5, "value": 26.0 }
    ]
  },
  "greedyResult": {
    "totalValue": 36.0,
    "totalDuration": 6,
    "scheduledCount": 1,
    "scheduledTasks": [
      { "id": "T1", "duration": 6, "value": 36.0 }
    ]
  },
  "valueDifference": 16.0,
  "efficiencyGapPercent": 30.77,
  "greedyMatchedOptimal": false,
  "conclusion": "Dynamic Programming outperformed the Greedy heuristic by 16 value points (30.77% efficiency gap). This empirically proves that the Greedy ratio choice can strand capacity in discrete 0/1 optimization."
}
```

---

## 4. Scenario Management Endpoints

### `GET /api/scenarios`
Lists all predefined academic scenarios stored in SQLite.

### `GET /api/scenarios/{id}`
Retrieves a specific scenario and its full task list.

---

## 5. Automated Testing Endpoint

### `POST /api/tests/run`
Executes the native `intellischedule_tests` binary and returns live execution stdout and exit code.

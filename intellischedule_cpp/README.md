# INTELLISCHEDULE – Intelligent Task Scheduling & Resource Optimization Platform

**Course:** Advanced Data Structures and Algorithms Analysis (ADSAA)  
**Language Standard:** C++20  
**Build System:** CMake 3.20+  
**Persistence:** SQLite 3  
**Network / API:** cpp-httplib & RESTful JSON (nlohmann/json)  
**Frontend:** Responsive Timeline & Interactive Gantt Visualizer  

---

## 1. Project Architecture & Directory Structure

```text
intellischedule_cpp/
├── CMakeLists.txt               # Unified C++20 CMake configuration
├── README.md                    # Setup, build, run, and troubleshooting guide
├── API_DOCUMENTATION.md         # Comprehensive REST API specifications & examples
├── ADSAA_REPORT.md              # Formal academic algorithms report & asymptotic analysis
├── FACULTY_DEMO.md              # Faculty evaluation script with realistic demo scenarios
├── .gitignore                   # Standard Git ignore file for CMake, VS Code, and SQLite
├── include/
│   ├── httplib.h                # Single-header lightweight C++ HTTP server (v0.15.3)
│   ├── nlohmann/
│   │   └── json.hpp             # Modern C++ JSON library (v3.11.3)
│   ├── models.hpp               # Domain models (Task, Scenario, ScheduleResult, Invariants)
│   ├── validator.hpp            # Independent schedule invariant verification engine
│   ├── database.hpp             # SQLite3 persistence abstraction & scenario seeder
│   └── algorithms/
│       ├── knapsack_dp.hpp      # 0/1 Knapsack DP (O(nW) exact optimization)
│       ├── knapsack_greedy.hpp  # Value-per-Duration heuristic (O(n log n) approximation)
│       ├── interval_scheduler.hpp # Earliest Finish Time (EFT) Greedy Scheduler (O(n log n))
│       └── graph_scheduler.hpp  # Kahn's Topological Sort, Cycle Detection & Critical Path
├── src/
│   ├── validator.cpp            # Independent schedule invariant auditor
│   ├── database.cpp             # SQLite database operations & default scenario seeds
│   ├── cli_engine.cpp           # CLI engine executable for automated pipelines & IPC
│   ├── server_main.cpp          # Standalone C++ HTTP microservice listening on port 8080
│   └── algorithms/
│       ├── knapsack_dp.cpp      # Full 2D DP matrix & deterministic ID backtracking
│       ├── knapsack_greedy.cpp  # Deterministic density sorting & capacity allocation
│       ├── interval_scheduler.cpp # EFT greedy selection with collision reporting
│       └── graph_scheduler.cpp  # DFS 3-coloring cycle extractor & Critical Path DP
└── tests/
    └── test_runner.cpp          # 11 automated unit tests verifying all problem invariants
```

---

## 2. Windows + Visual Studio Code Setup Guide

### Prerequisites
1. **Visual Studio Code** installed.
2. **C/C++ Compiler with C++20 support**:
   - **Option A (Recommended on Windows):** **Visual Studio 2022 Community** with the *"Desktop development with C++"* workload (MSVC v143 toolset).
   - **Option B:** **MinGW-w64 (GCC 12+)** via [MSYS2](https://www.msys2.org/) or [WinLibs](https://winlibs.com/). Add `bin` to your System `PATH`.
3. **CMake** 3.20 or newer (included with Visual Studio or downloadable from [cmake.org](https://cmake.org/download/)).
4. **SQLite3 Development Libraries**:
   - With MSVC: Install via vcpkg:
     ```powershell
     git clone https://github.com/microsoft/vcpkg.git
     .\vcpkg\bootstrap-vcpkg.bat
     .\vcpkg\vcpkg.exe install sqlite3:x64-windows
     ```
   - With MinGW:
     ```powershell
     pacman -S mingw-w64-x86_64-sqlite3
     ```

### VS Code Extensions Required
Install the following extensions from the VS Code Marketplace:
- **C/C++** (by Microsoft - `ms-vscode.cpptools`)
- **C/C++ Extension Pack** (by Microsoft)
- **CMake** (by twxs)
- **CMake Tools** (by Microsoft - `ms-vscode.cmake-tools`)

### Step-by-Step Setup in VS Code
1. Open VS Code and select **File -> Open Folder...**, then select the project root folder.
2. Press `Ctrl + Shift + P` and run: **CMake: Select a Kit**.
   - Choose **Visual Studio Community 2022 Release - amd64** (or **GCC 12+ x86_64-w64-mingw32**).
3. Press `Ctrl + Shift + P` and run: **CMake: Configure**.
   - If using vcpkg, add to `.vscode/settings.json`:
     ```json
     {
       "cmake.configureSettings": {
         "CMAKE_TOOLCHAIN_FILE": "C:/vcpkg/scripts/buildsystems/vcpkg.cmake"
       }
     }
     ```
4. Build the targets:
   - Press `F7` or click **Build** on the bottom status bar.
   - Or run from the integrated PowerShell terminal:
     ```powershell
     cd intellischedule_cpp
     mkdir build
     cd build
     cmake ..
     cmake --build . --config Release
     ```

---

## 3. Running the Binaries

### Target 1: Automated Unit Test Suite (`intellischedule_tests`)
Runs the 11 verification tests covering edge cases, DP vs brute-force equivalence, greedy counterexamples, cycle detection, and invariant validation:
```powershell
# Windows
.\build\Release\intellischedule_tests.exe

# Linux / macOS
./build/intellischedule_tests
```
**Expected Output:**
```text
==========================================================
  INTELLISCHEDULE C++20 AUTOMATED VERIFICATION SUITE       
==========================================================
[TEST 1] Running test_empty_task_set...
  [PASSED] test_empty_task_set
[TEST 2] Running test_one_task_fits_and_does_not_fit...
  [PASSED] test_one_task_fits_and_does_not_fit
[TEST 3] Running test_dp_vs_brute_force_exhaustive...
  [PASSED] test_dp_vs_brute_force_exhaustive
[TEST 4] Running test_greedy_vs_dp_counterexample...
  [PASSED] test_greedy_vs_dp_counterexample
[TEST 5] Running test_interval_scheduling_overlaps...
  [PASSED] test_interval_scheduling_overlaps
[TEST 6] Running test_equal_finish_times_deterministic_tie_breaking...
  [PASSED] test_equal_finish_times_deterministic_tie_breaking
[TEST 7] Running test_dependency_chain_and_branching_dag...
  [PASSED] test_dependency_chain_and_branching_dag
[TEST 8] Running test_dependency_cycle_detection...
  [PASSED] test_dependency_cycle_detection
[TEST 9] Running test_missing_dependency_id...
  [PASSED] test_missing_dependency_id
[TEST 10] Running test_invalid_negative_duration...
  [PASSED] test_invalid_negative_duration
[TEST 11] Running test_schedule_invariant_validation...
  [PASSED] test_schedule_invariant_validation
==========================================================
TEST SUMMARY: 11/11 PASSED - ALL ASSERTS VERIFIED!
==========================================================
```

### Target 2: Standalone C++ HTTP Microservice (`intellischedule_server`)
Runs the embedded C++ microservice on port 8080:
```powershell
# Windows
.\build\Release\intellischedule_server.exe 8080

# Linux / macOS
./build/intellischedule_server 8080
```

### Target 3: Full-Stack Web Platform (Express + Vite + C++ Native Engine)
From the workspace root:
```bash
npm install
npm run dev
```
Open your browser at `http://localhost:3000` to interact with the full timeline, Gantt chart, comparison matrix, and SQLite database.

---

## 4. Troubleshooting Guide

| Problem | Root Cause | Solution |
| :--- | :--- | :--- |
| `fatal error: sqlite3.h: No such file or directory` | SQLite3 dev headers not installed or vcpkg toolchain path not set | Install `sqlite3` via vcpkg (`vcpkg install sqlite3:x64-windows`) or MSYS2 (`pacman -S mingw-w64-x86_64-sqlite3`). Pass `-DCMAKE_TOOLCHAIN_FILE` to cmake. |
| `Permission denied` when running binary on Linux | Executable permission bit missing | Run `chmod +x ./build/intellischedule_*` |
| `undefined reference to pthread_create` | Posix threads not linked for `httplib.h` | Ensure `find_package(Threads REQUIRED)` and `target_link_libraries(... Threads::Threads)` in CMakeLists.txt. |
| Port 8080 or 3000 in use | Another instance is already bound to the port | Pass an alternative port: `./intellischedule_server 8081` or configure `PORT=3001 npm run dev`. |

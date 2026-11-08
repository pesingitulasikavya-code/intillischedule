# IntelliSchedule: Step-by-Step Git Commands

Follow these steps to initialize, stage, commit, and push the IntelliSchedule project to GitHub.

---

## 1. Initialize Local Git Repository

```bash
# Navigate to the project root directory
cd intellischedule_cpp

# Initialize a new Git repository
git init

# Verify active branch (set default to main)
git branch -M main
```

---

## 2. Check Repository Status and Ignore Files

Ensure that `build/`, `node_modules/`, and binary artifacts are ignored by `.gitignore`:
```bash
# Check untracked files
git status
```
*Note: Compiled binaries (`.exe`, `.so`), `build/`, and `*.db` should not appear in untracked files.*

---

## 3. Stage Source Code & Documentation

```bash
# Stage all source code, CMake configuration, tests, and documentation
git add CMakeLists.txt .gitignore README.md API_DOCUMENTATION.md ADSAA_REPORT.md FACULTY_DEMO.md
git add include/
git add src/
git add tests/

# Verify staged files
git status
```

---

## 4. Commit the Verified C++20 Project

```bash
git commit -m "feat(adsaa): Complete IntelliSchedule C++20 optimization platform

- Implement Mode A: 0/1 Knapsack DP (O(nW)) and greedy ratio heuristic
- Implement Mode B: Earliest Finish Time (EFT) interval scheduling (O(n log n))
- Implement Mode C: Kahn's topological sort and DFS 3-coloring cycle detection
- Add independent schedule invariant validation engine
- Implement SQLite3 persistence for tasks and scenarios
- Add 11 automated unit tests verifying correctness invariants
- Add CMake configuration, API docs, ADSAA report, and faculty demo script"
```

---

## 5. Link and Push to Remote GitHub Repository

```bash
# Create a repository on GitHub named 'intellischedule'
# Add the remote repository URL (replace with your GitHub username)
git remote add origin https://github.com/<your-username>/intellischedule.git

# Verify remote configuration
git remote -v

# Push commits to the main branch
git push -u origin main
```

---

## 6. Daily Workflow Commands

```bash
# Pull latest changes from remote
git pull origin main

# Create a feature branch for new algorithmic improvements
git checkout -b feature/multi-resource-interval-partitioning

# Stage, commit, and push feature branch
git add .
git commit -m "feat: Add multi-machine interval partitioning with min-heap"
git push origin feature/multi-resource-interval-partitioning
```

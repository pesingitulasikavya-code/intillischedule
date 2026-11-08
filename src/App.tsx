import React, { useState, useEffect, useMemo } from 'react';
import axios from 'axios';
import {
  Calendar,
  Clock,
  Cpu,
  Layers,
  Play,
  GitBranch,
  BarChart2,
  CheckCircle2,
  AlertTriangle,
  XCircle,
  Plus,
  Trash2,
  Edit2,
  RotateCcw,
  Terminal,
  Activity,
  Award,
  ChevronRight,
  ShieldCheck,
  Zap,
  Info
} from 'lucide-react';
import './styles.css';

interface Task {
  id: string;
  title: string;
  description?: string;
  duration: number;
  deadline?: number | null;
  value: number;
  priority: number;
  startTime?: number | null;
  finishTime?: number | null;
  prerequisiteIds: string[];
  scheduledStart?: number | null;
  scheduledFinish?: number | null;
  isScheduled?: boolean;
  exclusionReason?: string | null;
}

interface ValidationViolation {
  rule: string;
  taskId: string;
  message: string;
}

interface ScheduleResult {
  success: boolean;
  mode: string;
  algorithmName: string;
  assumptions: string;
  complexityExplanation: string;
  totalValue: number;
  totalDuration: number;
  scheduledCount: number;
  unscheduledCount: number;
  executionTimeMs: number;
  scheduledTasks: Task[];
  unscheduledTasks: Task[];
  topologicalOrder: string[];
  detectedCycle: string[];
  criticalPath: string[];
  isValidSchedule: boolean;
  validationViolations: ValidationViolation[];
  warnings: string[];
}

interface ModeAComparison {
  dpResult: ScheduleResult;
  greedyResult: ScheduleResult;
  valueDifference: number;
  efficiencyGapPercent: number;
  greedyMatchedOptimal: boolean;
  conclusion: string;
}

interface Scenario {
  id: string;
  name: string;
  description: string;
  mode: string;
  timeBudget: number;
  planningHorizon: number;
  tasks: Task[];
}

export default function App() {
  const [activeMode, setActiveMode] = useState<'MODE_A_MAX_VALUE_KNAPSACK' | 'MODE_B_INTERVAL_SCHEDULING' | 'MODE_C_PREREQUISITE_ORDERING'>('MODE_A_MAX_VALUE_KNAPSACK');
  const [timeBudget, setTimeBudget] = useState<number>(16);
  const [planningHorizon, setPlanningHorizon] = useState<number>(24);
  const [selectedAlgorithm, setSelectedAlgorithm] = useState<'DP' | 'GREEDY'>('DP');
  const [tasks, setTasks] = useState<Task[]>([]);
  const [scenarios, setScenarios] = useState<Scenario[]>([]);
  const [selectedScenarioId, setSelectedScenarioId] = useState<string>('scenario-mode-a-sprint');

  const [scheduleResult, setScheduleResult] = useState<ScheduleResult | null>(null);
  const [comparisonResult, setComparisonResult] = useState<ModeAComparison | null>(null);
  const [isComparing, setIsComparing] = useState<boolean>(false);
  const [loading, setLoading] = useState<boolean>(false);
  const [searchQuery, setSearchQuery] = useState<string>('');

  // Task Modal state
  const [isTaskModalOpen, setIsTaskModalOpen] = useState<boolean>(false);
  const [editingTask, setEditingTask] = useState<Task | null>(null);

  // Form Fields
  const [formId, setFormId] = useState<string>('');
  const [formTitle, setFormTitle] = useState<string>('');
  const [formDescription, setFormDescription] = useState<string>('');
  const [formDuration, setFormDuration] = useState<number>(4);
  const [formDeadline, setFormDeadline] = useState<string>('');
  const [formValue, setFormValue] = useState<number>(10);
  const [formPriority, setFormPriority] = useState<number>(1);
  const [formStartTime, setFormStartTime] = useState<string>('');
  const [formFinishTime, setFormFinishTime] = useState<string>('');
  const [formPrereqs, setFormPrereqs] = useState<string[]>([]);
  const [formError, setFormError] = useState<string>('');

  // Test Runner Drawer
  const [isTestModalOpen, setIsTestModalOpen] = useState<boolean>(false);
  const [testOutput, setTestOutput] = useState<string>('');
  const [testRunning, setTestRunning] = useState<boolean>(false);

  // Fetch initial scenarios on load
  useEffect(() => {
    loadScenarios();
  }, []);

  const loadScenarios = async () => {
    try {
      const res = await axios.get('/api/scenarios');
      if (Array.isArray(res.data) && res.data.length > 0) {
        setScenarios(res.data);
        loadScenarioById(res.data[0].id, res.data);
      }
    } catch (err) {
      console.error('Error loading scenarios:', err);
    }
  };

  const loadScenarioById = (id: string, scenarioList = scenarios) => {
    const sc = scenarioList.find((s) => s.id === id);
    if (!sc) return;
    setSelectedScenarioId(sc.id);
    setActiveMode(sc.mode as any);
    setTimeBudget(sc.timeBudget);
    setPlanningHorizon(sc.planningHorizon);
    setTasks(sc.tasks);
    setScheduleResult(null);
    setComparisonResult(null);
    setIsComparing(false);
  };

  // Generate Schedule
  const handleSchedule = async () => {
    setLoading(true);
    setIsComparing(false);
    try {
      const payload = {
        mode: activeMode,
        timeBudget,
        planningHorizonStart: 0,
        algorithm: selectedAlgorithm,
        scenarioId: selectedScenarioId,
        tasks
      };
      const res = await axios.post('/api/schedule', payload);
      setScheduleResult(res.data);
    } catch (err: any) {
      console.error('Scheduling error:', err);
      alert(err?.response?.data?.error || 'Schedule generation failed');
    } finally {
      setLoading(false);
    }
  };

  // Compare DP vs Greedy in Mode A
  const handleCompare = async () => {
    setLoading(true);
    setIsComparing(true);
    try {
      const payload = {
        timeBudget,
        tasks
      };
      const res = await axios.post('/api/compare', payload);
      setComparisonResult(res.data);
      setScheduleResult(res.data.dpResult);
    } catch (err: any) {
      console.error('Comparison error:', err);
      alert(err?.response?.data?.error || 'Comparison generation failed');
    } finally {
      setLoading(false);
    }
  };

  // Run C++ Test Suite
  const handleRunTests = async () => {
    setIsTestModalOpen(true);
    setTestRunning(true);
    setTestOutput('Running C++20 automated test suite (11 unit tests)...\n');
    try {
      const res = await axios.post('/api/tests/run');
      setTestOutput(res.data.output || 'Tests completed with no output.');
    } catch (err: any) {
      setTestOutput('Failed to execute test binary:\n' + (err?.message || err));
    } finally {
      setTestRunning(false);
    }
  };

  // Load a Deadlock Cycle Demo (Faculty Demo 4)
  const handleLoadCycleDemo = () => {
    const cycleTasks: Task[] = [
      { id: 'MOD-A', title: 'Database Migration Script', duration: 4, value: 10, priority: 1, prerequisiteIds: ['MOD-C'] },
      { id: 'MOD-B', title: 'Backend Authentication Service', duration: 3, value: 15, priority: 1, prerequisiteIds: ['MOD-A'] },
      { id: 'MOD-C', title: 'User Management API', duration: 5, value: 20, priority: 1, prerequisiteIds: ['MOD-B'] },
      { id: 'MOD-D', title: 'Frontend Login Interface', duration: 2, value: 5, priority: 2, prerequisiteIds: ['MOD-B'] }
    ];
    setActiveMode('MODE_C_PREREQUISITE_ORDERING');
    setTasks(cycleTasks);
    setScheduleResult(null);
    setComparisonResult(null);
    setIsComparing(false);
  };

  // Open modal to add or edit task
  const openTaskModal = (task?: Task) => {
    setFormError('');
    if (task) {
      setEditingTask(task);
      setFormId(task.id);
      setFormTitle(task.title);
      setFormDescription(task.description || '');
      setFormDuration(task.duration);
      setFormDeadline(task.deadline !== undefined && task.deadline !== null ? String(task.deadline) : '');
      setFormValue(task.value);
      setFormPriority(task.priority);
      setFormStartTime(task.startTime !== undefined && task.startTime !== null ? String(task.startTime) : '');
      setFormFinishTime(task.finishTime !== undefined && task.finishTime !== null ? String(task.finishTime) : '');
      setFormPrereqs(task.prerequisiteIds || []);
    } else {
      setEditingTask(null);
      const nextId = 'TASK-' + (tasks.length + 1);
      setFormId(nextId);
      setFormTitle('');
      setFormDescription('');
      setFormDuration(3);
      setFormDeadline('');
      setFormValue(10);
      setFormPriority(1);
      setFormStartTime('');
      setFormFinishTime('');
      setFormPrereqs([]);
    }
    setIsTaskModalOpen(true);
  };

  const handleSaveTaskForm = (e: React.FormEvent) => {
    e.preventDefault();
    setFormError('');

    if (!formTitle.trim()) {
      setFormError('Task title is required.');
      return;
    }

    if (formDuration <= 0) {
      setFormError('Task duration must be strictly positive (> 0).');
      return;
    }

    if (activeMode === 'MODE_B_INTERVAL_SCHEDULING') {
      const s = Number(formStartTime);
      const f = Number(formFinishTime);
      if (isNaN(s) || isNaN(f)) {
        setFormError('Fixed Start and Finish times are required for Mode B.');
        return;
      }
      if (s < 0 || f <= s) {
        setFormError('Start time must be >= 0 and Finish time must be strictly greater than Start.');
        return;
      }
    }

    const newTask: Task = {
      id: formId.trim() || 'TASK-' + Date.now(),
      title: formTitle.trim(),
      description: formDescription.trim(),
      duration: Number(formDuration),
      value: Number(formValue),
      priority: Number(formPriority),
      deadline: formDeadline.trim() !== '' ? Number(formDeadline) : null,
      startTime: formStartTime.trim() !== '' ? Number(formStartTime) : null,
      finishTime: formFinishTime.trim() !== '' ? Number(formFinishTime) : null,
      prerequisiteIds: formPrereqs,
    };

    if (editingTask) {
      setTasks(tasks.map((t) => (t.id === editingTask.id ? newTask : t)));
    } else {
      setTasks([...tasks, newTask]);
    }
    setIsTaskModalOpen(false);
  };

  const handleDeleteTask = (id: string) => {
    setTasks(tasks.filter((t) => t.id !== id));
    // Also remove from any prerequisite lists
    setTasks((prev) =>
      prev.map((t) => ({
        ...t,
        prerequisiteIds: t.prerequisiteIds.filter((pid) => pid !== id)
      }))
    );
  };

  // Filtered task table
  const filteredTasks = useMemo(() => {
    if (!searchQuery.trim()) return tasks;
    const q = searchQuery.toLowerCase();
    return tasks.filter(
      (t) =>
        t.title.toLowerCase().includes(q) ||
        t.id.toLowerCase().includes(q) ||
        (t.description && t.description.toLowerCase().includes(q))
    );
  }, [tasks, searchQuery]);

  // Dynamic axis ticks for timeline
  const axisTicks = useMemo(() => {
    const maxVal = Math.max(timeBudget, planningHorizon, 20);
    const step = maxVal <= 24 ? 2 : maxVal <= 60 ? 5 : 10;
    const ticks: number[] = [];
    for (let i = 0; i <= maxVal; i += step) {
      ticks.push(i);
    }
    return { ticks, maxVal };
  }, [timeBudget, planningHorizon]);

  return (
    <div>
      {/* 1. Header */}
      <header className="app-header">
        <div className="brand-section">
          <div className="brand-icon">
            <Cpu size={22} />
          </div>
          <div className="brand-text">
            <h1>
              INTELLISCHEDULE
              <span className="badge-cpp">C++20 NATIVE CORE</span>
            </h1>
            <span>Intelligent Task Scheduling &amp; Resource Optimization Platform</span>
          </div>
        </div>

        <div className="header-actions">
          <div className="scenario-selector">
            <label>Scenario:</label>
            <select
              className="scenario-select"
              value={selectedScenarioId}
              onChange={(e) => loadScenarioById(e.target.value)}
            >
              {scenarios.map((sc) => (
                <option key={sc.id} value={sc.id}>
                  {sc.name}
                </option>
              ))}
            </select>
          </div>

          <button className="btn btn-secondary btn-sm" onClick={handleLoadCycleDemo} title="Load a circular dependency scenario">
            <AlertTriangle size={14} color="#F59E0B" /> Cycle Deadlock Demo
          </button>

          <button className="btn btn-test btn-sm" onClick={handleRunTests} title="Run backend unit tests">
            <Terminal size={14} /> Run C++ Test Suite
          </button>
        </div>
      </header>

      {/* 2. Mode Navigation Tabs */}
      <div className="mode-nav-bar">
        <div className="mode-tabs">
          <button
            className={`mode-tab ${activeMode === 'MODE_A_MAX_VALUE_KNAPSACK' ? 'active' : ''}`}
            onClick={() => {
              setActiveMode('MODE_A_MAX_VALUE_KNAPSACK');
              setScheduleResult(null);
            }}
          >
            <Layers size={16} /> Mode A: Max-Value Selection
            <span className="mode-pill">0/1 KNAPSACK DP</span>
          </button>

          <button
            className={`mode-tab ${activeMode === 'MODE_B_INTERVAL_SCHEDULING' ? 'active' : ''}`}
            onClick={() => {
              setActiveMode('MODE_B_INTERVAL_SCHEDULING');
              setScheduleResult(null);
            }}
          >
            <Calendar size={16} /> Mode B: Interval Scheduling
            <span className="mode-pill">GREEDY EFT</span>
          </button>

          <button
            className={`mode-tab ${activeMode === 'MODE_C_PREREQUISITE_ORDERING' ? 'active' : ''}`}
            onClick={() => {
              setActiveMode('MODE_C_PREREQUISITE_ORDERING');
              setScheduleResult(null);
            }}
          >
            <GitBranch size={16} /> Mode C: Prerequisite Ordering
            <span className="mode-pill">DAG &amp; CYCLES</span>
          </button>
        </div>

        <div style={{ display: 'flex', alignItems: 'center', gap: '8px' }}>
          <span style={{ fontSize: '11px', color: 'var(--text-muted)' }}>SQLite Persistence Active</span>
          <div style={{ width: '8px', height: '8px', borderRadius: '50%', background: 'var(--emerald-500)' }} />
        </div>
      </div>

      {/* 3. Assumptions & Problem Formulation Banner */}
      <div className="assumptions-banner">
        <div>
          {activeMode === 'MODE_A_MAX_VALUE_KNAPSACK' && (
            <>
              <strong>FORMAL DEFINITION (MODE A):</strong> Given independent tasks with duration <em>d<sub>i</sub> &gt; 0</em> and value <em>v<sub>i</sub> &ge; 0</em>, maximize &sum; <em>v<sub>i</sub></em> subject to &sum; <em>d<sub>i</sub> &le; W</em>.
              <span style={{ marginLeft: '12px', color: '#CBD5E1' }}>
                Note: Greedy ratio heuristic is <em>not guaranteed optimal</em> for general 0/1 knapsack.
              </span>
            </>
          )}

          {activeMode === 'MODE_B_INTERVAL_SCHEDULING' && (
            <>
              <strong>FORMAL DEFINITION (MODE B):</strong> Given fixed intervals <em>[s<sub>i</sub>, f<sub>i</sub>)</em> on one resource, select maximum cardinality subset <em>S</em> such that no two intervals overlap.
              <span style={{ marginLeft: '12px', color: '#CBD5E1' }}>
                Earliest Finish Time (EFT) greedy strategy is proven optimal via "greedy stays ahead".
              </span>
            </>
          )}

          {activeMode === 'MODE_C_PREREQUISITE_ORDERING' && (
            <>
              <strong>FORMAL DEFINITION (MODE C):</strong> Given directed graph <em>G=(V, E)</em> of dependencies <em>(u &rarr; v)</em>, detect cycles via DFS 3-coloring. If acyclic, produce topological schedule and critical path.
            </>
          )}
        </div>

        <div className="complexity-tag">
          {activeMode === 'MODE_A_MAX_VALUE_KNAPSACK' && 'Time: O(n·W) | Space: O(n·W)'}
          {activeMode === 'MODE_B_INTERVAL_SCHEDULING' && 'Time: O(n log n) | Space: O(n)'}
          {activeMode === 'MODE_C_PREREQUISITE_ORDERING' && 'Time: O(V + E) | Space: O(V + E)'}
        </div>
      </div>

      {/* 4. Workspace */}
      <main className="workspace">
        {/* Control Strip */}
        <section className="control-strip">
          <div className="controls-left">
            {activeMode === 'MODE_A_MAX_VALUE_KNAPSACK' && (
              <>
                <div className="control-item">
                  <label htmlFor="time-budget-input">Time Budget (W):</label>
                  <input
                    id="time-budget-input"
                    type="number"
                    min="1"
                    className="control-input"
                    value={timeBudget}
                    onChange={(e) => setTimeBudget(Math.max(1, Number(e.target.value)))}
                  />
                </div>

                <div className="control-item">
                  <label htmlFor="algorithm-select">Algorithm:</label>
                  <select
                    id="algorithm-select"
                    className="control-input"
                    style={{ width: '160px' }}
                    value={selectedAlgorithm}
                    onChange={(e) => setSelectedAlgorithm(e.target.value as any)}
                  >
                    <option value="DP">0/1 Knapsack DP (Optimal)</option>
                    <option value="GREEDY">Greedy Ratio Heuristic</option>
                  </select>
                </div>
              </>
            )}

            {(activeMode === 'MODE_B_INTERVAL_SCHEDULING' || activeMode === 'MODE_C_PREREQUISITE_ORDERING') && (
              <div className="control-item">
                <label htmlFor="planning-horizon-input">Planning Horizon:</label>
                <input
                  id="planning-horizon-input"
                  type="number"
                  min="5"
                  className="control-input"
                  value={planningHorizon}
                  onChange={(e) => setPlanningHorizon(Math.max(5, Number(e.target.value)))}
                />
              </div>
            )}

            <div className="control-item">
              <span style={{ fontSize: '12px', color: 'var(--text-muted)' }}>
                Active Tasks: <b>{tasks.length}</b>
              </span>
            </div>
          </div>

          <div style={{ display: 'flex', gap: '10px' }}>
            <button className="btn btn-secondary" onClick={() => openTaskModal()}>
              <Plus size={15} /> Add Task
            </button>

            {activeMode === 'MODE_A_MAX_VALUE_KNAPSACK' && (
              <button className="btn btn-compare" onClick={handleCompare} disabled={loading || tasks.length === 0}>
                <BarChart2 size={15} /> Compare DP vs Greedy
              </button>
            )}

            <button className="btn btn-primary" onClick={handleSchedule} disabled={loading || tasks.length === 0}>
              <Play size={15} fill="currentColor" /> {loading ? 'Computing...' : 'Generate Schedule'}
            </button>
          </div>
        </section>

        {/* Schedule Invariant Validator Alert */}
        {scheduleResult && (
          <div className={`validator-card ${scheduleResult.isValidSchedule ? 'pass' : 'fail'}`}>
            <div style={{ display: 'flex', alignItems: 'center', gap: '10px' }}>
              {scheduleResult.isValidSchedule ? <ShieldCheck size={24} /> : <XCircle size={24} />}
              <div>
                <b style={{ fontSize: '13.5px' }}>
                  {scheduleResult.isValidSchedule
                    ? 'INDEPENDENT SCHEDULE VALIDATOR: ALL INVARIANTS SATISFIED'
                    : 'INDEPENDENT SCHEDULE VALIDATOR: SCHEDULE INVARIANTS VIOLATED'}
                </b>
                <span style={{ display: 'block', fontSize: '12px', opacity: 0.85 }}>
                  {scheduleResult.isValidSchedule
                    ? 'Checked non-negative durations, duplicate selections, interval non-overlap, and prerequisite precedence.'
                    : 'The generated schedule contains mathematical or constraint violations. See breakdown below.'}
                </span>
              </div>
            </div>

            <div style={{ textAlign: 'right', fontFamily: 'var(--font-mono)', fontSize: '12px' }}>
              Engine Runtime: <b>{scheduleResult.executionTimeMs.toFixed(3)} ms</b>
            </div>
          </div>
        )}

        {/* Cycle Alert Banner if cycle detected in Mode C */}
        {scheduleResult && scheduleResult.detectedCycle && scheduleResult.detectedCycle.length > 0 && (
          <div className="cycle-alert-banner">
            <div style={{ display: 'flex', alignItems: 'center', gap: '10px' }}>
              <XCircle size={24} color="#FDA4AF" />
              <div>
                <h3 style={{ fontSize: '15px', fontWeight: 800 }}>CIRCULAR DEPENDENCY DEADLOCK DETECTED</h3>
                <p style={{ fontSize: '12.5px', marginTop: '2px' }}>
                  A directed cycle was identified by DFS 3-coloring. Tasks in a dependency cycle can never be scheduled without deadlock.
                </p>
              </div>
            </div>

            <div className="cycle-path-trace">
              CYCLE PATH: {scheduleResult.detectedCycle.join(' ➔ ')}
            </div>

            <span style={{ fontSize: '11.5px', opacity: 0.9 }}>
              Remedy: Remove or reverse one of the dependency edges in the cycle to restore a Directed Acyclic Graph (DAG).
            </span>
          </div>
        )}

        {/* Validation Violations List */}
        {scheduleResult && scheduleResult.validationViolations && scheduleResult.validationViolations.length > 0 && (
          <div className="panel" style={{ borderColor: 'var(--rose-500)', background: 'rgba(225, 29, 72, 0.05)' }}>
            <div className="panel-title" style={{ color: 'var(--rose-500)' }}>
              <AlertTriangle size={18} /> Invariant Audit Failures ({scheduleResult.validationViolations.length})
            </div>
            <div className="violations-list">
              {scheduleResult.validationViolations.map((v, i) => (
                <div key={i} className="violation-item">
                  <b>[{v.rule}]</b> {v.taskId ? `Task '${v.taskId}': ` : ''}{v.message}
                </div>
              ))}
            </div>
          </div>
        )}

        {/* Metrics Grid */}
        {scheduleResult && (
          <div className="metrics-grid">
            <div className="metric-card">
              <small>Total Scheduled Value</small>
              <div className="metric-value">{scheduleResult.totalValue.toFixed(1)}</div>
              <div className="metric-sub">Aggregated priority utility</div>
            </div>

            <div className="metric-card">
              <small>Time Utilized</small>
              <div className="metric-value">
                {scheduleResult.totalDuration}
                {activeMode === 'MODE_A_MAX_VALUE_KNAPSACK' ? ` / ${timeBudget}` : ''}
              </div>
              <div className="metric-sub">
                {activeMode === 'MODE_A_MAX_VALUE_KNAPSACK'
                  ? `${((scheduleResult.totalDuration / timeBudget) * 100).toFixed(0)}% budget utilization`
                  : 'Earliest span completion'}
              </div>
            </div>

            <div className="metric-card">
              <small>Scheduled Tasks</small>
              <div className="metric-value">
                {scheduleResult.scheduledCount} / {tasks.length}
              </div>
              <div className="metric-sub">{scheduleResult.unscheduledCount} tasks excluded</div>
            </div>

            <div className="metric-card">
              <small>Algorithm Mode</small>
              <div className="metric-value" style={{ fontSize: '16px', lineHeight: 1.5 }}>
                {scheduleResult.algorithmName}
              </div>
              <div className="metric-sub">{scheduleResult.executionTimeMs.toFixed(3)} ms CPU time</div>
            </div>
          </div>
        )}

        {/* Mode A Comparison Panel */}
        {isComparing && comparisonResult && (
          <section className="panel" style={{ borderColor: 'rgba(99, 102, 241, 0.4)' }}>
            <div className="panel-header">
              <div className="panel-title">
                <BarChart2 size={18} color="var(--indigo-500)" />
                0/1 Knapsack: Dynamic Programming vs. Greedy Ratio Heuristic
              </div>
              <span className="badge-cpp">ALGORITHM COMPARISON MATRIX</span>
            </div>

            <div className="comparison-container">
              <div className="comp-box optimal">
                <div className="comp-header">
                  <div className="comp-title" style={{ color: 'var(--emerald-400)' }}>
                    0/1 Knapsack Dynamic Programming
                  </div>
                  <span className="badge-cpp" style={{ color: 'var(--emerald-400)', borderColor: 'var(--emerald-500)' }}>
                    GUARANTEED OPTIMAL
                  </span>
                </div>

                <div className="comp-stat-row">
                  <span>Total Value Achieved</span>
                  <b>{comparisonResult.dpResult.totalValue.toFixed(1)}</b>
                </div>
                <div className="comp-stat-row">
                  <span>Duration Utilized</span>
                  <b>{comparisonResult.dpResult.totalDuration} / {timeBudget}</b>
                </div>
                <div className="comp-stat-row">
                  <span>Tasks Selected</span>
                  <b>{comparisonResult.dpResult.scheduledCount} tasks</b>
                </div>
                <div className="comp-stat-row">
                  <span>Time Complexity</span>
                  <b>O(n · W)</b>
                </div>
                <div className="comp-stat-row">
                  <span>Execution Time</span>
                  <b>{comparisonResult.dpResult.executionTimeMs.toFixed(3)} ms</b>
                </div>
                <div className="comp-stat-row">
                  <span>Selected Task IDs</span>
                  <b style={{ color: 'var(--emerald-400)' }}>
                    {comparisonResult.dpResult.scheduledTasks.map((t) => t.id).join(', ')}
                  </b>
                </div>
              </div>

              <div className="comp-box heuristic">
                <div className="comp-header">
                  <div className="comp-title" style={{ color: 'var(--indigo-400)' }}>
                    Greedy Value-per-Duration Heuristic
                  </div>
                  <span className="badge-cpp" style={{ color: 'var(--indigo-400)', borderColor: 'var(--indigo-500)' }}>
                    HEURISTIC APPROXIMATION
                  </span>
                </div>

                <div className="comp-stat-row">
                  <span>Total Value Achieved</span>
                  <b>{comparisonResult.greedyResult.totalValue.toFixed(1)}</b>
                </div>
                <div className="comp-stat-row">
                  <span>Duration Utilized</span>
                  <b>{comparisonResult.greedyResult.totalDuration} / {timeBudget}</b>
                </div>
                <div className="comp-stat-row">
                  <span>Tasks Selected</span>
                  <b>{comparisonResult.greedyResult.scheduledCount} tasks</b>
                </div>
                <div className="comp-stat-row">
                  <span>Time Complexity</span>
                  <b>O(n log n)</b>
                </div>
                <div className="comp-stat-row">
                  <span>Execution Time</span>
                  <b>{comparisonResult.greedyResult.executionTimeMs.toFixed(3)} ms</b>
                </div>
                <div className="comp-stat-row">
                  <span>Selected Task IDs</span>
                  <b style={{ color: 'var(--indigo-400)' }}>
                    {comparisonResult.greedyResult.scheduledTasks.map((t) => t.id).join(', ')}
                  </b>
                </div>
              </div>

              <div className="conclusion-card">
                <div style={{ display: 'flex', alignItems: 'center', gap: '8px', marginBottom: '6px' }}>
                  <Award size={18} color="var(--amber-500)" />
                  <b style={{ color: '#FFFFFF', fontSize: '14px' }}>Mathematical Performance Analysis</b>
                </div>
                <p>{comparisonResult.conclusion}</p>
                <div style={{ marginTop: '8px', fontSize: '12px', color: 'var(--text-secondary)' }}>
                  Value Difference: <b>{comparisonResult.valueDifference.toFixed(1)} points</b> &bull; Efficiency Gap: <b>{comparisonResult.efficiencyGapPercent.toFixed(1)}%</b>
                </div>
              </div>
            </div>
          </section>
        )}

        {/* 5. Timeline / Gantt Visualization */}
        <section className="panel">
          <div className="panel-header">
            <div className="panel-title">
              <Calendar size={18} /> Schedule Timeline &amp; Gantt Visualizer
            </div>
            <div style={{ display: 'flex', gap: '14px', fontSize: '11px', color: 'var(--text-secondary)' }}>
              <span style={{ display: 'flex', alignItems: 'center', gap: '4px' }}>
                <span style={{ width: '10px', height: '10px', background: '#10B981', borderRadius: '2px', display: 'inline-block' }} /> Priority 1
              </span>
              <span style={{ display: 'flex', alignItems: 'center', gap: '4px' }}>
                <span style={{ width: '10px', height: '10px', background: '#3B82F6', borderRadius: '2px', display: 'inline-block' }} /> Priority 2
              </span>
              <span style={{ display: 'flex', alignItems: 'center', gap: '4px' }}>
                <span style={{ width: '10px', height: '10px', background: '#64748B', borderRadius: '2px', display: 'inline-block' }} /> Priority 3
              </span>
            </div>
          </div>

          <div className="timeline-container">
            {/* Horizontal Axis */}
            <div className="timeline-axis">
              {axisTicks.ticks.map((tick) => (
                <div key={tick} className="axis-tick">
                  t={tick}
                </div>
              ))}
            </div>

            {/* Timeline Task Bars Track */}
            <div className="timeline-bars-track">
              {scheduleResult && scheduleResult.scheduledTasks.length > 0 ? (
                scheduleResult.scheduledTasks.map((task) => {
                  const start = task.scheduledStart || 0;
                  const finish = task.scheduledFinish || start + task.duration;
                  const leftPercent = (start / axisTicks.maxVal) * 100;
                  const widthPercent = ((finish - start) / axisTicks.maxVal) * 100;
                  const priClass = task.priority === 1 ? 'bar-priority-1' : task.priority === 2 ? 'bar-priority-2' : 'bar-priority-3';

                  return (
                    <div key={task.id} className="timeline-task-bar">
                      <div
                        className={`task-bar-block ${priClass}`}
                        style={{
                          left: `${Math.min(99, Math.max(0, leftPercent))}%`,
                          width: `${Math.max(3, widthPercent)}%`,
                        }}
                        title={`${task.id}: ${task.title} [t=${start} to t=${finish}], Value: ${task.value}`}
                      >
                        {task.id} • {task.title} (dur: {task.duration}, val: {task.value})
                      </div>
                    </div>
                  );
                })
              ) : (
                <div style={{ textAlign: 'center', padding: '40px 0', color: 'var(--text-muted)', fontSize: '13px' }}>
                  No active schedule generated yet. Click <b>"Generate Schedule"</b> above to run the C++ optimization engine.
                </div>
              )}
            </div>
          </div>
        </section>

        {/* 6. Mode C: Dependency Graph & Critical Path View */}
        {activeMode === 'MODE_C_PREREQUISITE_ORDERING' && scheduleResult && (
          <section className="panel">
            <div className="panel-header">
              <div className="panel-title">
                <GitBranch size={18} /> Prerequisite Directed Graph &amp; Critical Path
              </div>
              {scheduleResult.criticalPath && scheduleResult.criticalPath.length > 0 && (
                <span className="badge-cpp" style={{ color: 'var(--amber-500)', borderColor: 'var(--amber-500)' }}>
                  CRITICAL PATH: {scheduleResult.criticalPath.join(' ➔ ')}
                </span>
              )}
            </div>

            <p style={{ fontSize: '12px', color: 'var(--text-secondary)', marginBottom: '14px' }}>
              Topological Ordering generated via Kahn's in-degree zero queue. Amber borders denote tasks lying on the project Critical Path (determining overall duration).
            </p>

            <div className="dag-nodes-grid">
              {(scheduleResult.topologicalOrder.length > 0 ? scheduleResult.topologicalOrder : tasks.map((t) => t.id)).map((id, index) => {
                const task = tasks.find((t) => t.id === id);
                if (!task) return null;
                const isOnCriticalPath = scheduleResult.criticalPath && scheduleResult.criticalPath.includes(id);
                const isInCycle = scheduleResult.detectedCycle && scheduleResult.detectedCycle.includes(id);

                return (
                  <div
                    key={id}
                    className={`dag-node-card ${isOnCriticalPath ? 'on-critical-path' : ''} ${isInCycle ? 'in-cycle' : ''}`}
                  >
                    <div className="dag-node-header">
                      <span className="dag-node-id">{id}</span>
                      <span style={{ fontSize: '10px', color: 'var(--text-muted)' }}>
                        Step #{index + 1}
                      </span>
                    </div>
                    <div className="dag-node-title">{task.title}</div>
                    <div className="dag-node-meta">
                      <span>Duration: {task.duration}h</span>
                      <span>Priority: P{task.priority}</span>
                    </div>
                    {task.prerequisiteIds && task.prerequisiteIds.length > 0 && (
                      <div style={{ marginTop: '6px', fontSize: '10.5px', color: 'var(--text-muted)' }}>
                        Prereqs: {task.prerequisiteIds.join(', ')}
                      </div>
                    )}
                  </div>
                );
              })}
            </div>
          </section>
        )}

        {/* 7. Unscheduled & Excluded Tasks Panel */}
        {scheduleResult && scheduleResult.unscheduledTasks && scheduleResult.unscheduledTasks.length > 0 && (
          <section className="panel" style={{ borderColor: 'rgba(245, 158, 11, 0.3)' }}>
            <div className="panel-header">
              <div className="panel-title" style={{ color: 'var(--amber-500)' }}>
                <AlertTriangle size={18} /> Unscheduled / Excluded Tasks ({scheduleResult.unscheduledTasks.length})
              </div>
              <span style={{ fontSize: '12px', color: 'var(--text-muted)' }}>
                Transparent Infeasibility &amp; Trade-off Reporting
              </span>
            </div>

            <div className="table-responsive">
              <table className="task-table">
                <thead>
                  <tr>
                    <th>Task ID</th>
                    <th>Title</th>
                    <th>Duration</th>
                    <th>Value</th>
                    <th>Exclusion Rationale</th>
                  </tr>
                </thead>
                <tbody>
                  {scheduleResult.unscheduledTasks.map((t) => (
                    <tr key={t.id} className="unscheduled">
                      <td style={{ fontFamily: 'var(--font-mono)', fontWeight: 700 }}>{t.id}</td>
                      <td>{t.title}</td>
                      <td>{t.duration}h</td>
                      <td>{t.value}</td>
                      <td style={{ color: '#FCD34D', fontSize: '12px' }}>
                        {t.exclusionReason || 'Excluded by optimization criteria'}
                      </td>
                    </tr>
                  ))}
                </tbody>
              </table>
            </div>
          </section>
        )}

        {/* 8. Task Management Table */}
        <section className="panel">
          <div className="panel-header">
            <div className="panel-title">
              <Layers size={18} /> Task Repository ({tasks.length} tasks)
            </div>
            <div style={{ display: 'flex', gap: '10px' }}>
              <input
                type="text"
                placeholder="Search tasks..."
                value={searchQuery}
                onChange={(e) => setSearchQuery(e.target.value)}
                style={{
                  background: 'var(--bg-surface-elevated)',
                  border: '1px solid var(--border-strong)',
                  borderRadius: '6px',
                  padding: '4px 10px',
                  color: '#FFFFFF',
                  fontSize: '12px',
                  width: '180px'
                }}
              />
              <button className="btn btn-primary btn-sm" onClick={() => openTaskModal()}>
                <Plus size={14} /> Add Task
              </button>
            </div>
          </div>

          <div className="table-responsive">
            <table className="task-table">
              <thead>
                <tr>
                  <th>ID</th>
                  <th>Title &amp; Description</th>
                  <th>Duration</th>
                  <th>Value</th>
                  <th>Ratio (v/d)</th>
                  <th>Priority</th>
                  {activeMode === 'MODE_B_INTERVAL_SCHEDULING' && <th>Interval [s, f)</th>}
                  {activeMode === 'MODE_C_PREREQUISITE_ORDERING' && <th>Prerequisites</th>}
                  <th>Deadline</th>
                  <th>Actions</th>
                </tr>
              </thead>
              <tbody>
                {filteredTasks.map((t) => {
                  const ratio = t.duration > 0 ? (t.value / t.duration).toFixed(2) : '0';
                  return (
                    <tr key={t.id}>
                      <td style={{ fontFamily: 'var(--font-mono)', fontWeight: 700, color: 'var(--emerald-400)' }}>
                        {t.id}
                      </td>
                      <td>
                        <b style={{ display: 'block' }}>{t.title}</b>
                        {t.description && (
                          <small style={{ color: 'var(--text-muted)', fontSize: '11px' }}>
                            {t.description}
                          </small>
                        )}
                      </td>
                      <td>{t.duration}h</td>
                      <td style={{ fontFamily: 'var(--font-mono)' }}>{t.value}</td>
                      <td style={{ fontFamily: 'var(--font-mono)', color: 'var(--indigo-400)' }}>{ratio}</td>
                      <td>
                        <span
                          style={{
                            fontSize: '11px',
                            fontWeight: 700,
                            padding: '2px 6px',
                            borderRadius: '4px',
                            background: t.priority === 1 ? 'rgba(16, 185, 129, 0.15)' : t.priority === 2 ? 'rgba(59, 130, 246, 0.15)' : 'rgba(148, 163, 184, 0.15)',
                            color: t.priority === 1 ? '#34D399' : t.priority === 2 ? '#60A5FA' : '#94A3B8'
                          }}
                        >
                          P{t.priority}
                        </span>
                      </td>
                      {activeMode === 'MODE_B_INTERVAL_SCHEDULING' && (
                        <td style={{ fontFamily: 'var(--font-mono)' }}>
                          {t.startTime !== null && t.finishTime !== null ? `[${t.startTime}, ${t.finishTime})` : '—'}
                        </td>
                      )}
                      {activeMode === 'MODE_C_PREREQUISITE_ORDERING' && (
                        <td>
                          {t.prerequisiteIds && t.prerequisiteIds.length > 0 ? (
                            <span style={{ fontFamily: 'var(--font-mono)', fontSize: '11px' }}>
                              {t.prerequisiteIds.join(', ')}
                            </span>
                          ) : (
                            <span style={{ color: 'var(--text-muted)' }}>None</span>
                          )}
                        </td>
                      )}
                      <td>{t.deadline !== null && t.deadline !== undefined ? `t=${t.deadline}` : '—'}</td>
                      <td>
                        <div style={{ display: 'flex', gap: '6px' }}>
                          <button
                            className="btn btn-secondary btn-sm"
                            style={{ padding: '4px 6px' }}
                            onClick={() => openTaskModal(t)}
                            title="Edit task"
                          >
                            <Edit2 size={13} />
                          </button>
                          <button
                            className="btn btn-secondary btn-sm"
                            style={{ padding: '4px 6px', color: '#FDA4AF' }}
                            onClick={() => handleDeleteTask(t.id)}
                            title="Delete task"
                          >
                            <Trash2 size={13} />
                          </button>
                        </div>
                      </td>
                    </tr>
                  );
                })}
              </tbody>
            </table>
          </div>
        </section>
      </main>

      {/* 9. Task Create/Edit Modal */}
      {isTaskModalOpen && (
        <div className="modal-overlay" onClick={() => setIsTaskModalOpen(false)}>
          <div className="modal-content" onClick={(e) => e.stopPropagation()}>
            <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', marginBottom: '16px' }}>
              <h3 style={{ fontSize: '16px', fontWeight: 800 }}>
                {editingTask ? 'Edit Task' : 'Create New Task'}
              </h3>
              <button
                style={{ background: 'none', border: 'none', color: 'var(--text-muted)', cursor: 'pointer' }}
                onClick={() => setIsTaskModalOpen(false)}
              >
                ✕
              </button>
            </div>

            {formError && (
              <div style={{ background: 'rgba(225, 29, 72, 0.2)', border: '1px solid var(--rose-500)', padding: '10px', borderRadius: '6px', color: '#FDA4AF', fontSize: '12px', marginBottom: '14px' }}>
                {formError}
              </div>
            )}

            <form onSubmit={handleSaveTaskForm}>
              <div className="form-row">
                <div className="form-group">
                  <label htmlFor="form-task-id">Task ID</label>
                  <input
                    id="form-task-id"
                    className="form-control"
                    value={formId}
                    onChange={(e) => setFormId(e.target.value)}
                    required
                  />
                </div>
                <div className="form-group">
                  <label htmlFor="form-priority">Priority</label>
                  <select
                    id="form-priority"
                    className="form-control"
                    value={formPriority}
                    onChange={(e) => setFormPriority(Number(e.target.value))}
                  >
                    <option value={1}>High (P1)</option>
                    <option value={2}>Medium (P2)</option>
                    <option value={3}>Low (P3)</option>
                  </select>
                </div>
              </div>

              <div className="form-group">
                <label htmlFor="form-title">Task Title</label>
                <input
                  id="form-title"
                  className="form-control"
                  placeholder="e.g. Database Index Optimization"
                  value={formTitle}
                  onChange={(e) => setFormTitle(e.target.value)}
                  required
                />
              </div>

              <div className="form-group">
                <label htmlFor="form-description">Description</label>
                <textarea
                  id="form-description"
                  className="form-control"
                  rows={2}
                  placeholder="Task details and scope..."
                  value={formDescription}
                  onChange={(e) => setFormDescription(e.target.value)}
                />
              </div>

              <div className="form-row">
                <div className="form-group">
                  <label htmlFor="form-duration">Duration (positive integer)</label>
                  <input
                    id="form-duration"
                    type="number"
                    min="1"
                    className="form-control"
                    value={formDuration}
                    onChange={(e) => setFormDuration(Number(e.target.value))}
                    required
                  />
                </div>
                <div className="form-group">
                  <label htmlFor="form-value">Utility Value (non-negative)</label>
                  <input
                    id="form-value"
                    type="number"
                    min="0"
                    step="0.5"
                    className="form-control"
                    value={formValue}
                    onChange={(e) => setFormValue(Number(e.target.value))}
                    required
                  />
                </div>
              </div>

              {activeMode === 'MODE_B_INTERVAL_SCHEDULING' && (
                <div className="form-row">
                  <div className="form-group">
                    <label htmlFor="form-start-time">Fixed Start Time (s)</label>
                    <input
                      id="form-start-time"
                      type="number"
                      min="0"
                      className="form-control"
                      value={formStartTime}
                      onChange={(e) => setFormStartTime(e.target.value)}
                      required
                    />
                  </div>
                  <div className="form-group">
                    <label htmlFor="form-finish-time">Fixed Finish Time (f &gt; s)</label>
                    <input
                      id="form-finish-time"
                      type="number"
                      min="1"
                      className="form-control"
                      value={formFinishTime}
                      onChange={(e) => setFormFinishTime(e.target.value)}
                      required
                    />
                  </div>
                </div>
              )}

              <div className="form-row">
                <div className="form-group">
                  <label htmlFor="form-deadline">Deadline (Optional)</label>
                  <input
                    id="form-deadline"
                    type="number"
                    min="1"
                    placeholder="e.g. 24"
                    className="form-control"
                    value={formDeadline}
                    onChange={(e) => setFormDeadline(e.target.value)}
                  />
                </div>

                {activeMode === 'MODE_C_PREREQUISITE_ORDERING' && (
                  <div className="form-group">
                    <label htmlFor="form-prereqs">Prerequisite Task IDs (comma-separated)</label>
                    <input
                      id="form-prereqs"
                      className="form-control"
                      placeholder="e.g. PIPE-01, PIPE-02"
                      value={formPrereqs.join(', ')}
                      onChange={(e) =>
                        setFormPrereqs(
                          e.target.value
                            .split(',')
                            .map((s) => s.trim())
                            .filter(Boolean)
                        )
                      }
                    />
                  </div>
                )}
              </div>

              <div style={{ display: 'flex', justifyContent: 'flex-end', gap: '10px', marginTop: '18px' }}>
                <button type="button" className="btn btn-secondary" onClick={() => setIsTaskModalOpen(false)}>
                  Cancel
                </button>
                <button type="submit" className="btn btn-primary">
                  {editingTask ? 'Save Changes' : 'Create Task'}
                </button>
              </div>
            </form>
          </div>
        </div>
      )}

      {/* 10. Test Runner Live Modal */}
      {isTestModalOpen && (
        <div className="modal-overlay" onClick={() => setIsTestModalOpen(false)}>
          <div className="modal-content" style={{ maxWidth: '720px' }} onClick={(e) => e.stopPropagation()}>
            <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', marginBottom: '14px' }}>
              <div style={{ display: 'flex', alignItems: 'center', gap: '8px' }}>
                <Terminal size={18} color="var(--emerald-400)" />
                <h3 style={{ fontSize: '15px', fontWeight: 800 }}>
                  C++20 Automated Unit Test Verification Suite
                </h3>
              </div>
              <button
                style={{ background: 'none', border: 'none', color: 'var(--text-muted)', cursor: 'pointer' }}
                onClick={() => setIsTestModalOpen(false)}
              >
                ✕
              </button>
            </div>

            <div className="test-terminal-modal">
              {testOutput}
            </div>

            <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', marginTop: '14px' }}>
              <span style={{ fontSize: '11px', color: 'var(--text-secondary)' }}>
                Target binary: <code>intellischedule_tests</code> &bull; Built via CMake C++20
              </span>
              <button className="btn btn-secondary btn-sm" onClick={() => setIsTestModalOpen(false)}>
                Close Output
              </button>
            </div>
          </div>
        </div>
      )}

      {/* Footer */}
      <footer style={{ borderTop: '1px solid var(--border-subtle)', padding: '24px 28px', marginTop: '40px', fontSize: '12px', color: 'var(--text-muted)', display: 'flex', justifyContent: 'space-between', flexWrap: 'wrap', gap: '12px' }}>
        <div>
          <b>INTELLISCHEDULE</b> &bull; Advanced Data Structures and Algorithms Analysis (ADSAA) Mini-Project
        </div>
        <div>
          C++20 Native Engine &bull; cpp-httplib &bull; nlohmann/json &bull; SQLite3 Persistence
        </div>
      </footer>
    </div>
  );
}

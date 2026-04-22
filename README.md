# TaskSched

A real-time task scheduling simulator written in C++.

## Overview

TaskSched models the logic of an RTOS scheduler. A task set (periodic, aperiodic, sporadic) is defined in `main.cpp`, a scheduling algorithm is selected at runtime, and the simulator runs a deterministic discrete-time loop. It is a study tool for comparing scheduling policies on identical workloads.

## Features

**Task hierarchy.** Abstract `Task` base with three concrete types: `PeriodicTask`, `AperiodicTask`, and `SporadicTask`. The latter uses diamond inheritance with a virtual shared base.

**Scheduler.** Single-core, preemptive, discrete-time simulation loop covering task release, preemption, dispatch, and completion. Soft real-time model: a missed task continues executing and is tracked in statistics without being terminated.

**Scheduling policies (Strategy pattern).** The algorithm is decoupled from the scheduler via an abstract `SchedulingPolicy` base:

- `PriorityPolicy` — fixed priority assigned manually per task
- `RateMonotonicPolicy` — fixed priority derived from period (shorter period = higher priority)
- `DeadlineMonotonicPolicy` — fixed priority derived from relative deadline (shorter deadline = higher priority)
- `EDFPolicy` — dynamic priority, earliest absolute deadline first

**Statistics & export.** Per-tick CPU utilization, deadline miss counters, per-task response times, preemption counts. State snapshots exported to CSV for offline analysis.

**Live dashboard.** Terminal dashboard rendered on a background thread while the simulation runs.

## Build & Run

```bash
make clean
make run
```

Task definitions are hardcoded in `main.cpp`. The constructor signature for `PeriodicTask` is:

```cpp
PeriodicTask(id, name, priority, wcet, deadline, period, first_release=0)
```

---

## Scheduling policies — academic background

### Implicit vs. constrained deadlines

A periodic task is characterised by its worst-case execution time `C`, relative deadline `D`, and period `T`.

- **Implicit deadline (`D = T`):** the task must complete before its next activation. The deadline is implicit in the period; RM is optimal for this class (Liu & Layland, 1973).
- **Constrained deadline (`D < T`):** the task must complete before `D`, which is strictly earlier than the next activation. There is a slack window of `T − D` ticks during which the CPU is free from this task's perspective. RM ignores this window and assigns priority solely by period, which is suboptimal. DM accounts for deadline tightness and is optimal among all fixed-priority preemptive algorithms for this class (Leung & Whitehead, 1982).

### Task set

The task set is designed to expose all four policies simultaneously. Tasks T1–T3 have constrained deadlines (`D < T`); tasks T4–T10 have implicit deadlines (`D = T`).

```cpp
tasks = {
    new PeriodicTask(1,  "T1",  20, 1,  4, 40),  // prio=20,  wcet=1, D=4,  T=40  | D<T: DM rank 2,  RM rank 10
    new PeriodicTask(2,  "T2",  30, 1,  6, 30),  // prio=30,  wcet=1, D=6,  T=30  | D<T: DM rank 4,  RM rank  9
    new PeriodicTask(3,  "T3",  10, 1,  3, 24),  // prio=10,  wcet=1, D=3,  T=24  | D<T: DM rank 1,  RM rank  7
    new PeriodicTask(4,  "T4",  35, 1,  5,  5),  // prio=35,  wcet=1, D=5,  T=5   | D=T: DM rank 3,  RM rank  1
    new PeriodicTask(5,  "T5",  40, 1,  8,  8),  // prio=40,  wcet=1, D=8,  T=8   | D=T: DM rank 5,  RM rank  2
    new PeriodicTask(6,  "T6",  60, 2, 10, 10),  // prio=60,  wcet=2, D=10, T=10  | D=T: DM rank 6,  RM rank  3
    new PeriodicTask(7,  "T7",  70, 1, 12, 12),  // prio=70,  wcet=1, D=12, T=12  | D=T: DM rank 7,  RM rank  4
    new PeriodicTask(8,  "T8",  80, 1, 15, 15),  // prio=80,  wcet=1, D=15, T=15  | D=T: DM rank 8,  RM rank  5
    new PeriodicTask(9,  "T9",  90, 1, 20, 20),  // prio=90,  wcet=1, D=20, T=20  | D=T: DM rank 9,  RM rank  6
    new PeriodicTask(10, "T10",100, 1, 25, 25),  // prio=100, wcet=1, D=25, T=25  | D=T: DM rank 10, RM rank  8
};
```

```
U = 1/40 + 1/30 + 1/24 + 1/5 + 1/8 + 2/10 + 1/12 + 1/15 + 1/20 + 1/25 ≈ 0.865
Hyperperiod = LCM(40, 30, 24, 5, 8, 10, 12, 15, 20, 25) = 600 ticks
Simulation duration: 600 ticks (one full hyperperiod)
```

### Priority ordering per policy

| Task | D  | T  | DM rank | RM rank | Priority rank |
|------|----|----|---------|---------|---------------|
| T3   |  3 | 24 |  **1**  |    7    |      10       |
| T1   |  4 | 40 |  **2**  |   10    |       9       |
| T4   |  5 |  5 |    3    |  **1**  |       7       |
| T2   |  6 | 30 |  **4**  |    9    |       8       |
| T5   |  8 |  8 |    5    |    2    |       6       |
| T6   | 10 | 10 |    6    |    3    |       5       |
| T7   | 12 | 12 |    7    |    4    |       4       |
| T8   | 15 | 15 |    8    |    5    |       3       |
| T9   | 20 | 20 |    9    |    6    |       2       |
| T10  | 25 | 25 |   10    |    8    |       1       |

The critical divergence is on T1, T2, and T3: DM assigns them ranks 2, 4, and 1 (reflecting tight deadlines), while RM assigns them ranks 10, 9, and 7 (reflecting long periods). The manual Priority ordering assigns them the three lowest priorities, the worst possible assignment for tasks with the tightest deadlines.

### DM schedulability — response time analysis

For a fixed-priority preemptive system, the worst-case response time `R_i` of task `τ_i` satisfies the recurrence:

```
R_i^(0) = C_i
R_i^(n+1) = C_i + Σ_{j ∈ hp(i)} ⌈ R_i^(n) / T_j ⌉ · C_j
```

where `hp(i)` is the set of tasks with strictly higher DM priority. The iteration converges when `R_i^(n+1) = R_i^(n)`. A task meets its deadline iff `R_i ≤ D_i`.

Applying the analysis to all ten tasks under DM priority order:

| Task | hp(i)    | R  | D  | Schedulable |
|------|----------|----|----|-------------|
| T3   | —        |  1 |  3 | ✓           |
| T1   | T3       |  2 |  4 | ✓           |
| T4   | T3, T1   |  3 |  5 | ✓           |
| T2   | T3–T4    |  4 |  6 | ✓           |
| T5   | T3–T2    |  5 |  8 | ✓           |
| T6   | T3–T5    |  8 | 10 | ✓           |
| T7   | T3–T6    | 10 | 12 | ✓           |
| T8   | T3–T7    | 15 | 15 | ✓ (tight)   |
| T9   | T3–T8    | 19 | 20 | ✓ (tight)   |
| T10  | T3–T9    | 20 | 25 | ✓           |

All tasks satisfy `R_i ≤ D_i` under DM — the task set is DM-schedulable with zero deadline misses. Because DM is optimal among fixed-priority preemptive algorithms, no other static-priority assignment can improve on this result.

### Observed results — 600-tick simulation (one full hyperperiod)

| Policy   | CPU%  | Deadline misses |
|----------|-------|-----------------|
| EDF      | 86.5% | 0               |
| DM       | 86.5% | 0               |
| RM       | 86.5% | 46              |
| Priority | 100%  | 140             |

### Why these results follow from theory

**EDF (0 misses):** The Liu & Layland optimality theorem guarantees that EDF never produces a deadline miss on a uniprocessor provided `U ≤ 1`. Since `U ≈ 0.865 < 1`, zero misses are guaranteed regardless of the task set structure.

**DM (0 misses):** DM is optimal among fixed-priority preemptive algorithms for constrained-deadline task sets. The response time analysis above confirms that every task satisfies `R_i ≤ D_i` under DM priority assignment. CPU utilization matches EDF exactly because no cycles are lost to cascading misses.

**RM (46 misses):** RM assigns priorities by period alone and is blind to relative deadlines. T3 (`D=3, T=24`) receives RM rank 7: six higher-priority tasks can collectively inject up to eight ticks of interference within its three-tick deadline window, making a miss structurally inevitable at every activation. T1 (`D=4, T=40`) and T2 (`D=6, T=30`) suffer analogous failures at RM ranks 10 and 9. The soft real-time model allows missed tasks to continue executing, occupying CPU time that reduces available slack for subsequent jobs and producing a cascade of secondary misses beyond the three structurally guaranteed ones — explaining the 46 observed misses over 600 ticks.

**Priority (140 misses, CPU 100%):** The manual priority assignment is the reverse of the DM order: tasks with the longest deadlines receive the highest priority. T4 (`D=5, T=5`) is assigned rank 7; six higher-priority tasks collectively inject more than four ticks of interference within its five-tick deadline window, causing T4 to miss its deadline on virtually every one of its 120 activations over 600 ticks. T3 (`D=3`) receives the lowest manual priority (rank 10) and misses on every activation. The high release frequency of T4 (period=5) continuously refills the ready queue with a partially-completed job, eliminating idle ticks and saturating CPU utilization at 100%.

### CPU% as a diagnostic signal

Both EDF and DM produce `CPU% = U ≈ 86.5%`, the theoretical minimum: every busy tick corresponds to useful work, and idle ticks represent genuine processor slack. RM reaches the same value in this simulation because soft real-time accounting includes missed-task execution in the utilization measure.

Priority saturates at 100% because T4's perpetual misses keep the ready queue permanently non-empty. A CPU utilization strictly above `U` is therefore a direct indicator of scheduling pathology under soft real-time semantics: cycles are being consumed by tasks that have already violated their deadlines rather than by useful, on-time execution.

---

## To-do

**Short term:** external configuration via file, schedulability analysis output at startup (response-time analysis per task).

**Medium term:** multi-core scheduling with a load balancer.

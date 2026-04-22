# TaskSched

A real-time task scheduling simulator written in C++.

## Overview

TaskSched models the logic of an RTOS scheduler. You define a task set (periodic, aperiodic, sporadic), pick a scheduling algorithm, and run a deterministic discrete-time simulation. It is a study tool for comparing scheduling policies on the same task set.

## Features

**Task hierarchy.** Abstract `Task` base with three concrete types: `PeriodicTask`, `AperiodicTask`, and `SporadicTask`. The latter uses diamond inheritance with a virtual shared base.

**Scheduler.** Single-core, preemptive, discrete-time simulation loop with task release, preemption, dispatch, and completion handling. Soft real-time model: missed tasks continue executing and are tracked in statistics.

**Scheduling policies (Strategy pattern).** The algorithm is decoupled from the scheduler via an abstract `SchedulingPolicy` base:

- `PriorityPolicy` — fixed priority assigned manually
- `RateMonotonicPolicy` — fixed priority derived from period (shorter period = higher priority)
- `DeadlineMonotonicPolicy` — fixed priority derived from relative deadline (shorter deadline = higher priority)
- `EDFPolicy` — dynamic priority, earliest absolute deadline first

**Statistics & export.** Per-tick CPU utilization, deadline miss counters, per-task response times, preemption counts. State snapshots exported to CSV for analysis.

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

## Scheduling policies — academic background

### When policies produce identical schedules

For a task set with **implicit deadlines** (D = T for all tasks) and manual priorities that match the RM ordering, all four policies produce the same schedule. This is a known theoretical result:

- RM and DM are identical when D = T (deadline ordering equals period ordering)
- EDF with D = T maintains the same relative ordering of absolute deadlines as RM throughout the hyperperiod
- If manual priorities also follow the RM order, Priority matches too

### When policies diverge

Differences appear when tasks have **constrained deadlines** (D < T). Example task set that exposes all four policies simultaneously:

```cpp
tasks = {
    new PeriodicTask(1, "T1", 30, 1,  6,  6),  // wcet=1, D=6,  T=6
    new PeriodicTask(2, "T2", 10, 1,  5,  8),  // wcet=1, D=5,  T=8
    new PeriodicTask(3, "T3", 50, 2,  8, 10),  // wcet=2, D=8,  T=10
    new PeriodicTask(4, "T4", 40, 2,  7, 15),  // wcet=2, D=7,  T=15
    new PeriodicTask(5, "T5", 20, 3, 12, 20),  // wcet=3, D=12, T=20
    new PeriodicTask(6, "T6", 60, 3, 10, 30),  // wcet=3, D=10, T=30
};
// U = 1/6 + 1/8 + 2/10 + 2/15 + 3/20 + 3/30 = 0.875
// Hyperperiod = LCM(6,8,10,15,20,30) = 120
```

Priority ordering per policy at t=0:

| Task | RM rank | DM rank | Priority rank | EDF (abs deadline) |
|------|---------|---------|---------------|--------------------|
| T1   | 1 (T=6) | 2 (D=6) | 4             | 6                  |
| T2   | 2 (T=8) | 1 (D=5) | 6             | 5                  |
| T3   | 3 (T=10)| 4 (D=8) | 2             | 8                  |
| T4   | 4 (T=15)| 3 (D=7) | 3             | 7                  |
| T5   | 5 (T=20)| 6 (D=12)| 5             | 12                 |
| T6   | 6 (T=30)| 5 (D=10)| 1             | 10                 |

Observed results over 360 ticks:

| Policy   | CPU%  | Deadline misses |
|----------|-------|-----------------|
| EDF      | 87.5% | 0               |
| DM       | 87.5% | 0               |
| RM       | 87.5% | 33              |
| Priority | 100%  | 72              |

### Why these results follow from theory

**EDF (0 misses):** The Liu & Layland theorem guarantees that EDF is optimal for a single processor — a feasible task set (U <= 1) will never miss a deadline under EDF. Since U = 0.875 < 1, zero misses are guaranteed.

**DM (0 misses):** Deadline Monotonic is provably optimal among fixed-priority algorithms for constrained-deadline task sets (D <= T). Assigning priority by relative deadline minimizes the chance of a tight-deadline task being preempted by a task with a looser deadline.

**RM (33 misses):** Rate Monotonic is optimal only for implicit-deadline task sets (D = T). When D < T, it ignores deadlines entirely. T2 (D=5, T=8) receives lower RM priority than T1 (T=6), even though T2 has the tighter deadline, causing periodic misses.

**Priority (72 misses, CPU 100%):** Manually assigned priorities in the wrong order send T6 (period=30) to the CPU before T2 (deadline=5, priority=10). T2 misses its deadline almost every period. The soft real-time model keeps missed tasks running, filling idle slots and driving CPU utilization to 100%.

The CPU% divergence is itself informative: 87.5% for correct policies matches U exactly (no wasted cycles), while 100% for Priority signals that missed tasks are consuming cycles that should have been idle.

## To-do

**Short term:** external configuration via file, schedulability analysis (response-time analysis for fixed-priority policies).

**Medium term:** multi-core scheduling with a load balancer.

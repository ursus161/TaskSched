# TaskSched

A real-time task scheduling simulator written in C++.

## Overview

TaskSched models the logic of an RTOS scheduler. You define a task set (periodic, aperiodic, sporadic), pick a scheduling algorithm, and run a deterministic discrete-time simulation. It is a study tool for comparing scheduling policies on the same task set.

## Features

**Task hierarchy.** Abstract `Task` base with three concrete types: `PeriodicTask`, `AperiodicTask`, and `SporadicTask`. The latter uses diamond inheritance with a virtual shared base.

**Scheduler.** Single-core, preemptive, discrete-time simulation loop with task release, preemption, dispatch, and completion handling. Tasks are not owned by the scheduler — the caller manages their lifetime.

**Scheduling policies (Strategy pattern).** The algorithm is decoupled from the scheduler via an abstract `SchedulingPolicy` base:
- `PriorityPolicy` — fixed priority
- `RateMonotonicPolicy` — priority derived from period
- `EDFPolicy` — earliest absolute deadline first

## Build & Run

```bash
make clean
make run
```

Task definitions are currently hardcoded in `main.cpp`.

## To-do

**Short term:** asynchronous logger on a dedicated thread, external configuration via file, schedulability analysis.

**Medium term:** multi-core scheduling with a load balancer.

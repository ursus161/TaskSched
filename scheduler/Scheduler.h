#pragma once

#include "Task.h"
#include <vector>
#include <memory>
#include "policies/SchedulingPolicy.h"
#include "queue/ReadyQueue.h"
#include "stats/Stats.h"
#include "stats/EventQueue.h"
#include "trace/TraceSink.h"

class Scheduler {
private:
    SchedulingPolicy* policy;
    Stats* stats;
    EventQueue* event_queue;

    // sink pentru trace-ul CSV; nu il detinem (optional, poate fi nullptr)
    TraceSink* trace_sink;

    // toate taskurile din sistem, nu le detinem
    std::vector<Task*> tasks;

    // taskul care ruleaza acum, nullptr daca CPU idle
    Task* current_running;

    // timpul curent al simularii in tickuri
    int current_time;

    // ready queue; mereu un HeapReadyQueue, tinut in spatele interfetei ca sa
    // putem adauga cozi per-core la trecerea pe multi-core
    std::unique_ptr<ReadyQueue> ready_queue;

    void dispatch(Task* new_running);

    int task_start_time = 0;

    std::unique_ptr<ReadyQueue> makeQueue(SchedulingPolicy* p);

    // emite un rand in trace; subject e taskul implicat (nullptr daca N/A)
    void emitEvent(const std::string& event, Task* subject);
    // rand per-tick / de sfarsit cu campuri de rulare explicite
    void emitRow(const std::string& event, Task* subject,
                 int running_id, int cpu_busy);

public:
    Scheduler();
    Scheduler(SchedulingPolicy* policy, Stats* stats, EventQueue* event_queue,
              TraceSink* trace_sink = nullptr);
    Scheduler(const Scheduler& sched);
    Scheduler& operator=(const Scheduler& other);
    ~Scheduler() = default;

    friend std::ostream& operator<<(std::ostream& out, const Scheduler& sched);
    friend std::istream& operator>>(std::istream& in, Scheduler& sched);

    void setPolicy(SchedulingPolicy* p);
    void addTask(Task* task);
    void run(int duration);
};

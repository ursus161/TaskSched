#pragma once

#include "Task.h"
#include <vector>
#include <memory>
#include "policies/SchedulingPolicy.h"
#include "queue/ReadyQueue.h"
#include "stats/Stats.h"
#include "stats/EventQueue.h"

class Scheduler {
private:
    SchedulingPolicy* policy;
    Stats* stats;
    EventQueue* event_queue;

    // toate taskurile din sistem, nu le detinem
    std::vector<Task*> tasks;

    // taskul care ruleaza acum, nullptr daca CPU idle
    Task* current_running;

    // timpul curent al simularii in tickuri
    int current_time;

    // ready queue; implementarea concreta e aleasa in functie de policy->isDynamic()
    std::unique_ptr<ReadyQueue> ready_queue;

    void dispatch(Task* new_running);

    int task_start_time = 0;

    std::unique_ptr<ReadyQueue> makeQueue(SchedulingPolicy* p);

public:
    Scheduler();
    Scheduler(SchedulingPolicy* policy, Stats* stats, EventQueue* event_queue);
    Scheduler(const Scheduler& sched);
    Scheduler& operator=(const Scheduler& other);
    ~Scheduler() = default;

    friend std::ostream& operator<<(std::ostream& out, const Scheduler& sched);
    friend std::istream& operator>>(std::istream& in, Scheduler& sched);

    void setPolicy(SchedulingPolicy* p);
    void addTask(Task* task);
    void run(int duration);
};

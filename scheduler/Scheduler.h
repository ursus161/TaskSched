#pragma once

#include "Task.h"
#include <vector>
#include <queue>
#include "policies/SchedulingPolicy.h"
#include "stats/Stats.h"
// pentru comparat taskuri dupa prioritate
// returneaza true daca a are prioritate mai mica decat b
// priority_queue e max-heap, deci cel cu prioritate mai mare iese primul
// ideea e ca priority queue vrea sa mi instantieze comparatorul PolicyComparator cmp, motivul pentru care l am facut struct
// si o sa am in implementarea interna a PQ cmp.operator()(task1,task2)
struct PolicyComparator {

    SchedulingPolicy* policy;

    bool operator()(Task* a, Task* b) const {
        return !policy->isHigherPriority(a,b);
    }
};

class Scheduler {
private:

    SchedulingPolicy* policy;

    Stats* stats;

    // toate taskurile din sistem, nu le detinem
    std::vector<Task*> tasks;

    // taskul care ruleaza acum, nullptr daca CPU idle
    Task* current_running;  

    // timpul curent al simularii in tickuri


    int current_time;

    // ready queue ordonata dupa prioritate, cele 3 elemente din pq sunt tipul de obiect tinut in pq, unde le stocheaza si dupa ce metoda imi face heap ul intern
    std::priority_queue<Task*, std::vector<Task*>, PolicyComparator> ready_queue;

    void dispatch(Task* new_running);

    int task_start_time =0;

public:
    Scheduler();
    Scheduler(SchedulingPolicy* policy, Stats* stats);
    Scheduler(Scheduler& sched);
    Scheduler& operator=(const Scheduler& other);
    ~Scheduler() = default;

    friend std::ostream& operator<<(std::ostream& out, const Scheduler& sched);
    friend std::istream& operator>>(std::istream& in, Scheduler& sched);

    
    void setPolicy(SchedulingPolicy* p);
    void addTask(Task* task); // probabil citirea de aici o vom face ori de la un CSV ori de la un user, detaliu de implementare pe care l vom vedea ulterior
    void run(int duration); // main logic shall be here
};
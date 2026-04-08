#pragma once

#include "Task.h"
#include <vector>
#include <queue>

// pentru comparat taskuri dupa prioritate
// returneaza true daca a are prioritate mai mica decat b
// priority_queue e max-heap, deci cel cu prioritate mai mare iese primul
// ideea e ca priority queue vrea sa mi instantieze comparatorul TaskCompare cmp, motivul pentru care l am facut struct
// si o sa am in implementarea interna a PQ cmp.operator()(task1,task2)
struct TaskCompare {
    bool operator()(Task* a, Task* b) const {
        return a->getPriority() < b->getPriority();
    }
};

class Scheduler {
private:
    // toate taskurile din sistem, nu le detinem
    std::vector<Task*> tasks;

    // taskul care ruleaza acum, nullptr daca CPU idle
    Task* current_running;

    // timpul curent al simularii in tickuri
    int current_time;

    // ready queue ordonata dupa prioritate, cele 3 elemente din pq sunt tipul de obiect tinut in pq, unde le stocheaza si dupa ce metoda imi face heap ul intern
    std::priority_queue<Task*, std::vector<Task*>, TaskCompare> ready_queue;

public:
    Scheduler();
    Scheduler(Scheduler& sched);
    void addTask(Task* task); // probabil citirea de aici o vom face ori de la un CSV ori de la un user, detaliu de implementare pe care l vom vedea ulterior
    void run(int duration); // main logic shall be here
};
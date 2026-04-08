#include "tasks/PeriodicTask.h"
#include "tasks/AperiodicTask.h"
#include "scheduler/Scheduler.h"

int main() {
    // cateva taskuri periodice de test
    PeriodicTask* t1 = new PeriodicTask(1, "sensor", 10, 2, 10, 10);
    PeriodicTask* t2 = new PeriodicTask(2, "display", 5, 3, 20, 20);
    AperiodicTask* t3 = new AperiodicTask(3, "button", 15, 2, 8, 5);

    Scheduler sched;
    sched.addTask(t1);
    sched.addTask(t2);
    sched.addTask(t3);

    sched.run(30);

    delete t1;
    delete t2;
    delete t3;

    return 0;
}
#include "tasks/PeriodicTask.h"
#include "scheduler/Scheduler.h"
#include "scheduler/policies/PriorityPolicy.h"
#include "scheduler/policies/EDFPolicy.h"
#include "scheduler/policies/RateMonotonicPolicy.h"
#include "scheduler/policies/SchedulingPolicy.h"

int main() {
    // task A: prioritate mica, deadline strans
    PeriodicTask* tA = new PeriodicTask(1, "A", 5,  3, 10, 10);
    // task B: prioritate mare, deadline larg
    PeriodicTask* tB = new PeriodicTask(2, "B", 10, 4, 20, 20);

    // schimba intre PriorityPolicy si EDFPolicy ca sa vezi diferenta
    SchedulingPolicy* policy = new PriorityPolicy();

    Scheduler sched(policy);
    sched.addTask(tA);
    sched.addTask(tB);

    sched.run(25);

    delete tA;
    delete tB;
    delete policy;

    return 0;
}
#include "tasks/PeriodicTask.h"
#include "tasks/AperiodicTask.h"
#include "tasks/SporadicTask.h"
#include "scheduler/Scheduler.h"
#include "scheduler/policies/PriorityPolicy.h"
#include "scheduler/policies/RateMonotonicPolicy.h"
#include "scheduler/policies/EDFPolicy.h"
#include "scheduler/stats/Stats.h"
#include <iostream>
#include <vector>
using namespace std;

vector<Task*> buildTasks() {
    return {

        new PeriodicTask(1, "fast",       2,  2, 6,  6),
        new PeriodicTask(2, "regular",    5,  3, 12, 12),
        new PeriodicTask(3, "heavy",     15,  6, 30, 30),
        new PeriodicTask(4, "tight",      8,  1, 4,  10),
        new AperiodicTask(5, "alert",    20,  2, 5,  9),
    //    new SporadicTask(5, "stergator", 15, 5, 10, 6, {3,13,15,22,39}),
        new AperiodicTask(6, "user_cmd", 12,  3, 8, 18),
        new SporadicTask(7, "interrupt", 18,  2, 4, 6, {7, 16, 28, 38, 50})
    };
}

int main() {
    vector<SchedulingPolicy*> policies = {
        new PriorityPolicy(),
        new RateMonotonicPolicy(),
        new EDFPolicy()
    };

    vector<Stats> results;

    for (SchedulingPolicy* policy : policies) {
        vector<Task*> tasks = buildTasks();
        Stats stats;
        for (Task* t : tasks) {
            stats.registerTask(t->getId(), t->getName(), t->getType());
        }

        Scheduler sched(policy, &stats);
        for (Task* t : tasks) sched.addTask(t);

        cout << "\n=== Running with " << policy->getName() << " ===\n";
        sched.run(100);

        results.push_back(stats);

        for (Task* t : tasks) delete t;
    }

    // raport per algoritm
    cout << "\n=== Final reports ===\n";
    for (size_t i = 0; i < policies.size(); i++) {
        cout << "\n[" << policies[i]->getName() << "]\n" << results[i];
    }

    // comparatie pe metrici
    cout << "\n=== Winners ===\n";
    cout << "Best CPU utilization: ";
    int best = 0;
    for (size_t i = 1; i < results.size(); i++)
        if (results[i].getCpuUtilization() > results[best].getCpuUtilization()) best = i;
    cout << policies[best]->getName() << " (" << results[best].getCpuUtilization() << "%)\n";

    cout << "Fewest deadline misses: ";
    best = 0;
    for (size_t i = 1; i < results.size(); i++)
        if (results[i].getTotalDeadlineMisses() < results[best].getTotalDeadlineMisses()) best = i;
    cout << policies[best]->getName() << " (" << results[best].getTotalDeadlineMisses() << ")\n";

    cout << "Fewest preemptions: ";
    best = 0;
    for (size_t i = 1; i < results.size(); i++)
        if (results[i].getTotalPreemptions() < results[best].getTotalPreemptions()) best = i;
    cout << policies[best]->getName() << " (" << results[best].getTotalPreemptions() << ")\n";

    for (SchedulingPolicy* p : policies) delete p;
    return 0;
}
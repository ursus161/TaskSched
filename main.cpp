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
        new PeriodicTask(1, "T1_Fast", 10, 2, 5, 5),
        new PeriodicTask(2, "T2_Mid", 5, 4, 8, 15),
        new PeriodicTask(3, "T3_Heavy", 2, 6, 20, 20),
        new SporadicTask(4, "T4_Spor", 15, 2, 4, 10, {3, 18, 33, 45, 60}),
        new AperiodicTask(5, "T5_Aper", 1, 3, 50, 12)       
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
        stats.exportToCSV("scheduler/stats/csv/timeline_" + policy->getName() + ".csv");

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
    int worst = results.size() - 1;
    for (size_t i = 1; i < results.size(); i++){
        if (results[i].getCpuUtilization() < results[worst].getCpuUtilization()) worst = i;

        if (results[i].getCpuUtilization() > results[best].getCpuUtilization()) best = i;}

    cout << policies[best]->getName() << " (" << results[best].getCpuUtilization() << "%)\n";
    cout << "Worst CPU utilization: ";
    cout<<policies[worst]->getName() << " (" << results[worst].getCpuUtilization() << "%)\n";
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
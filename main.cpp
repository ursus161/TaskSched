#include "tasks/PeriodicTask.h"
#include "tasks/AperiodicTask.h"
#include "tasks/SporadicTask.h"
#include "scheduler/Scheduler.h"
#include "scheduler/policies/EDFPolicy.h"
#include "scheduler/stats/Stats.h"
#include "scheduler/stats/EventQueue.h"
#include "scheduler/dashboard/Dashboard.h"
#include <iostream>
#include <vector>
#include <thread>
#include <stdexcept>

using namespace std;

int main() {
    vector<Task*> tasks;
    try {
        EventQueue queue;
        Stats stats;
        EDFPolicy policy;

        tasks = {
        new PeriodicTask(1, "T_P10",  20, 2,  10,  10),   // U = 2/10  = 0.20
        new PeriodicTask(2, "T_P20",  15, 3,  20,  20),   // U = 3/20  = 0.15
        new PeriodicTask(3, "T_P50",  10, 10, 50,  50),   // U = 10/50 = 0.20
        new PeriodicTask(4, "T_P100", 5,  15, 100, 100),  // U = 15/100= 0.15
            //am sum(U) = 0.7,
        };

        for (Task* t : tasks) {
            stats.registerTask(t->getId(), t->getName(), t->getType());
        }

        Scheduler sched(&policy, &stats, &queue);
        for (Task* t : tasks) sched.addTask(t);

        Dashboard dashboard(&queue);
        std::thread dashboard_thread(&Dashboard::run, &dashboard);  // porneste dashboard pe alt thread

        sched.run(10000);

        dashboard_thread.join();

        stats.exportToCSV("scheduler/stats/csv/timeline_" + policy.getName() + ".csv");

        // // citim events din queue
        // cout << "\n=== Events collected ===\n";
        // while (true) {
        //     Event e = queue.pop();
        //     cout << "t=" << e.time << " type=" << (int)e.type << " task_id=" << e.task_id << "\n";
        //     if (e.type == EventType::EndOfSimulation) break;
        // }

    } catch (const std::bad_alloc& e) {
        std::cerr << "[main] Memorie insuficienta: " << e.what() << "\n";
        for (Task* t : tasks) delete t;
        return 1;
    } catch (const std::invalid_argument& e) {
        std::cerr << "[main] Parametru invalid: " << e.what() << "\n";
        for (Task* t : tasks) delete t;
        return 2;
    } catch (const std::runtime_error& e) {
        std::cerr << "[main] Eroare runtime: " << e.what() << "\n";
        for (Task* t : tasks) delete t;
        return 3;
    } catch (const std::exception& e) {
        std::cerr << "[main] Exceptie necunoscuta: " << e.what() << "\n";
        for (Task* t : tasks) delete t;
        return 4;
    } catch (...) {
        std::cerr << "[main] Exceptie non-standard, oprire fortata\n";
        for (Task* t : tasks) delete t;
        return 5;
    }

    for (Task* t : tasks) delete t;
    return 0;
}






// #include "tasks/PeriodicTask.h"
// #include "tasks/AperiodicTask.h"
// #include "tasks/SporadicTask.h"
// #include "scheduler/Scheduler.h"
// #include "scheduler/policies/PriorityPolicy.h"
// #include "scheduler/policies/RateMonotonicPolicy.h"
// #include "scheduler/policies/EDFPolicy.h"
// #include "scheduler/stats/Stats.h"
// #include <iostream>
// #include <vector>
// using namespace std;
// vector<Task*> buildTasks() {
//     return {
//         new PeriodicTask(1,  "T_10ms",    20, 1,  10,  10),    
//         new PeriodicTask(2,  "T_15ms",    18, 2,  15,  15),    
//         new PeriodicTask(3,  "T_25ms",    15, 3,  25,  25),    
//         new PeriodicTask(4,  "T_50ms",    12, 5,  50,  50), 
//         new PeriodicTask(5,  "T_100ms",   10, 8,  100, 100),   
//         new PeriodicTask(6,  "T_200ms",   8,  12, 200, 200),   
//         new PeriodicTask(7,  "T_500ms",   5,  20, 500, 500),   
//         new SporadicTask(8,  "Btn_AC",    25, 2, 4, 30, {15, 78, 142, 230, 310, 450, 620, 800}),
//         new SporadicTask(9,  "Btn_Horn",  30, 1, 2, 50, {33, 120, 200, 350, 500, 680, 900}),
//         new SporadicTask(10, "Sens_Temp", 22, 3, 6, 80, {40, 180, 350, 520, 700}),
//         new AperiodicTask(11, "Boot_Init",    1, 5,  20,  5),
//         new AperiodicTask(12, "Firmware_Upd", 2, 10, 100, 250),
//         new AperiodicTask(13, "Diag_Check",   3, 8,  50,  600)
//     };
// }
// int main() {
//     vector<SchedulingPolicy*> policies = {
//         new PriorityPolicy(),
//         new RateMonotonicPolicy(),
//         new EDFPolicy()
//     };

//     vector<Stats> results;

//     for (SchedulingPolicy* policy : policies) {
//         vector<Task*> tasks = buildTasks();
//         Stats stats;
//         for (Task* t : tasks) {
//             stats.registerTask(t->getId(), t->getName(), t->getType());
//         }

//         Scheduler sched(policy, &stats);
//         for (Task* t : tasks) sched.addTask(t);

//         cout << "\n=== Running with " << policy->getName() << " ===\n";
//         sched.run(1000);
//         stats.exportToCSV("scheduler/stats/csv/timeline_" + policy->getName() + ".csv");

//         results.push_back(stats);

//         for (Task* t : tasks) delete t;
//     }

//     // raport per algoritm
//     cout << "\n=== Final reports ===\n";
//     for (size_t i = 0; i < policies.size(); i++) {
//         cout << "\n[" << policies[i]->getName() << "]\n" << results[i];
//     }

//     // comparatie pe metrici
//     cout << "\n=== Winners ===\n";
//     cout << "Best CPU utilization: ";
//     int best = 0;
//     int worst = results.size() - 1;
//     for (size_t i = 1; i < results.size(); i++){
//         if (results[i].getCpuUtilization() < results[worst].getCpuUtilization()) worst = i;

//         if (results[i].getCpuUtilization() > results[best].getCpuUtilization()) best = i;}

//     cout << policies[best]->getName() << " (" << results[best].getCpuUtilization() << "%)\n";
//     cout << "Worst CPU utilization: ";
//     cout<<policies[worst]->getName() << " (" << results[worst].getCpuUtilization() << "%)\n";
//     cout << "Fewest deadline misses: ";
//     best = 0;   
//     for (size_t i = 1; i < results.size(); i++)
//         if (results[i].getTotalDeadlineMisses() < results[best].getTotalDeadlineMisses()) best = i;
//     cout << policies[best]->getName() << " (" << results[best].getTotalDeadlineMisses() << ")\n";

//     cout << "Fewest preemptions: ";
//     best = 0;
//     for (size_t i = 1; i < results.size(); i++)
//         if (results[i].getTotalPreemptions() < results[best].getTotalPreemptions()) best = i;
//     cout << policies[best]->getName() << " (" << results[best].getTotalPreemptions() << ")\n";

//      for (SchedulingPolicy* p : policies) delete p;
//     return 0;
// }
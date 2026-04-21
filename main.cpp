#include "tasks/PeriodicTask.h"
#include "tasks/AperiodicTask.h"
#include "tasks/SporadicTask.h"
#include "scheduler/Scheduler.h"
#include "scheduler/policies/PriorityPolicy.h"
#include "scheduler/policies/RateMonotonicPolicy.h"
#include "scheduler/policies/EDFPolicy.h"
#include "scheduler/stats/Stats.h"
#include "scheduler/stats/EventQueue.h"
#include "scheduler/dashboard/Dashboard.h"
#include <iostream>
#include <vector>
#include <thread>
#include <stdexcept>
#include <limits>

using namespace std;

int main() {
    vector<Task*> tasks;
    SchedulingPolicy* policy = nullptr;
    int exit_code = 0;

    int width = Dashboard::getTerminalWidth();
    int padding_size = max(0, (width - 40) / 2);
    string padding(padding_size, ' ');

    try {
        bool keep_running = true;
        while (keep_running) {
            for (Task* t : tasks) delete t;
            tasks.clear();
            delete policy;
            policy = nullptr;

            // alegere policy
            int choice;
            while (policy == nullptr) {
                cout << "\033[H\033[J" << flush;
                cout << "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n";
                cout << "\n" << padding << "=== Alege algoritmul de Task Scheduling: ===\n";
                cout << padding << "1. Priority\n";
                cout << padding << "2. Rate Monotonic\n";
                cout << padding << "3. EDF\n";
                cout << padding << "Optiune: ";

                if (!(cin >> choice)) {
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    cout << padding << "Input invalid, introdu un numar.\n";
                    continue;
                }

                switch (choice) {
                    case 1: policy = new PriorityPolicy(); break;
                    case 2: policy = new RateMonotonicPolicy(); break;
                    case 3: policy = new EDFPolicy(); break;
                    default: cout << padding << "Optiune invalida, incearca din nou.\n";
                }
            }

            EventQueue queue;
            Stats stats;

            tasks = {
                new PeriodicTask(1, "T_A", 20, 2, 10, 10),  // U=0.20
                new PeriodicTask(2, "T_B", 15, 4, 20, 20),  // U=0.20
                new PeriodicTask(3, "T_C", 10, 9, 50, 50),  // U=0.18
                // sum(U)=0.58 < RM bound (75.68%) toate 3 policy-uri: 0 misses, CPU%=58
            };

            for (Task* t : tasks)
                stats.registerTask(t->getId(), t->getName(), t->getType());

            Scheduler sched(policy, &stats, &queue);
            for (Task* t : tasks) sched.addTask(t);

            Dashboard dashboard(&queue, &stats);
            thread dashboard_thread(&Dashboard::run, &dashboard);

            sched.run(200);
            dashboard_thread.join();

            string snapshot_file = "scheduler/stats/csv/snapshot_" + policy->getName() + ".csv";
            stats.exportSnapshotCSV(snapshot_file);

            // meniu post-simulare
            bool in_menu = true;
            while (in_menu) {
                cout << "\033[H\033[J" << flush;
                
                cout << "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n";
                cout << "\n" << padding << "=== Simulare terminata (" << policy->getName() << ") ===\n";
                cout << padding << "1. Vezi snapshot-ul sistemului la un tick\n";
                cout << padding << "2. Ruleaza din nou\n";
                cout << padding << "3. Iesire\n";
                cout << padding << "Optiune: ";

                if (!(cin >> choice)) {
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    continue;
                }

                switch (choice) {
                    case 1: {
                        cout << padding << "Tick: ";
                        int tick;
                        if (!(cin >> tick)) {
                            cin.clear();
                            cin.ignore(numeric_limits<streamsize>::max(), '\n');
                            cout << padding << "Tick invalid.\n";
                            break;
                        }
                        auto rows = Stats::getSnapshotAt(snapshot_file, tick);
                        if (rows.empty()) {
                            cout << padding << "Niciun snapshot disponibil pentru t<=" << tick << ".\n";
                            break;
                        }
                        cout << "\n" << padding << "=== Snapshot la t=" << rows[0].tick << " ===\n";
                        cout << padding << "CPU: " << rows[0].cpu_task
                             << " | Utilizare: " << rows[0].cpu_util << "%\n";
                        for (const auto& r : rows)
                            cout << padding << "  [" << r.task_id << "] " << r.task_name << " -> " << r.state << "\n";
                        cout << "\n" << padding << "Apasa Enter pentru a continua...";
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                        cin.get();
                        break;
                    }
                    case 2:
                        in_menu = false;
                        break;
                    case 3:
                        in_menu = false;
                        keep_running = false;
                        break;
                    default:
                        cout << padding << "Optiune invalida.\n";
                }
            }
        }

    } catch (const bad_alloc& e) {
        cerr << "[main] Memorie insuficienta: " << e.what() << "\n";
        exit_code = 1;
    } catch (const invalid_argument& e) {
        cerr << "[main] Parametru invalid: " << e.what() << "\n";
        exit_code = 2;
    } catch (const runtime_error& e) {
        cerr << "[main] Eroare runtime: " << e.what() << "\n";
        exit_code = 3;
    } catch (const exception& e) {
        cerr << "[main] Exceptie necunoscuta: " << e.what() << "\n";
        exit_code = 4;
    } catch (...) {
        cerr << "[main] Exceptie non-standard\n";
        exit_code = 5;
    }

    for (Task* t : tasks) delete t;
    delete policy;
    return exit_code;
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
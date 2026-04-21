#include "tasks/PeriodicTask.h"
#include "tasks/AperiodicTask.h"
#include "tasks/SporadicTask.h"
#include "scheduler/Scheduler.h"
#include "scheduler/policies/PriorityPolicy.h"
#include "scheduler/policies/RateMonotonicPolicy.h"
#include "scheduler/policies/DeadlineMonotonicPolicy.h"
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
                cout<< padding << "4. Deadline Monotonic\n";
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
                    case 4: policy = new DeadlineMonotonicPolicy(); break;
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
                cout<< "\n" << padding << "CPU% final: " << stats.getCpuUtilization() <<  "  |  " << "Deadline Misses: "<< stats.getTotalDeadlineMisses() << "\n";
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
    return exit_code; //pot printr un program extern sa dau handle in functie de codul pe care l returnez OS ului
}





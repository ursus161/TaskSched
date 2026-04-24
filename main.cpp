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
#include <fstream>
#include <vector>
#include <thread>
#include <stdexcept>
#include <limits>

using namespace std;

static void clearScreen() {
    cout << "\033[H\033[J" << flush;
    cout << "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n";
}

static bool readInt(int& val) {
    if (!(cin >> val)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return false;
    }
    return true;
}

static void waitEnter() {
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cin.get();
}

static void showSnapshotAt(const string& file, int tick, const string& padding) {
    auto rows = Stats::getSnapshotAt(file, tick);
    if (rows.empty()) {
        cout << padding << "Niciun snapshot disponibil pentru t<=" << tick << ".\n";
        return;
    }
    cout << "\n" << padding << "=== Snapshot la t=" << rows[0].tick << " ===\n";
    cout << padding << "CPU: " << rows[0].cpu_task << " | Utilizare: " << rows[0].cpu_util << "%\n";
    for (const auto& r : rows)
        cout << padding << "  [" << r.task_id << "] " << r.task_name << " -> " << r.state << "\n";
    cout << "\n" << padding << "Apasa Enter pentru a continua...";
    waitEnter();
}

static SchedulingPolicy* selectPolicy(const string& padding) {
    SchedulingPolicy* policy = nullptr;
    int choice;
    while (policy == nullptr) {
        clearScreen();
        cout << "\n" << padding << "=== Alege algoritmul de Task Scheduling: ===\n";
        cout << padding << "1. Priority\n";
        cout << padding << "2. Rate Monotonic\n";
        cout << padding << "3. EDF\n";
        cout << padding << "4. Deadline Monotonic\n";
        cout << padding << "Optiune: ";

        if (!readInt(choice)) {
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
    return policy;
}

static vector<Task*> createTasks() {
    // tasks = {
    //     new PeriodicTask(1, "T_A", 20, 2, 10, 10),  // U=0.20
    //     new PeriodicTask(2, "T_B", 15, 4, 20, 20),  // U=0.20
    //     new PeriodicTask(3, "T_C", 10, 9, 50, 50),  // U=0.18
    //     // sum(U)=0.58 < RM bound (75.68%) toate 3 policy-uri: 0 misses, CPU%=58
    // };

    // tasks = {
    //     new PeriodicTask(1, "T_A", 20, 2,  8,  8),  // deadline=8,  period=8
    //     new PeriodicTask(2, "T_B", 10, 2,  4, 12),  // deadline=4,  period=12
    //     new PeriodicTask(3, "T_C", 30, 3, 10, 20),  // deadline=10, period=20
    // };
    // U = 0.25 + 0.167 + 0.15 = 0.567 < ln(2)

    // Deadlines constranste (D < T): DM le da prioritate mare, RM le ignora
    // Deadlines implicite (D = T): DM si RM sunt echivalente pe aceste taskuri
    return {
        new PeriodicTask(1,  "T1",  20, 1,  4, 40),  // prio=20,  wcet=1, D=4,  T=40 | D<T: DM rank 2,  RM rank 10
        new PeriodicTask(2,  "T2",  30, 1,  6, 30),  // prio=30,  wcet=1, D=6,  T=30 | D<T: DM rank 4,  RM rank  9
        new PeriodicTask(3,  "T3",  10, 1,  3, 24),  // prio=10,  wcet=1, D=3,  T=24 | D<T: DM rank 1,  RM rank  7
        new PeriodicTask(4,  "T4",  35, 1,  5,  5),  // prio=35,  wcet=1, D=5,  T=5  | D=T: DM rank 3,  RM rank  1
        new PeriodicTask(5,  "T5",  40, 1,  8,  8),  // prio=40,  wcet=1, D=8,  T=8  | D=T: DM rank 5,  RM rank  2
        new PeriodicTask(6,  "T6",  60, 2, 10, 10),  // prio=60,  wcet=2, D=10, T=10 | D=T: DM rank 6,  RM rank  3
        new PeriodicTask(7,  "T7",  70, 1, 12, 12),  // prio=70,  wcet=1, D=12, T=12 | D=T: DM rank 7,  RM rank  4
        new PeriodicTask(8,  "T8",  80, 1, 15, 15),  // prio=80,  wcet=1, D=15, T=15 | D=T: DM rank 8,  RM rank  5
        new PeriodicTask(9,  "T9",  90, 1, 20, 20),  // prio=90,  wcet=1, D=20, T=20 | D=T: DM rank 9,  RM rank  6
        new PeriodicTask(10, "T10",100, 1, 25, 25),  // prio=100, wcet=1, D=25, T=25 | D=T: DM rank 10, RM rank  8
    };
    // U = 1/40+1/30+1/24+1/5+1/8+2/10+1/12+1/15+1/20+1/25 = 0.865 < 1
    // EDF:      0 misses  (U < 1, optim dinamic)
    // DM:       0 misses  (optim fixed-priority pentru D<=T, verificat prin RTA)
    // RM:       misses pe T1,T2,T3 (RM ignora deadlineurile stramte, le da prioritate mica)
    // Priority: misses pe T3,T1,T2,T4 (prioritatile manuale sunt in ordine inversa DM)
}

int main() {
    vector<Task*> tasks;
    SchedulingPolicy* policy = nullptr;
    int exit_code = 0;

    int width = Dashboard::getTerminalWidth();
    string padding(max(0, (width - 40) / 2), ' ');

    try {
        bool keep_running = true;
        while (keep_running) {
            for (Task* t : tasks) delete t;
            tasks.clear();
            delete policy;

            // alegere policy
            policy = selectPolicy(padding);

            // meniu pre-rulare
            string snapshot_file = "scheduler/stats/csv/snapshot_" + policy->getName() + ".csv";
            int choice = 0;
            bool should_run = true;
            bool go_back = false;

            bool in_pre_menu = true;
            while (in_pre_menu) {
                clearScreen();
                cout << "\n" << padding << "=== " << policy->getName() << " ===\n";
                cout << padding << "1. Ruleaza simularea\n";
                cout << padding << "2. Afiseaza datele de la ultima rulare\n";
                cout << padding << "3. Inapoi\n";
                cout << padding << "Optiune: ";

                if (!readInt(choice)) continue;

                if (choice == 1) { should_run = true;  in_pre_menu = false; }
                else if (choice == 2) {
                    ifstream test(snapshot_file);
                    if (!test.is_open()) {
                        cout << "\n" << padding << "Nu exista date de la o rulare anterioara pentru " << policy->getName() << ".\n";
                        cout << padding << "Apasa Enter pentru a continua...";
                        waitEnter();
                    } else {
                        should_run = false;
                        in_pre_menu = false;
                    }
                } else if (choice == 3) { go_back = true; in_pre_menu = false; }
                else cout << padding << "Optiune invalida.\n";
            }

            if (go_back) continue; // inapoi la selectia de policy

            if (!should_run) {
                // browsing prin ultima rulare
                auto summary = Stats::getSummaryFromCSV(snapshot_file);
                bool in_last_run = true;
                while (in_last_run) {
                    clearScreen();
                    cout << "\n" << padding << "=== Ultima rulare: " << policy->getName() << " ===\n";
                    if (summary.valid)
                        cout << padding << "CPU%: " << summary.cpu_util << "  |  Deadline Misses: " << summary.deadline_misses << "\n";
                    else
                        cout << padding << "(summary indisponibil - CSV vechi, ruleaza din nou)\n";
                    cout << padding << "1. Vezi snapshot-ul la un tick\n";
                    cout << padding << "2. Inapoi\n";
                    cout << padding << "Optiune: ";

                    if (!readInt(choice)) continue;

                    if (choice == 1) {
                        cout << padding << "Tick: ";
                        int tick;
                        if (!readInt(tick)) { cout << padding << "Tick invalid.\n"; continue; }
                        showSnapshotAt(snapshot_file, tick, padding);
                    } else if (choice == 2) { in_last_run = false; }
                    else cout << padding << "Optiune invalida.\n";
                }
                continue; // inapoi la selectia de policy
            }

            EventQueue queue;
            Stats stats;
            tasks = createTasks();

            for (Task* t : tasks)
                stats.registerTask(t->getId(), t->getName(), t->getType());

            Scheduler sched(policy, &stats, &queue);
            for (Task* t : tasks) sched.addTask(t);

            Dashboard dashboard(&queue, &stats, policy, tasks);
            thread dashboard_thread(&Dashboard::run, &dashboard);

            sched.run(600); //hyperperioada este lcm din perioade, un nr de tickuri astfel incat totul sa fie rulat
            dashboard_thread.join();

            stats.exportSnapshotCSV(snapshot_file);

            // meniu post-simulare
            bool in_menu = true;
            while (in_menu) {
                clearScreen();
                cout << "\n" << padding << "=== Simulare terminata (" << policy->getName() << ") ===\n";
                cout << "\n" << padding << "CPU% final: " << stats.getCpuUtilization() << "  |  Deadline Misses: " << stats.getTotalDeadlineMisses() << "\n";
                cout << padding << "1. Vezi snapshot-ul sistemului la un tick\n";
                cout << padding << "2. Ruleaza din nou\n";
                cout << padding << "3. Iesire\n";
                cout << padding << "Optiune: ";

                if (!readInt(choice)) continue;

                if (choice == 1) {
                    cout << padding << "Tick: ";
                    int tick;
                    if (!readInt(tick)) { cout << padding << "Tick invalid.\n"; continue; }
                    showSnapshotAt(snapshot_file, tick, padding);
                } else if (choice == 2) { in_menu = false; }
                else if (choice == 3) { in_menu = false; keep_running = false; }
                else cout << padding << "Optiune invalida.\n";
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

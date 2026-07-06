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
#include "scheduler/trace/CsvTraceSink.h"
#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <stdexcept>
#include <limits>
#include <ctime>

using namespace std;

// ---- durata simularii = o hiperperioada completa a task set-ului din createTasks() ----
static constexpr int DEFAULT_DURATION = 600;

static bool readInt(int& val) {
    if (!(cin >> val)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return false;
    }
    return true;
}

// creeaza o instanta de policy dupa un tag (nume scurt sau numar de meniu)
static unique_ptr<SchedulingPolicy> makePolicy(const string& tag) {
    if (tag == "1" || tag == "priority")                    return make_unique<PriorityPolicy>();
    if (tag == "2" || tag == "rm" || tag == "ratemonotonic") return make_unique<RateMonotonicPolicy>();
    if (tag == "3" || tag == "edf")                          return make_unique<EDFPolicy>();
    if (tag == "4" || tag == "dm" || tag == "deadlinemonotonic") return make_unique<DeadlineMonotonicPolicy>();
    return nullptr;
}

static unique_ptr<SchedulingPolicy> selectPolicyInteractive() {
    while (true) {
        cout << "\n=== Alege algoritmul de Task Scheduling: ===\n";
        cout << "1. Priority\n";
        cout << "2. Rate Monotonic\n";
        cout << "3. EDF\n";
        cout << "4. Deadline Monotonic\n";
        cout << "0. Iesire\n";
        cout << "Optiune: ";

        int choice;
        if (!readInt(choice)) { cout << "Input invalid, introdu un numar.\n"; continue; }
        if (choice == 0) return nullptr;

        auto policy = makePolicy(to_string(choice));
        if (policy) return policy;
        cout << "Optiune invalida, incearca din nou.\n";
    }
}

// task set fix; vezi README pentru derivarea rankurilor per policy si a hiperperioadei (600)
static vector<Task*> createTasks() {
    Task::resetIdCounter();
    return {
        new PeriodicTask(  "T1",  20, 1,  4, 40),  // prio=20,  wcet=1, D=4,  T=40 | D<T: DM rank 2,  RM rank 10
        new PeriodicTask(  "T2",  30, 1,  6, 30),  // prio=30,  wcet=1, D=6,  T=30 | D<T: DM rank 4,  RM rank  9
        new PeriodicTask(  "T3",  10, 1,  3, 24),  // prio=10,  wcet=1, D=3,  T=24 | D<T: DM rank 1,  RM rank  7
        new PeriodicTask(  "T4",  35, 1,  5,  5),  // prio=35,  wcet=1, D=5,  T=5  | D=T: DM rank 3,  RM rank  1
        new PeriodicTask(  "T5",  40, 1,  8,  8),  // prio=40,  wcet=1, D=8,  T=8  | D=T: DM rank 5,  RM rank  2
        new PeriodicTask(  "T6",  60, 2, 10, 10),  // prio=60,  wcet=2, D=10, T=10 | D=T: DM rank 6,  RM rank  3
        new PeriodicTask(  "T7",  70, 1, 12, 12),  // prio=70,  wcet=1, D=12, T=12 | D=T: DM rank 7,  RM rank  4
        new PeriodicTask(  "T8",  80, 1, 15, 15),  // prio=80,  wcet=1, D=15, T=15 | D=T: DM rank 8,  RM rank  5
        new PeriodicTask(  "T9",  90, 1, 20, 20),  // prio=90,  wcet=1, D=20, T=20 | D=T: DM rank 9,  RM rank  6
        new PeriodicTask( "T10", 100, 1, 25, 25),  // prio=100, wcet=1, D=25, T=25 | D=T: DM rank 10, RM rank  8
    };
    // U = 1/40+1/30+1/24+1/5+1/8+2/10+1/12+1/15+1/20+1/25 = 0.865 < 1
}

// numele policy-ului fara spatii, pentru numele de fisier (ex. "Rate Monotonic" -> "RateMonotonic")
static string sanitize(const string& name) {
    string out;
    for (char c : name) if (c != ' ') out += c;
    return out;
}

static string timestamp() {
    time_t now = time(nullptr);
    char buf[32];
    strftime(buf, sizeof(buf), "%Y%m%d_%H%M%S", localtime(&now));
    return string(buf);
}

// ruleaza o simulare completa pentru un policy si scrie trace-ul CSV. intoarce calea CSV.
static string runSimulation(SchedulingPolicy* policy, const string& out_path, int duration) {
    string path = out_path.empty()
        ? "traces/trace_" + sanitize(policy->getName()) + "_" + timestamp() + ".csv"
        : out_path;

    EventQueue queue;
    Stats stats;
    auto sink = make_unique<CsvTraceSink>(path);

    vector<Task*> tasks = createTasks();
    for (Task* t : tasks)
        stats.registerTask(t->getId(), t->getName(), t->getType());

    Scheduler sched(policy, &stats, &queue, sink.get());
    for (Task* t : tasks) sched.addTask(t);

    sched.run(duration);

    cout << "\n=== Simulare terminata (" << policy->getName() << ") ===\n";
    cout << "CPU%: " << stats.getCpuUtilization()
         << "  |  Deadline misses: " << stats.getTotalDeadlineMisses()
         << "  |  Preemptions: " << stats.getTotalPreemptions() << "\n";
    cout << "Trace scris in: " << path << "\n";

    for (Task* t : tasks) delete t;
    return path;
}

static void printUsage(const char* prog) {
    cout << "Utilizare: " << prog << " [--policy=NAME] [--out=PATH] [--duration=N]\n"
         << "  --policy=NAME  priority | rm | edf | dm  (fara asta: meniu interactiv)\n"
         << "  --out=PATH     calea CSV de output (implicit traces/trace_<policy>_<timestamp>.csv)\n"
         << "  --duration=N   numar de tickuri (implicit " << DEFAULT_DURATION << ")\n";
}

int main(int argc, char** argv) {
    string policy_arg, out_arg;
    int duration = DEFAULT_DURATION;

    // parsare argumente simple --key=value
    for (int i = 1; i < argc; i++) {
        string a = argv[i];
        if (a == "-h" || a == "--help") { printUsage(argv[0]); return 0; }
        else if (a.rfind("--policy=", 0) == 0)   policy_arg = a.substr(9);
        else if (a.rfind("--out=", 0) == 0)      out_arg = a.substr(6);
        else if (a.rfind("--duration=", 0) == 0) duration = stoi(a.substr(11));
        else { cerr << "Argument necunoscut: " << a << "\n"; printUsage(argv[0]); return 2; }
    }

    try {
        if (duration <= 0)
            throw invalid_argument("--duration trebuie sa fie > 0");

        if (!policy_arg.empty()) {
            // mod non-interactiv: o singura rulare, util pentru scripturi/comparatii
            auto policy = makePolicy(policy_arg);
            if (!policy) { cerr << "Policy necunoscut: " << policy_arg << "\n"; printUsage(argv[0]); return 2; }
            runSimulation(policy.get(), out_arg, duration);
            return 0;
        }

        // mod interactiv: alege policy, ruleaza, repeta
        while (true) {
            auto policy = selectPolicyInteractive();
            if (!policy) break;
            runSimulation(policy.get(), out_arg, duration);
        }

    } catch (const bad_alloc& e) {
        cerr << "[main] Memorie insuficienta: " << e.what() << "\n";  return 1;
    } catch (const invalid_argument& e) {
        cerr << "[main] Parametru invalid: " << e.what() << "\n";     return 2;
    } catch (const runtime_error& e) {
        cerr << "[main] Eroare runtime: " << e.what() << "\n";        return 3;
    } catch (const exception& e) {
        cerr << "[main] Exceptie: " << e.what() << "\n";              return 4;
    }

    return 0;
}

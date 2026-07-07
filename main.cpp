#include "scheduler/Scheduler.h"
#include "scheduler/policies/PriorityPolicy.h"
#include "scheduler/policies/RateMonotonicPolicy.h"
#include "scheduler/policies/DeadlineMonotonicPolicy.h"
#include "scheduler/policies/EDFPolicy.h"
#include "scheduler/stats/Stats.h"
#include "scheduler/stats/EventQueue.h"
#include "scheduler/trace/CsvTraceSink.h"
#include "tasks/TaskSetLoader.h"
#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <string_view>
#include <stdexcept>
#include <limits>
#include <ctime>
#include <filesystem>

using namespace std;

// durata implicita = o hiperperioada a task set-ului default (configs/default.csv)
static constexpr int DEFAULT_DURATION = 600;
// task set-ul folosit cand nu se da --taskset
static constexpr std::string_view DEFAULT_TASKSET = "configs/default.csv";

static bool readInt(int& val) {
    if (!(cin >> val)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return false;
    }
    return true;
}

// creeaza un policy dupa un tag (nume scurt sau numar de meniu)
static unique_ptr<SchedulingPolicy> makePolicy(const string& tag) {
    if (tag == "1" || tag == "priority")                         return make_unique<PriorityPolicy>();
    if (tag == "2" || tag == "rm" || tag == "ratemonotonic")     return make_unique<RateMonotonicPolicy>();
    if (tag == "3" || tag == "edf")                              return make_unique<EDFPolicy>();
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

// tag scurt pt numele fisierului de trace (ex. "Rate Monotonic" -> "rm")
static string policyTag(const string& name) {
    if (name == "EDF") return "edf";
    if (name == "Rate Monotonic") return "rm";
    if (name == "DeadlineMonotonic") return "dm";
    if (name == "Priority") return "priority";
    return name;
}

static string timestamp() {
    time_t now = time(nullptr);
    char buf[32];
    strftime(buf, sizeof(buf), "%Y%m%d_%H%M%S", localtime(&now));
    return string(buf);
}

// numele fisierului fara director si fara extensie (configs/ts1.csv -> ts1)
static string baseNoExt(const string& path) {
    return filesystem::path(path).stem().string();
}

// ruleaza o simulare completa: incarca taskset-ul, ruleaza policy-ul, scrie trace-ul
static void runSimulation(SchedulingPolicy* policy, const string& taskset_path,
                          const string& out_path, int duration) {
    // incarc taskurile inainte de a crea sink-ul, ca un CSV invalid sa nu lase un trace gol
    vector<unique_ptr<Task>> tasks = TaskSetLoader::load(taskset_path, duration);

    EventQueue queue;
    Stats stats;
    auto sink = make_unique<CsvTraceSink>(out_path);

    for (auto& t : tasks)
        stats.registerTask(t->getId(), t->getName(), t->getType());

    Scheduler sched(policy, &stats, &queue, sink.get());
    for (auto& t : tasks) sched.addTask(t.get());  // scheduler-ul nu detine taskurile

    sched.run(duration);

    cout << "\n=== Simulare terminata (" << policy->getName() << ", " << baseNoExt(taskset_path) << ") ===\n";
    cout << "CPU%: " << stats.getCpuUtilization()
         << "  |  Deadline misses: " << stats.getTotalDeadlineMisses()
         << "  |  Preemptions: " << stats.getTotalPreemptions() << "\n";
    cout << "Trace scris in: " << out_path << "\n";
}

// alege calea de output: --out explicit, altfel derivat din taskset+policy, altfel timestamp
static string resolveOut(const string& out_arg, const string& taskset_arg,
                         SchedulingPolicy* policy) {
    if (!out_arg.empty()) return out_arg;
    if (!taskset_arg.empty())
        return "traces/" + baseNoExt(taskset_arg) + "_" + policyTag(policy->getName()) + ".csv";
    return "traces/trace_" + policyTag(policy->getName()) + "_" + timestamp() + ".csv";
}

static void printUsage(const char* prog) {
    cout << "Utilizare: " << prog << " [--policy=NAME] [--taskset=PATH] [--out=PATH] [--duration=N]\n"
         << "  --policy=NAME   priority | rm | edf | dm  (fara asta: meniu interactiv)\n"
         << "  --taskset=PATH  CSV cu task set-ul (fara asta: " << DEFAULT_TASKSET << ")\n"
         << "  --out=PATH      calea CSV de output (implicit derivat din taskset + policy)\n"
         << "  --duration=N    numar de tickuri (implicit " << DEFAULT_DURATION << ")\n";
}

int main(int argc, char** argv) {
    string policy_arg, taskset_arg, out_arg;
    int duration = DEFAULT_DURATION;

    for (int i = 1; i < argc; i++) {
        string a = argv[i];
        if (a == "-h" || a == "--help") { printUsage(argv[0]); return 0; }
        else if (a.rfind("--policy=", 0) == 0)   policy_arg = a.substr(9);
        else if (a.rfind("--taskset=", 0) == 0)  taskset_arg = a.substr(10);
        else if (a.rfind("--out=", 0) == 0)      out_arg = a.substr(6);
        else if (a.rfind("--duration=", 0) == 0) duration = stoi(a.substr(11));
        else { cerr << "Argument necunoscut: " << a << "\n"; printUsage(argv[0]); return 2; }
    }

    string taskset = taskset_arg.empty() ? string(DEFAULT_TASKSET) : taskset_arg;

    try {
        if (duration <= 0)
            throw invalid_argument("--duration trebuie sa fie > 0");

        if (!policy_arg.empty()) {
            // mod non-interactiv: o singura rulare (folosit de run_experiments.py)
            auto policy = makePolicy(policy_arg);
            if (!policy) { cerr << "Policy necunoscut: " << policy_arg << "\n"; printUsage(argv[0]); return 2; }
            runSimulation(policy.get(), taskset, resolveOut(out_arg, taskset_arg, policy.get()), duration);
            return 0;
        }

        // mod interactiv: alege policy, ruleaza pe task set-ul default, repeta
        while (true) {
            auto policy = selectPolicyInteractive();
            if (!policy) break;
            runSimulation(policy.get(), taskset, resolveOut(out_arg, taskset_arg, policy.get()), duration);
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

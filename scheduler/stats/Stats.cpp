#include "Stats.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include "../tasks/PeriodicTask.h"

static std::string stateToString(TaskState s) {
    switch (s) {
        case TaskState::Inactive:  return "inactive";
        case TaskState::Ready:     return "ready";
        case TaskState::Running:   return "running";
        case TaskState::Blocked:   return "blocked";
        case TaskState::Finished:  return "finished";
        case TaskState::Missed:    return "missed";
        default:                   return "unknown";
    }
}
Stats::Stats()
    : active_ticks(0), idle_ticks(0),
      total_preemptions(0), total_deadline_misses(0) {}

Stats::Stats(const Stats& other)
    : active_ticks(other.active_ticks),
      idle_ticks(other.idle_ticks),
      total_preemptions(other.total_preemptions),
      total_deadline_misses(other.total_deadline_misses),
      per_task(other.per_task),
      snapshot_log(other.snapshot_log) {}

Stats& Stats::operator=(const Stats& other) {
    if (this == &other) return *this;
    active_ticks = other.active_ticks;
    idle_ticks = other.idle_ticks;
    total_preemptions = other.total_preemptions;
    total_deadline_misses = other.total_deadline_misses;
    per_task = other.per_task;
    snapshot_log = other.snapshot_log;
    return *this;
}

int Stats::getActiveTicks() const { return active_ticks; }
int Stats::getIdleTicks() const { return idle_ticks; }
int Stats::getTotalPreemptions() const { return total_preemptions; }
int Stats::getTotalDeadlineMisses() const { return total_deadline_misses; }

double Stats::getCpuUtilization() const {
    int total = active_ticks + idle_ticks; 
    if (total == 0) return 0.0;
    return 100.0 * active_ticks / total;
}

const std::unordered_map<int, TaskStats>& Stats::getPerTask() const {
    return per_task; // intoarce referinta la map-ul
}

//merita explorata diferenta dintre metoda computeUtilization si getCPUUtilization
//cea din urma este masurata la runtime, active/total, este post-factum
//pt metoda de aici, este un test teoretic inainte de runtime pt logica aplicatiei
//niciun policy nu salveaza un U > 1.0, am misses garantate, U - cpu_util = cat am pierdut in missuri la deadline
double Stats::computeUtilization(const std::vector<Task*>& tasks) {
    double total = 0.0;
    for (Task* t : tasks) {
        // dynamic_cast pentru ca doar periodicele si sporadicele au perioada
        // sporadicele mostenesc PeriodicTask, deci cast-ul prinde si pe ele
        auto periodic = dynamic_cast<PeriodicTask*>(t);
        if (!periodic) continue;  // skip aperiodic
        
        int period = periodic->getPeriod();
        if (period <= 0) continue;  // sanity check
        
        total += (static_cast<double>(t->getWCET()) / period);
    }
    return total;
}


void Stats::registerTask(int task_id, const std::string& name, const std::string& type) {
    per_task[task_id] = TaskStats(name, type);
}

void Stats::onRelease(int task_id) {
    if (!per_task.count(task_id))
        throw std::invalid_argument("[Stats] onRelease: task_id " + std::to_string(task_id) + " neinregistrat");
    per_task[task_id].onRelease();
}

void Stats::onPreempt(int task_id) {
    if (!per_task.count(task_id))
        throw std::invalid_argument("[Stats] onPreempt: task_id " + std::to_string(task_id) + " neinregistrat");
    per_task[task_id].onPreempt();
    total_preemptions++;
}

void Stats::onComplete(int task_id, int response_time) {
    if (!per_task.count(task_id))
        throw std::invalid_argument("[Stats] onComplete: task_id " + std::to_string(task_id) + " neinregistrat");
    per_task[task_id].onComplete(response_time);
}

void Stats::onDeadlineMiss(int task_id) {
    if (!per_task.count(task_id))
        throw std::invalid_argument("[Stats] onDeadlineMiss: task_id " + std::to_string(task_id) + " neinregistrat");
    per_task[task_id].onDeadlineMiss();
    total_deadline_misses++;
}

void Stats::onTick(bool cpu_active) {
    if (cpu_active) active_ticks++;
    else idle_ticks++;
}

void Stats::recordSnapshot(int tick, const std::string& cpu_task, double cpu_util, const std::vector<Task*>& tasks) {
    for (Task* t : tasks)
        snapshot_log.push_back({tick, cpu_task, cpu_util, t->getId(), t->getName(), stateToString(t->getState())});
}

void Stats::exportSnapshotCSV(const std::string& filename) const {
    std::ofstream fout(filename);
    if (!fout.is_open())
        throw std::runtime_error("Nu pot deschide fisierul pentru snapshot: " + filename);
    try {
        fout << "Tick,CPU_Task,CPU_Util%,TaskID,Name,State\n";
        for (const auto& row : snapshot_log)
            fout << row.tick << "," << row.cpu_task << "," << row.cpu_util
                 << "," << row.task_id << "," << row.task_name << "," << row.state << "\n";
        fout.close();
        if (fout.fail())
            throw std::runtime_error("Eroare la scriere snapshot: " + filename);
    } catch (...) {
        fout.close();
        throw;
    }
}

std::vector<SnapshotRow> Stats::getSnapshotAt(const std::string& filename, int tick) {
    std::ifstream fin(filename);
    if (!fin.is_open())
        throw std::runtime_error("Nu pot deschide fisierul snapshot: " + filename);

    std::string line;
    std::getline(fin, line); // skip header

    int best_tick = -1;
    std::vector<SnapshotRow> all_rows;

    while (std::getline(fin, line)) {
        std::istringstream ss(line);
        std::string token;
        SnapshotRow row;
        std::getline(ss, token, ',');   row.tick = std::stoi(token);
        std::getline(ss, row.cpu_task, ',');
        std::getline(ss, token, ',');   row.cpu_util = std::stod(token);
        std::getline(ss, token, ',');   row.task_id = std::stoi(token);
        std::getline(ss, row.task_name, ',');
        std::getline(ss, row.state);
        if (row.tick <= tick && row.tick > best_tick)
            best_tick = row.tick;
        all_rows.push_back(row);
    }

    if (best_tick == -1) return {};
    std::vector<SnapshotRow> result;
    for (const auto& r : all_rows)
        if (r.tick == best_tick) result.push_back(r);
    return result;
}


std::ostream& operator<<(std::ostream& out, const Stats& s) {
    out << "=== Simulation Report ===\n";
    out << "CPU utilization: " << s.getCpuUtilization() << "%\n";
    out << "Active ticks: " << s.active_ticks
    
    << " | Idle ticks: " << s.idle_ticks << "\n";
    out << "Total preemptions: " << s.total_preemptions << "\n";
    out << "Total deadline misses: " << s.total_deadline_misses << "\n";
    out << "--- Per task ---\n";
    for (const auto& pair : s.per_task) {
        out << "  [id=" << pair.first << "] " << pair.second << "\n";
    }
    return out;
}

std::istream& operator>>(std::istream& in, Stats& s) {
    // reset si citeste cate taskuri sa inregistreze
    int n;
    in >> n;
    for (int i = 0; i < n; i++) {
        int id;
        TaskStats ts;
        in >> id >> ts;
        s.per_task[id] = ts;
    }
    return in;
}
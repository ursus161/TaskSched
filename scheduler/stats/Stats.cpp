#include "Stats.h"
#include <fstream>
#include <stdexcept>
#include "../tasks/PeriodicTask.h"
Stats::Stats()
    : active_ticks(0), idle_ticks(0),
      total_preemptions(0), total_deadline_misses(0) {}

Stats::Stats(const Stats& other)
    : active_ticks(other.active_ticks),
      idle_ticks(other.idle_ticks),
      total_preemptions(other.total_preemptions),
      total_deadline_misses(other.total_deadline_misses),
      per_task(other.per_task) {}

Stats& Stats::operator=(const Stats& other) {
    if (this == &other) return *this;
    active_ticks = other.active_ticks;
    idle_ticks = other.idle_ticks;
    total_preemptions = other.total_preemptions;
    total_deadline_misses = other.total_deadline_misses;
    per_task = other.per_task;
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
    try {
        per_task.at(task_id).onRelease();
    } catch (const std::out_of_range&) {
        std::cerr << "[Stats] Warning: onRelease - task_id " << task_id << " neinregistrat, eveniment ignorat\n";
    }
}

void Stats::onPreempt(int task_id) {
    try {
        per_task.at(task_id).onPreempt();
        total_preemptions++;
    } catch (const std::out_of_range&) {
        std::cerr << "[Stats] Warning: onPreempt - task_id " << task_id << " neinregistrat, eveniment ignorat\n";
    }
}

void Stats::onComplete(int task_id, int response_time) {
    try {
        per_task.at(task_id).onComplete(response_time);
    } catch (const std::out_of_range&) {
        std::cerr << "[Stats] Warning: onComplete - task_id " << task_id << " neinregistrat, eveniment ignorat\n";
    }
}

void Stats::onDeadlineMiss(int task_id) {
    try {
        per_task.at(task_id).onDeadlineMiss();
        total_deadline_misses++;
    } catch (const std::out_of_range&) {
        std::cerr << "[Stats] Warning: onDeadlineMiss - task_id " << task_id << " neinregistrat, eveniment ignorat\n";
    }
}

void Stats::onTick(bool cpu_active) {
    if (cpu_active) active_ticks++;
    else idle_ticks++;
}

void Stats::recordExecution(const std::string& task_name, int start, int end) {
    if (start < end) { 
        timeline.push_back({task_name, start, end});
    }
}

void Stats::exportToCSV(const std::string& filename) const {
    std::ofstream fout(filename);
    if (!fout.is_open()) {
        throw std::runtime_error("Nu pot deschide fisierul pentru export: " + filename);
    }
    try {
        fout << "Task,Start,End\n"; // orice csv are un header
        for (const auto& rec : timeline) {
            fout << rec.task_name << "," << rec.start_time << "," << rec.end_time << "\n";
        }
        fout.close();
        if (fout.fail()) {
            throw std::runtime_error("Eroare la scriere in fisier: " + filename);
        }
    } catch (...) {
        fout.close();
        throw;
    }
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
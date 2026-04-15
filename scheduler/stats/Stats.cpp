#include "Stats.h"
#include <exception>
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

void Stats::registerTask(int task_id, const std::string& name, const std::string& type) {
    per_task[task_id] = TaskStats(name, type);
}

void Stats::onRelease(int task_id) {
    per_task[task_id].onRelease();
}

void Stats::onPreempt(int task_id) {
    per_task[task_id].onPreempt();
    total_preemptions++;
}

void Stats::onComplete(int task_id, int response_time) {
    per_task[task_id].onComplete(response_time);
}

void Stats::onDeadlineMiss(int task_id) {
    per_task[task_id].onDeadlineMiss();
    total_deadline_misses++;
}

void Stats::onTick(bool cpu_active) {
    if (cpu_active) active_ticks++;
    else idle_ticks++;
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
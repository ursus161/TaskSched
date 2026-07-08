#include "Stats.h"
#include <stdexcept>
#include "../tasks/PeriodicTask.h"

Stats::Stats()
    : active_ticks(0), idle_ticks(0),
      total_preemptions(0), total_deadline_misses(0), total_drops(0) {}

Stats::Stats(const Stats& other)
    : active_ticks(other.active_ticks),
      idle_ticks(other.idle_ticks),
      total_preemptions(other.total_preemptions),
      total_deadline_misses(other.total_deadline_misses),
      total_drops(other.total_drops),
      per_task(other.per_task) {}

Stats& Stats::operator=(const Stats& other) {
    if (this == &other) return *this;
    active_ticks = other.active_ticks;
    idle_ticks = other.idle_ticks;
    total_preemptions = other.total_preemptions;
    total_deadline_misses = other.total_deadline_misses;
    total_drops = other.total_drops;
    per_task = other.per_task;
    return *this;
}

int Stats::getActiveTicks() const { return active_ticks; }
int Stats::getIdleTicks() const { return idle_ticks; }
int Stats::getTotalPreemptions() const { return total_preemptions; }
int Stats::getTotalDeadlineMisses() const { return total_deadline_misses; }
int Stats::getTotalDrops() const { return total_drops; }

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

void Stats::onDrop(int task_id) {
    if (!per_task.count(task_id))
        throw std::invalid_argument("[Stats] onDrop: task_id " + std::to_string(task_id) + " neinregistrat");
    per_task[task_id].onDrop();
    total_drops++;
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
    out << "Total drops (joburi tardy abandonate): " << s.total_drops << "\n";
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
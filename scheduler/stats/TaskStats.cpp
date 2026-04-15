#include "TaskStats.h"
#include <exception>

TaskStats::TaskStats()
    : jobs_released(0), jobs_completed(0), deadline_misses(0),
      preemptions(0), total_response_time(0), name(""), type("") {}

TaskStats::TaskStats(const std::string& name, const std::string& type)
    : jobs_released(0), jobs_completed(0), deadline_misses(0),
      preemptions(0), total_response_time(0), name(name), type(type) {}

TaskStats::TaskStats(const TaskStats& other)
    : jobs_released(other.jobs_released),
      jobs_completed(other.jobs_completed),
      deadline_misses(other.deadline_misses),
      preemptions(other.preemptions),
      total_response_time(other.total_response_time),
      name(other.name), type(other.type) {}

TaskStats& TaskStats::operator=(const TaskStats& other) {
    if (this == &other) return *this;
    jobs_released = other.jobs_released;
    jobs_completed = other.jobs_completed;
    deadline_misses = other.deadline_misses;
    preemptions = other.preemptions;
    total_response_time = other.total_response_time;
    name = other.name;
    type = other.type;
    return *this;
}

int TaskStats::getJobsReleased() const { return jobs_released; }
int TaskStats::getJobsCompleted() const { return jobs_completed; }
int TaskStats::getDeadlineMisses() const { return deadline_misses; }
int TaskStats::getPreemptions() const { return preemptions; }
int TaskStats::getTotalResponseTime() const { return total_response_time; }

double TaskStats::getAverageResponseTime() const {
    if (jobs_completed == 0) return 0.0;
    return static_cast<double>(total_response_time) / jobs_completed;
}

const std::string& TaskStats::getName() const { return name; }
const std::string& TaskStats::getType() const { return type; }

void TaskStats::onRelease() { jobs_released++; }

void TaskStats::onComplete(int response_time) {
    jobs_completed++;
    total_response_time += response_time;
}

void TaskStats::onDeadlineMiss() { deadline_misses++; }
void TaskStats::onPreempt() { preemptions++; }

std::ostream& operator<<(std::ostream& out, const TaskStats& ts) {
    out << ts.name << " (" << ts.type << "): "
        << "jobs=" << ts.jobs_completed << "/" << ts.jobs_released
        << " misses=" << ts.deadline_misses
        << " preempts=" << ts.preemptions
        << " avgResp=" << ts.getAverageResponseTime();
    return out;
}

std::istream& operator>>(std::istream& in, TaskStats& ts) {
    in >> ts.name >> ts.type;
    return in;
}
#include "AperiodicTask.h"

AperiodicTask::AperiodicTask(int id, const std::string& name, int priority,
                             int WCET, int deadline, int arrival_time)
    : Task(id, name, priority, WCET, deadline),
      arrival_time(arrival_time),
      released(false) {}

AperiodicTask::AperiodicTask(const AperiodicTask& other)

    : Task(other),
      arrival_time(other.arrival_time),
      released(other.released) {}

bool AperiodicTask::isReadyAt(int current_time) const {
    
    return !released && current_time >= arrival_time;
}

void AperiodicTask::release(int current_time) {
    //analog periodic task
    this->remaining_time = WCET;
    this->absolute_deadline = current_time + deadline;
    this->state = TaskState::Ready;
    //aici nu conteaza de cate ori a fost released
    this->released = true;
}

std::string AperiodicTask::getType() const {
    return "AperiodicTask";
}
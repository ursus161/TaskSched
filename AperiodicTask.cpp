#include "AperiodicTask.h"
using namespace std;

AperiodicTask::AperiodicTask(int id, const string& name, int priority,
                             int worstCaseExecutionTime, int deadline, int arrival_time)
    : Task(id, name, priority, worstCaseExecutionTime, deadline),
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
    this->remaining_time = worstCaseExecutionTime;
    this->absolute_deadline = current_time + deadline;
    this->state = TaskState::Ready;
    //aici nu conteaza de cate ori a fost released
    this->released = true;
}

string AperiodicTask::getType() const {
    return "AperiodicTask";
}
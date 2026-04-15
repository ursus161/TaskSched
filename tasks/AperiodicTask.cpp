#include "AperiodicTask.h"
#include <iostream>
using namespace std;

AperiodicTask::AperiodicTask(int id, const string& name, int priority,
                             int worstCaseExecutionTime, int deadline, int arrival_time)
    : Task(id, name, priority, worstCaseExecutionTime, deadline),
      arrival_time(arrival_time),
      released(false) {}

AperiodicTask::AperiodicTask()
:Task(), 
arrival_time(0),
released(false){}

AperiodicTask::AperiodicTask(const AperiodicTask& other)

    : Task(other),
      arrival_time(other.arrival_time),
      released(other.released) {}

AperiodicTask& AperiodicTask::operator=(const AperiodicTask& other) {
    if (this == &other) return *this;
    Task::operator=(other);
    arrival_time = other.arrival_time;
    released = other.released;
    return *this;
}

std::ostream& operator<<(std::ostream& out, const AperiodicTask& at) {
    out << static_cast<const Task&>(at)
        << " arrival=" << at.arrival_time
        << " released=" << at.released;
    return out;
}

std::istream& operator>>(std::istream& in, AperiodicTask& at) {
    in >> static_cast<Task&>(at) >> at.arrival_time;
    return in;
}

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
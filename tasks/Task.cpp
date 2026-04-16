#include "Task.h"
#include <iostream>
using namespace std;

Task::Task(int id, const string& name, int priority, int worstCaseExecutionTime, int deadline)
    :id(id),
     name(name), 
     priority(priority),
     worstCaseExecutionTime(worstCaseExecutionTime),
     deadline(deadline),
     state(TaskState::Inactive),
     remaining_time(0),
     absolute_deadline(0) {}

Task::Task(const Task& other)
    : id(other.id), name(other.name), priority(other.priority),
      worstCaseExecutionTime(other.worstCaseExecutionTime), deadline(other.deadline),
      state(other.state), remaining_time(other.remaining_time),
      absolute_deadline(other.absolute_deadline) {}

Task::Task() 
    : id(0), name(""), priority(0), worstCaseExecutionTime(0), deadline(0),
      state(TaskState::Inactive), remaining_time(0), absolute_deadline(0) {}

Task::~Task() {}


Task& Task::operator=(const Task& other) {
    if (this == &other) return *this;
    id = other.id;
    name = other.name;
    priority = other.priority;
    worstCaseExecutionTime = other.worstCaseExecutionTime;
    deadline = other.deadline;
    state = other.state;
    remaining_time = other.remaining_time;
    absolute_deadline = other.absolute_deadline;
    return *this;
}

std::ostream& operator<<(std::ostream& out, const Task& t) {
    out << "[id=" << t.id << " name=" << t.name
        << " priority=" << t.priority
        << " WCET=" << t.worstCaseExecutionTime
        << " deadline=" << t.deadline << "]";
    return out;
}

std::istream& operator>>(std::istream& in, Task& t) {
    cout<< "Task id: ";
    in >> t.id ;
    cout<< "Task name: ";
    in>> t.name ;
    cout<<"Task priority: ";
    in>> t.priority;
    cout<<"Task WCET: ";
    in>> t.worstCaseExecutionTime ;
    cout<<"Task deadline: ";
    in>> t.deadline;
    return in;
}


int Task::getId() const { return id; }

const string& Task::getName() const { return name; }

int Task::getPriority() const { return priority; }
int Task::getDeadline() const { return deadline; }
int Task::getWCET() const { return worstCaseExecutionTime;}

TaskState Task::getState() const { return state; }
int Task::getRemainingTime() const { return remaining_time; }
int Task::getAbsoluteDeadline() const { return absolute_deadline; }

void Task::setPriority(int p) { this->priority = p; }
void Task::setState(TaskState s) { this->state = s; }
void Task::setRemainingTime(int t) { this->remaining_time = t; }
void Task::setAbsoluteDeadline(int d) { this->absolute_deadline = d; }
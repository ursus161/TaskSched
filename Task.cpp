#include "Task.h"
using namespace std;

Task::Task(int id, const string& name, int priority, int worstCaseExecutionTime, int deadline)
    :id(id),
     name(name), 
     priority(priority),
     worstCaseExecutionTime(worstCaseExecutionTime),
     deadline(deadline),
     state(TaskState::Ready),
     remaining_time(0),
     absolute_deadline(0) {}

Task::Task(const Task& other)
    : id(other.id), name(other.name), priority(other.priority),
      worstCaseExecutionTime(other.worstCaseExecutionTime), deadline(other.deadline),
      state(other.state), remaining_time(other.remaining_time),
      absolute_deadline(other.absolute_deadline) {}

Task::Task() 
    : id(0), name(""), priority(0), worstCaseExecutionTime(0), deadline(0),
      state(TaskState::Ready), remaining_time(0), absolute_deadline(0) {}

Task::~Task() {}

int Task::getId() const { return id; }

const string& Task::getName() const { return name; }

int Task::getPriority() const { return priority; }
int Task::getDeadline() const { return deadline; }

TaskState Task::getState() const { return state; }
int Task::getRemainingTime() const { return remaining_time; }
int Task::getAbsoluteDeadline() const { return absolute_deadline; }

void Task::setPriority(int p) { this->priority = p; }
void Task::setState(TaskState s) { this->state = s; }
void Task::setRemainingTime(int t) { this->remaining_time = t; }
void Task::setAbsoluteDeadline(int d) { this->absolute_deadline = d; }
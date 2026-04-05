#include "Task.h"

Task::Task(int id, const std::string& name, int priority, int WCET, int deadline)
    :id(id),
     name(name), 
     priority(priority),
     WCET(WCET),
     deadline(deadline),
     state(TaskState::Ready),
     remaining_time(0),
     absolute_deadline(0) {}


Task::Task() 
    : id(0), name(""), priority(0), WCET(0), deadline(0),
      state(TaskState::Ready), remaining_time(0), absolute_deadline(0) {}

Task::~Task() {}

int Task::getId() const { return id; }

const std::string& Task::getName() const { return name; }

int Task::getPriority() const { return priority; }
int Task::getDeadline() const { return deadline; }

TaskState Task::getState() const { return state; }
int Task::getRemainingTime() const { return remaining_time; }
int Task::getAbsoluteDeadline() const { return absolute_deadline; }

void Task::setPriority(int p) { this->priority = p; }
void Task::setState(TaskState s) { this->state = s; }
void Task::setRemainingTime(int t) { this->remaining_time = t; }
void Task::setAbsoluteDeadline(int d) { this->absolute_deadline = d; }
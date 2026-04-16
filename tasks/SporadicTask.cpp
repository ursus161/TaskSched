#include "SporadicTask.h"
#include <iostream>
using namespace std;

SporadicTask::SporadicTask(int id, const string& name, int priority,
                           int worstCaseExecutionTime, int deadline, int minimumInterArrivalTime,
                           const vector<int>& triggers)

    : Task(id, name, priority, worstCaseExecutionTime, deadline),
      PeriodicTask(id, name, priority, worstCaseExecutionTime, deadline, minimumInterArrivalTime, first_release= 0),
      AperiodicTask(id, name, priority, worstCaseExecutionTime, deadline, 0),
      minimumInterArrivalTime(minimumInterArrivalTime),
      last_activation(-minimumInterArrivalTime),
      triggers(triggers),
      next_trigger_index(0) {}

SporadicTask::SporadicTask():
Task(),
PeriodicTask(),
AperiodicTask(),
minimumInterArrivalTime(0),
last_activation(0),
triggers({}),
next_trigger_index(0) {}

SporadicTask::SporadicTask(const SporadicTask& other)
    : Task(other),
    
      PeriodicTask(other),
      AperiodicTask(other),
      minimumInterArrivalTime(other.minimumInterArrivalTime),
      last_activation(other.last_activation),
      triggers(other.triggers),
      next_trigger_index(other.next_trigger_index) {}

bool SporadicTask::isReadyAt(int current_time) const {
    if (next_trigger_index >= (int)triggers.size()) return false;
    if (current_time < triggers[next_trigger_index]) return false;
    if (current_time - last_activation < minimumInterArrivalTime) return false;
    return true;
}


SporadicTask& SporadicTask::operator=(const SporadicTask& other) {
    if (this == &other) return *this;
    Task::operator=(other);              // baza
    PeriodicTask::operator=(other);      // copiaza partea de PeriodicTask
    AperiodicTask::operator=(other);     // copiaza partea de AperiodicTask

    minimumInterArrivalTime = other.minimumInterArrivalTime;
    last_activation = other.last_activation;
    triggers = other.triggers;
    next_trigger_index = other.next_trigger_index;

    return *this;
}

std::ostream& operator<<(std::ostream& out, const SporadicTask& st) {
    out << static_cast<const Task&>(st)
        << " period=" << st.period
        << " arrival=" << st.arrival_time
        << " MIT=" << st.minimumInterArrivalTime
        << " triggers=" << st.triggers.size();
    return out;
}

std::istream& operator>>(std::istream& in, SporadicTask& st) {
    in >> static_cast<Task&>(st)
       >> st.period
       >> st.arrival_time
       >> st.minimumInterArrivalTime;
    
    int n;
    in >> n;
    st.triggers.clear();
    for (int i = 0; i < n; i++) {
        int trigger;
        in >> trigger;
        st.triggers.push_back(trigger);
    }
    return in;
}

void SporadicTask::release(int current_time) {
    remaining_time = worstCaseExecutionTime;
    absolute_deadline = current_time + deadline;
    state = TaskState::Ready;
    last_activation = current_time;
    next_trigger_index++;
}

string SporadicTask::getType() const {
    return "SporadicTask";
}
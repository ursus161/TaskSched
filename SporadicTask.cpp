#include "SporadicTask.h"
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
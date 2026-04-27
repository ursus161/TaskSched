#include <string>
#include <iostream>
#include <stdexcept>
#include "PeriodicTask.h"
using namespace std;

PeriodicTask::PeriodicTask(const string& name, int priority,
                 int worstCaseExecutionTime, int deadline, int period, int first_release ) :

                 Task(name,priority,worstCaseExecutionTime,deadline),

                period(period),
                first_release(first_release),
                jobs_released(0) {
    if (period <= 0)
        throw std::invalid_argument("PeriodicTask '" + name + "': perioada trebuie sa fie > 0");
    if (worstCaseExecutionTime > period)
        throw std::invalid_argument("PeriodicTask '" + name + "': WCET (" + std::to_string(worstCaseExecutionTime) +
                                    ") depaseste perioada (" + std::to_string(period) + ")");
};

PeriodicTask::PeriodicTask()
:Task(),
period(0),
first_release(0),
jobs_released(0)  { };

PeriodicTask::PeriodicTask(const PeriodicTask& other)

    : Task(other),
      period(other.period),
      first_release(other.first_release),
      jobs_released(other.jobs_released) {}
 
      

PeriodicTask& PeriodicTask::operator=(const PeriodicTask& other) {
    if (this == &other) return *this;
    Task::operator=(other);   // apeleaza operator= al bazei pentru campurile mostenite, analog constructorilor de mai sus
    period = other.period;
    first_release = other.first_release;
    jobs_released = other.jobs_released;
    return *this;
}

std::ostream& operator<<(std::ostream& out, const PeriodicTask& pt) {
    out << static_cast<const Task&>(pt)   // apeleaza operator<< al bazei
        << " period=" << pt.period
        << " jobs_released=" << pt.jobs_released;
    return out;
}

std::istream& operator>>(std::istream& in, PeriodicTask& pt) {
    in >> static_cast<Task&>(pt) >> pt.period; // refolosesc cod ca mai sus ( multumim polimorfism )
    
    return in;
}


int PeriodicTask::getPeriod() const { 
    return this->period;
}
bool PeriodicTask::isReadyAt(int current_time) const{ 

    return current_time >= first_release + jobs_released * period;
    //logica : first_release + jobs_released * period; momentan urmatorului release
    // care inca nu a avut loc
    // am jobs_released = 0 urmatorul e la first_release + 0 * period = first_release
    // task ul e ready daca current time a atins sau depasit momentul current_time

};

void PeriodicTask::release(int current_time){
    // reporneste taskul, intra in ready queue daca e valid de metoda anterior definita

     this->remaining_time = worstCaseExecutionTime;
    this->absolute_deadline = current_time + deadline;
   this->state = TaskState::Ready;
    this->jobs_released++ ; 

}

string PeriodicTask::getType() const{

    return "PeriodicTask";
    
    //fiind doar citit, as fi putut opta pentru string_view 
    //avand in vedere marimea redusa a string ului returnat, compilator face small string optimization
    // deci se va aloca pe stiva, nu pe heap, overhead ul fata de pointerul in cazul string_view este aproape nul

}
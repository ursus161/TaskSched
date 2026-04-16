#include "Scheduler.h"
#include "Task.h"
#include <iostream>
using namespace std;

Scheduler::Scheduler()
    : policy(nullptr),
      current_running(nullptr),
      current_time(0),
      ready_queue(PolicyComparator{nullptr}) {}


Scheduler::Scheduler(SchedulingPolicy* policy, Stats* stats)
    : policy(policy),
      stats(stats),
      current_running(nullptr),
      current_time(0),
      ready_queue(PolicyComparator{policy}) {}


Scheduler::Scheduler(Scheduler& sched)
    : policy(sched.policy),
      tasks(sched.tasks),
      current_running(sched.current_running),
      current_time(sched.current_time),
      ready_queue(sched.ready_queue) {}

   
      
Scheduler& Scheduler::operator=(const Scheduler& other) {
    if (this == &other) return *this;
    policy = other.policy;
    stats = other.stats;
    tasks = other.tasks;
    current_running = other.current_running;
    current_time = other.current_time;
    ready_queue = other.ready_queue;
    return *this;
}

std::ostream& operator<<(std::ostream& out, const Scheduler& sched) {
    out << "Scheduler["
        << "algorithm=" << (sched.policy ? sched.policy->getName() : "none")
        << " time=" << sched.current_time
        << " tasks=" << sched.tasks.size()
        << " running=" << (sched.current_running ? sched.current_running->getName() : "idle")
        << "]";
    return out;
}

std::istream& operator>>(std::istream& in, Scheduler& sched) {
    // nu prea are sens sa citesti un scheduler intreg de la tastatura
    // citesti cate taskuri apoi doar numarul lor (taskurile se adauga cu addTask)
    return in;
}


void Scheduler::addTask(Task* task) {
    tasks.push_back(task); //adaug in pq
}


void Scheduler::setPolicy(SchedulingPolicy* p) {

    policy = p;
    // recreeaza ready_queue cu noul comparator, reinitializez depinde de usecase

    ready_queue = std::priority_queue<Task*, std::vector<Task*>, PolicyComparator>(PolicyComparator{p});
}

void Scheduler::dispatch(Task* new_running) {   
    if (current_running != nullptr) {
        stats->recordExecution(current_running->getName(), task_start_time, current_time);

        current_running->setState(TaskState::Ready);
        ready_queue.push(current_running);//asta e partea de preemptie
    }
    current_running = new_running;
    current_running->setState(TaskState::Running); // si asta e partea de dispatch      
    ready_queue.pop();
    stats->onPreempt(current_running->getId());   

    task_start_time = current_time;
}

void Scheduler::run(int duration) {
    for (current_time = 0; current_time < duration; current_time++) {
        // verifica ce taskuri devin ready acum + deadline miss check
        for (Task* t : tasks) {
                    // release pentru taskuri care devin ready
             if (t->isReadyAt(current_time) && t->getState() != TaskState::Ready  && t->getState() != TaskState::Running) {
                                
                        t->release(current_time); //devine disponibil ptr pq, imi reseteaza si toate datele taskului
                        ready_queue.push(t);
                        stats->onRelease(t->getId()); //increment la contorul intern
                }
                            // deadline miss check pentru taskuri active
                if (t->getState() != TaskState::Finished && t->getState() != TaskState::Inactive && current_time > t->getAbsoluteDeadline()) {
                            
                        stats->onDeadlineMiss(t->getId()); //la fel, prelucrez statisticile in acest caz
                      
                        //marchez finished si ies 
                        t->setState(TaskState::Finished); 
                        if (current_running == t) current_running = nullptr;
                }
        }
        //decizia de scheduling
                if (!ready_queue.empty()) {

                    Task* top = ready_queue.top();

                    if (current_running == nullptr || policy->isHigherPriority(top, current_running))  {  dispatch(top);  } //context switch
                }
                        //executa 1 tick din taskul curent
                if (current_running != nullptr) {


                current_running->setRemainingTime(current_running->getRemainingTime() - 1);

                stats->onTick(true); //adica acest task a fost activ in acest tick, util ptr cpu% and stuff like that

                cout << "[t=" << current_time << "] running task "
                << current_running->getId() << " ("
                << current_running->getName() << ")" << endl;
                
                if (current_running->getRemainingTime() == 0) { //e gata jobul taskului

                        int response_time = (current_time + 1) - (current_running->getAbsoluteDeadline() - current_running->getDeadline());

                        stats->onComplete(current_running->getId(), response_time);

                        stats->recordExecution(current_running->getName(), task_start_time, current_time + 1);

                        current_running->setState(TaskState::Finished);
                        current_running = nullptr;
                }
          } else {//asta e cpu-idle path
                        stats->onTick(false);   
                        cout << "[t=" << current_time << "] CPU idle" << endl;
                }
         }
   }
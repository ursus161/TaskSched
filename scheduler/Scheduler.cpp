#include "Scheduler.h"
#include "Task.h"
#include <thread>
#include <chrono>
#include <iostream>
#include <stdexcept>
using namespace std;

Scheduler::Scheduler()
    : policy(nullptr),
      current_running(nullptr),
      current_time(0),
      ready_queue(PolicyComparator{nullptr}) {}


Scheduler::Scheduler(SchedulingPolicy* policy, Stats* stats, EventQueue* event_queue)
    : policy(policy),
      stats(stats),
      event_queue(event_queue),
      current_running(nullptr),
      current_time(0),
      ready_queue(PolicyComparator{policy}) {}


Scheduler::Scheduler( const Scheduler& sched)
    : policy(sched.policy),
      stats(sched.stats),
      event_queue(sched.event_queue),
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
    if (!task)
        throw std::invalid_argument("Scheduler::addTask: pointer null, taskul nu a fost creat");
    tasks.push_back(task); //adaug in pq
}


void Scheduler::setPolicy(SchedulingPolicy* p) {

    policy = p;
    // recreeaza ready_queue cu noul comparator, reinitializez depinde de usecase

    ready_queue = std::priority_queue<Task*, std::vector<Task*>, PolicyComparator>(PolicyComparator{p});
}

void Scheduler::dispatch(Task* new_running) {
    if (!new_running)
        throw std::logic_error("Scheduler::dispatch: incercare de dispatch cu task null");
    if (current_running != nullptr) {
        stats->recordExecution(current_running->getName(), task_start_time, current_time);

        current_running->setState(TaskState::Ready);
        ready_queue.push(current_running);//asta e partea de preemptie

        event_queue->push({EventType::Preempt, current_time, current_running->getId(), current_running->getName()});
        stats->onPreempt(current_running->getId());
    }
    current_running = new_running;
    current_running->setState(TaskState::Running); // si asta e partea de dispatch
    ready_queue.pop();
    event_queue->push({EventType::Dispatch, current_time, current_running->getId(), current_running->getName()}); //adaug in event_queue evenimentul cand este dat jos din capul pq
    task_start_time = current_time;
    
}



void Scheduler::run(int duration) {
    if (!policy)      throw std::runtime_error("Scheduler: nicio politica setata");
    if (!stats)       throw std::runtime_error("Scheduler: stats neinitializat");
    if (!event_queue) throw std::runtime_error("Scheduler: event_queue neinitializata");
    if (duration <= 0)
        throw std::invalid_argument("Scheduler::run: durata trebuie sa fie > 0 (primit: " + std::to_string(duration) + ")");
    try {
    for (current_time = 0; current_time < duration; current_time++) {


        // verifica ce taskuri devin ready acum + deadline miss check
        for (Task* t : tasks) {

            // release pentru taskuri care devin ready
            if (t->isReadyAt(current_time) && t->getState() != TaskState::Ready && t->getState() != TaskState::Running) {
             
                t->release(current_time); //devine disponibil ptr pq, imi reseteaza si toate datele taskului
              
                event_queue->push({EventType::Release, current_time, t->getId(), t->getName()});
                ready_queue.push(t);
             
             
                stats->onRelease(t->getId()); //increment la contorul intern
            }

            // deadline miss check pentru taskuri active
            // soft real-time: taskul continua sa ruleze dupa miss, doar inregistram in statistici o singura data
            if (t->getState() != TaskState::Finished && t->getState() != TaskState::Inactive 
                && t->getState() != TaskState::Missed && current_time > t->getAbsoluteDeadline()) {
                event_queue->push({EventType::DeadlineMiss, current_time, t->getId(), t->getName()});
                
                stats->onDeadlineMiss(t->getId()); //la fel, prelucrez statisticile in acest caz
                t->setState(TaskState::Missed); //marchez starea ca sa nu mai raportez acelasi miss la tick-urile urmatoare
            }
        }
        //decizia de scheduling
        if (!ready_queue.empty()) {
            Task* top = ready_queue.top();
            if (current_running == nullptr || policy->isHigherPriority(top, current_running)) {
                dispatch(top);
            } //context switch
        }


        //executa 1 tick din taskul curent
        if (current_running != nullptr) {
            current_running->setRemainingTime(current_running->getRemainingTime() - 1);
            stats->onTick(true); //adica acest task a fost activ in acest tick, util ptr cpu% and stuff like that
 
            
            if (current_running->getRemainingTime() == 0) { //e gata jobul taskului
                event_queue->push({EventType::Complete, current_time, current_running->getId(), current_running->getName()});
              
              
                int response_time = (current_time + 1) - (current_running->getAbsoluteDeadline() - current_running->getDeadline());
                stats->onComplete(current_running->getId(), response_time);
                stats->recordExecution(current_running->getName(), task_start_time, current_time + 1);
                current_running->setState(TaskState::Finished);
                current_running = nullptr;
            }   
        } else { //asta e cpu-idle path
            stats->onTick(false);
        }
            event_queue->push({EventType::Tick, current_time, -1, "idle"});

      std::this_thread::sleep_for(std::chrono::milliseconds(150)); 
    }
    event_queue->push({EventType::EndOfSimulation, current_time, -1, ""}); // -1 la task id pt ca nu mai ruleaza nimic
    } catch (const std::exception& e) {
        std::cerr << "[Scheduler] Eroare in simulare la t=" << current_time << ": " << e.what() << "\n";
        event_queue->push({EventType::EndOfSimulation, current_time, -1, ""}); // oprire curata si in caz de exceptie
        throw;
    }
}
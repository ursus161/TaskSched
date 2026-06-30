#include "Scheduler.h"
#include "queue/HeapReadyQueue.h"
#include "queue/LinearReadyQueue.h"
#include "Task.h"
#include <thread>
#include <chrono>
#include <iostream>
#include <stdexcept>
using namespace std;

std::unique_ptr<ReadyQueue> Scheduler::makeQueue(SchedulingPolicy* p) {
    if (p->isDynamic())
        return std::make_unique<LinearReadyQueue>(p);
    return std::make_unique<HeapReadyQueue>(p);
}

Scheduler::Scheduler()
    : policy(nullptr),
      stats(nullptr),
      event_queue(nullptr),
      current_running(nullptr),
      current_time(0),
      ready_queue(nullptr) {}


Scheduler::Scheduler(SchedulingPolicy* policy, Stats* stats, EventQueue* event_queue)
    : policy(policy),
      stats(stats),
      event_queue(event_queue),
      current_running(nullptr),
      current_time(0),
      ready_queue(makeQueue(policy)) {}


Scheduler::Scheduler(const Scheduler& sched)
    : policy(sched.policy),
      stats(sched.stats),
      event_queue(sched.event_queue),
      tasks(sched.tasks),
      current_running(sched.current_running),
      current_time(sched.current_time),
      ready_queue(sched.policy ? makeQueue(sched.policy) : nullptr) {}


Scheduler& Scheduler::operator=(const Scheduler& other) {
    if (this == &other) return *this;
    policy        = other.policy;
    stats         = other.stats;
    event_queue   = other.event_queue;
    tasks         = other.tasks;
    current_running = other.current_running;
    current_time  = other.current_time;
    ready_queue   = other.policy ? makeQueue(other.policy) : nullptr;
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
    return in;
}


void Scheduler::addTask(Task* task) {
    if (!task)
        throw std::invalid_argument("Scheduler::addTask: pointer null, taskul nu a fost creat");
    tasks.push_back(task);
}


void Scheduler::setPolicy(SchedulingPolicy* p) {
    policy = p;
    ready_queue = makeQueue(p);
}

void Scheduler::dispatch(Task* new_running) {
    if (!new_running)
        throw std::logic_error("Scheduler::dispatch: incercare de dispatch cu task null");
    if (current_running != nullptr) {
        current_running->setState(TaskState::Ready);
        ready_queue->push(current_running);
        event_queue->push({EventType::Preempt, current_time, current_running->getId(), current_running->getName()});
        stats->onPreempt(current_running->getId());
    }
    current_running = new_running;
    current_running->setState(TaskState::Running);
    ready_queue->pop();
    event_queue->push({EventType::Dispatch, current_time, current_running->getId(), current_running->getName()});
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
        bool state_changed = false;

        // verifica ce taskuri devin ready acum + deadline miss check
        for (Task* t : tasks) {

            if (t->isReadyAt(current_time) && t->getState() != TaskState::Ready && t->getState() != TaskState::Running) {
                t->release(current_time);
                event_queue->push({EventType::Release, current_time, t->getId(), t->getName()});
                ready_queue->push(t);
                stats->onRelease(t->getId());
                state_changed = true;
            }

            // soft real-time: taskul continua sa ruleze dupa miss, doar inregistram o singura data
            if (t->getState() != TaskState::Finished && t->getState() != TaskState::Inactive
                && t->getState() != TaskState::Missed && current_time > t->getAbsoluteDeadline()) {
                event_queue->push({EventType::DeadlineMiss, current_time, t->getId(), t->getName()});
                stats->onDeadlineMiss(t->getId());
                t->setState(TaskState::Missed);
                state_changed = true;
            }
        }

        // refresh time pentru politici dinamice; no-op pentru cele statice
        policy->setCurrentTime(current_time);

        // decizia de scheduling
        if (!ready_queue->empty()) {
            Task* top = ready_queue->peek();
            if (current_running == nullptr || policy->isHigherPriority(top, current_running)) {
                dispatch(top);
                state_changed = true;
            }
        }


        // executa 1 tick din taskul curent
        if (current_running != nullptr) {
            current_running->setRemainingTime(current_running->getRemainingTime() - 1);
            stats->onTick(true);

            if (current_running->getRemainingTime() == 0) {
                event_queue->push({EventType::Complete, current_time, current_running->getId(), current_running->getName()});

                int response_time = (current_time + 1) - (current_running->getAbsoluteDeadline() - current_running->getDeadline());
                stats->onComplete(current_running->getId(), response_time);
                current_running->setState(TaskState::Finished);
                current_running = nullptr;
                state_changed = true;
            }
        } else {
            stats->onTick(false);
        }

        if (state_changed)
            stats->recordSnapshot(current_time,
                                  current_running ? current_running->getName() : "IDLE",
                                  stats->getCpuUtilization(), tasks);

        event_queue->push({EventType::Tick, current_time, -1, "idle"});

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    event_queue->push({EventType::EndOfSimulation, current_time, -1, ""});
    } catch (const std::exception& e) {
        std::cerr << "[Scheduler] Eroare in simulare la t=" << current_time << ": " << e.what() << "\n";
        event_queue->push({EventType::EndOfSimulation, current_time, -1, ""});
        throw;
    }
}

#include "Scheduler.h"
#include "queue/HeapReadyQueue.h"
#include "Task.h"
#include <iostream>
#include <stdexcept>
using namespace std;

std::unique_ptr<ReadyQueue> Scheduler::makeQueue(SchedulingPolicy* p) {
    // toate politicile ramase sunt cu prioritate statica pe durata unui job -> heap.
    // ramane in spatele interfetei ReadyQueue pentru extensii viitoare (cozi per-core)
    return std::make_unique<HeapReadyQueue>(p);
}

Scheduler::Scheduler()
    : policy(nullptr),
      stats(nullptr),
      event_queue(nullptr),
      trace_sink(nullptr),
      current_running(nullptr),
      current_time(0),
      ready_queue(nullptr) {}


Scheduler::Scheduler(SchedulingPolicy* policy, Stats* stats, EventQueue* event_queue,
                     TraceSink* trace_sink)
    : policy(policy),
      stats(stats),
      event_queue(event_queue),
      trace_sink(trace_sink),
      current_running(nullptr),
      current_time(0),
      ready_queue(makeQueue(policy)) {}


Scheduler::Scheduler(const Scheduler& sched)
    : policy(sched.policy),
      stats(sched.stats),
      event_queue(sched.event_queue),
      trace_sink(sched.trace_sink),
      tasks(sched.tasks),
      current_running(sched.current_running),
      current_time(sched.current_time),
      ready_queue(sched.policy ? makeQueue(sched.policy) : nullptr) {}


Scheduler& Scheduler::operator=(const Scheduler& other) {
    if (this == &other) return *this;
    policy        = other.policy;
    stats         = other.stats;
    event_queue   = other.event_queue;
    trace_sink    = other.trace_sink;
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

void Scheduler::emitRow(const std::string& event, Task* subject,
                        int running_id, int cpu_busy) {
    if (!trace_sink) return;
    trace_sink->onEvent(TraceRecord{
        current_time,
        event,
        subject ? subject->getId() : -1,
        subject ? subject->getName() : std::string(),
        subject ? subject->getRemainingTime() : -1,
        subject ? subject->getAbsoluteDeadline() : -1,
        running_id,
        cpu_busy
    });
}

void Scheduler::emitEvent(const std::string& event, Task* subject) {
    // campurile de rulare reflecta starea CPU in momentul evenimentului
    int running_id = current_running ? current_running->getId() : -1;
    int cpu_busy   = current_running ? 1 : 0;
    emitRow(event, subject, running_id, cpu_busy);
}

void Scheduler::dispatch(Task* new_running) {
    if (!new_running)
        throw std::logic_error("Scheduler::dispatch: incercare de dispatch cu task null");
    if (current_running != nullptr) {
        current_running->setState(TaskState::Ready);
        ready_queue->push(current_running);
        event_queue->push({EventType::Preempt, current_time, current_running->getId(), current_running->getName()});
        // preempt: inregistram taskul dat la o parte (current_running), inainte de reassign
        emitEvent("preempt", current_running);
        stats->onPreempt(current_running->getId());
    }
    current_running = new_running;
    current_running->setState(TaskState::Running);
    ready_queue->pop();
    event_queue->push({EventType::Dispatch, current_time, current_running->getId(), current_running->getName()});
    emitEvent("dispatch", current_running);
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

            if (t->isReadyAt(current_time) && t->getState() != TaskState::Ready && t->getState() != TaskState::Running) {
                t->release(current_time);
                event_queue->push({EventType::Release, current_time, t->getId(), t->getName()});
                emitEvent("release", t);
                ready_queue->push(t);
                stats->onRelease(t->getId());
            }

            // soft real-time: taskul continua sa ruleze dupa miss, doar inregistram o singura data
            if (t->getState() != TaskState::Finished && t->getState() != TaskState::Inactive
                && t->getState() != TaskState::Missed && current_time > t->getAbsoluteDeadline()) {
                event_queue->push({EventType::DeadlineMiss, current_time, t->getId(), t->getName()});
                emitEvent("deadline_miss", t);
                stats->onDeadlineMiss(t->getId());
                t->setState(TaskState::Missed);
            }
        }

        // decizia de scheduling
        if (!ready_queue->empty()) {
            Task* top = ready_queue->peek();
            if (current_running == nullptr || policy->isHigherPriority(top, current_running)) {
                dispatch(top);
            }
        }

        // executa 1 tick din taskul curent
        int ran_task_id = -1;
        int cpu_busy = 0;
        if (current_running != nullptr) {
            ran_task_id = current_running->getId();  // capturat inainte de posibila terminare
            cpu_busy = 1;
            current_running->setRemainingTime(current_running->getRemainingTime() - 1);
            stats->onTick(true);

            if (current_running->getRemainingTime() == 0) {
                event_queue->push({EventType::Complete, current_time, current_running->getId(), current_running->getName()});
                emitEvent("complete", current_running);

                int response_time = (current_time + 1) - (current_running->getAbsoluteDeadline() - current_running->getDeadline());
                stats->onComplete(current_running->getId(), response_time);
                current_running->setState(TaskState::Finished);
                current_running = nullptr;
            }
        } else {
            stats->onTick(false);
        }

        // un rand per tick ca sa se poata reconstrui utilizarea/idle-ul offline
        event_queue->push({EventType::Tick, current_time, -1, "idle"});
        emitRow("tick", nullptr, ran_task_id, cpu_busy);
    }
    event_queue->push({EventType::EndOfSimulation, current_time, -1, ""});
    emitRow("end", nullptr, -1, 0);
    if (trace_sink) trace_sink->flush();
    } catch (const std::exception& e) {
        std::cerr << "[Scheduler] Eroare in simulare la t=" << current_time << ": " << e.what() << "\n";
        event_queue->push({EventType::EndOfSimulation, current_time, -1, ""});
        if (trace_sink) trace_sink->flush();
        throw;
    }
}

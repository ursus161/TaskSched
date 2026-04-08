#include "Scheduler.h"
#include "Task.h"
#include <iostream>
using namespace std;

Scheduler::Scheduler()
    : current_running(nullptr), current_time(0) {}

void Scheduler::addTask(Task* task) {
    tasks.push_back(task); //adaug in pq
}

void Scheduler::dispatch(Task* new_running) {
    if (current_running != nullptr) {
        current_running->setState(TaskState::Ready);
        ready_queue.push(current_running);
    }
    current_running = new_running;
    current_running->setState(TaskState::Running);
    ready_queue.pop();
}

void Scheduler::run(int duration) {
    for (current_time = 0; current_time < duration; current_time++) {

        // verifica ce taskuri devin ready acum
        for (Task* t : tasks) {
            if (t->isReadyAt(current_time) && t->getState() != TaskState::Ready  && t->getState() != TaskState::Running) {
                t->release(current_time); //devine disponibil ptr pq, imi reseteaza si toate datele taskului
                ready_queue.push(t);
            }
        }

        //decide cine ruleaza  
        if (!ready_queue.empty()) {
            Task* top = ready_queue.top();
            if (current_running == nullptr || (top->getPriority() > current_running->getPriority()))  {
                // CPU idle, luam topul, nu vreau sa stea degeaba core-ul meu
               dispatch(top);
            
            }
        }

        //executa 1 tick din taskul curent
        if (current_running != nullptr) {
            int remainingTime = current_running->getRemainingTime();
            current_running->setRemainingTime(remainingTime - 1);

            cout << "[t=" << current_time << "] running task "
                 << current_running->getId() << " ("
                 << current_running->getName() << ")" << endl;

            // pas 4: verifica daca a terminat
            if (current_running->getRemainingTime() == 0) {
                current_running->setState(TaskState::Finished);
                current_running = nullptr;
            }
        } else {
            cout << "[t=" << current_time << "] CPU idle" << endl;
        }
    }
}
#include "LinearReadyQueue.h"

LinearReadyQueue::LinearReadyQueue(SchedulingPolicy* p) : policy(p) {}

void LinearReadyQueue::push(Task* t) { tasks.push_back(t); }

Task* LinearReadyQueue::peek() const {
    if (tasks.empty()) return nullptr;
    Task* best = tasks[0];
    for (size_t i = 1; i < tasks.size(); ++i)
        if (policy->isHigherPriority(tasks[i], best))
            best = tasks[i];
    return best;
}

void LinearReadyQueue::pop() {
    Task* best = peek();
    if (!best) return;
    auto it = std::find(tasks.begin(), tasks.end(), best);
    if (it != tasks.end()) tasks.erase(it);
}

bool LinearReadyQueue::empty() const { return tasks.empty(); }

void LinearReadyQueue::clear() { tasks.clear(); }

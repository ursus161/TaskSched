#pragma once
#include "ReadyQueue.h"
#include "../policies/SchedulingPolicy.h"
#include <queue>
#include <vector>

// PolicyComparator sta langa HeapReadyQueue, singura clasa care il foloseste
struct PolicyComparator {
    SchedulingPolicy* policy;
    bool operator()(Task* a, Task* b) const {
        return !policy->isHigherPriority(a, b);
    }
};

// implementare heap pentru politici cu prioritate stabila (Priority/RM/EDF/DM).
// O(log n) push/pop, O(1) peek; corect doar daca prioritatile nu se schimba
// pe durata unui job (ceea ce e cazul pt toate politicile din proiect)
class HeapReadyQueue : public ReadyQueue {
    std::priority_queue<Task*, std::vector<Task*>, PolicyComparator> heap;
public:
    explicit HeapReadyQueue(SchedulingPolicy* p);

    void push(Task* t) override;
    Task* peek() const override;
    void pop() override;
    void remove(Task* t) override;
    bool empty() const override;
    void clear() override;
};

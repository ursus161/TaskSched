#pragma once
#include "ReadyQueue.h"
#include "../policies/SchedulingPolicy.h"
#include <vector>
#include <algorithm>

// implementare vector+scan pentru politici dinamice (ex. LLF).
// peek() re-evalueaza prioritatea live la fiecare apel, deci e corect
// chiar daca laxitatile se schimba intre doua tick-uri consecutive
class LinearReadyQueue : public ReadyQueue {
    std::vector<Task*> tasks;
    SchedulingPolicy* policy;
public:
    explicit LinearReadyQueue(SchedulingPolicy* p);

    void push(Task* t) override;
    Task* peek() const override;
    void pop() override;
    bool empty() const override;
    void clear() override;
};

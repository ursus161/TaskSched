#pragma once
#include "SchedulingPolicy.h"

// Priority Scheduling clasic - prioritate statica (exact ceea ce implementasem in commit ul anterior)
class PriorityPolicy : public SchedulingPolicy {
public:
    bool isHigherPriority(Task* a, Task* b) const override {
        return a->getPriority() > b->getPriority();
    }

    std::string getName() const override { return "Priority"; }
    
    PriorityPolicy() = default;
    PriorityPolicy(const PriorityPolicy& other) = default;
    PriorityPolicy& operator=(const PriorityPolicy& other) = default;

    friend std::ostream& operator<<(std::ostream& out, const PriorityPolicy& p )  {
        out << p.getName();
        return out;
    }
    friend std::istream& operator>>(std::istream& in, PriorityPolicy& p)  {
        return in;
    }

};
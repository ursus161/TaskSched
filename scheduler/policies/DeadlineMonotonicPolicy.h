#pragma once
#include "StaticPriorityPolicy.h"


class DeadlineMonotonicPolicy : public StaticPriorityPolicy {
public:
    bool isHigherPriority(Task* a, Task* b) const override {
        return a->getDeadline() < b->getDeadline(); 

        //pentru sistemele care contin taskuri aperiodice predictibilitatea hard este diminuata, nu putem avea garantii de predictibilitate sub acestea
    }

    std::string getName() const override { return "DeadlineMonotonic"; }

    DeadlineMonotonicPolicy() = default;
    DeadlineMonotonicPolicy(const DeadlineMonotonicPolicy& other) = default;
    DeadlineMonotonicPolicy& operator=(const DeadlineMonotonicPolicy& other) = default;

friend std::ostream& operator<<(std::ostream& out, const DeadlineMonotonicPolicy& p) {
    out <<p.getName();
    return out;
}
friend std::istream& operator>>(std::istream& in, DeadlineMonotonicPolicy&) {
    return in;
}
};
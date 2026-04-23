#pragma once
#include "SchedulingPolicy.h"

class DeadlineMonotonicPolicy : public SchedulingPolicy {
public:
    bool isHigherPriority(Task* a, Task* b) const override {
        return a->getDeadline() < b->getDeadline(); 

        //pentru sistemele care contin taskuri aperiodice predictibilitatea hard este diminuata, nu putem avea garantii de predictibilitate sub acestea
    }

    bool verifySchedulability(const std::vector<Task*>& allTasks) const override {
        if (Stats::computeUtilization(allTasks) > 1.0) return false;
        return runRTA(sortedByPriority(allTasks));
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
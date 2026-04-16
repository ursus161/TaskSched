#pragma once
#include "SchedulingPolicy.h"
// EDF inseamna Earliest Deadline First - deadline absolut mai mic = prioritate mai mare
// se foloseste de obicei ca sa am cpu ul la 100%

class EDFPolicy : public SchedulingPolicy {
public:
    bool isHigherPriority(Task* a, Task* b) const override {
        return a->getAbsoluteDeadline() < b->getAbsoluteDeadline();
    }

    std::string getName() const override { return "EDF"; }

    EDFPolicy() = default;
    EDFPolicy(const EDFPolicy& other) = default;
    EDFPolicy& operator=(const EDFPolicy& other) = default;

friend std::ostream& operator<<(std::ostream& out, const EDFPolicy& p) {
    out <<p.getName();
    return out;
}
friend std::istream& operator>>(std::istream& in, EDFPolicy&) {
    return in;
}
};
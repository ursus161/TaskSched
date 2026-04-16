#include "SchedulingPolicy.h"

// rate monotonic --> perioada mai mica = prioritate mai mare, practic prioritizez cel care dureaza cel mai putin
// merge doar pe taskuri periodice, pt ca practic am prioritatea invers proportionala cu perioada
// daca n am perioada nu am prioritate, ceea ce e imposibil
//sub 69% cpu usage algoritmul garanteaza 0 deadline misses   



class RateMonotonicPolicy : public SchedulingPolicy {
public:
    bool isHigherPriority(Task* a, Task* b) const override {
        auto priorityA = dynamic_cast<PeriodicTask*>(a);
        auto priorityB = dynamic_cast<PeriodicTask*>(b);

        if (!priorityA || !priorityB) return false;  // fallback daca nu sunt periodice
        return priorityA->getPeriod() < priorityB->getPeriod();
    }   

    std::string getName() const override { return "Rate Monotonic"; }

    RateMonotonicPolicy() = default;
    RateMonotonicPolicy(const RateMonotonicPolicy& other) = default;
    RateMonotonicPolicy& operator=(const RateMonotonicPolicy& other) = default;

    friend std::ostream& operator<<(std::ostream& out, const RateMonotonicPolicy& p) {
        out << p.getName();
        return out;
    }
    friend std::istream& operator>>(std::istream& in, RateMonotonicPolicy&) {
        return in;
    }
};

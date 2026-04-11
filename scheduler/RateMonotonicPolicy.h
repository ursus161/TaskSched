#include "SchedulerPolicy.h"

// rate monotonic --> perioada mai mica = prioritate mai mare, practic prioritizez cel care dureaza cel mai putin
// merge doar pe taskuri periodice, pt ca practic am prioritatea invers proportionala cu perioada
// daca n am perioada nu am prioritate, ceea ce e imposibil
//atinge in jur de 69% utilizare maxima cpu, demonstrat matematic

class RateMonotonicPolicy : public SchedulingPolicy {
public:
    bool isHigherPriority(Task* a, Task* b) const override {
        auto priorityA = dynamic_cast<PeriodicTask*>(a);
        auto priorityB = dynamic_cast<PeriodicTask*>(b);

        if (!priorityA || !priorityB) return false;  // fallback daca nu sunt periodice
        return priorityA->getPeriod() < priorityB->getPeriod();
    }

    std::string getName() const override { return "Rate Monotonic"; }
};

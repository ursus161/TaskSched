#pragma once
#include "SchedulingPolicy.h"
#include "../stats/Stats.h"

// LLF = least laxity first
// laxitate = absolute_deadline - current_time - remaining_time
// mai putina laxitate inseamna mai putina "margine" inainte de deadline, deci prioritate mai mare.
//
// de ce nu heap: laxitatea se schimba la fiecare tick pentru toate taskurile din coada,
// deci un heap construit la tick t este invalid la tick t+1. scan liniar cu re-evaluare
// live (LinearReadyQueue) rezolva problema fara overhead suplimentar pentru seturi mici de taskuri.
//
// setCurrentTime e apelat de scheduler o data pe tick, inainte de selectie.
// am ales aceasta varianta in loc de a pasa timpul ca parametru la isHigherPriority
// deoarece signatura isHigherPriority este definita in baza si folosita si de heap.
class LLFPolicy : public SchedulingPolicy {
    int current_time = 0;
public:
    bool isDynamic() const override { return true; }

    void setCurrentTime(int t) override { current_time = t; }

    bool isHigherPriority(Task* a, Task* b) const override {
        int lax_a = a->getAbsoluteDeadline() - current_time - a->getRemainingTime();
        int lax_b = b->getAbsoluteDeadline() - current_time - b->getRemainingTime();
        if (lax_a != lax_b) return lax_a < lax_b;
        // tie-break: deadline absolut mai mic evita context switch-uri la laxitate egala
        return a->getAbsoluteDeadline() < b->getAbsoluteDeadline();
    }

    bool verifySchedulability(const std::vector<Task*>& allTasks) const override {
        // LLF este optim pe single-core preemptiv, deci U <= 1.0 garanteaza schedulabilitate
        return Stats::computeUtilization(allTasks) <= 1.0;
    }

    std::string getName() const override { return "LLF"; }

    LLFPolicy() = default;
    LLFPolicy(const LLFPolicy& other) = default;
    LLFPolicy& operator=(const LLFPolicy& other) = default;

    friend std::ostream& operator<<(std::ostream& out, const LLFPolicy& p) {
        out << p.getName();
        return out;
    }
    friend std::istream& operator>>(std::istream& in, LLFPolicy&) {
        return in;
    }
};

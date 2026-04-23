#pragma once

#include "../tasks/Task.h"
#include "../tasks/PeriodicTask.h"
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>

// clasa de baza abstracta - de aici o sa am fiecare clasa in functie de ce algoritm de task scheduler vreau sa folosesc
class SchedulingPolicy {
public:
    virtual ~SchedulingPolicy() = default;

    // returneaza true daca a are prioritate stricta mai mare decat b
    //asta o sa implementez doar in fiecare clasa derivata
    virtual bool isHigherPriority(Task* a, Task* b) const = 0;
    virtual bool verifySchedulability(const std::vector<Task*>& allTasks) const = 0;

    virtual std::string getName() const = 0;

protected:
    std::vector<Task*> sortedByPriority(const std::vector<Task*>& tasks) const {
        std::vector<Task*> sorted = tasks;
        std::sort(sorted.begin(), sorted.end(), [this](Task* a, Task* b) {
            return isHigherPriority(a, b);
        });
        return sorted;
    }
    //Response Type Analysis
    bool runRTA(const std::vector<Task*>& sorted) const {
        for (size_t i = 0; i < sorted.size(); ++i) {
            if (!dynamic_cast<PeriodicTask*>(sorted[i])) continue;
            double Ci = sorted[i]->getWCET();
            double Di = sorted[i]->getDeadline();
            double t = 0;
            for (size_t j = 0; j <= i; ++j) t += sorted[j]->getWCET();
            while (true) {
                double interference = 0;
                for (size_t j = 0; j < i; ++j) {
                    auto pj = dynamic_cast<PeriodicTask*>(sorted[j]);
                    if (pj) interference += std::ceil(t / pj->getPeriod()) * pj->getWCET();
                }
                double next_t = Ci + interference;
                if (next_t > Di) return false;
                if (std::abs(next_t - t) < 1e-9) break;
                t = next_t;
            }
        }
        return true;
    }

    SchedulingPolicy() = default;
    SchedulingPolicy(const SchedulingPolicy& other) = default;
    SchedulingPolicy& operator=(const SchedulingPolicy& other) = default;

    friend std::ostream& operator<<(std::ostream& out, const SchedulingPolicy& p) {
        out << p.getName();
        return out;
    }

    friend std::istream& operator>>(std::istream& in, SchedulingPolicy&) { 
        return in;  // nu citim nimic, clasa nu are niciun atribut
    }
};


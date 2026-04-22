#pragma once
#include "SchedulingPolicy.h"
#include "../tasks/PeriodicTask.h"
#include <vector>
#include "stats/Stats.h"
#include <cmath>
#include <algorithm>

class StaticPriorityPolicy : public SchedulingPolicy {
public:
    StaticPriorityPolicy() = default;
    virtual ~StaticPriorityPolicy() = default;

    PolicyType getPolicyType() const override { return PolicyType::Static; }

    // implementarea algoritmului RTA (Response Time Analysis)
    virtual bool verifySchedulability(const std::vector<Task*>& allTasks) const {
        std::vector<Task*> sorted = allTasks;

        //early fallback pt un U nonconform
        if (Stats::computeUtilization(allTasks) > 1.0) {
                return false; 
            }

        // sort dupa prioritatile policy ului
        std::sort(sorted.begin(), sorted.end(), [this](Task* a, Task* b) {
            return isHigherPriority(a, b);
        });

        for (size_t i = 0; i < sorted.size(); ++i) {

            //daca este aperiodic nu ii putem aplica acest test
        
            auto periodic_i = dynamic_cast<PeriodicTask*>(sorted[i]);
              if (!periodic_i) continue;


            double Ci = sorted[i]->getWCET();
            double Di = sorted[i]->getDeadline();
            
            // t_0 = suma timpilor de exec până la i (suma timp de exec al taskului curent si toti cu prioritate mai mare)
            double t = 0;
            for (size_t j = 0; j <= i; ++j) t += sorted[j]->getWCET();

            while (true) {
                double interference = 0;
                for (size_t j = 0; j < i; ++j) {
                    //doar taskurile cu model temporar periodice sau sporadice produc inferenta predictibila
                    auto p_j = dynamic_cast<PeriodicTask*>(sorted[j]);
                    if (p_j) {
                        // Formula Audsley: ceil(t / Tj) * Cj
                        interference += std::ceil(t / p_j->getPeriod()) * p_j->getWCET();
                    }
                }
                double next_t = Ci + interference;
                if (next_t > Di) return false; // esec : task neplanificabil
                if (next_t == t) break;       // succes: pentru task-ul i
                t = next_t;
            }
        }
        return true;
    }
};
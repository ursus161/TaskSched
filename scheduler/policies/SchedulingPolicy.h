#pragma once

#include "../tasks/Task.h"
#include "../tasks/PeriodicTask.h"
#include <string>



enum class PolicyType { Static, Dynamic };


// clasa de baza abstracta - de aici o sa am fiecare clasa in functie de ce algoritm de task scheduler vreau sa folosesc
class SchedulingPolicy {
public:
    virtual ~SchedulingPolicy() = default;

    // returneaza true daca a are prioritate stricta mai mare decat b
    //asta o sa implementez doar in fiecare clasa derivata
    virtual bool isHigherPriority(Task* a, Task* b) const = 0;

    virtual std::string getName() const = 0;

    virtual PolicyType getPolicyType() const =0;

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


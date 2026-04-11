#pragma once

#include "../tasks/Task.h"
#include "../tasks/PeriodicTask.h"
#include <string>

// clasa de baza abstracta - de aici o sa am fiecare clasa in functie de ce algoritm de task scheduler vreau sa folosesc
class SchedulingPolicy {
public:
    virtual ~SchedulingPolicy() = default;

    // returneaza true daca a are prioritate stricta mai mare decat b
    //asta o sa implementez doar in fiecare clasa derivata
    virtual bool isHigherPriority(Task* a, Task* b) const = 0;

    virtual std::string getName() const = 0;
};


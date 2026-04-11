#include "SchedulerPolicy.h"


// Priority Scheduling clasic - prioritate statica (exact ceea ce implementasem in commit ul anterior)
class PriorityPolicy : public SchedulingPolicy {
public:
    bool isHigherPriority(Task* a, Task* b) const override {
        return a->getPriority() > b->getPriority();
    }

    std::string getName() const override { return "Priority"; }
};

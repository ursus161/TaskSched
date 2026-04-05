#pragma once
#include "Task.h"

class AperiodicTask : virtual public Task {
protected:

    int arrival_time; //cand ajunge in sistem signalul ptr task
    bool released;

public:
    AperiodicTask(int id, const std::string& name, int priority,
                  int wcet, int deadline, int arrival_time);

    ~AperiodicTask() override = default;

    bool isReadyAt(int current_time) const override;
    void release(int current_time) override;
    std::string getType() const override;
};
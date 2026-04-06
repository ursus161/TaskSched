#pragma once

#include "Task.h"
using namespace std;

class PeriodicTask : virtual public Task {
protected:
    int period;
    int first_release;
    int jobs_released;

public:
    PeriodicTask(int id, const string& name, int priority,
                 int worstCaseExecutionTime, int deadline, int period, int first_release );

    PeriodicTask(const PeriodicTask& other);
    
    ~PeriodicTask() override = default;

    bool isReadyAt(int current_time) const override;
    void release(int current_time) override;
    string getType() const override;
};
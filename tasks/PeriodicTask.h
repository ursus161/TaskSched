#pragma once

#include "Task.h"

class PeriodicTask : virtual public Task {
protected:
    int period;
    int first_release;
    int jobs_released;

public:
    PeriodicTask( const std::string& name, int priority,
        int worstCaseExecutionTime, int deadline, int period, int first_release = 0);
        
    PeriodicTask();

    PeriodicTask(const PeriodicTask& other);
    
    ~PeriodicTask() override = default;

    PeriodicTask& operator=(const PeriodicTask& other);

    friend std::ostream& operator<<(std::ostream& out, const PeriodicTask& pt);
    friend std::istream& operator>>(std::istream& in, PeriodicTask& pt);

    bool isReadyAt(int current_time) const override;
    void release(int current_time) override;
    std::string getType() const override;

    int getPeriod() const;

};
#pragma once
#include "../tasks/Task.h"
using namespace std;
class AperiodicTask : virtual public Task {
protected:

    int arrival_time; //cand ajunge in sistem signalul ptr task
    bool released;

public:
    
    AperiodicTask(int id, const string& name, int priority,
                  int worstCaseExecutionTime, int deadline, int arrival_time);
    
    AperiodicTask();
    AperiodicTask(const AperiodicTask& other);
        
    ~AperiodicTask() override = default;

    bool isReadyAt(int current_time) const override;
    void release(int current_time) override;
    string getType() const override;
};
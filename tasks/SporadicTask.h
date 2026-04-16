#pragma once

#include "PeriodicTask.h"
#include "AperiodicTask.h"
#include <vector>
using namespace std;

class SporadicTask : public PeriodicTask, public AperiodicTask {
protected:
    int minimumInterArrivalTime; //intervalul minim garantat intre doua activari ale taskului
    //nu pot sa apas un buton de doua ori intr un interval de N unitati de timp spre ex
    int last_activation;

    vector<int> triggers; //realmente chestiile astea vor veni de la hardware/user, insa pentru o simulare determinista voi defini eu un vector
    int next_trigger_index;

public:
    SporadicTask(int id, const string& name, int priority,
                 int worstCaseExecutionTime, int deadline, int minimumInterArrivalTime,
                 const vector<int>& triggers);
    SporadicTask(const SporadicTask& other);

    SporadicTask();
    ~SporadicTask() override = default;

    SporadicTask& operator=(const SporadicTask& other);

    friend std::ostream& operator<<(std::ostream& out, const SporadicTask& st);
    friend std::istream& operator>>(std::istream& in, SporadicTask& st);

    bool isReadyAt(int current_time) const override;
    void release(int current_time) override;
    string getType() const override;
};
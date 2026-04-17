#pragma once
#include "../scheduler/stats/EventQueue.h"

class Dashboard {
private:

    EventQueue* queue;

public:

    Dashboard();
    Dashboard(const Dashboard& dashboard) ;
    ~Dashboard() = default;
    Dashboard&  operator=(const Dashboard& dashboard) ;
    Dashboard(EventQueue* q); // idee bazata pe htop-ul din linux

    friend std::ostream& operator<<(std::ostream& out, const Dashboard& sched);
    friend std::istream& operator>>(std::istream& in, Dashboard& sched);

    void run();  // loop care citeste events pana primeste EndOfSimulation ( optiune din enum ul de events)
};
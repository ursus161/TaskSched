#pragma once
#include "../scheduler/stats/EventQueue.h"
#include "../stats/Stats.h"
#include <unordered_map>
#include <string>

struct TaskRow {

    std::string name;
    int releases = 0;
    int completes = 0;
    int misses = 0;
    std::string state="Inactive"; //initializari default
};
class Dashboard {
private:

    EventQueue* queue;
 
    //astea sunt starile globale ale CPU ului single core 

    int current_time = 0;
    int running_id = -1;
    std::string running_name = "idle"; //ce ruleaza activ acum
    int idle_ticks = 0;
    //map de la task_id la taskrow
    std::unordered_map<int, TaskRow> rows;
    int getTerminalWidth() const;
    std::string colorForState(const std::string& state);
public:

    Dashboard();
    Dashboard(const Dashboard& dashboard) ;
    ~Dashboard() = default;
    Dashboard&  operator=(const Dashboard& dashboard) ;
    Dashboard(EventQueue* q); // idee bazata pe htop-ul din linux

    friend std::ostream& operator<<(std::ostream& out, const Dashboard& sched);
    friend std::istream& operator>>(std::istream& in, Dashboard& sched);


    //actualizeaza starea interna 
    void processEvent(const Event& e);

    //deseneaza starea interna si da update pe terminal
    void render(); 

    void run();  // loop care citeste events pana primeste EndOfSimulation ( optiune din enum ul de events)
};
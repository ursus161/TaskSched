#pragma once
#include "../scheduler/stats/EventQueue.h"
#include "../stats/Stats.h"
#include "../policies/SchedulingPolicy.h"
#include <unordered_map>
#include <string>
#include <vector>

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
    Stats* stats;       // sursa autoritara pentru CPU%, idle_ticks — elimina duplicarea de stare

    //astea sunt starile globale ale CPU ului single core

    bool schedulable = false;
    std::string policy_name = "N/A";

    int current_time = 0;
    int running_id = -1;
    std::string running_name = "idle"; //ce ruleaza activ acum
    // idle_ticks eliminat — il citim direct din Stats, fara intarziere de un tick
    bool just_completed = false;
    int last_completed_id = -1; // retine id-ul taskului care tocmai a terminat, pentru a nu reseta running_id daca un dispatch nou a venit in acelasi tick

    //map de la task_id la taskrow
    std::unordered_map<int, TaskRow> rows;

    static std::string colorForState(const std::string& state);
public:

    Dashboard();
    Dashboard(const Dashboard& dashboard) ;
    ~Dashboard() = default;
    Dashboard&  operator=(const Dashboard& dashboard) ;
    Dashboard(EventQueue* q, Stats* s, const SchedulingPolicy* p, const std::vector<Task*>& tasks);

    friend std::ostream& operator<<(std::ostream& out, const Dashboard& sched);
    friend std::istream& operator>>(std::istream& in, Dashboard& sched);


    static int getTerminalWidth();

    //actualizeaza starea interna
    void processEvent(const Event& e);

    //deseneaza starea interna si da update pe terminal
    void render();

    void run();  // loop care citeste events pana primeste EndOfSimulation ( optiune din enum ul de events)
};
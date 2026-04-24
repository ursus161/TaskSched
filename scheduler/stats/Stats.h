#pragma once

#include "TaskStats.h"
#include <unordered_map>
#include <iostream>
#include <vector>
#include "../tasks/Task.h"


struct SnapshotRow {
    int tick;
    std::string cpu_task;
    double cpu_util;
    int task_id;
    std::string task_name;
    std::string state;
};

struct SummaryData {
    int deadline_misses;
    double cpu_util;
    bool valid; // false daca CSV-ul nu contine META linii
};

class Stats {
private:
    int active_ticks;
    int idle_ticks;
    int total_preemptions;
    int total_deadline_misses;
    std::unordered_map<int, TaskStats> per_task; //dictionar task id si statisticile taskului
    std::vector<SnapshotRow> snapshot_log;

public:
    Stats();
    Stats(const Stats& other);
    Stats& operator=(const Stats& other);
    ~Stats() = default;
    
    
    int getActiveTicks() const;
    int getIdleTicks() const;
    int getTotalPreemptions() const;
    int getTotalDeadlineMisses() const;
    double getCpuUtilization() const;
    const std::unordered_map<int, TaskStats>& getPerTask() const;


    //utilizarea teoretica a priori, utila pentru ratemonotonic pentru a verifica daca un sistem e supraincarcat (atinge maxim lim -> n (2^1/n -1) ~ 69%)
    //asta garanteaza ca pana la o utilitate teoretica din acea constanta, nu voi avea deadline misses  
    static double computeUtilization(const std::vector<Task*>& tasks);

    void registerTask(int task_id, const std::string& name, const std::string& type);

    // in general modificatoare de stare a sistemului
    void onRelease(int task_id);
    void onPreempt(int task_id);
    void onComplete(int task_id, int response_time);
    void onDeadlineMiss(int task_id);
    void onTick(bool cpu_active);


    void recordSnapshot(int tick, const std::string& cpu_task, double cpu_util, const std::vector<Task*>& tasks);
    void exportSnapshotCSV(const std::string& filename) const;
    static std::vector<SnapshotRow> getSnapshotAt(const std::string& filename, int tick);
    static SummaryData getSummaryFromCSV(const std::string& filename);



    friend std::ostream& operator<<(std::ostream& out, const Stats& s);
    friend std::istream& operator>>(std::istream& in, Stats& s);
};
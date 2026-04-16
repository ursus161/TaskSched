#pragma once

#include "TaskStats.h"
#include <unordered_map>
#include <iostream>
#include <vector>


struct GanttRecord{
    std::string task_name;
    int start_time;
    int end_time;

};

class Stats {
private:
    int active_ticks;
    int idle_ticks;
    int total_preemptions;
    int total_deadline_misses;
    std::unordered_map<int, TaskStats> per_task; //dictionar task id si statisticile taskului
    std::vector<GanttRecord> timeline;

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

    void registerTask(int task_id, const std::string& name, const std::string& type);

    // in general modificatoare de stare a sistemului
    void onRelease(int task_id);
    void onPreempt(int task_id);
    void onComplete(int task_id, int response_time);
    void onDeadlineMiss(int task_id);
    void onTick(bool cpu_active);


    //export datele din recordExecution
    void exportToCSV(const std::string& filename) const;
    //fiecare pushback de aici o sa fie cate un rand in CSV
    void recordExecution(const std::string& task_name, int start, int end);



    friend std::ostream& operator<<(std::ostream& out, const Stats& s);
    friend std::istream& operator>>(std::istream& in, Stats& s);
};
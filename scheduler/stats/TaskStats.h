#pragma once

#include <string>
#include <iostream>
//statisticile specifice unui task, cum am aici viziunea ar fi sa fie asemanator in dashboard cu o linie dintr un htop


class TaskStats {
private:
    int jobs_released;
    int jobs_completed;
    int deadline_misses;
    int drops;       // joburi tardy abandonate la venirea unui release nou (soft-RT overlap)
    int preemptions; // context switches
    int total_response_time; //suma timpilor de raspuns ptr toate joburile taskului, adica finish - release
    std::string name;
    std::string type; // "Periodic" / "Aperiodic" / "Sporadic"

public:
    TaskStats();
    TaskStats(const std::string& name, const std::string& type);
    TaskStats(const TaskStats& other);
    TaskStats& operator=(const TaskStats& other);
    ~TaskStats() = default;

    int getJobsReleased() const;
    int getJobsCompleted() const;
    int getDeadlineMisses() const;
    int getDrops() const;
    int getPreemptions() const;
    int getTotalResponseTime() const;
    double getAverageResponseTime() const;
    const std::string& getName() const;
    const std::string& getType() const;
//modifica starea obiectului
    void onRelease();
    void onComplete(int response_time);
    void onDeadlineMiss();
    void onDrop();
    void onPreempt();

    friend std::ostream& operator<<(std::ostream& out, const TaskStats& ts);
    friend std::istream& operator>>(std::istream& in, TaskStats& ts);
};
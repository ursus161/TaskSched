#pragma once
#include "TraceSink.h"
#include <fstream>
#include <string>

// scrie fiecare TraceRecord ca un rand intr-un singur CSV de trace pentru rulare.
// ordinea coloanelor este fixa:
//   tick,event,task_id,task_name,remaining_time,absolute_deadline,running_task_id,cpu_busy
class CsvTraceSink : public TraceSink {
    std::ofstream out;
    std::string path;

public:
    // creeaza directorul parinte daca lipseste si scrie randul de header.
    explicit CsvTraceSink(const std::string& file_path);
    ~CsvTraceSink() override;

    void onEvent(const TraceRecord& r) override;
    void flush() override;

    const std::string& getPath() const { return path; }
};

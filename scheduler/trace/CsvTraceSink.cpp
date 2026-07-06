#include "CsvTraceSink.h"
#include <filesystem>
#include <stdexcept>

CsvTraceSink::CsvTraceSink(const std::string& file_path)
    : path(file_path) {
    // creeaza traces/ (sau orice director parinte din --out) daca nu exista
    std::filesystem::path p(path);
    if (p.has_parent_path())
        std::filesystem::create_directories(p.parent_path());

    out.open(path);
    if (!out.is_open())
        throw std::runtime_error("CsvTraceSink: nu pot deschide fisierul pentru trace: " + path);

    out << "tick,event,task_id,task_name,remaining_time,"
           "absolute_deadline,running_task_id,cpu_busy\n";
}

CsvTraceSink::~CsvTraceSink() {
    if (out.is_open()) out.close();
}

void CsvTraceSink::onEvent(const TraceRecord& r) {
    out << r.tick << ','
        << r.event << ','
        << r.task_id << ','
        << r.task_name << ','
        << r.remaining_time << ','
        << r.absolute_deadline << ','
        << r.running_task_id << ','
        << r.cpu_busy << '\n';
}

void CsvTraceSink::flush() {
    out.flush();
}

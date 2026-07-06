#pragma once
#include <string>

// o inregistrare in trace-ul de simulare: un rand din CSV-ul de output.
// campurile care nu se aplica unui eveniment folosesc conventia -1 (int) / "" (string).
// vezi traces/schema.md pentru semantica completa a coloanelor.
struct TraceRecord {
    int tick;
    std::string event;      // "release","dispatch","preempt","complete","deadline_miss","tick","end"
    int task_id;            // -1 daca N/A
    std::string task_name;  // "" daca N/A
    int remaining_time;     // -1 daca N/A
    int absolute_deadline;  // -1 daca N/A
    int running_task_id;    // -1 daca CPU idle
    int cpu_busy;           // 1 daca un task a rulat in acest tick, altfel 0
    // policy nu e o coloana: e acelasi pe tot fisierul, se vede din numele fisierului
};

// interfata de sink pentru trace: decupleaza bucla de simulare de destinatia output-ului.
// implementarea concreta (ex. CsvTraceSink) decide unde/cum se scriu inregistrarile.
class TraceSink {
public:
    virtual ~TraceSink() = default;
    virtual void onEvent(const TraceRecord& r) = 0;
    virtual void flush() = 0;
};

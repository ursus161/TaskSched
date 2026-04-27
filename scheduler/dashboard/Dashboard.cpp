#include "Dashboard.h"
#include <iostream>
#include <stdexcept>
#include <iomanip>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <sys/ioctl.h>
    #include <unistd.h>
#endif

Dashboard::Dashboard() : queue(nullptr), stats(nullptr) {}
Dashboard::Dashboard(EventQueue* q, Stats* s, const SchedulingPolicy* p, const std::vector<Task*>& tasks)
    : queue(q), stats(s) {
    if (p && !tasks.empty()) {
        policy_name = p->getName();
        schedulable = p->verifySchedulability(tasks);
    }
}
Dashboard::Dashboard(const Dashboard& other)
    : queue(other.queue), stats(other.stats),
      schedulable(other.schedulable), policy_name(other.policy_name) {}

Dashboard& Dashboard::operator=(const Dashboard& other) {
    if (this == &other) return *this;
    queue = other.queue;
    stats = other.stats;
    return *this;
}

std::ostream& operator<<(std::ostream& out, const Dashboard& d) {
    out << "Dashboard[queue=" << (d.queue ? "connected" : "null") << "]";
    return out;
}

std::istream& operator>>(std::istream& in, Dashboard& d) {
    return in;  // nimic de citit
}

int Dashboard::getTerminalWidth() {
    #ifdef _WIN32

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole != INVALID_HANDLE_VALUE && GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        // srWindow = portiunea vizibila a buffer-ului consolei
        
        return csbi.srWindow.Right - csbi.srWindow.Left + 1;
    }
    return 80;  // fallback default

    #else
    //unix system based instructions, va urma si un fix pentru OS Windows
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {// syscall ul a reusit 
        return w.ws_col;  // numarul de coloane
    }
    return 80;  // fallback default

    #endif
}

std::string Dashboard::colorForState(const std::string& state) {
    if (state == "Running")  return "\033[32m";  // verde
    if (state == "Ready")    return "\033[33m";  // galben
    if (state == "Missed")   return "\033[31m";  // rosu
    return "\033[0m";  // default   
}

void Dashboard::run() {
    if (!queue)  throw std::runtime_error("Dashboard: queue neinitializata");
    if (!stats)  throw std::runtime_error("Dashboard: stats neinitializat");
    try {
        int last_time = -999;
        while (true) {
            Event e = queue->pop();

            // daca time s-a schimbat, desenez STAREA VECHEA (pentru last_time)
            if (e.time != last_time && last_time >= 0) {
                render();
               // std::this_thread::sleep_for(std::chrono::milliseconds(700));  // pauza doar cand desenez
            }

            processEvent(e);       // dupa render, updatez starea pentru noul time
            last_time = e.time;
            
            if (e.type == EventType::EndOfSimulation || e.type == EventType::Tick) {render();}

            if (e.type == EventType::EndOfSimulation) {
                break;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "[Dashboard] Eroare in loop: " << e.what() << "\n";
        throw;
    }
}

void Dashboard::processEvent(const Event& e) {
    if (e.type == EventType::EndOfSimulation) return;
    current_time = e.time;
    
    // daca e primul event pentru task-ul asta, initializeaza-i numele ( implicit randul in UM)

    if (e.task_id != -1 && // nu vreau sa afisez pt endofsimulation nimic
         rows.find(e.task_id) == rows.end()) { //taskul nu e in map

        rows[e.task_id].name = e.task_name;

    }
    
    switch (e.type) {
        case EventType::Release:
            rows[e.task_id].releases++;
            rows[e.task_id].state = "Ready";
            break;
            
        case EventType::Dispatch:
            running_id = e.task_id;
            running_name = e.task_name;
            rows[e.task_id].state = "Running";
            break;
        
        case EventType::Preempt:
            rows[e.task_id].state = "Ready";
            break;
            

        case EventType::Complete:
            rows[e.task_id].completes++;
            rows[e.task_id].state = "Finished";
            if (running_id == e.task_id) {
                just_completed = true; //  clearing running_id pana dupa Tick
                last_completed_id = e.task_id;
            }
            break;

        case EventType::Tick:
            if (just_completed) {
                // resetam running doar daca nu a venit un Dispatch nou in acelasi tick
                // daca a venit Dispatch, running_id e deja alt task, nu last_completed_id
                if (running_id == last_completed_id) {
                    running_id = -1;
                    running_name = "idle";
                }
                just_completed = false;
            }
            // idle_ticks nu mai e tinut local — il citim din Stats in render()
            break;

        case EventType::DeadlineMiss:
            rows[e.task_id].misses++;
            rows[e.task_id].state = "Missed";
            break;

        default:
            break;
    }
}

void Dashboard::render() {
    std::cout << "\033[H\033[J" << std::flush;  // clear screen + cursor home ca sa reincep scrierea de date
    std::cout << "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n";
    const int content_width = 50;  // latimea aprox 
    int term_width = getTerminalWidth();

    //se bazeaza pe compilatoarele moderne pe cmov, still branchless insa inainte aveam probleme de UB
    int  padding_size = std::max(0,(term_width - content_width) / 2);
    std::string padding(padding_size, ' ');  //padding size spatii

    std::cout << padding << "\033[1;36m=== Task Scheduler Dashboard ===\033[0m\n";

    // citim direct din Stats — sursa autoritara, fara intarziere de un tick
    double cpu_pct = stats->getCpuUtilization();
    int idle = stats->getIdleTicks();

    std::cout << padding << "Time: " << current_time + 1 << " tick  |  CPU: "
            << std::fixed << std::setprecision(1) << cpu_pct << "%"
            << "  |  Idle ticks: " << idle << "\n"
            << "                                                               |  Is schedulable? [" << policy_name << "]: "
            << (schedulable ? "\033[32mYes\033[0m" : "\033[31mNo\033[0m") << "\n";
    std::cout << padding << "Running: " << (running_id == -1 ? "idle" : running_name) << "\n";
    std::cout << padding ;   std::cout << "----------------------------------------\n";
    std::cout << padding << std::left
              << std::setw(4) << "ID"
              << std::setw(12) << "Name"
              << std::setw(12) << "State"
              << std::setw(6) << "Rel"
              << std::setw(6) << "Comp"
              << std::setw(6) << "Miss" << "\n";
            
    std::cout << padding ;   std::cout << "----------------------------------------\n";
    
    for (const auto& [id, row] : rows) {
            std::cout << padding << std::left
                    << std::setw(4) << id
                    << std::setw(12) << row.name
                    << colorForState(row.state)
                    << std::setw(12) << row.state
                    << "\033[0m"  // reset dupa state
                    << std::setw(6) << row.releases
                    << std::setw(6) << row.completes
                    << std::setw(6) << row.misses << "\n";
        }
    std::cout << std::flush;//golesc bufferul si fortez scrierea pe ecran
    //imi scria cu interziere fara flush si aveam probleme de sincronizare
}
#include "Dashboard.h"
#include <iostream>
#include <sys/ioctl.h>
#include <unistd.h>
#include <iomanip>

Dashboard::Dashboard() : queue(nullptr) {}
Dashboard::Dashboard(EventQueue* q) : queue(q) {}
Dashboard::Dashboard(const Dashboard& other) : queue(other.queue) {}

Dashboard& Dashboard::operator=(const Dashboard& other) {
    if (this == &other) return *this;
    queue = other.queue;
    return *this;
}

std::ostream& operator<<(std::ostream& out, const Dashboard& d) {
    out << "Dashboard[queue=" << (d.queue ? "connected" : "null") << "]";
    return out;
}

std::istream& operator>>(std::istream& in, Dashboard& d) {
    return in;  // nimic de citit
}

int Dashboard::getTerminalWidth() const {
    //unix system based instructions, va urma si un fix pentru OS Windows
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
        return w.ws_col;  // numarul de coloane
    }
    return 80;  // fallback default
}

std::string Dashboard::colorForState(const std::string& state) {
    if (state == "Running")  return "\033[32m";  // verde
    if (state == "Ready")    return "\033[33m";  // galben
    if (state == "Missed")   return "\033[31m";  // rosu
    return "\033[0m";  // default   
}

void Dashboard::run() {
    int last_time = -999;
    while (true) {  
        Event e = queue->pop();
        
        // daca time s-a schimbat, desenez STAREA VECHEA (pentru last_time)
        if (e.time != last_time && last_time >= 0) {
            render();
            std::this_thread::sleep_for(std::chrono::milliseconds(700));  // pauza doar cand desenez
        }
        
        processEvent(e);       // dupa render, updatez starea pentru noul time
        last_time = e.time;
        
        if (e.type == EventType::EndOfSimulation) {
            render();          // ultimul desen
            break;
        }
    }
}

void Dashboard::processEvent(const Event& e) {
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
                running_id = -1;
                running_name = "idle";
            }
            break;
        
        case EventType::Tick:
            if (running_id == -1) idle_ticks++; //folosesc la cpu% 
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

    int total = std::max(1, current_time + 1);
    int active = std::max(0, total - idle_ticks);
    double cpu_pct = 100.0 * active / total;

    std::cout << padding << "Time: " << current_time << " tick  |  CPU: " 
            << std::fixed << std::setprecision(1) << cpu_pct << "%\n";
    std::cout << padding ;std::cout << "Running: " << running_name << "\n";
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
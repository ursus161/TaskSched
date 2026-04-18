#include "Dashboard.h"
#include <iostream>
#include <sys/ioctl.h>
#include <unistd.h>

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
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
        return w.ws_col;  // numarul de coloane
    }
    return 80;  // fallback default
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
    int  padding_size = 0 * (padding_size < 0) + ((term_width - content_width) / 2)*(padding_size >= 0);
    
    std::string padding(padding_size, ' ');  //padding size spatii
    

  std::cout << padding ;  std::cout << "=== TaskSched Dashboard ===\n";
    std::cout << padding ;std::cout << "Time: " << current_time << " tick\n";
    std::cout << padding ;std::cout << "Running: " << running_name << "\n";
 std::cout << padding ;   std::cout << "----------------------------------------\n";
  std::cout << padding ;  std::cout << "ID  Name       State     Rel  Comp  Miss\n";
  std::cout << padding ;  std::cout << "----------------------------------------\n";
    
    for (const auto& [id, row] : rows) {//afisez datele 
        std::cout << padding 
                  <<  id << "   " 
                  << row.name << "     "
                  << row.state << "    "
                  << row.releases << "    "
                  << row.completes << "    "
                  << row.misses << "\n";
    }
    
    std::cout << std::flush;//golesc bufferul si fortez scrierea pe ecran
    //imi scria cu interziere fara flush si aveam probleme de sincronizare

}
#include "Dashboard.h"
#include <iostream>


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


void Dashboard::run() {
    while (true) {  
        Event e = queue->pop();
        
        std::cout << "[DASHBOARD] event at t=" << e.time 
                  << " type=" << (int)e.type 
                  << " task=" << e.task_id << std::endl;

        if (e.type == EventType::EndOfSimulation) break;
    }
}
#pragma once
#include <iostream>

// tipurile de evenimente care pot circula prin queue
// fiecare corespunde unui moment interesant din simulare

enum class EventType {
    //in functie de tipurile acestea de evenimente o sa lucrez pe logica de dashboard
    Release,
    Dispatch,
    Preempt,
    Complete,
    DeadlineMiss, //asta ar fi grav, mai ales in RTOS unde pot sa am sisteme ABS care dau fail, nu doar un stergator
    Tick,
    EndOfSimulation
};

struct Event {
    EventType type; //fiecare event care cate un type, o sa fie o coloana separata in dashboard ul final
    int time;         // tick-ul cand s-a intamplat
    int task_id;      // id-ul taskului implicat, -1 daca nu se aplica
                    // vreau sa stiu la fiecare lucru cum s a miscat si cum a progresat
    std::string task_name;
};
 
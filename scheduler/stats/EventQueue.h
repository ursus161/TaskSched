#pragma once

#include "Event.h"
#include <queue>
#include <mutex>
#include <condition_variable>

// coada thread-safe de evenimente in care scheduler-ul (producer) pune evenimente.
// a fost gandita pentru un consumer separat (dashboard-ul, scos acum); o pastrez
// ca punct de extensie thread-safe (mutex + condition_variable) daca revine un consumer
class EventQueue {
private:
    std::queue<Event> q;              // coada propriu-zisa
    std::mutex mutex;                   // protejeaza q de acces simultan
    std::condition_variable condition_variable;       // semnalizeaza cand apare ceva nou in q, nu consum resursele cpu ului aiurea

public:
    // producer: pune un event in coada si trezeste consumer-ul
    void push(const Event& event);

    // consumer: scoate un event din coada, blocheaza daca e goala
    Event pop();
};
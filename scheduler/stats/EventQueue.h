#pragma once

#include "Event.h"
#include <queue>
#include <mutex>
#include <condition_variable>

// incerc sa folosesc o arhitectura asemanatoare rabbitmq pentru comunicarea intre cele 2 thread-uri
// coada thread-safe prin care scheduler-ul (producer) trimite evenimente
// si dashboard-ul (consumer) le consuma
// folosesc un mutex sa previn eventualele race conditions, nu vreau 2 threaduri la queue in acelasi timp
// si condition_variable pentru a pune consumer-ul la somn cand coada e goala, in loc sa o verifice continuu (busy wait)
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
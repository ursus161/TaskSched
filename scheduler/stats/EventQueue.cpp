#include "EventQueue.h"

void EventQueue::push(const Event& event) {
    // iau lacatul ca sa pot modifica q in siguranta
    std::lock_guard<std::mutex> lock(mutex);
    q.push(event);
    // apoi trezesc un consumer care dormea pe cv
    condition_variable.notify_one();
}

Event EventQueue::pop() {

    std::unique_lock<std::mutex> lock(mutex); //pot elibera oricand in scope

    // daca coada e goala, dorm pe cv pana cineva face notify
    // cv.wait elibereaza automat lacatul cat dorm, si il reia la trezire
    condition_variable.wait(    
        lock, 
        [this] { return !q.empty(); } //am scris un lambda ptr simplitate
    );

    Event e = q.front();
    q.pop();
    return e;
}
#pragma once
#include "Task.h"

// abstractizare peste coada de ready tasks.
// implementarea curenta e HeapReadyQueue (prioritati statice pe durata unui job).
// interfata ramane abstracta ca sa putem adauga cozi per-core la trecerea pe multi-core.
class ReadyQueue {
public:
    virtual ~ReadyQueue() = default;
    virtual void push(Task* t) = 0;
    virtual Task* peek() const = 0;  // cel mai prioritar task, nullptr daca goala
    virtual void pop() = 0;          // elimina taskul returnat de peek()
    virtual void remove(Task* t) = 0; // scoate un task anume din coada (jobul abandonat la overlap)
    virtual bool empty() const = 0;
    virtual void clear() = 0;
};

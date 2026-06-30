#pragma once
#include "Task.h"

// abstractizare peste coada de ready tasks.
// politica decide implementarea concreta: heap pentru prioritati statice,
// scan liniar pentru politici dinamice (ex. LLF) unde prioritatea se schimba la fiecare tick
class ReadyQueue {
public:
    virtual ~ReadyQueue() = default;
    virtual void push(Task* t) = 0;
    virtual Task* peek() const = 0;  // cel mai prioritar task, nullptr daca goala
    virtual void pop() = 0;          // elimina taskul returnat de peek()
    virtual bool empty() const = 0;
    virtual void clear() = 0;
};

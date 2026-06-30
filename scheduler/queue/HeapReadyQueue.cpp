#include "HeapReadyQueue.h"

HeapReadyQueue::HeapReadyQueue(SchedulingPolicy* p)
    : heap(PolicyComparator{p}) {}

void HeapReadyQueue::push(Task* t) { heap.push(t); }

Task* HeapReadyQueue::peek() const {
    return heap.empty() ? nullptr : heap.top();
}

void HeapReadyQueue::pop() {
    if (!heap.empty()) heap.pop();
}

bool HeapReadyQueue::empty() const { return heap.empty(); }

void HeapReadyQueue::clear() {
    while (!heap.empty()) heap.pop();
}

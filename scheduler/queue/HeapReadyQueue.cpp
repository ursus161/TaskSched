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

// scoate un task anume (nu neaparat topul): golim heap-ul intr-un vector, sarim
// peste t, apoi reconstruim. O(n log n) dar n = nr de taskuri, foarte mic.
void HeapReadyQueue::remove(Task* t) {
    std::vector<Task*> keep;
    while (!heap.empty()) {
        Task* x = heap.top();
        heap.pop();
        if (x != t) keep.push_back(x);
    }
    for (Task* x : keep) heap.push(x);
}

bool HeapReadyQueue::empty() const { return heap.empty(); }

void HeapReadyQueue::clear() {
    while (!heap.empty()) heap.pop();
}

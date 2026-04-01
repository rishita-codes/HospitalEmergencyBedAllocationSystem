#include "priority_queue.h"
#include "basic_structs.h"
#include <algorithm>

void MaxHeap::heapifyDown(int i) {
    int largest = i;
    int l = left(i);
    int r = right(i);

    if (l < heap.size() &&
        heap[l]->getPriority() > heap[largest]->getPriority())
        largest = l;

    if (r < heap.size() &&
        heap[r]->getPriority() > heap[largest]->getPriority())
        largest = r;

    if (largest != i) {
        std::swap(heap[i], heap[largest]);
        heapifyDown(largest);
    }
}

void MaxHeap::heapifyUp(int i) {
    while (i != 0 &&
           heap[parent(i)]->getPriority() < heap[i]->getPriority()) {
        std::swap(heap[i], heap[parent(i)]);
        i = parent(i);
    }
}

void MaxHeap::insert(Patient* p) {
    heap.push_back(p);
    heapifyUp(heap.size() - 1);
}

Patient* MaxHeap::extractMax() {
    if (heap.empty()) return nullptr;

    Patient* root = heap[0];

    if (heap.size() == 1) {
        heap.pop_back();
        return root;
    }

    heap[0] = heap.back();
    heap.pop_back();
    heapifyDown(0);

    return root;
}

Patient* MaxHeap::top() const {
    if (heap.empty()) return nullptr;
    return heap[0];
}

bool MaxHeap::remove(const std::string& patient_id) {
    for (int i = 0; i < heap.size(); i++) {
        if (heap[i]->getId() == patient_id) {
            heap[i] = heap.back();
            heap.pop_back();
            if (i < heap.size()) {
                heapifyDown(i);
                heapifyUp(i);
            }
            return true;
        }
    }
    return false;
}

std::vector<Patient*> MaxHeap::getPatients() const {
    return heap;
}

void MaxHeap::rebuildHeap() {
    std::vector<Patient*> temp(heap);
    heap.clear();
    for (Patient* p : temp) {
        insert(p);
    }
}
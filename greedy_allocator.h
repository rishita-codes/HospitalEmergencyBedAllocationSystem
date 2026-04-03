#ifndef GREEDY_ALLOCATOR_H
#define GREEDY_ALLOCATOR_H

#include <vector>
#include "basic_structs.h"
#include "priority_queue.h"

class Database;
class MaxHeap;

class GreedyAllocator {
    std::vector<Bed>& beds;
    MaxHeap& waitingPatients;
    Database* db;

public:
    GreedyAllocator(std::vector<Bed>& b, MaxHeap& heap, Database* db = nullptr);

    bool isCompatible(BedType b, ESI e);
    void allocate();
};

#endif

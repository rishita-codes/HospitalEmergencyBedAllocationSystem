#ifndef HOSPITAL_H
#define HOSPITAL_H

#include "basic_structs.h"
#include "priority_queue.h"
#include "greedy_allocator.h"
#include "optimal_allocator.h"
#include <vector>

class Hospital {
private:
    std::vector<Bed> beds;
    MaxHeap waitingPatients;

public:
    Hospital(std::vector<Bed> b);
    void addPatient(Patient* p);
    void assignBeds();
};

#endif
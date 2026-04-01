#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H

#include <vector>
#include <algorithm>
#include <string>

class Patient;
class MaxHeap {
private:
    std::vector<Patient*> heap;

    int parent(int i) { return (i - 1) / 2; }
    int left(int i)   { return 2 * i + 1; }
    int right(int i)  { return 2 * i + 2; }

    void heapifyDown(int i);
    void heapifyUp(int i);

public:
    void insert(Patient* p);
    Patient* extractMax();
    Patient* top() const;
    bool remove(const std::string& patient_id);
    std::vector<Patient*> getPatients() const;
    void rebuildHeap();
};

#endif
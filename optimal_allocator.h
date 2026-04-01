#ifndef OPTIMAL_ALLOCATOR_H
#define OPTIMAL_ALLOCATOR_H

#include <vector>
#include "basic_structs.h"

class Database;

class OptimalAllocator {
private:
    std::vector<Patient*> patients;
    std::vector<Bed>& beds;

    std::vector<int> freeBedsIndex;
    std::vector<std::vector<int>> costMatrix;
    std::vector<int> match;

    static const int INF_PENALTY = 100000;

    bool isCompatible(BedType b, ESI e);
    
    std::vector<int> hungarian(std::vector<std::vector<int>> cost);

public:
    OptimalAllocator(std::vector<Patient*> p, std::vector<Bed>& b);

    void buildCostMatrix();
    void computeOptimal();
    void showAllocation();
    void applyAllocation(Database* db = nullptr);
    const std::vector<int>& getMatch() const;
};

#endif
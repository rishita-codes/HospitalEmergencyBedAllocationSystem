#include "greedy_allocator.h"
#include "basic_structs.h"
#include "priority_queue.h"
#include "database.h"

GreedyAllocator::GreedyAllocator(std::vector<Bed>& b, MaxHeap& heap, Database* db)
    : beds(b), waitingPatients(heap), db(db)
{}

bool GreedyAllocator::isCompatible(BedType b, ESI e) {
    if (e == ESI1 && b == ICU)           return true;
    if (e == ESI2 && (b == ICU || b == EMERGENCY)) return true;
    if (e == ESI3 && (b == GENERAL || b == EMERGENCY)) return true;
    if (e == ESI4) return true;
    return false;
}

void GreedyAllocator::allocate() {
    for (auto& bed : beds) {
        if (bed.occupied()) continue;

        std::vector<Patient*> skipped;

        while (true) {
            Patient* p = waitingPatients.extractMax();
            if (!p) break;

            if (isCompatible(bed.getType(), p->getESI())) {
                bed.assignPatient(p->getId());
                if (db) {
                    db->assignBed(p->getId(), bed.getBedId());
                }
                break;
            } else {
                skipped.push_back(p);
            }
        }

        for (Patient* s : skipped) {
            s->updatePriority();
            waitingPatients.insert(s);
        }
    }
}

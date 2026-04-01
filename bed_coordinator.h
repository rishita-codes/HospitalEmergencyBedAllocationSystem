#ifndef BED_COORDINATOR_H
#define BED_COORDINATOR_H

#include "database.h"
#include "basic_structs.h"
#include "priority_queue.h"
#include "greedy_allocator.h"
#include "optimal_allocator.h"
#include "dijkstra.h"
#include <vector>

extern MaxHeap waitingPatients;
void coordinatorInterface(Database& db, std::vector<Bed>& beds, int hospital_id);
void rebuildWaitingHeap(const std::vector<Patient*>& patients);
void showPatientQueue();
void addNewPatient(const std::string& prefix, Database& db);
void greedyAllocate(std::vector<Bed>& beds);
void optimalAllocate(std::vector<Bed>& beds, Database& db);
void showAllBeds(const std::vector<Bed>& beds);
void showNearestHospitals(Database& db, Dijkstra& dijkstra, int from_hospital);

#endif

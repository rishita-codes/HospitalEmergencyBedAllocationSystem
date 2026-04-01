#include "bed_coordinator.h"
#include <iostream>
#include <algorithm>
#include <vector>
#include <climits>
#include <chrono>

extern MaxHeap waitingPatients;

static std::vector<Patient*> waiting_list;

static bool hasFreeBeds(const std::vector<Bed>& beds) {
    for (const Bed& b : beds) {
        if (!b.occupied()) {
            return true;
        }
    }
    return false;
}

void rebuildWaitingHeap(const std::vector<Patient*>& patients) {
    waiting_list = patients;
    waitingPatients = MaxHeap();
    for (Patient* p : patients) {
        p->updatePriority();
        waitingPatients.insert(p);
    }
}

void showPatientQueue() {
    std::cout << "\nWaiting patients (top 20):\n";
    auto temp = waitingPatients;
    int count = 0;
    while (Patient* p = temp.extractMax()) {
        std::cout << "  " << p->getId()
                  << " [" << esiToString(p->getESI()) << "] "
                  << p->getName()
                  << " (pri=" << p->getPriority() << ")\n";
        ++count;
        if (count >= 20) break;
    }
}

void addNewPatient(const std::string& prefix, Database& db) {
    std::string name;
    int age, esi_val;

    std::cout << "Enter name: ";
    std::cin.ignore();
    std::getline(std::cin, name);
    std::cout << "Enter age: ";
    std::cin >> age;
    std::cout << "Enter ESI (1=ESI1, 2=ESI2, 3=ESI3, 4=ESI4): ";
    std::cin >> esi_val;

    if (esi_val < 1 || esi_val > 4) {
        std::cout << "Invalid ESI.\n";
        return;
    }

    ESI esi = static_cast<ESI>(esi_val);
    DateTime now = currentTime();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    std::string id = prefix + std::to_string(ms);

    Patient* p = new Patient(id, name, age, esi, now);
    db.insertPatient(p);
    waiting_list.push_back(p);
    p->updatePriority();
    waitingPatients.insert(p);

    std::cout << "Added patient: " << id << " -> " << name << "\n";
}

void greedyAllocate(std::vector<Bed>& beds) {
    GreedyAllocator ga(beds, waitingPatients);
    ga.allocate();
    std::cout << "Greedy allocation completed.\n";
}

void optimalAllocate(std::vector<Bed>& beds, Database& db) {
    std::vector<Patient*> pending;
    auto temp = waitingPatients;
    while (Patient* p = temp.extractMax()) {
        pending.push_back(p);
    }

    if (pending.empty()) {
        std::cout << "No patients in the waiting queue to allocate.\n";
        return;
    }

    bool hasFree = false;
    for (const Bed& b : beds) {
        if (!b.occupied()) {
            hasFree = true;
            break;
        }
    }
    if (!hasFree) {
        std::cout << "No free beds available for this hospital.\n";
        return;
    }

    OptimalAllocator opt(pending, beds);
    opt.computeOptimal();
    opt.showAllocation();

    std::cout << "Save optimal assignment? (y/n): ";
    char answer;
    std::cin >> answer;
    if (answer == 'y' || answer == 'Y') {
        opt.applyAllocation(&db);
        const std::vector<int>& match = opt.getMatch();
        std::vector<Patient*> remaining;
        for (int i = 0; i < match.size(); i++) {
            if (match[i] == -1) {
                remaining.push_back(pending[i]);
            }
        }
        rebuildWaitingHeap(remaining);
        std::cout << "Optimal allocation saved.\n";
    } else {
        std::cout << "Optimal allocation discarded.\n";
    }
}

void showAllBeds(const std::vector<Bed>& beds) {
    std::cout << "\nAll beds (status):\n";
    for (const Bed& b : beds) {
        std::cout << "  " << b.getBedId()
                  << " -> " << (b.occupied() ? b.getAssignedPatientId() : "FREE")
                  << " (type=" << bedTypeToString(b.getType()) << ")\n";
    }
}

void showNearestHospitals(Database& db, Dijkstra& dijkstra, int from_hospital) {
    int source_index = from_hospital > 0 ? from_hospital - 1 : from_hospital;
    if (source_index < 0) source_index = 0;

    std::vector<int> distances = dijkstra.shortestPath(source_index);
    std::cout << "\nNearest hospitals (excluding current hospital):\n";
    for (int i = 0; i < distances.size(); i++) {
        if (i == source_index) continue;
        if (distances[i] == INT_MAX) continue;
        int hospital_id = i + 1;
        std::string hospital_name = db.getHospitalName(hospital_id);
        std::cout << "  " << hospital_name << " (ID " << hospital_id << ")"
                  << " -> distance " << distances[i] << "\n";
    }
}

void coordinatorInterface(Database& db, std::vector<Bed>& beds, int hospital_id) {
    rebuildWaitingHeap(waiting_list);

    int V = 3;  // number of hospitals
    Dijkstra dijkstra(V);
    dijkstra.addEdge(0, 1, 10);
    dijkstra.addEdge(1, 2, 15);

    while (true) {
        std::cout << "\n-----------------------------\n";
        std::cout << "BED COORDINATOR MENU\n";
        std::cout << "1. Check patient queue\n";
        std::cout << "2. Add new patient to queue\n";
        std::cout << "3. Greedy allocate waiting patients\n";
        std::cout << "4. Optimal allocate (Hungarian) waiting patients\n";
        std::cout << "5. Check nearest hospitals\n";
        std::cout << "6. Check bed status of this hospital\n";
        std::cout << "7. Manual bed assignment (single patient)\n";
        std::cout << "0. Logout\n";
        std::cout << "Enter choice: ";

        int choice;
        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            std::cout << "Invalid input.\n";
            continue;
        }

        switch (choice) {
            case 1:
                showPatientQueue();
                break;

            case 2:
                addNewPatient("P", db);
                break;

            case 3: {
                beds = db.getBeds(hospital_id);
                if (!hasFreeBeds(beds)) {
                    std::cout << "No beds available for greedy allocation.\n";
                    break;
                }

                std::vector<Bed> previewBeds = beds;
                MaxHeap previewHeap = waitingPatients;
                GreedyAllocator previewGa(previewBeds, previewHeap, nullptr);
                previewGa.allocate();

                std::cout << "\nGreedy allocation preview:\n";
                showAllBeds(previewBeds);

                std::cout << "Apply this greedy allocation? (y/n): ";
                char confirm;
                std::cin >> confirm;
                if (confirm == 'y' || confirm == 'Y') {
                    GreedyAllocator ga(beds, waitingPatients, &db);
                    ga.allocate();
                    rebuildWaitingHeap(waitingPatients.getPatients());
                    std::cout << "Greedy allocation completed.\n";
                    showAllBeds(beds);
                } else {
                    std::cout << "Greedy allocation canceled.\n";
                }
                break;
            }

            case 4:
                beds = db.getBeds(hospital_id);
                if (!hasFreeBeds(beds)) {
                    std::cout << "No beds available for optimal allocation.\n";
                    break;
                }
                optimalAllocate(beds, db);
                showAllBeds(beds);
                break;

            case 5:
                    showNearestHospitals(db, dijkstra, hospital_id);

            case 6:
                beds = db.getBeds(hospital_id);
                showAllBeds(beds);
                break;

            case 7: {
                std::cout << "Enter patient_id bed_id: ";
                std::string pid, bid;
                std::cin >> pid >> bid;

                beds = db.getBeds(hospital_id);
                Bed* bed = nullptr;
                for (Bed& b : beds) {
                    if (b.getBedId() == bid && !b.occupied()) {
                        bed = &b;
                        break;
                    }
                }
                if (!bed) {
                    std::cout << "Bed not found or occupied.\n";
                    break;
                }

                db.assignBed(pid, bid);
                bed->assignPatient(pid);
                if (waitingPatients.remove(pid)) {
                    rebuildWaitingHeap(waitingPatients.getPatients());
                }
                std::cout << "Bed " << bid << " assigned to " << pid << "\n";
                break;
            }

            case 0:
                std::cout << "Logging out...\n";
                return;

            default:
                std::cout << "Invalid choice.\n";
                break;
        }
    }
}

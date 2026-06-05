#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>

#include "database.h"
#include "greedy_allocator.h"
#include "optimal_allocator.h"

// Simple JSON output helper
std::string jsonEscape(const std::string &s) {
    std::string out;
    for (char c : s) {
        if (c == '"') out += "\\\"";
        else if (c == '\\') out += "\\\\";
        else out += c;
    }
    return out;
}

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Usage: allocator_cli <algorithm> <hospital_id>\n";
        return 1;
    }

    std::string algorithm = argv[1];
    int hospital_id = std::stoi(argv[2]);

    const char* env = std::getenv("DATABASE_URL");
    std::string conninfo;
    if (env && env[0] != '\0') conninfo = env;
    else conninfo = "host=localhost dbname=hospital_db user=postgres";

    Database db(conninfo);
    if (!db.isConnected()) {
        std::cerr << "DB connection failed\n";
        return 2;
    }

    // Get available beds (not occupied, not reserved)
    std::vector<Bed> beds = db.getAvailableBeds(hospital_id);

    // Get waiting patients for this hospital only
    std::vector<Patient*> patients = db.getWaitingPatients(hospital_id);

    if (patients.empty() || beds.empty()) {
        std::cout << "[]\n";
        return 0;
    }

    std::vector<std::pair<std::string, std::string>> assignments;

    if (algorithm == "greedy") {
        // preview: make copies
        std::vector<Bed> previewBeds = beds;
        MaxHeap heap;
        for (Patient* p : patients) {
            p->updatePriority();
            heap.insert(p);
        }
        GreedyAllocator ga(previewBeds, heap, nullptr);
        ga.allocate();
        for (auto &b : previewBeds) {
            if (b.occupied() && !b.getAssignedPatientId().empty()) {
                assignments.emplace_back(b.getAssignedPatientId(), b.getBedId());
            }
        }
    } else {
        // optimal
        std::vector<Patient*> pending;
        MaxHeap tempHeap;
        for (Patient* p : patients) {
            pending.push_back(p);
        }
        OptimalAllocator opt(pending, beds);
        opt.computeOptimal();
        const std::vector<int>& match = opt.getMatch();
        for (int i = 0; i < match.size(); ++i) {
            if (match[i] != -1) {
                int bedIdx = match[i];
                // freeBedsIndex is internal to OptimalAllocator; we used beds directly so match indexes into freeBedsIndex
                // In this CLI we constructed OptimalAllocator with freeBedsIndex based on beds vector, so bedIdx refers to index within freeBedsIndex
                // The OptimalAllocator in repo stores freeBedsIndex; to avoid accessing internals, reproduce mapping:
                // Here OptimalAllocator::getMatch returns match where value is index into freeBedsIndex; we can query bed id via beds[freeBedsIndex[match[i]]] but freeBedsIndex is private.
                // As a quick approach, re-run buildCostMatrix logic here: since we constructed with "beds" as freeBeds, match[j] corresponds to j-th free bed index, so map directly:
                // For safety, if match[i] is within beds size, map directly.
                if (match[i] >= 0 && match[i] < (int)beds.size()) {
                    assignments.emplace_back(pending[i]->getId(), beds[match[i]].getBedId());
                }
            }
        }
    }

    // Output JSON array
    std::ostringstream out;
    out << "[";
    for (size_t i = 0; i < assignments.size(); ++i) {
        if (i) out << ",";
        out << "{\"patient_id\":\"" << jsonEscape(assignments[i].first) << "\",\"bed_id\":\"" << jsonEscape(assignments[i].second) << "\"}";
    }
    out << "]";

    std::cout << out.str() << std::endl;
    return 0;
}

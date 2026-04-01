/*
g++ -std=c++17 main.cpp database.cpp priority_queue.cpp basic_structs.cpp greedy_allocator.cpp optimal_allocator.cpp dijkstra.cpp bipartite_matching.cpp bed_coordinator.cpp nurse_interface.cpp -I/usr/include/postgresql -lpq -o hospital_app

*/

#include <iostream>
#include <vector>
#include <algorithm>
#include <cctype>

#include "database.h"
#include "bed_coordinator.h"
#include "basic_structs.h"
#include "priority_queue.h"
#include "greedy_allocator.h"
#include "optimal_allocator.h"
#include "dijkstra.h"

extern MaxHeap waitingPatients;
MaxHeap waitingPatients;

void coordinatorInterface(Database &mydb, std::vector<Bed> &beds, int hospital_id);
void nurseInterface(Database &db, std::vector<Bed> &beds, int hospital_id);      

int main()
{
    Database db("host=localhost dbname=mydb user=postgres password=ohho83yo");

    if (!db.isConnected())
    {
        std::cerr << "Could not connect to database.\n";
        return 1;
    }

    db.ensureCorrectSchema();
    if (!db.hasSeedData())
    {
        db.seedSampleData();
    }

    std::string username, password;
    std::cout << "Enter username: ";
    std::cin >> username;
    std::cout << "Enter password: ";
    std::cin >> password;

    User u = db.login(username, password);

    if (u.user_id == -1)
    {
        std::cout << "Login failed.\n";
        return 1;
    }

    std::string hospital_name = db.getHospitalName(u.hospital_id);
    std::cout << "\n=====================================================================\n";
    std::cout << "  HOSPITAL EMERGENCY BED ALLOCATION SYSTEM\n";
    std::cout << "=======================================================================\n";
    std::cout << "  Current Hospital: " << hospital_name << " (ID " << u.hospital_id << ")\n";
    std::cout << "=====================================================================\n";
    std::cout << "Welcome " << u.role << "!\n";

    std::vector<Patient *> patients = db.getWaitingPatients();
    rebuildWaitingHeap(patients);

    auto trim = [&](std::string s)
    {
        size_t start = s.find_first_not_of(" \t\n\r");
        if (start == std::string::npos)
            return std::string();
        size_t end = s.find_last_not_of(" \t\n\r");
        return s.substr(start, end - start + 1);
    };

    std::string role = trim(u.role);
    std::transform(role.begin(), role.end(), role.begin(),
                   [](unsigned char c)
                   { return std::tolower(c); });

    std::vector<Bed> beds;

    if (role.find("coordinator") != std::string::npos)
    {
        coordinatorInterface(db, beds, u.hospital_id);
    }
    else if (role.find("nurse") != std::string::npos)
    {
        nurseInterface(db, beds, u.hospital_id);
    }
    else
    {
        std::cout << "Role not supported: '" << u.role << "'\n";
    }

    return 0;
}
/*
g++ login.cpp -I/usr/include/postgresql -lpq -o app
g++ login.cpp database.cpp priority_queue.cpp -I/usr/include/postgresql -lpq -o app

g++ -std=c++17 \
    login.cpp \
    database.cpp \
    priority_queue.cpp \
    basic_structs.cpp \
    -I/usr/include/postgresql \
    -lpq \
    -o app

./app

*/
#include "login.h"
#include "database.h"
#include "basic_structs.h"
#include "priority_queue.h"
#include "greedy_allocator.h"
#include <iostream>

static MaxHeap waitingPatients;

// Add some test patients once (or read from DB later)
void populateTestPatients() {
    DateTime now = currentTime();
    Patient* p1 = new Patient("P001", "Alice", 45, ESI1, now);
    p1->updatePriority();
    waitingPatients.insert(p1);

    Patient* p2 = new Patient("P002", "Bob", 30, ESI3, now);
    p2->updatePriority();
    waitingPatients.insert(p2);
}

User handleLogin(Database& db) {
    std::string username, password;

    std::cout << "Enter username: ";
    std::cin >> username;
    std::cout << "Enter password: ";
    std::cin >> password;

    User u = db.login(username, password);

    if (u.user_id == -1) {
        std::cout << "Login failed.\n";
    } else {
        std::cout << "Welcome " << u.role << "!\n";
    }

    return u;
}

void startSession(Database& db, User u, std::vector<Bed>& beds) {
    populateTestPatients();

    if (u.role == "coordinator") {
        
    } else if (u.role == "nurse") {
    } else {
        std::cout << "Role not supported yet.\n";
    }
}
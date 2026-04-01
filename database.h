#ifndef DATABASE_H
#define DATABASE_H

#include "basic_structs.h"
#include <string>
#include <vector>

extern "C" {
    #include <libpq-fe.h>
}

class Database {
private:
    PGconn* conn;

public:
    Database(const std::string& conninfo);
    ~Database();

    bool isConnected();

    void insertPatient(Patient* p);
    std::vector<Patient*> getWaitingPatients();
    std::string getHospitalName(int hospital_id);
    bool hasSeedData();
    void ensureCorrectSchema();
    void seedSampleData();
    std::vector<Bed> getBeds(int hospital_id = -1);
    void assignBed(const std::string& patient_id, const std::string& bed_id);
    void freeBed(const std::string& bed_id);
    void deletePatient(const std::string& patient_id);
    User login(std::string username, std::string password);
};

#endif
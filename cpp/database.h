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
    std::vector<Patient*> getWaitingPatients(int hospital_id = -1);
    std::string getHospitalName(int hospital_id);
    bool hasSeedData();
    void ensureCorrectSchema();
    void seedSampleData();
    std::vector<Bed> getBeds(int hospital_id = -1);
    void assignBed(const std::string& patient_id, const std::string& bed_id);
    void freeBed(const std::string& bed_id);
    void deletePatient(const std::string& patient_id);
    User login(std::string username, std::string password);

    // New patient interface methods
    Patient* patientLogin(const std::string& phone_number);
    int createPatientRequest(const std::string& phone_number, const std::string& name, int age, int esi_level, int desired_bed_type, int preferred_hospital_id = -1);
    std::vector<std::tuple<int, std::string, int>> findAvailableHospitals(int desired_bed_type, int max_results = 3);
    int createReservation(int request_id, int patient_id, const std::string& bed_id, int hospital_id);
    void expireReservations();
    std::vector<Bed> getAvailableBeds(int hospital_id);
};

#endif
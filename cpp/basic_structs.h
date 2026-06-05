#ifndef BASIC_STRUCTS_H
#define BASIC_STRUCTS_H

#include <string>
#include <vector>
#include <chrono>
#include <ctime>
#include <tuple>
#include <climits>

extern "C" {
    #include <libpq-fe.h>
}

struct DateTime {
    int year, month, day;
    int hour, min, sec;

    bool operator<(const DateTime& other) const {
        return std::tie(year, month, day, hour, min, sec) <
               std::tie(other.year, other.month, other.day, other.hour, other.min, other.sec);
    }
};

std::time_t toEpoch(const DateTime& dt);
long long subtract(const DateTime& later, const DateTime& earlier);
DateTime currentTime();

enum ESI {
    ESI1 = 1,
    ESI2 = 2,
    ESI3 = 3,
    ESI4 = 4
};

inline std::string esiToString(ESI value) {
    switch (value) {
        case ESI1: return "ESI1";
        case ESI2: return "ESI2";
        case ESI3: return "ESI3";
        case ESI4: return "ESI4";
        default: return "UNKNOWN";
    }
}

class Patient {
private:
    std::string patient_id, name, phone_number;
    int age;
    ESI esi;
    double priority;
    DateTime arrival_time;

public:
    Patient(std::string p_id, std::string n, int a, ESI e, DateTime at);

    std::string getId() { return patient_id; }
    std::string getName() { return name; }
    std::string getPhoneNumber() { return phone_number; }
    int getAge() { return age; }
    ESI getESI() { return esi; }
    DateTime getArrivalTime() { return arrival_time; }

    double getPriority() const { return priority; }

    void setId(std::string id) { patient_id = id; }
    void setPhoneNumber(std::string phone) { phone_number = phone; }

    double calculatePriority();
    void updatePriority();
};

enum BedType {
    ICU,
    EMERGENCY,
    GENERAL,
    SPECIALITY
};

inline std::string bedTypeToString(BedType value) {
    switch (value) {
        case ICU: return "ICU";
        case EMERGENCY: return "EMERGENCY";
        case GENERAL: return "GENERAL";
        case SPECIALITY: return "SPECIALITY";
        default: return "UNKNOWN";
    }
}

class Bed {
private:
    std::string bed_id, assigned_patient_id;
    BedType type;
    bool is_occupied;
    int hospital_id;

public:
    Bed(std::string b_id, BedType t, bool occ = false, std::string p_id = "", int hosp_id = -1);

    std::string getBedId() const { return bed_id; }
    std::string getAssignedPatientId() const { return assigned_patient_id; }
    BedType getType() const;
    bool occupied() const;
    int getHospitalId() const;

    void assignPatient(const std::string& p_id);
    void freeBed();
};

struct User {
    int user_id;
    std::string role;
    int hospital_id;
};

#endif
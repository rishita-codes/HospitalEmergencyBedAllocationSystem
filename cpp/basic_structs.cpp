#include "basic_structs.h"
#include <ctime>
#include <tuple>

std::time_t toEpoch(const DateTime& dt) {
    std::tm t{};
    t.tm_year = dt.year - 1900;
    t.tm_mon  = dt.month - 1;
    t.tm_mday = dt.day;
    t.tm_hour = dt.hour;
    t.tm_min  = dt.min;
    t.tm_sec  = dt.sec;
    t.tm_isdst = -1;
    return std::mktime(&t);
}

long long subtract(const DateTime& later, const DateTime& earlier) {
    return static_cast<long long>(toEpoch(later) - toEpoch(earlier));
}

DateTime currentTime() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm* local_time = std::localtime(&now_time_t);
    DateTime cur;
    cur.year  = local_time->tm_year + 1900;
    cur.month = local_time->tm_mon  + 1;
    cur.day   = local_time->tm_mday;
    cur.hour  = local_time->tm_hour;
    cur.min   = local_time->tm_min;
    cur.sec   = local_time->tm_sec;
    return cur;
}

Patient::Patient(std::string p_id, std::string n, int a, ESI e, DateTime at)
    : patient_id(p_id), name(n), phone_number(""), age(a), esi(e), arrival_time(at), priority(0.0)
{}

double Patient::calculatePriority() {
    int weight;
    DateTime cur = currentTime();
    double wait_time = subtract(cur, arrival_time) / 60.0;  // minutes
    switch(esi) {
        case ESI::ESI1: weight = 100; break;
        case ESI::ESI2: weight = 70;  break;
        case ESI::ESI3: weight = 40;  break;
        case ESI::ESI4: weight = 10;  break;
    }
    return weight + wait_time;
}

void Patient::updatePriority() {
    priority = calculatePriority();
}

Bed::Bed(std::string b_id, BedType t, bool occ, std::string p_id, int hosp_id)
    : bed_id(b_id), type(t), is_occupied(occ), assigned_patient_id(p_id), hospital_id(hosp_id)
{}

int Bed::getHospitalId() const {
    return hospital_id;
}

void Bed::assignPatient(const std::string& p_id) {
    assigned_patient_id = p_id;
    is_occupied = true;
}

void Bed::freeBed() {
    assigned_patient_id = "";
    is_occupied = false;
}

BedType Bed::getType() const {
    return type;
}

bool Bed::occupied() const {
    return is_occupied;
}
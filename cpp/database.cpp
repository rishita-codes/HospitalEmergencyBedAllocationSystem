/*
g++ -std=c++17 \
    login.cpp database.cpp priority_queue.cpp basic_structs.cpp \
    -I/usr/include/postgresql -lpq -o app
*/

#include "database.h"
#include <iostream>
#include <cstdlib>

Database::Database(const std::string& conninfo) {
    conn = PQconnectdb(conninfo.c_str());

    if (conn == nullptr || PQstatus(conn) != CONNECTION_OK) {
        std::cerr << "Database connection failed: "
                  << PQerrorMessage(conn) << std::endl;
    } else {
        std::cerr << "Connected to database successfully!\n";
    }
}

Database::~Database() {
    if (conn) {
        PQfinish(conn);
    }
}

bool Database::isConnected() {
    return conn && PQstatus(conn) == CONNECTION_OK;
}

void Database::insertPatient(Patient* p) {
    std::string query =
        "INSERT INTO patients (name, age, esi_level, phone_number) "
        "VALUES ($1, $2, $3, $4) "
        "RETURNING patient_id;";

    const char* paramValues[4];
    int paramLengths[4] = {0, 0, 0, 0};
    int paramFormats[4]  = {0, 0, 0, 0};

    std::string name = p->getName();
    std::string age  = std::to_string(p->getAge());
    std::string esi  = std::to_string(static_cast<int>(p->getESI()));
    std::string phone = p->getPhoneNumber();

    paramValues[0] = name.c_str();
    paramValues[1] = age.c_str();
    paramValues[2] = esi.c_str();
    paramValues[3] = phone.c_str();

    PGresult* res = PQexecParams(
        conn,
        query.c_str(),
        4,
        NULL,
        paramValues,
        paramLengths,
        paramFormats,
        0
    );

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        std::cerr << "Insert Patient failed: "
                  << PQerrorMessage(conn) << std::endl;
    } else {
        // Update patient ID from returned value
        if (PQntuples(res) > 0) {
            int new_id = std::stoi(PQgetvalue(res, 0, 0));
            p->setId(std::to_string(new_id));
        }
    }

    PQclear(res);
}

std::vector<Patient*> Database::getWaitingPatients(int hospital_id) {
    std::vector<Patient*> patients;

    std::string query =
        "SELECT DISTINCT p.patient_id, p.name, p.age, p.esi_level, p.phone_number, p.created_at "
        "FROM patients p "
        "JOIN requests r ON r.patient_id = p.patient_id "
        "WHERE r.status = 'pending' ";

    std::vector<std::string> params;
    if (hospital_id >= 0) {
        query += "AND r.preferred_hospital_id = $1 ";
        params.push_back(std::to_string(hospital_id));
    }

    query +=
        "AND p.patient_id NOT IN ("
        " SELECT assigned_patient_id FROM beds WHERE assigned_patient_id IS NOT NULL"
        " ) AND p.patient_id NOT IN ("
        " SELECT patient_id FROM reservations WHERE status = 'active'"
        " ) "
        "ORDER BY p.created_at ASC;";

    PGresult* res = nullptr;
    if (params.empty()) {
        res = PQexec(conn, query.c_str());
    } else {
        const char* paramValues[1] = { params[0].c_str() };
        res = PQexecParams(conn, query.c_str(), 1, NULL, paramValues, NULL, NULL, 0);
    }
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        std::cerr << "Fetch waiting patients failed: "
                  << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        return patients;
    }

    int rows = PQntuples(res);
    for (int i = 0; i < rows; i++) {
        int id = std::atoi(PQgetvalue(res, i, 0));
        std::string name = PQgetvalue(res, i, 1);
        int age = std::atoi(PQgetvalue(res, i, 2));
        ESI esi = static_cast<ESI>(std::atoi(PQgetvalue(res, i, 3)));
        std::string phone = PQgetvalue(res, i, 4);
        std::string created_at = PQgetvalue(res, i, 5);

        DateTime dt;
        if (created_at.size() >= 19) {
            dt.year  = std::atoi(created_at.substr(0, 4).c_str());
            dt.month = std::atoi(created_at.substr(5, 2).c_str());
            dt.day   = std::atoi(created_at.substr(8, 2).c_str());
            dt.hour  = std::atoi(created_at.substr(11, 2).c_str());
            dt.min   = std::atoi(created_at.substr(14, 2).c_str());
            dt.sec   = std::atoi(created_at.substr(17, 2).c_str());
        } else {
            dt = currentTime();
        }

        Patient* p = new Patient(std::to_string(id), name, age, esi, dt);
        p->setPhoneNumber(phone);
        p->updatePriority();
        patients.push_back(p);
    }

    PQclear(res);
    return patients;
}

std::string Database::getHospitalName(int hospital_id) {
    std::string query =
        "SELECT name FROM hospitals WHERE hospital_id = $1;";

    std::string hospital_id_str = std::to_string(hospital_id);
    const char* paramValues[1] = { hospital_id_str.c_str() };

    PGresult* res = PQexecParams(
        conn,
        query.c_str(),
        1,
        NULL,
        paramValues,
        NULL,
        NULL,
        0
    );

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        PQclear(res);
        return "Hospital " + hospital_id_str;
    }

    if (PQntuples(res) == 0) {
        PQclear(res);
        return "Hospital " + hospital_id_str;
    }

    std::string name = PQgetvalue(res, 0, 0);
    PQclear(res);
    return name;
}

bool Database::hasSeedData() {
    PGresult* res = PQexec(conn, "SELECT EXISTS (SELECT 1 FROM users);");
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        std::cerr << "Seed check failed: " << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        return false;
    }

    bool exists = std::string(PQgetvalue(res, 0, 0)) == "t";
    PQclear(res);
    return exists;
}

void Database::ensureCorrectSchema() {
    PGresult* res = PQexec(conn, R"(
ALTER TABLE patients DROP CONSTRAINT IF EXISTS patients_esi_check;
ALTER TABLE patients ADD CONSTRAINT patients_esi_check CHECK (esi_level >= 1 AND esi_level <= 4);
)");
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        std::cerr << "Schema fix failed: " << PQerrorMessage(conn) << std::endl;
    }
    PQclear(res);
}

void Database::seedSampleData() {
    const char* dataSql = R"(
ALTER TABLE patients DROP CONSTRAINT IF EXISTS patients_esi_check;
ALTER TABLE patients ADD CONSTRAINT patients_esi_check CHECK (esi_level >= 1 AND esi_level <= 4);

INSERT INTO hospitals (hospital_id, name)
SELECT 1, 'St. Mary General Hospital'
WHERE NOT EXISTS (SELECT 1 FROM hospitals WHERE hospital_id = 1);
INSERT INTO hospitals (hospital_id, name)
SELECT 2, 'City Central Emergency Hospital'
WHERE NOT EXISTS (SELECT 1 FROM hospitals WHERE hospital_id = 2);
INSERT INTO hospitals (hospital_id, name)
SELECT 3, 'Riverside Medical Center'
WHERE NOT EXISTS (SELECT 1 FROM hospitals WHERE hospital_id = 3);

INSERT INTO users (user_id, username, password_hash, role, hospital_id)
SELECT 1, 'coord1', 'pass123', 'coordinator', 1
WHERE NOT EXISTS (SELECT 1 FROM users WHERE username = 'coord1');
INSERT INTO users (user_id, username, password_hash, role, hospital_id)
SELECT 2, 'nurse1', 'nurse123', 'nurse', 1
WHERE NOT EXISTS (SELECT 1 FROM users WHERE username = 'nurse1');
INSERT INTO users (user_id, username, password_hash, role, hospital_id)
SELECT 3, 'coord2', 'pass123', 'coordinator', 2
WHERE NOT EXISTS (SELECT 1 FROM users WHERE username = 'coord2');
INSERT INTO users (user_id, username, password_hash, role, hospital_id)
SELECT 4, 'nurse2', 'nurse123', 'nurse', 2
WHERE NOT EXISTS (SELECT 1 FROM users WHERE username = 'nurse2');
INSERT INTO users (user_id, username, password_hash, role, hospital_id)
SELECT 5, 'coord3', 'pass123', 'coordinator', 3
WHERE NOT EXISTS (SELECT 1 FROM users WHERE username = 'coord3');
INSERT INTO users (user_id, username, password_hash, role, hospital_id)
SELECT 6, 'nurse3', 'nurse123', 'nurse', 3
WHERE NOT EXISTS (SELECT 1 FROM users WHERE username = 'nurse3');

INSERT INTO beds (bed_id, type, is_occupied, assigned_patient_id, hospital_id)
SELECT 'H1-ICU1', 0, false, NULL, 1
WHERE NOT EXISTS (SELECT 1 FROM beds WHERE bed_id = 'H1-ICU1');
INSERT INTO beds (bed_id, type, is_occupied, assigned_patient_id, hospital_id)
SELECT 'H1-EMG1', 1, false, NULL, 1
WHERE NOT EXISTS (SELECT 1 FROM beds WHERE bed_id = 'H1-EMG1');
INSERT INTO beds (bed_id, type, is_occupied, assigned_patient_id, hospital_id)
SELECT 'H1-GEN1', 2, false, NULL, 1
WHERE NOT EXISTS (SELECT 1 FROM beds WHERE bed_id = 'H1-GEN1');
INSERT INTO beds (bed_id, type, is_occupied, assigned_patient_id, hospital_id)
SELECT 'H1-SP1', 3, false, NULL, 1
WHERE NOT EXISTS (SELECT 1 FROM beds WHERE bed_id = 'H1-SP1');
INSERT INTO beds (bed_id, type, is_occupied, assigned_patient_id, hospital_id)
SELECT 'H1-GEN2', 2, false, NULL, 1
WHERE NOT EXISTS (SELECT 1 FROM beds WHERE bed_id = 'H1-GEN2');

INSERT INTO beds (bed_id, type, is_occupied, assigned_patient_id, hospital_id)
SELECT 'H2-ICU1', 0, false, NULL, 2
WHERE NOT EXISTS (SELECT 1 FROM beds WHERE bed_id = 'H2-ICU1');
INSERT INTO beds (bed_id, type, is_occupied, assigned_patient_id, hospital_id)
SELECT 'H2-EMG1', 1, false, NULL, 2
WHERE NOT EXISTS (SELECT 1 FROM beds WHERE bed_id = 'H2-EMG1');
INSERT INTO beds (bed_id, type, is_occupied, assigned_patient_id, hospital_id)
SELECT 'H2-GEN1', 2, false, NULL, 2
WHERE NOT EXISTS (SELECT 1 FROM beds WHERE bed_id = 'H2-GEN1');
INSERT INTO beds (bed_id, type, is_occupied, assigned_patient_id, hospital_id)
SELECT 'H2-SP1', 3, false, NULL, 2
WHERE NOT EXISTS (SELECT 1 FROM beds WHERE bed_id = 'H2-SP1');
INSERT INTO beds (bed_id, type, is_occupied, assigned_patient_id, hospital_id)
SELECT 'H2-GEN2', 2, false, NULL, 2
WHERE NOT EXISTS (SELECT 1 FROM beds WHERE bed_id = 'H2-GEN2');

INSERT INTO beds (bed_id, type, is_occupied, assigned_patient_id, hospital_id)
SELECT 'H3-ICU1', 0, false, NULL, 3
WHERE NOT EXISTS (SELECT 1 FROM beds WHERE bed_id = 'H3-ICU1');
INSERT INTO beds (bed_id, type, is_occupied, assigned_patient_id, hospital_id)
SELECT 'H3-EMG1', 1, false, NULL, 3
WHERE NOT EXISTS (SELECT 1 FROM beds WHERE bed_id = 'H3-EMG1');
INSERT INTO beds (bed_id, type, is_occupied, assigned_patient_id, hospital_id)
SELECT 'H3-GEN1', 2, false, NULL, 3
WHERE NOT EXISTS (SELECT 1 FROM beds WHERE bed_id = 'H3-GEN1');
INSERT INTO beds (bed_id, type, is_occupied, assigned_patient_id, hospital_id)
SELECT 'H3-SP1', 3, false, NULL, 3
WHERE NOT EXISTS (SELECT 1 FROM beds WHERE bed_id = 'H3-SP1');
INSERT INTO beds (bed_id, type, is_occupied, assigned_patient_id, hospital_id)
SELECT 'H3-GEN2', 2, false, NULL, 3
WHERE NOT EXISTS (SELECT 1 FROM beds WHERE bed_id = 'H3-GEN2');

INSERT INTO patients (name, age, esi_level, phone_number, created_at)
SELECT 'Maria Chen', 58, 1, '555-0100', NOW()
WHERE NOT EXISTS (SELECT 1 FROM patients WHERE phone_number = '555-0100');
INSERT INTO patients (name, age, esi_level, phone_number, created_at)
SELECT 'Samuel Ortiz', 46, 2, '555-0101', NOW()
WHERE NOT EXISTS (SELECT 1 FROM patients WHERE phone_number = '555-0101');
INSERT INTO patients (name, age, esi_level, phone_number, created_at)
SELECT 'Priya Singh', 29, 3, '555-0102', NOW()
WHERE NOT EXISTS (SELECT 1 FROM patients WHERE phone_number = '555-0102');
INSERT INTO patients (name, age, esi_level, phone_number, created_at)
SELECT 'Ethan Brooks', 65, 1, '555-0103', NOW()
WHERE NOT EXISTS (SELECT 1 FROM patients WHERE phone_number = '555-0103');
INSERT INTO patients (name, age, esi_level, phone_number, created_at)
SELECT 'Nina Ahmed', 34, 4, '555-0104', NOW()
WHERE NOT EXISTS (SELECT 1 FROM patients WHERE phone_number = '555-0104');
)";

    PGresult* res = PQexec(conn, dataSql);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        std::cerr << "Seed sample data failed: " << PQerrorMessage(conn) << std::endl;
    }
    PQclear(res);
}

std::vector<Bed> Database::getBeds(int hospital_id) {
    std::vector<Bed> beds;

    std::string query =
        "SELECT bed_id, type, is_occupied, assigned_patient_id, hospital_id FROM beds";
    PGresult* res;
    std::string hospital_id_str;

    if (hospital_id >= 0) {
        query += " WHERE hospital_id = $1;";
        hospital_id_str = std::to_string(hospital_id);
        const char* paramValues[1] = { hospital_id_str.c_str() };
        res = PQexecParams(
            conn,
            query.c_str(),
            1,
            NULL,
            paramValues,
            NULL,
            NULL,
            0
        );
    } else {
        query += ";";
        res = PQexec(conn, query.c_str());
    }

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        std::cerr << "Fetch beds failed: "
                  << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        return beds;
    }

    int rows = PQntuples(res);

    for (int i = 0; i < rows; i++) {
        std::string id = PQgetvalue(res, i, 0);
        int type       = std::atoi(PQgetvalue(res, i, 1));
        bool occ       = (std::string(PQgetvalue(res, i, 2)) == "t");
        std::string p_id_col = PQgetvalue(res, i, 3);
        std::string p_id =
            (PQgetisnull(res, i, 3) ? "" : p_id_col);
        int hosp_id = std::atoi(PQgetvalue(res, i, 4));

        beds.emplace_back(id, static_cast<BedType>(type), occ, p_id, hosp_id);
    }

    PQclear(res);
    return beds;
}

std::vector<Bed> Database::getAvailableBeds(int hospital_id) {
    std::vector<Bed> beds;

    std::string query =
        "SELECT bed_id, type, is_occupied, assigned_patient_id, hospital_id FROM beds "
        "WHERE is_occupied = false AND bed_id NOT IN (SELECT bed_id FROM reservations WHERE status = 'active')";

    PGresult* res;
    std::string hospital_id_str;
    if (hospital_id >= 0) {
        query += " AND hospital_id = $1;";
        hospital_id_str = std::to_string(hospital_id);
        const char* paramValues[1] = { hospital_id_str.c_str() };
        res = PQexecParams(
            conn,
            query.c_str(),
            1,
            NULL,
            paramValues,
            NULL,
            NULL,
            0
        );
    } else {
        query += ";";
        res = PQexec(conn, query.c_str());
    }

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        std::cerr << "Fetch available beds failed: "
                  << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        return beds;
    }

    int rows = PQntuples(res);
    for (int i = 0; i < rows; i++) {
        std::string id = PQgetvalue(res, i, 0);
        int type       = std::atoi(PQgetvalue(res, i, 1));
        bool occ       = (std::string(PQgetvalue(res, i, 2)) == "t");
        std::string p_id_col = PQgetvalue(res, i, 3);
        std::string p_id = (PQgetisnull(res, i, 3) ? "" : p_id_col);
        int hosp_id = std::atoi(PQgetvalue(res, i, 4));

        beds.emplace_back(id, static_cast<BedType>(type), occ, p_id, hosp_id);
    }

    PQclear(res);
    return beds;
}

void Database::assignBed(const std::string& patient_id, const std::string& bed_id) {
    const char* paramValues[2] = {
        patient_id.c_str(),
        bed_id.c_str()
    };

    PGresult* res = PQexecParams(
        conn,
        "UPDATE beds SET is_occupied = true, assigned_patient_id = $1 "
        "WHERE bed_id = $2;",
        2,
        NULL,
        paramValues,
        NULL,
        NULL,
        0
    );

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        std::cerr << "Assign bed failed: "
                  << PQerrorMessage(conn) << std::endl;
    }

    PQclear(res);
}

void Database::freeBed(const std::string& bed_id) {
    const char* paramValues[1] = { bed_id.c_str() };

    PGresult* res = PQexecParams(
        conn,
        "UPDATE beds SET is_occupied = false, assigned_patient_id = NULL "
        "WHERE bed_id = $1;",
        1,
        NULL,
        paramValues,
        NULL,
        NULL,
        0
    );

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        std::cerr << "Free bed failed: "
                  << PQerrorMessage(conn) << std::endl;
    }

    PQclear(res);
}

void Database::deletePatient(const std::string& patient_id) {
    const char* paramValues[1] = { patient_id.c_str() };

    PGresult* res = PQexecParams(
        conn,
        "DELETE FROM patients WHERE patient_id = $1;",
        1,
        NULL,
        paramValues,
        NULL,
        NULL,
        0
    );

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        std::cerr << "Delete patient failed: "
                  << PQerrorMessage(conn) << std::endl;
    }

    PQclear(res);
}

User Database::login(std::string username, std::string password) {
    User u;
    u.user_id = -1;

    std::string query =
        "SELECT user_id, role, hospital_id "
        "FROM users "
        "WHERE username = $1 AND password_hash = $2;";

    const char* paramValues[2] = { username.c_str(), password.c_str() };

    PGresult* res = PQexecParams(
        conn,
        query.c_str(),
        2,
        NULL,
        paramValues,
        NULL,
        NULL,
        0
    );

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        std::cerr << "Login query failed: "
                  << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        return u;
    }

    if (PQntuples(res) == 0) {
        std::cout << "Login failed!\n";
        PQclear(res);
        return u;
    }

    u.user_id     = std::stoi(PQgetvalue(res, 0, 0));
    u.role        = PQgetvalue(res, 0, 1);
    u.hospital_id = std::stoi(PQgetvalue(res, 0, 2));

    std::cout << "Login successful!\n";
    std::cout << "Role: " << u.role << "\n";

    PQclear(res);
    return u;
}

// New patient interface methods
Patient* Database::patientLogin(const std::string& phone_number) {
    std::string query =
        "SELECT patient_id, name, age, esi_level, phone_number, created_at "
        "FROM patients WHERE phone_number = $1;";

    const char* paramValues[1] = { phone_number.c_str() };

    PGresult* res = PQexecParams(
        conn,
        query.c_str(),
        1,
        NULL,
        paramValues,
        NULL,
        NULL,
        0
    );

    if (PQresultStatus(res) != PGRES_TUPLES_OK || PQntuples(res) == 0) {
        PQclear(res);
        return nullptr;
    }

    int id = std::atoi(PQgetvalue(res, 0, 0));
    std::string name = PQgetvalue(res, 0, 1);
    int age = std::atoi(PQgetvalue(res, 0, 2));
    ESI esi = static_cast<ESI>(std::atoi(PQgetvalue(res, 0, 3)));
    std::string phone = PQgetvalue(res, 0, 4);
    std::string created_at = PQgetvalue(res, 0, 5);

    DateTime dt;
    if (created_at.size() >= 19) {
        dt.year  = std::atoi(created_at.substr(0, 4).c_str());
        dt.month = std::atoi(created_at.substr(5, 2).c_str());
        dt.day   = std::atoi(created_at.substr(8, 2).c_str());
        dt.hour  = std::atoi(created_at.substr(11, 2).c_str());
        dt.min   = std::atoi(created_at.substr(14, 2).c_str());
        dt.sec   = std::atoi(created_at.substr(17, 2).c_str());
    } else {
        dt = currentTime();
    }

    Patient* p = new Patient(std::to_string(id), name, age, esi, dt);
    p->setPhoneNumber(phone);

    PQclear(res);
    return p;
}

int Database::createPatientRequest(const std::string& phone_number, const std::string& name, int age, int esi_level, int desired_bed_type, int preferred_hospital_id) {
    // First, ensure patient exists
    Patient* existing = patientLogin(phone_number);
    int patient_id;
    if (existing) {
        patient_id = std::stoi(existing->getId());
        delete existing;
    } else {
        // Create new patient
        std::string insert_query =
            "INSERT INTO patients (phone_number, name, age, esi_level) "
            "VALUES ($1, $2, $3, $4) RETURNING patient_id;";

        std::string age_str = std::to_string(age);
        std::string esi_str = std::to_string(esi_level);
        const char* insert_params[4] = {
            phone_number.c_str(),
            name.c_str(),
            age_str.c_str(),
            esi_str.c_str()
        };

        PGresult* insert_res = PQexecParams(
            conn,
            insert_query.c_str(),
            4,
            NULL,
            insert_params,
            NULL,
            NULL,
            0
        );

        if (PQresultStatus(insert_res) != PGRES_TUPLES_OK) {
            PQclear(insert_res);
            return -1;
        }

        patient_id = std::atoi(PQgetvalue(insert_res, 0, 0));
        PQclear(insert_res);
    }

    // Create request
    std::string request_query =
        "INSERT INTO requests (patient_id, phone_number, name, age, esi_level, desired_bed_type, preferred_hospital_id, expires_at) "
        "VALUES ($1, $2, $3, $4, $5, $6, $7, NOW() + INTERVAL '30 minutes') RETURNING request_id;";

    std::string patient_id_str = std::to_string(patient_id);
    std::string age_str = std::to_string(age);
    std::string esi_str = std::to_string(esi_level);
    std::string desired_bed_type_str = std::to_string(desired_bed_type);
    std::string preferred_hospital_id_str;
    const char* preferred_hospital_id_cstr = nullptr;
    if (preferred_hospital_id > 0) {
        preferred_hospital_id_str = std::to_string(preferred_hospital_id);
        preferred_hospital_id_cstr = preferred_hospital_id_str.c_str();
    }

    const char* request_params[7] = {
        patient_id_str.c_str(),
        phone_number.c_str(),
        name.c_str(),
        age_str.c_str(),
        esi_str.c_str(),
        desired_bed_type_str.c_str(),
        preferred_hospital_id_cstr
    };

    PGresult* request_res = PQexecParams(
        conn,
        request_query.c_str(),
        7,
        NULL,
        request_params,
        NULL,
        NULL,
        0
    );

    if (PQresultStatus(request_res) != PGRES_TUPLES_OK) {
        PQclear(request_res);
        return -1;
    }

    int request_id = std::atoi(PQgetvalue(request_res, 0, 0));
    PQclear(request_res);
    return request_id;
}

std::vector<std::tuple<int, std::string, int>> Database::findAvailableHospitals(int desired_bed_type, int max_results) {
    std::vector<std::tuple<int, std::string, int>> results;

    // Find hospitals with available beds of desired type or higher
    std::string query = R"(
        SELECT h.hospital_id, h.name, COUNT(b.bed_id) as available_count
        FROM hospitals h
        JOIN beds b ON h.hospital_id = b.hospital_id
        WHERE b.is_occupied = false
        AND b.type >= $1
        AND b.bed_id NOT IN (
            SELECT bed_id FROM reservations WHERE status = 'active'
        )
        GROUP BY h.hospital_id, h.name
        ORDER BY available_count DESC
        LIMIT $2;
    )";

    std::string desired_bed_type_str = std::to_string(desired_bed_type);
    std::string max_results_str = std::to_string(max_results);
    const char* params[2] = {
        desired_bed_type_str.c_str(),
        max_results_str.c_str()
    };

    PGresult* res = PQexecParams(
        conn,
        query.c_str(),
        2,
        NULL,
        params,
        NULL,
        NULL,
        0
    );

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        PQclear(res);
        return results;
    }

    int rows = PQntuples(res);
    for (int i = 0; i < rows; i++) {
        int hospital_id = std::atoi(PQgetvalue(res, i, 0));
        std::string name = PQgetvalue(res, i, 1);
        int count = std::atoi(PQgetvalue(res, i, 2));
        results.emplace_back(hospital_id, name, count);
    }

    PQclear(res);
    return results;
}

int Database::createReservation(int request_id, int patient_id, const std::string& bed_id, int hospital_id) {
    std::string query =
        "INSERT INTO reservations (request_id, patient_id, bed_id, hospital_id, reserved_until) "
        "VALUES ($1, $2, $3, $4, NOW() + INTERVAL '30 minutes') RETURNING reservation_id;";

    std::string request_id_str = std::to_string(request_id);
    std::string patient_id_str = std::to_string(patient_id);
    std::string hospital_id_str = std::to_string(hospital_id);
    const char* params[4] = {
        request_id_str.c_str(),
        patient_id_str.c_str(),
        bed_id.c_str(),
        hospital_id_str.c_str()
    };

    PGresult* res = PQexecParams(
        conn,
        query.c_str(),
        4,
        NULL,
        params,
        NULL,
        NULL,
        0
    );

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        PQclear(res);
        return -1;
    }

    int reservation_id = std::atoi(PQgetvalue(res, 0, 0));
    PQclear(res);
    return reservation_id;
}

void Database::expireReservations() {
    std::string query =
        "UPDATE reservations SET status = 'expired' "
        "WHERE status = 'active' AND reserved_until < NOW();";

    PGresult* res = PQexec(conn, query.c_str());
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        std::cerr << "Expire reservations failed: " << PQerrorMessage(conn) << std::endl;
    }
    PQclear(res);
}
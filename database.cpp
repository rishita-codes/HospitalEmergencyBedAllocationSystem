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
        std::cout << "Connected to database successfully!\n";
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
        "INSERT INTO patients (patient_id, name, age, esi, arrival_time) "
        "VALUES ($1, $2, $3, $4, NOW()) "
        "ON CONFLICT (patient_id) DO NOTHING;";

    const char* paramValues[4];
    int paramLengths[4] = {0, 0, 0, 0};
    int paramFormats[4]  = {0, 0, 0, 0};

    std::string id   = p->getId();
    std::string name = p->getName();
    std::string age  = std::to_string(p->getAge());
    std::string esi  = std::to_string(static_cast<int>(p->getESI()));

    paramValues[0] = id.c_str();
    paramValues[1] = name.c_str();
    paramValues[2] = age.c_str();
    paramValues[3] = esi.c_str();

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

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        std::cerr << "Insert Patient failed: "
                  << PQerrorMessage(conn) << std::endl;
    }

    PQclear(res);
}

std::vector<Patient*> Database::getWaitingPatients() {
    std::vector<Patient*> patients;

    std::string query =
        "SELECT p.patient_id, p.name, p.age, p.esi, p.arrival_time "
        "FROM patients p "
        "WHERE p.patient_id NOT IN ("
        " SELECT assigned_patient_id FROM beds WHERE assigned_patient_id IS NOT NULL"
        " );";

    PGresult* res = PQexec(conn, query.c_str());
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        std::cerr << "Fetch waiting patients failed: "
                  << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        return patients;
    }

    int rows = PQntuples(res);
    for (int i = 0; i < rows; i++) {
        std::string id= PQgetvalue(res, i, 0);
        std::string name= PQgetvalue(res, i, 1);
        int age = std::atoi(PQgetvalue(res, i, 2));
        ESI esi= static_cast<ESI>(std::atoi(PQgetvalue(res, i, 3)));
        std::string arrival = PQgetvalue(res, i, 4);

        DateTime dt;
        if (arrival.size() >= 19) {
            dt.year  = std::atoi(arrival.substr(0, 4).c_str());
            dt.month = std::atoi(arrival.substr(5, 2).c_str());
            dt.day   = std::atoi(arrival.substr(8, 2).c_str());
            dt.hour  = std::atoi(arrival.substr(11, 2).c_str());
            dt.min   = std::atoi(arrival.substr(14, 2).c_str());
            dt.sec   = std::atoi(arrival.substr(17, 2).c_str());
        } else {
            dt = currentTime();
        }

        Patient* p = new Patient(id, name, age, esi, dt);
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
ALTER TABLE patients ADD CONSTRAINT patients_esi_check CHECK (esi >= 1 AND esi <= 4);
)");
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        std::cerr << "Schema fix failed: " << PQerrorMessage(conn) << std::endl;
    }
    PQclear(res);
}

void Database::seedSampleData() {
    const char* dataSql = R"(
ALTER TABLE patients DROP CONSTRAINT IF EXISTS patients_esi_check;
ALTER TABLE patients ADD CONSTRAINT patients_esi_check CHECK (esi >= 1 AND esi <= 4);

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

INSERT INTO patients (patient_id, name, age, esi, arrival_time)
SELECT 'P100', 'Maria Chen', 58, 1, NOW()
WHERE NOT EXISTS (SELECT 1 FROM patients WHERE patient_id = 'P100');
INSERT INTO patients (patient_id, name, age, esi, arrival_time)
SELECT 'P101', 'Samuel Ortiz', 46, 2, NOW()
WHERE NOT EXISTS (SELECT 1 FROM patients WHERE patient_id = 'P101');
INSERT INTO patients (patient_id, name, age, esi, arrival_time)
SELECT 'P102', 'Priya Singh', 29, 3, NOW()
WHERE NOT EXISTS (SELECT 1 FROM patients WHERE patient_id = 'P102');
INSERT INTO patients (patient_id, name, age, esi, arrival_time)
SELECT 'P103', 'Ethan Brooks', 65, 1, NOW()
WHERE NOT EXISTS (SELECT 1 FROM patients WHERE patient_id = 'P103');
INSERT INTO patients (patient_id, name, age, esi, arrival_time)
SELECT 'P104', 'Nina Ahmed', 34, 4, NOW()
WHERE NOT EXISTS (SELECT 1 FROM patients WHERE patient_id = 'P104');
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
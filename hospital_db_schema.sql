-- Hospital emergency bed allocation system database schema

-- 1. users: coordinator/nurse/system accounts
CREATE TABLE IF NOT EXISTS users (
    user_id SERIAL PRIMARY KEY,
    username TEXT NOT NULL UNIQUE,
    password_hash TEXT NOT NULL,
    role TEXT NOT NULL CHECK (role IN ('coordinator', 'nurse', 'admin')),
    hospital_id INT NOT NULL REFERENCES hospitals(hospital_id)
);

-- 2. hospitals: facility information and optional location data
CREATE TABLE IF NOT EXISTS hospitals (
    hospital_id SERIAL PRIMARY KEY,
    name TEXT NOT NULL,
    address TEXT,
    city TEXT,
    state TEXT,
    latitude DOUBLE PRECISION,
    longitude DOUBLE PRECISION,
    created_at TIMESTAMP WITH TIME ZONE DEFAULT NOW()
);

-- 3. beds: bed inventory and assignment state
CREATE TABLE IF NOT EXISTS beds (
    bed_id TEXT PRIMARY KEY,
    hospital_id INT NOT NULL REFERENCES hospitals(hospital_id),
    type INT NOT NULL CHECK (type >= 0 AND type <= 3),
    is_occupied BOOLEAN NOT NULL DEFAULT FALSE,
    assigned_patient_id INT REFERENCES patients(patient_id),
    created_at TIMESTAMP WITH TIME ZONE DEFAULT NOW()
);

-- 4. patients: patient identities and phone-based login
CREATE TABLE IF NOT EXISTS patients (
    patient_id SERIAL PRIMARY KEY,
    phone_number TEXT NOT NULL UNIQUE,
    name TEXT NOT NULL,
    age INT NOT NULL CHECK (age >= 0),
    esi_level INT NOT NULL CHECK (esi_level >= 1 AND esi_level <= 4),
    preferred_bed_type INT CHECK (preferred_bed_type >= 0 AND preferred_bed_type <= 3),
    created_at TIMESTAMP WITH TIME ZONE DEFAULT NOW()
);

-- 5. requests: patient bed requests from the portal
CREATE TABLE IF NOT EXISTS requests (
    request_id SERIAL PRIMARY KEY,
    patient_id INT REFERENCES patients(patient_id),
    phone_number TEXT NOT NULL,
    name TEXT NOT NULL,
    age INT NOT NULL CHECK (age >= 0),
    esi_level INT NOT NULL CHECK (esi_level >= 1 AND esi_level <= 4),
    desired_bed_type INT NOT NULL CHECK (desired_bed_type >= 0 AND desired_bed_type <= 3),
    status TEXT NOT NULL DEFAULT 'pending' CHECK (status IN ('pending', 'matched', 'cancelled')),
    preferred_hospital_id INT REFERENCES hospitals(hospital_id),
    created_at TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    expires_at TIMESTAMP WITH TIME ZONE
);

-- 6. reservations: temporary bed reservations for patient requests
CREATE TABLE IF NOT EXISTS reservations (
    reservation_id SERIAL PRIMARY KEY,
    request_id INT NOT NULL REFERENCES requests(request_id),
    patient_id INT NOT NULL REFERENCES patients(patient_id),
    bed_id TEXT NOT NULL REFERENCES beds(bed_id),
    hospital_id INT NOT NULL REFERENCES hospitals(hospital_id),
    reserved_until TIMESTAMP WITH TIME ZONE NOT NULL,
    status TEXT NOT NULL DEFAULT 'active' CHECK (status IN ('active', 'expired', 'fulfilled', 'cancelled')),
    created_at TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    cancelled_at TIMESTAMP WITH TIME ZONE
);

-- Convenience indexes
CREATE INDEX IF NOT EXISTS idx_requests_status ON requests(status);
CREATE INDEX IF NOT EXISTS idx_reservations_status ON reservations(status);
CREATE INDEX IF NOT EXISTS idx_beds_hospital_type ON beds(hospital_id, type, is_occupied);
CREATE INDEX IF NOT EXISTS idx_patients_phone ON patients(phone_number);

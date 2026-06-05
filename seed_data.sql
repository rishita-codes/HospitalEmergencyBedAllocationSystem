-- Insert seed data for Hospital EBAS

-- 1. Insert Hospitals FIRST (no dependencies)
INSERT INTO hospitals (name, address, city, state, latitude, longitude)
SELECT 'St. Mary General Hospital', '123 Main Street', 'New York', 'NY', 40.7128, -74.0060
WHERE NOT EXISTS (SELECT 1 FROM hospitals WHERE name = 'St. Mary General Hospital');
INSERT INTO hospitals (name, address, city, state, latitude, longitude)
SELECT 'City Central Emergency Hospital', '456 Park Avenue', 'Los Angeles', 'CA', 34.0522, -118.2437
WHERE NOT EXISTS (SELECT 1 FROM hospitals WHERE name = 'City Central Emergency Hospital');
INSERT INTO hospitals (name, address, city, state, latitude, longitude)
SELECT 'Riverside Medical Center', '789 River Road', 'Chicago', 'IL', 41.8781, -87.6298
WHERE NOT EXISTS (SELECT 1 FROM hospitals WHERE name = 'Riverside Medical Center');
INSERT INTO hospitals (name, address, city, state, latitude, longitude)
SELECT 'Valley Health Systems', '321 Health Drive', 'Houston', 'TX', 29.7604, -95.3698
WHERE NOT EXISTS (SELECT 1 FROM hospitals WHERE name = 'Valley Health Systems');
INSERT INTO hospitals (name, address, city, state, latitude, longitude)
SELECT 'Metropolitan Hospital', '654 Downtown Ave', 'Phoenix', 'AZ', 33.4484, -112.0742
WHERE NOT EXISTS (SELECT 1 FROM hospitals WHERE name = 'Metropolitan Hospital');

-- 2. Insert Users (needs hospitals to exist)
INSERT INTO users (username, password_hash, role, hospital_id) VALUES
('coord1', 'pass123', 'coordinator', 1),
('nurse1', 'nurse123', 'nurse', 1),
('nurse2', 'nurse123', 'nurse', 1),
('coord2', 'pass123', 'coordinator', 2),
('nurse3', 'nurse123', 'nurse', 2),
('nurse4', 'nurse123', 'nurse', 2),
('coord3', 'pass123', 'coordinator', 3),
('nurse5', 'nurse123', 'nurse', 3),
('coord4', 'pass123', 'coordinator', 4),
('nurse6', 'nurse123', 'nurse', 4),
('admin1', 'admin123', 'admin', 1),
('admin2', 'admin123', 'admin', 2)
ON CONFLICT (username) DO NOTHING;

-- 3. Insert Patients (no dependencies)
INSERT INTO patients (phone_number, name, age, esi_level, preferred_bed_type) VALUES
('5551234567', 'Maria Chen', 58, 1, 0),
('5559876543', 'Samuel Ortiz', 46, 2, 1),
('5552468101', 'Priya Singh', 29, 3, 2),
('5557654321', 'Ethan Brooks', 65, 1, 0),
('5553691215', 'Nina Ahmed', 34, 2, 2),
('5558024680', 'James Wilson', 72, 1, 0),
('5551357924', 'Sarah Martinez', 41, 3, 2),
('5559135797', 'David Thompson', 55, 2, 1),
('5552468135', 'Emma Johnson', 27, 3, 3),
('5556789012', 'Robert Lee', 68, 2, 1),
('5553456789', 'Jessica Chen', 35, 3, 2),
('5557890123', 'Michael Garcia', 52, 2, 1),
('5554567890', 'Anna Kowalski', 44, 3, 2),
('5558901234', 'Christopher Brown', 39, 2, 1),
('5552345678', 'Linda Davis', 61, 1, 0),
('5551122334', 'George Harris', 53, 2, 1),
('5551223445', 'Susan Lewis', 47, 3, 2),
('5551334556', 'Paul Walker', 71, 1, 0),
('5551445667', 'Rachel White', 36, 2, 1),
('5551556778', 'Thomas Hall', 48, 3, 2),
('5551667889', 'Jennifer Allen', 55, 2, 1),
('5551778990', 'Daniel Young', 42, 3, 2),
('5551889001', 'Michelle King', 64, 1, 0),
('5551990112', 'Anthony Wright', 51, 2, 1),
('5552001223', 'Karen Lopez', 38, 3, 2),
('5552112334', 'Steven Hill', 59, 2, 1),
('5552223445', 'Lisa Scott', 33, 3, 2),
('5552334556', 'Andrew Green', 66, 1, 0),
('5552445667', 'Betty Adams', 44, 2, 1),
('5552556778', 'Joshua Nelson', 49, 3, 2),
('5552667889', 'Sandra Carter', 62, 1, 0),
('5552778990', 'Kevin Mitchell', 54, 2, 1),
('5552889001', 'Ashley Perez', 31, 3, 2),
('5552990112', 'Brian Roberts', 57, 2, 1),
('5553001223', 'Pamela Phillips', 45, 3, 2)
ON CONFLICT (phone_number) DO NOTHING;

-- 4. Insert Beds (needs hospitals and optionally patients)
INSERT INTO beds (bed_id, hospital_id, type, is_occupied, assigned_patient_id) VALUES
-- St. Mary General Hospital (Hospital 1)
('H1-ICU-01', 1, 0, false, NULL),
('H1-ICU-02', 1, 0, true, 1),
('H1-EMG-01', 1, 1, false, NULL),
('H1-EMG-02', 1, 1, true, 4),
('H1-GEN-01', 1, 2, false, NULL),
('H1-GEN-02', 1, 2, true, 2),
('H1-GEN-03', 1, 2, false, NULL),
('H1-SP-01', 1, 3, false, NULL),
('H1-SP-02', 1, 3, true, 6),
-- City Central Emergency Hospital (Hospital 2)
('H2-ICU-01', 2, 0, false, NULL),
('H2-ICU-02', 2, 0, false, NULL),
('H2-EMG-01', 2, 1, true, 8),
('H2-EMG-02', 2, 1, false, NULL),
('H2-GEN-01', 2, 2, true, 5),
('H2-GEN-02', 2, 2, false, NULL),
('H2-GEN-03', 2, 2, true, 10),
('H2-SP-01', 2, 3, false, NULL),
-- Riverside Medical Center (Hospital 3)
('H3-ICU-01', 3, 0, false, NULL),
('H3-EMG-01', 3, 1, false, NULL),
('H3-EMG-02', 3, 1, true, 14),
('H3-GEN-01', 3, 2, false, NULL),
('H3-GEN-02', 3, 2, true, 7),
('H3-SP-01', 3, 3, false, NULL),
('H3-SP-02', 3, 3, true, 12),
-- Valley Health Systems (Hospital 4)
('H4-ICU-01', 4, 0, false, NULL),
('H4-ICU-02', 4, 0, false, NULL),
('H4-EMG-01', 4, 1, false, NULL),
('H4-GEN-01', 4, 2, true, 11),
('H4-GEN-02', 4, 2, false, NULL),
('H4-SP-01', 4, 3, false, NULL),
-- Metropolitan Hospital (Hospital 5)
('H5-ICU-01', 5, 0, true, 9),
('H5-EMG-01', 5, 1, false, NULL),
('H5-GEN-01', 5, 2, false, NULL),
('H5-GEN-02', 5, 2, true, 3),
('H5-SP-01', 5, 3, false, NULL)
ON CONFLICT (bed_id) DO NOTHING;

-- 5. Insert Requests (needs patients and hospitals)
INSERT INTO requests (patient_id, phone_number, name, age, esi_level, desired_bed_type, status, preferred_hospital_id, expires_at)
SELECT 1, '5551234567', 'Maria Chen', 58, 1, 0, 'matched', 1, NOW() + INTERVAL '1 hour'
WHERE NOT EXISTS (
  SELECT 1 FROM requests WHERE patient_id = 1 AND status = 'matched' AND preferred_hospital_id = 1 AND desired_bed_type = 0
);
INSERT INTO requests (patient_id, phone_number, name, age, esi_level, desired_bed_type, status, preferred_hospital_id, expires_at)
SELECT 4, '5557654321', 'Ethan Brooks', 65, 1, 0, 'matched', 1, NOW() + INTERVAL '2 hours'
WHERE NOT EXISTS (
  SELECT 1 FROM requests WHERE patient_id = 4 AND status = 'matched' AND preferred_hospital_id = 1 AND desired_bed_type = 0
);
INSERT INTO requests (patient_id, phone_number, name, age, esi_level, desired_bed_type, status, preferred_hospital_id, expires_at)
SELECT 2, '5559876543', 'Samuel Ortiz', 46, 2, 1, 'matched', 1, NOW() + INTERVAL '30 minutes'
WHERE NOT EXISTS (
  SELECT 1 FROM requests WHERE patient_id = 2 AND status = 'matched' AND preferred_hospital_id = 1 AND desired_bed_type = 1
);
INSERT INTO requests (patient_id, phone_number, name, age, esi_level, desired_bed_type, status, preferred_hospital_id, expires_at)
SELECT 5, '5553691215', 'Nina Ahmed', 34, 2, 2, 'matched', 2, NOW() + INTERVAL '45 minutes'
WHERE NOT EXISTS (
  SELECT 1 FROM requests WHERE patient_id = 5 AND status = 'matched' AND preferred_hospital_id = 2 AND desired_bed_type = 2
);
INSERT INTO requests (patient_id, phone_number, name, age, esi_level, desired_bed_type, status, preferred_hospital_id, expires_at)
SELECT 6, '5558024680', 'James Wilson', 72, 1, 0, 'pending', 1, NOW() + INTERVAL '20 minutes'
WHERE NOT EXISTS (
  SELECT 1 FROM requests WHERE patient_id = 6 AND status = 'pending' AND preferred_hospital_id = 1 AND desired_bed_type = 0
);
INSERT INTO requests (patient_id, phone_number, name, age, esi_level, desired_bed_type, status, preferred_hospital_id, expires_at)
SELECT 7, '5551357924', 'Sarah Martinez', 41, 3, 2, 'pending', 3, NOW() + INTERVAL '15 minutes'
WHERE NOT EXISTS (
  SELECT 1 FROM requests WHERE patient_id = 7 AND status = 'pending' AND preferred_hospital_id = 3 AND desired_bed_type = 2
);
INSERT INTO requests (patient_id, phone_number, name, age, esi_level, desired_bed_type, status, preferred_hospital_id, expires_at)
SELECT 13, '5554567890', 'Anna Kowalski', 44, 3, 2, 'pending', NULL, NOW() + INTERVAL '10 minutes'
WHERE NOT EXISTS (
  SELECT 1 FROM requests WHERE patient_id = 13 AND status = 'pending' AND desired_bed_type = 2 AND preferred_hospital_id IS NULL
);
INSERT INTO requests (patient_id, phone_number, name, age, esi_level, desired_bed_type, status, preferred_hospital_id, expires_at)
SELECT 9, '5552468135', 'Emma Johnson', 27, 3, 3, 'pending', 5, NOW() + INTERVAL '25 minutes'
WHERE NOT EXISTS (
  SELECT 1 FROM requests WHERE patient_id = 9 AND status = 'pending' AND preferred_hospital_id = 5 AND desired_bed_type = 3
);

-- 6. Insert Reservations (needs all other tables)
INSERT INTO reservations (request_id, patient_id, bed_id, hospital_id, reserved_until, status)
SELECT 1, 1, 'H1-ICU-02', 1, NOW() + INTERVAL '30 minutes', 'active'
WHERE NOT EXISTS (
  SELECT 1 FROM reservations WHERE request_id = 1 AND bed_id = 'H1-ICU-02'
);
INSERT INTO reservations (request_id, patient_id, bed_id, hospital_id, reserved_until, status)
SELECT 2, 4, 'H1-EMG-02', 1, NOW() + INTERVAL '25 minutes', 'active'
WHERE NOT EXISTS (
  SELECT 1 FROM reservations WHERE request_id = 2 AND bed_id = 'H1-EMG-02'
);
INSERT INTO reservations (request_id, patient_id, bed_id, hospital_id, reserved_until, status)
SELECT 3, 2, 'H1-GEN-02', 1, NOW() + INTERVAL '20 minutes', 'active'
WHERE NOT EXISTS (
  SELECT 1 FROM reservations WHERE request_id = 3 AND bed_id = 'H1-GEN-02'
);
INSERT INTO reservations (request_id, patient_id, bed_id, hospital_id, reserved_until, status)
SELECT 4, 5, 'H2-GEN-01', 2, NOW() + INTERVAL '30 minutes', 'active'
WHERE NOT EXISTS (
  SELECT 1 FROM reservations WHERE request_id = 4 AND bed_id = 'H2-GEN-01'
);
INSERT INTO reservations (request_id, patient_id, bed_id, hospital_id, reserved_until, status)
SELECT 6, 8, 'H2-EMG-01', 2, NOW() + INTERVAL '15 minutes', 'active'
WHERE NOT EXISTS (
  SELECT 1 FROM reservations WHERE request_id = 6 AND bed_id = 'H2-EMG-01'
);
INSERT INTO reservations (request_id, patient_id, bed_id, hospital_id, reserved_until, status)
SELECT 5, 6, 'H1-ICU-01', 1, NOW() + INTERVAL '5 minutes', 'active'
WHERE NOT EXISTS (
  SELECT 1 FROM reservations WHERE request_id = 5 AND bed_id = 'H1-ICU-01'
);

-- 7. Add pending patients to waiting queues for each hospital (3-4 per hospital)
-- St. Mary General Hospital (Hospital 1) - pending patients
INSERT INTO requests (patient_id, phone_number, name, age, esi_level, desired_bed_type, status, preferred_hospital_id, expires_at)
SELECT 16, '5551122334', 'George Harris', 53, 2, 1, 'pending', 1, NOW() + INTERVAL '30 minutes'
WHERE NOT EXISTS (SELECT 1 FROM requests WHERE patient_id = 16 AND preferred_hospital_id = 1 AND status = 'pending');
INSERT INTO requests (patient_id, phone_number, name, age, esi_level, desired_bed_type, status, preferred_hospital_id, expires_at)
SELECT 17, '5551223445', 'Susan Lewis', 47, 3, 2, 'pending', 1, NOW() + INTERVAL '28 minutes'
WHERE NOT EXISTS (SELECT 1 FROM requests WHERE patient_id = 17 AND preferred_hospital_id = 1 AND status = 'pending');
INSERT INTO requests (patient_id, phone_number, name, age, esi_level, desired_bed_type, status, preferred_hospital_id, expires_at)
SELECT 18, '5551334556', 'Paul Walker', 71, 1, 0, 'pending', 1, NOW() + INTERVAL '32 minutes'
WHERE NOT EXISTS (SELECT 1 FROM requests WHERE patient_id = 18 AND preferred_hospital_id = 1 AND status = 'pending');
INSERT INTO requests (patient_id, phone_number, name, age, esi_level, desired_bed_type, status, preferred_hospital_id, expires_at)
SELECT 19, '5551445667', 'Rachel White', 36, 2, 1, 'pending', 1, NOW() + INTERVAL '26 minutes'
WHERE NOT EXISTS (SELECT 1 FROM requests WHERE patient_id = 19 AND preferred_hospital_id = 1 AND status = 'pending');

-- City Central Emergency Hospital (Hospital 2) - pending patients
INSERT INTO requests (patient_id, phone_number, name, age, esi_level, desired_bed_type, status, preferred_hospital_id, expires_at)
SELECT 20, '5551556778', 'Thomas Hall', 48, 3, 2, 'pending', 2, NOW() + INTERVAL '25 minutes'
WHERE NOT EXISTS (SELECT 1 FROM requests WHERE patient_id = 20 AND preferred_hospital_id = 2 AND status = 'pending');
INSERT INTO requests (patient_id, phone_number, name, age, esi_level, desired_bed_type, status, preferred_hospital_id, expires_at)
SELECT 21, '5551667889', 'Jennifer Allen', 55, 2, 1, 'pending', 2, NOW() + INTERVAL '30 minutes'
WHERE NOT EXISTS (SELECT 1 FROM requests WHERE patient_id = 21 AND preferred_hospital_id = 2 AND status = 'pending');
INSERT INTO requests (patient_id, phone_number, name, age, esi_level, desired_bed_type, status, preferred_hospital_id, expires_at)
SELECT 22, '5551778990', 'Daniel Young', 42, 3, 2, 'pending', 2, NOW() + INTERVAL '22 minutes'
WHERE NOT EXISTS (SELECT 1 FROM requests WHERE patient_id = 22 AND preferred_hospital_id = 2 AND status = 'pending');
INSERT INTO requests (patient_id, phone_number, name, age, esi_level, desired_bed_type, status, preferred_hospital_id, expires_at)
SELECT 23, '5551889001', 'Michelle King', 64, 1, 0, 'pending', 2, NOW() + INTERVAL '29 minutes'
WHERE NOT EXISTS (SELECT 1 FROM requests WHERE patient_id = 23 AND preferred_hospital_id = 2 AND status = 'pending');

-- Riverside Medical Center (Hospital 3) - pending patients
INSERT INTO requests (patient_id, phone_number, name, age, esi_level, desired_bed_type, status, preferred_hospital_id, expires_at)
SELECT 24, '5551990112', 'Anthony Wright', 51, 2, 1, 'pending', 3, NOW() + INTERVAL '27 minutes'
WHERE NOT EXISTS (SELECT 1 FROM requests WHERE patient_id = 24 AND preferred_hospital_id = 3 AND status = 'pending');
INSERT INTO requests (patient_id, phone_number, name, age, esi_level, desired_bed_type, status, preferred_hospital_id, expires_at)
SELECT 25, '5552001223', 'Karen Lopez', 38, 3, 2, 'pending', 3, NOW() + INTERVAL '24 minutes'
WHERE NOT EXISTS (SELECT 1 FROM requests WHERE patient_id = 25 AND preferred_hospital_id = 3 AND status = 'pending');
INSERT INTO requests (patient_id, phone_number, name, age, esi_level, desired_bed_type, status, preferred_hospital_id, expires_at)
SELECT 26, '5552112334', 'Steven Hill', 59, 2, 1, 'pending', 3, NOW() + INTERVAL '31 minutes'
WHERE NOT EXISTS (SELECT 1 FROM requests WHERE patient_id = 26 AND preferred_hospital_id = 3 AND status = 'pending');
INSERT INTO requests (patient_id, phone_number, name, age, esi_level, desired_bed_type, status, preferred_hospital_id, expires_at)
SELECT 27, '5552223445', 'Lisa Scott', 33, 3, 2, 'pending', 3, NOW() + INTERVAL '23 minutes'
WHERE NOT EXISTS (SELECT 1 FROM requests WHERE patient_id = 27 AND preferred_hospital_id = 3 AND status = 'pending');

-- Valley Health Systems (Hospital 4) - pending patients
INSERT INTO requests (patient_id, phone_number, name, age, esi_level, desired_bed_type, status, preferred_hospital_id, expires_at)
SELECT 28, '5552334556', 'Andrew Green', 66, 1, 0, 'pending', 4, NOW() + INTERVAL '28 minutes'
WHERE NOT EXISTS (SELECT 1 FROM requests WHERE patient_id = 28 AND preferred_hospital_id = 4 AND status = 'pending');
INSERT INTO requests (patient_id, phone_number, name, age, esi_level, desired_bed_type, status, preferred_hospital_id, expires_at)
SELECT 29, '5552445667', 'Betty Adams', 44, 2, 1, 'pending', 4, NOW() + INTERVAL '26 minutes'
WHERE NOT EXISTS (SELECT 1 FROM requests WHERE patient_id = 29 AND preferred_hospital_id = 4 AND status = 'pending');
INSERT INTO requests (patient_id, phone_number, name, age, esi_level, desired_bed_type, status, preferred_hospital_id, expires_at)
SELECT 30, '5552556778', 'Joshua Nelson', 49, 3, 2, 'pending', 4, NOW() + INTERVAL '30 minutes'
WHERE NOT EXISTS (SELECT 1 FROM requests WHERE patient_id = 30 AND preferred_hospital_id = 4 AND status = 'pending');
INSERT INTO requests (patient_id, phone_number, name, age, esi_level, desired_bed_type, status, preferred_hospital_id, expires_at)
SELECT 31, '5552667889', 'Sandra Carter', 62, 1, 0, 'pending', 4, NOW() + INTERVAL '25 minutes'
WHERE NOT EXISTS (SELECT 1 FROM requests WHERE patient_id = 31 AND preferred_hospital_id = 4 AND status = 'pending');

-- Metropolitan Hospital (Hospital 5) - pending patients
INSERT INTO requests (patient_id, phone_number, name, age, esi_level, desired_bed_type, status, preferred_hospital_id, expires_at)
SELECT 32, '5552778990', 'Kevin Mitchell', 54, 2, 1, 'pending', 5, NOW() + INTERVAL '29 minutes'
WHERE NOT EXISTS (SELECT 1 FROM requests WHERE patient_id = 32 AND preferred_hospital_id = 5 AND status = 'pending');
INSERT INTO requests (patient_id, phone_number, name, age, esi_level, desired_bed_type, status, preferred_hospital_id, expires_at)
SELECT 33, '5552889001', 'Ashley Perez', 31, 3, 2, 'pending', 5, NOW() + INTERVAL '27 minutes'
WHERE NOT EXISTS (SELECT 1 FROM requests WHERE patient_id = 33 AND preferred_hospital_id = 5 AND status = 'pending');
INSERT INTO requests (patient_id, phone_number, name, age, esi_level, desired_bed_type, status, preferred_hospital_id, expires_at)
SELECT 34, '5552990112', 'Brian Roberts', 57, 2, 1, 'pending', 5, NOW() + INTERVAL '24 minutes'
WHERE NOT EXISTS (SELECT 1 FROM requests WHERE patient_id = 34 AND preferred_hospital_id = 5 AND status = 'pending');
INSERT INTO requests (patient_id, phone_number, name, age, esi_level, desired_bed_type, status, preferred_hospital_id, expires_at)
SELECT 35, '5553001223', 'Pamela Phillips', 45, 3, 2, 'pending', 5, NOW() + INTERVAL '32 minutes'
WHERE NOT EXISTS (SELECT 1 FROM requests WHERE patient_id = 35 AND preferred_hospital_id = 5 AND status = 'pending');

-- Verification
SELECT 'Seed data inserted successfully!' as status;
SELECT 'Hospitals: ' || COUNT(*) FROM hospitals;
SELECT 'Users: ' || COUNT(*) FROM users;
SELECT 'Patients: ' || COUNT(*) FROM patients;
SELECT 'Beds: ' || COUNT(*) FROM beds;
SELECT 'Requests: ' || COUNT(*) FROM requests;
SELECT 'Reservations: ' || COUNT(*) FROM reservations;
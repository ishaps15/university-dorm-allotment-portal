-- ============================================================================
-- UNIVERSITY DORM ALLOTMENT PORTAL - SEED DATA
-- Campus: Durgapur, West Bengal (NIT Durgapur Campus Model)
-- ============================================================================

USE university_dorm_db;

-- ----------------------------------------------------------------------------
-- 1. SEED USERS (ADMIN, WARDENS, STUDENTS)
-- Passwords:
--   admin:   "Admin@NITD2025" (salt: 'salt_admin_nitdgp_20250101_key01')
--   warden1: "Warden@Tagore"  (salt: 'salt_ward_tagore_20250101_key02')
--   warden2: "Warden@Nivedita"(salt: 'salt_ward_nivedi_20250101_key03')
--   student1:"Pass@Student1"  (salt: 'salt_stud_roll01_20250101_key04')
--   student2:"Pass@Student2"  (salt: 'salt_stud_roll02_20250101_key05')
--   student3:"Pass@Student3"  (salt: 'salt_stud_roll03_20250101_key06')
--   student4:"Pass@Student4"  (salt: 'salt_stud_roll04_20250101_key07')
--   student5:"Pass@Student5"  (salt: 'salt_stud_roll05_20250101_key08')
-- ----------------------------------------------------------------------------

-- Hashing with SHA2(CONCAT(password, '$', salt, '$NIT_DGP_DORM_SALT'), 256)
INSERT INTO users (user_id, username, password_hash, salt, role, is_active) VALUES
(1, 'chief_warden', SHA2(CONCAT('Admin@NITD2025', '$salt_admin_nitdgp_20250101_key01$NIT_DGP_DORM_SALT'), 256), 'salt_admin_nitdgp_20250101_key01', 'ADMIN', 1),
(2, 'warden_tagore', SHA2(CONCAT('Warden@Tagore', '$salt_ward_tagore_20250101_key02$NIT_DGP_DORM_SALT'), 256), 'salt_ward_tagore_20250101_key02', 'WARDEN', 1),
(3, 'warden_nivedita', SHA2(CONCAT('Warden@Nivedita', '$salt_ward_nivedi_20250101_key03$NIT_DGP_DORM_SALT'), 256), 'salt_ward_nivedi_20250101_key03', 'WARDEN', 1),
(4, '21CS8014', SHA2(CONCAT('Pass@Student1', '$salt_stud_roll01_20250101_key04$NIT_DGP_DORM_SALT'), 256), 'salt_stud_roll01_20250101_key04', 'STUDENT', 1),
(5, '21EC8022', SHA2(CONCAT('Pass@Student2', '$salt_stud_roll02_20250101_key05$NIT_DGP_DORM_SALT'), 256), 'salt_stud_roll02_20250101_key05', 'STUDENT', 1),
(6, '22ME8005', SHA2(CONCAT('Pass@Student3', '$salt_stud_roll03_20250101_key06$NIT_DGP_DORM_SALT'), 256), 'salt_stud_roll03_20250101_key06', 'STUDENT', 1),
(7, '22EE8031', SHA2(CONCAT('Pass@Student4', '$salt_stud_roll04_20250101_key07$NIT_DGP_DORM_SALT'), 256), 'salt_stud_roll04_20250101_key07', 'STUDENT', 1),
(8, '23CS8045', SHA2(CONCAT('Pass@Student5', '$salt_stud_roll05_20250101_key08$NIT_DGP_DORM_SALT'), 256), 'salt_stud_roll05_20250101_key08', 'STUDENT', 1);

-- ----------------------------------------------------------------------------
-- 2. SEED STUDENTS
-- ----------------------------------------------------------------------------
INSERT INTO students (student_id, user_id, roll_number, full_name, department, year_of_study, gender, email, phone, cgpa, home_distance_km, city, state) VALUES
(1, 4, '21CS8014', 'Sourav Mukherjee', 'Computer Science & Engineering', 4, 'M', 'sm.21cs8014@nitdgp.ac.in', '+91-9830112233', 9.15, 175.50, 'Kolkata', 'West Bengal'),
(2, 5, '21EC8022', 'Ananya Banerjee', 'Electronics & Comm. Engineering', 4, 'F', 'ab.21ec8022@nitdgp.ac.in', '+91-9830223344', 9.42, 210.00, 'Siliguri', 'West Bengal'),
(3, 6, '22ME8005', 'Rahul Sharma', 'Mechanical Engineering', 3, 'M', 'rs.22me8005@nitdgp.ac.in', '+91-9830334455', 8.35, 450.00, 'Patna', 'Bihar'),
(4, 7, '22EE8031', 'Pooja Sen', 'Electrical Engineering', 3, 'F', 'ps.22ee8031@nitdgp.ac.in', '+91-9830445566', 8.78, 95.00, 'Asansol', 'West Bengal'),
(5, 8, '23CS8045', 'Debanjan Ghosh', 'Computer Science & Engineering', 2, 'M', 'dg.23cs8045@nitdgp.ac.in', '+91-9830556677', 8.90, 18.50, 'Durgapur', 'West Bengal');

-- ----------------------------------------------------------------------------
-- 3. SEED HOSTELS (DORMS IN CAMPUS)
-- ----------------------------------------------------------------------------
INSERT INTO hostels (hostel_id, name, code, type, total_floors, total_rooms, total_capacity, current_occupancy, warden_user_id, contact_phone, address) VALUES
(1, 'Rabindranath Tagore Hall (Hall 11)', 'HALL-11', 'MALE', 4, 120, 240, 2, 2, '+91-343-2754111', 'East Campus, M.G. Avenue, Durgapur, WB - 713209'),
(2, 'Sister Nivedita Hall (Hall 12)', 'HALL-12', 'FEMALE', 4, 100, 200, 2, 3, '+91-343-2754112', 'North Campus, M.G. Avenue, Durgapur, WB - 713209'),
(3, 'Netaji Subhash Hall (Hall 2)', 'HALL-02', 'MALE', 3, 80, 160, 0, 2, '+91-343-2754102', 'South Campus, M.G. Avenue, Durgapur, WB - 713209'),
(4, 'Mother Teresa Hall (Hall 13)', 'HALL-13', 'FEMALE', 4, 110, 220, 0, 3, '+91-343-2754113', 'West Campus, M.G. Avenue, Durgapur, WB - 713209');

-- ----------------------------------------------------------------------------
-- 4. SEED ROOMS
-- ----------------------------------------------------------------------------
INSERT INTO rooms (room_id, hostel_id, room_number, floor, room_type, capacity, current_occupancy, monthly_fee, status) VALUES
-- Tagore Hall (Hostel 1 - Male)
(1, 1, 'T-101', 1, 'DOUBLE', 2, 2, 1600.00, 'OCCUPIED'),
(2, 1, 'T-102', 1, 'DOUBLE', 2, 0, 1600.00, 'AVAILABLE'),
(3, 1, 'T-201', 2, 'SINGLE', 1, 0, 2400.00, 'AVAILABLE'),
(4, 1, 'T-202', 2, 'TRIPLE', 3, 0, 1200.00, 'AVAILABLE'),
(5, 1, 'T-301', 3, 'DOUBLE', 2, 0, 1600.00, 'MAINTENANCE'),

-- Sister Nivedita Hall (Hostel 2 - Female)
(6, 2, 'N-101', 1, 'DOUBLE', 2, 2, 1600.00, 'OCCUPIED'),
(7, 2, 'N-102', 1, 'DOUBLE', 2, 0, 1600.00, 'AVAILABLE'),
(8, 2, 'N-201', 2, 'SINGLE', 1, 0, 2400.00, 'AVAILABLE'),
(9, 2, 'N-202', 2, 'TRIPLE', 3, 0, 1200.00, 'AVAILABLE'),

-- Netaji Subhash Hall (Hostel 3 - Male)
(10, 3, 'NS-101', 1, 'DOUBLE', 2, 0, 1500.00, 'AVAILABLE'),
(11, 3, 'NS-102', 1, 'DOUBLE', 2, 0, 1500.00, 'AVAILABLE'),

-- Mother Teresa Hall (Hostel 4 - Female)
(12, 4, 'MT-101', 1, 'DOUBLE', 2, 0, 1500.00, 'AVAILABLE'),
(13, 4, 'MT-102', 1, 'DOUBLE', 2, 0, 1500.00, 'AVAILABLE');

-- ----------------------------------------------------------------------------
-- 5. SEED ALLOTMENT APPLICATIONS
-- ----------------------------------------------------------------------------
INSERT INTO allotment_applications (application_id, student_id, preferred_hostel_id, preferred_room_type, merit_score, status, remarks) VALUES
(1, 1, 1, 'DOUBLE', 88.75, 'ALLOTTED', 'High merit senior 4th yr'),
(2, 2, 2, 'DOUBLE', 91.20, 'ALLOTTED', 'Top departmental merit'),
(3, 3, 1, 'DOUBLE', 79.50, 'ALLOTTED', 'Outstation domicile priority'),
(4, 4, 2, 'DOUBLE', 82.40, 'ALLOTTED', 'Allocated Sister Nivedita N-101'),
(5, 5, 1, 'SINGLE', 74.00, 'PENDING', 'Awaiting next round vacancy');

-- ----------------------------------------------------------------------------
-- 6. SEED ALLOTMENTS
-- ----------------------------------------------------------------------------
INSERT INTO allotments (allotment_id, application_id, student_id, room_id, allotted_date, valid_until, is_active, dues_cleared, checkin_time) VALUES
(1, 1, 1, 1, '2025-01-05', '2025-12-31', 1, 1, '2025-01-06 10:15:00'),
(2, 3, 3, 1, '2025-01-05', '2025-12-31', 1, 1, '2025-01-06 11:30:00'),
(3, 2, 2, 6, '2025-01-05', '2025-12-31', 1, 1, '2025-01-07 09:45:00'),
(4, 4, 4, 6, '2025-01-05', '2025-12-31', 1, 1, '2025-01-07 14:20:00');

-- ----------------------------------------------------------------------------
-- 7. SEED MAINTENANCE COMPLAINTS
-- ----------------------------------------------------------------------------
INSERT INTO maintenance_complaints (complaint_id, student_id, room_id, category, description, status, filed_at) VALUES
(1, 1, 1, 'ELECTRICAL', 'Ceiling fan regulator sparking intermittently on speed 4.', 'IN_PROGRESS', '2025-01-12 14:00:00'),
(2, 4, 6, 'PLUMBING', 'Washroom faucet dripping continuously causing water waste.', 'OPEN', '2025-01-15 09:30:00'),
(3, 3, 1, 'NETWORK', 'LAN port wall socket loose, ethernet disconnects frequently.', 'RESOLVED', '2025-01-08 16:20:00');

-- ----------------------------------------------------------------------------
-- 8. SEED AUDIT LOGS
-- ----------------------------------------------------------------------------
INSERT INTO audit_logs (log_id, user_id, username, action, entity_affected, entity_id, details) VALUES
(1, 1, 'chief_warden', 'SYSTEM_INIT', 'SYSTEM', 0, 'Database provisioned for Spring 2025 Semester at Durgapur campus.'),
(2, 2, 'warden_tagore', 'ROOM_ALLOTMENT', 'ALLOTMENTS', 1, 'Allotted Room T-101 to Sourav Mukherjee (21CS8014).'),
(3, 3, 'warden_nivedita', 'ROOM_ALLOTMENT', 'ALLOTMENTS', 3, 'Allotted Room N-101 to Ananya Banerjee (21EC8022).');

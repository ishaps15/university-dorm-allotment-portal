-- ============================================================================
-- UNIVERSITY DORM ALLOTMENT PORTAL - DATABASE SCHEMA (3NF NORMALIZED)
-- Location: Durgapur, West Bengal
-- Database Engine: MySQL / MariaDB (InnoDB)
-- ============================================================================

DROP DATABASE IF EXISTS university_dorm_db;
CREATE DATABASE university_dorm_db CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
USE university_dorm_db;

-- ----------------------------------------------------------------------------
-- 1. USERS & AUTHENTICATION TABLE
-- Stores credentials with salted SHA-256 hashes and role-based access control.
-- ----------------------------------------------------------------------------
CREATE TABLE users (
    user_id INT AUTO_INCREMENT PRIMARY KEY,
    username VARCHAR(64) NOT NULL UNIQUE,
    password_hash CHAR(64) NOT NULL,
    salt CHAR(32) NOT NULL,
    role ENUM('STUDENT', 'WARDEN', 'ADMIN') NOT NULL DEFAULT 'STUDENT',
    is_active TINYINT(1) NOT NULL DEFAULT 1,
    last_login TIMESTAMP NULL DEFAULT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_user_auth (username, is_active)
) ENGINE=InnoDB;

-- ----------------------------------------------------------------------------
-- 2. STUDENTS PROFILE TABLE (1:1 with Users for role=STUDENT)
-- Contains demographic, academic, and distance info for merit allocation.
-- ----------------------------------------------------------------------------
CREATE TABLE students (
    student_id INT AUTO_INCREMENT PRIMARY KEY,
    user_id INT NOT NULL UNIQUE,
    roll_number VARCHAR(32) NOT NULL UNIQUE,
    full_name VARCHAR(128) NOT NULL,
    department VARCHAR(64) NOT NULL,
    year_of_study TINYINT NOT NULL CHECK (year_of_study BETWEEN 1 AND 5),
    gender ENUM('M', 'F', 'O') NOT NULL,
    email VARCHAR(128) NOT NULL UNIQUE,
    phone VARCHAR(20) NOT NULL,
    cgpa DECIMAL(3,2) NOT NULL CHECK (cgpa >= 0.00 AND cgpa <= 10.00),
    home_distance_km DECIMAL(7,2) NOT NULL DEFAULT 0.00,
    city VARCHAR(64) NOT NULL,
    state VARCHAR(64) NOT NULL DEFAULT 'West Bengal',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE,
    INDEX idx_student_merit (cgpa DESC, home_distance_km DESC),
    INDEX idx_student_dept_yr (department, year_of_study)
) ENGINE=InnoDB;

-- ----------------------------------------------------------------------------
-- 3. HOSTELS (DORMS) TABLE
-- Campus hostels in Durgapur (e.g. Hall 1, Hall 2, Tagore, Nivedita)
-- ----------------------------------------------------------------------------
CREATE TABLE hostels (
    hostel_id INT AUTO_INCREMENT PRIMARY KEY,
    name VARCHAR(128) NOT NULL UNIQUE,
    code VARCHAR(16) NOT NULL UNIQUE,
    type ENUM('MALE', 'FEMALE', 'COED') NOT NULL,
    total_floors INT NOT NULL DEFAULT 4,
    total_rooms INT NOT NULL DEFAULT 0,
    total_capacity INT NOT NULL DEFAULT 0,
    current_occupancy INT NOT NULL DEFAULT 0,
    warden_user_id INT NULL,
    contact_phone VARCHAR(20),
    address VARCHAR(255) DEFAULT 'Campus Sector, Durgapur, West Bengal - 713209',
    FOREIGN KEY (warden_user_id) REFERENCES users(user_id) ON DELETE SET NULL,
    INDEX idx_hostel_type (type)
) ENGINE=InnoDB;

-- ----------------------------------------------------------------------------
-- 4. ROOMS TABLE (Strong 3NF with composite uniqueness & capacity tracking)
-- ----------------------------------------------------------------------------
CREATE TABLE rooms (
    room_id INT AUTO_INCREMENT PRIMARY KEY,
    hostel_id INT NOT NULL,
    room_number VARCHAR(16) NOT NULL,
    floor INT NOT NULL DEFAULT 0,
    room_type ENUM('SINGLE', 'DOUBLE', 'TRIPLE', 'QUAD') NOT NULL DEFAULT 'DOUBLE',
    capacity INT NOT NULL CHECK (capacity > 0),
    current_occupancy INT NOT NULL DEFAULT 0 CHECK (current_occupancy <= capacity),
    monthly_fee DECIMAL(8,2) NOT NULL DEFAULT 1500.00,
    status ENUM('AVAILABLE', 'PARTIAL', 'OCCUPIED', 'MAINTENANCE') NOT NULL DEFAULT 'AVAILABLE',
    UNIQUE KEY uq_hostel_room (hostel_id, room_number),
    FOREIGN KEY (hostel_id) REFERENCES hostels(hostel_id) ON DELETE CASCADE,
    INDEX idx_room_availability (hostel_id, status, current_occupancy, capacity),
    INDEX idx_room_type (room_type, status)
) ENGINE=InnoDB;

-- ----------------------------------------------------------------------------
-- 5. ALLOTMENT APPLICATIONS
-- ----------------------------------------------------------------------------
CREATE TABLE allotment_applications (
    application_id INT AUTO_INCREMENT PRIMARY KEY,
    student_id INT NOT NULL,
    preferred_hostel_id INT,
    preferred_room_type ENUM('SINGLE', 'DOUBLE', 'TRIPLE', 'QUAD') NOT NULL DEFAULT 'DOUBLE',
    merit_score DECIMAL(6,2) NOT NULL DEFAULT 0.00,
    status ENUM('PENDING', 'APPROVED', 'REJECTED', 'ALLOTTED') NOT NULL DEFAULT 'PENDING',
    applied_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    reviewed_at TIMESTAMP NULL DEFAULT NULL,
    remarks VARCHAR(255) DEFAULT NULL,
    FOREIGN KEY (student_id) REFERENCES students(student_id) ON DELETE CASCADE,
    FOREIGN KEY (preferred_hostel_id) REFERENCES hostels(hostel_id) ON DELETE SET NULL,
    INDEX idx_app_status_merit (status, merit_score DESC)
) ENGINE=InnoDB;

-- ----------------------------------------------------------------------------
-- 6. ALLOTMENTS TABLE (Active and Historical Accommodations)
-- ----------------------------------------------------------------------------
CREATE TABLE allotments (
    allotment_id INT AUTO_INCREMENT PRIMARY KEY,
    application_id INT NULL,
    student_id INT NOT NULL,
    room_id INT NOT NULL,
    allotted_date DATE NOT NULL,
    valid_until DATE NOT NULL,
    is_active TINYINT(1) NOT NULL DEFAULT 1,
    dues_cleared TINYINT(1) NOT NULL DEFAULT 1,
    checkin_time TIMESTAMP NULL DEFAULT NULL,
    checkout_time TIMESTAMP NULL DEFAULT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (application_id) REFERENCES allotment_applications(application_id) ON DELETE SET NULL,
    FOREIGN KEY (student_id) REFERENCES students(student_id) ON DELETE RESTRICT,
    FOREIGN KEY (room_id) REFERENCES rooms(room_id) ON DELETE RESTRICT,
    INDEX idx_allot_student (student_id, is_active),
    INDEX idx_allot_room (room_id, is_active)
) ENGINE=InnoDB;

-- ----------------------------------------------------------------------------
-- 7. ROOM SWAP REQUESTS (Between two students with approval workflow)
-- ----------------------------------------------------------------------------
CREATE TABLE room_swap_requests (
    swap_id INT AUTO_INCREMENT PRIMARY KEY,
    applicant_student_id INT NOT NULL,
    target_student_id INT NOT NULL,
    applicant_room_id INT NOT NULL,
    target_room_id INT NOT NULL,
    reason TEXT NOT NULL,
    status ENUM('PENDING', 'APPROVED', 'REJECTED', 'CANCELLED') NOT NULL DEFAULT 'PENDING',
    requested_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    approved_by_user_id INT NULL,
    decision_at TIMESTAMP NULL DEFAULT NULL,
    FOREIGN KEY (applicant_student_id) REFERENCES students(student_id),
    FOREIGN KEY (target_student_id) REFERENCES students(student_id),
    FOREIGN KEY (applicant_room_id) REFERENCES rooms(room_id),
    FOREIGN KEY (target_room_id) REFERENCES rooms(room_id),
    FOREIGN KEY (approved_by_user_id) REFERENCES users(user_id) ON DELETE SET NULL
) ENGINE=InnoDB;

-- ----------------------------------------------------------------------------
-- 8. MAINTENANCE COMPLAINTS (Ticketing system for facility issues)
-- ----------------------------------------------------------------------------
CREATE TABLE maintenance_complaints (
    complaint_id INT AUTO_INCREMENT PRIMARY KEY,
    student_id INT NOT NULL,
    room_id INT NOT NULL,
    category ENUM('ELECTRICAL', 'PLUMBING', 'CARPENTRY', 'CLEANLINESS', 'NETWORK', 'OTHER') NOT NULL,
    description TEXT NOT NULL,
    status ENUM('OPEN', 'IN_PROGRESS', 'RESOLVED') NOT NULL DEFAULT 'OPEN',
    filed_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    resolved_at TIMESTAMP NULL DEFAULT NULL,
    resolution_notes TEXT,
    FOREIGN KEY (student_id) REFERENCES students(student_id) ON DELETE CASCADE,
    FOREIGN KEY (room_id) REFERENCES rooms(room_id) ON DELETE CASCADE,
    INDEX idx_complaint_status (status, category)
) ENGINE=InnoDB;

-- ----------------------------------------------------------------------------
-- 9. FEE TRANSACTIONS
-- ----------------------------------------------------------------------------
CREATE TABLE fee_transactions (
    transaction_id INT AUTO_INCREMENT PRIMARY KEY,
    student_id INT NOT NULL,
    allotment_id INT NOT NULL,
    amount DECIMAL(8,2) NOT NULL,
    payment_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    transaction_ref VARCHAR(64) NOT NULL UNIQUE,
    status ENUM('PAID', 'PENDING', 'FAILED') NOT NULL DEFAULT 'PAID',
    FOREIGN KEY (student_id) REFERENCES students(student_id),
    FOREIGN KEY (allotment_id) REFERENCES allotments(allotment_id)
) ENGINE=InnoDB;

-- ----------------------------------------------------------------------------
-- 10. AUDIT LOGS (Security, Tracking, Non-Repudiation)
-- ----------------------------------------------------------------------------
CREATE TABLE audit_logs (
    log_id INT AUTO_INCREMENT PRIMARY KEY,
    user_id INT NULL,
    username VARCHAR(64) NOT NULL,
    action VARCHAR(64) NOT NULL,
    entity_affected VARCHAR(64) NOT NULL,
    entity_id INT NOT NULL,
    details TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE SET NULL,
    INDEX idx_audit_time (created_at)
) ENGINE=InnoDB;

-- ----------------------------------------------------------------------------
-- TRIGGERS FOR DATA INTEGRITY AND REAL-TIME VACANCY TRACKING
-- ----------------------------------------------------------------------------
DELIMITER $$

-- Trigger 1: After inserting an active allotment, increment room occupancy and update status
CREATE TRIGGER trg_allotment_insert AFTER INSERT ON allotments
FOR EACH ROW
BEGIN
    IF NEW.is_active = 1 THEN
        UPDATE rooms
        SET current_occupancy = current_occupancy + 1,
            status = CASE
                WHEN current_occupancy + 1 >= capacity THEN 'OCCUPIED'
                ELSE 'PARTIAL'
            END
        WHERE room_id = NEW.room_id;

        UPDATE hostels
        SET current_occupancy = current_occupancy + 1
        WHERE hostel_id = (SELECT hostel_id FROM rooms WHERE room_id = NEW.room_id);
    END IF;
END$$

-- Trigger 2: After updating allotment to inactive (vacating/checkout), decrement occupancy
CREATE TRIGGER trg_allotment_update AFTER UPDATE ON allotments
FOR EACH ROW
BEGIN
    IF OLD.is_active = 1 AND NEW.is_active = 0 THEN
        UPDATE rooms
        SET current_occupancy = GREATEST(0, current_occupancy - 1),
            status = CASE
                WHEN GREATEST(0, current_occupancy - 1) = 0 THEN 'AVAILABLE'
                ELSE 'PARTIAL'
            END
        WHERE room_id = NEW.room_id;

        UPDATE hostels
        SET current_occupancy = GREATEST(0, current_occupancy - 1)
        WHERE hostel_id = (SELECT hostel_id FROM rooms WHERE room_id = NEW.room_id);
    END IF;
END$$

DELIMITER ;

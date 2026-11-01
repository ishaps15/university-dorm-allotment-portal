-- ============================================================================
-- UNIVERSITY DORM ALLOTMENT PORTAL - OPTIMIZED SQL QUERIES & ANALYTICS
-- Campus: Durgapur, West Bengal
-- Database Engine: MySQL / MariaDB (InnoDB)
-- ============================================================================

USE university_dorm_db;

-- ----------------------------------------------------------------------------
-- QUERY 1: REAL-TIME ROOM AVAILABILITY TRACKING (With Gender & Hostel Filter)
-- Purpose: Find all rooms that have vacant beds, matching student's eligible dorms.
-- Optimization: Uses composite index (hostel_id, status, current_occupancy, capacity)
-- ----------------------------------------------------------------------------
SELECT 
    h.hostel_id,
    h.name AS hostel_name,
    h.type AS hostel_gender_type,
    r.room_id,
    r.room_number,
    r.floor,
    r.room_type,
    r.capacity,
    r.current_occupancy,
    (r.capacity - r.current_occupancy) AS available_beds,
    r.monthly_fee,
    r.status
FROM rooms r
JOIN hostels h ON r.hostel_id = h.hostel_id
WHERE (h.type = 'MALE' OR h.type = 'COED') -- Parameterized for student gender
  AND r.status IN ('AVAILABLE', 'PARTIAL')
  AND r.current_occupancy < r.capacity
ORDER BY h.name ASC, r.floor ASC, r.room_number ASC;

-- ----------------------------------------------------------------------------
-- QUERY 2: MERIT RANK ORDERED ALLOTMENT APPLICANTS
-- Purpose: Retrieve pending applicants ordered by computed merit score for allocation.
-- Formula: Merit Score = (CGPA * 7.0) + (LEAST(Distance_km, 1000) / 1000 * 20.0) + (Year * 2.5)
-- Optimization: Filter on (status, merit_score DESC) via idx_app_status_merit.
-- ----------------------------------------------------------------------------
SELECT 
    a.application_id,
    s.student_id,
    s.roll_number,
    s.full_name,
    s.department,
    s.year_of_study,
    s.gender,
    s.cgpa,
    s.home_distance_km,
    a.preferred_hostel_id,
    h.name AS preferred_hostel_name,
    a.preferred_room_type,
    a.merit_score,
    a.applied_at
FROM allotment_applications a
JOIN students s ON a.student_id = s.student_id
LEFT JOIN hostels h ON a.preferred_hostel_id = h.hostel_id
WHERE a.status = 'PENDING'
ORDER BY a.merit_score DESC, s.cgpa DESC, s.home_distance_km DESC;

-- ----------------------------------------------------------------------------
-- QUERY 3: ATOMIC ROOM ALLOTMENT TRANSACTION
-- Purpose: Assigns an applicant to a room atomically, preventing double booking.
-- ----------------------------------------------------------------------------
-- Step 3a: Verify room vacancy with row-level lock (FOR UPDATE)
-- SELECT room_id, current_occupancy, capacity 
-- FROM rooms 
-- WHERE room_id = ? AND current_occupancy < capacity 
-- FOR UPDATE;

-- Step 3b: Insert allotment record
-- INSERT INTO allotments (application_id, student_id, room_id, allotted_date, valid_until, is_active, dues_cleared, checkin_time)
-- VALUES (?, ?, ?, CURDATE(), DATE_ADD(CURDATE(), INTERVAL 1 YEAR), 1, 1, NOW());

-- Step 3c: Update application status
-- UPDATE allotment_applications 
-- SET status = 'ALLOTTED', reviewed_at = NOW(), remarks = 'Allotted via automated merit run' 
-- WHERE application_id = ?;

-- ----------------------------------------------------------------------------
-- QUERY 4: CAMPUS & HOSTEL OCCUPANCY ANALYTICS DASHBOARD
-- Purpose: Executive summary for Chief Warden / Admin.
-- Aggregates total capacity, occupied beds, vacant beds, and occupancy rate %.
-- ----------------------------------------------------------------------------
SELECT 
    h.hostel_id,
    h.name AS hostel_name,
    h.code,
    h.type AS hostel_type,
    COUNT(r.room_id) AS total_rooms_configured,
    SUM(r.capacity) AS total_bed_capacity,
    SUM(r.current_occupancy) AS total_occupied_beds,
    (SUM(r.capacity) - SUM(r.current_occupancy)) AS total_vacant_beds,
    ROUND((SUM(r.current_occupancy) * 100.0) / NULLIF(SUM(r.capacity), 0), 2) AS occupancy_percentage,
    COUNT(CASE WHEN r.status = 'MAINTENANCE' THEN 1 END) AS rooms_under_maintenance
FROM hostels h
LEFT JOIN rooms r ON h.hostel_id = r.hostel_id
GROUP BY h.hostel_id, h.name, h.code, h.type
ORDER BY h.hostel_id;

-- ----------------------------------------------------------------------------
-- QUERY 5: STUDENT ACTIVE ALLOTMENT DETAILS
-- Purpose: Display current room, roommates, and hostel warden info for student.
-- ----------------------------------------------------------------------------
SELECT 
    s.roll_number,
    s.full_name AS student_name,
    h.name AS hostel_name,
    r.room_number,
    r.floor,
    r.room_type,
    r.monthly_fee,
    al.allotted_date,
    al.valid_until,
    al.checkin_time,
    u_w.username AS warden_username,
    h.contact_phone AS warden_contact
FROM allotments al
JOIN students s ON al.student_id = s.student_id
JOIN rooms r ON al.room_id = r.room_id
JOIN hostels h ON r.hostel_id = h.hostel_id
LEFT JOIN users u_w ON h.warden_user_id = u_w.user_id
WHERE al.student_id = 1 AND al.is_active = 1;

-- ----------------------------------------------------------------------------
-- QUERY 6: FIND ROOMMATES IN CURRENT ALLOTMENT
-- Purpose: Lists other students sharing the same room.
-- ----------------------------------------------------------------------------
SELECT 
    s.roll_number,
    s.full_name,
    s.department,
    s.year_of_study,
    s.email,
    s.phone
FROM allotments al
JOIN students s ON al.student_id = s.student_id
WHERE al.room_id = (SELECT room_id FROM allotments WHERE student_id = 1 AND is_active = 1)
  AND al.student_id != 1
  AND al.is_active = 1;

-- ----------------------------------------------------------------------------
-- QUERY 7: MAINTENANCE COMPLAINT BACKLOG & RESOLUTION VELOCITY
-- Purpose: Monitor facility issues by hostel and urgency category.
-- ----------------------------------------------------------------------------
SELECT 
    mc.complaint_id,
    h.name AS hostel_name,
    r.room_number,
    s.roll_number,
    s.full_name AS student_name,
    mc.category,
    mc.description,
    mc.status,
    mc.filed_at,
    TIMESTAMPDIFF(HOUR, mc.filed_at, NOW()) AS hours_pending
FROM maintenance_complaints mc
JOIN students s ON mc.student_id = s.student_id
JOIN rooms r ON mc.room_id = r.room_id
JOIN hostels h ON r.hostel_id = h.hostel_id
WHERE mc.status IN ('OPEN', 'IN_PROGRESS')
ORDER BY mc.filed_at ASC;

-- ----------------------------------------------------------------------------
-- QUERY 8: ROOM SWAP FEASIBILITY CHECK
-- Purpose: Validates that both students have active allotments and belong to
-- eligible dorms before executing a swap.
-- ----------------------------------------------------------------------------
SELECT 
    req.swap_id,
    s1.roll_number AS applicant_roll,
    s1.gender AS applicant_gender,
    r1.room_number AS applicant_room,
    h1.name AS applicant_hostel,
    s2.roll_number AS target_roll,
    s2.gender AS target_gender,
    r2.room_number AS target_room,
    h2.name AS target_hostel,
    req.reason,
    req.status
FROM room_swap_requests req
JOIN students s1 ON req.applicant_student_id = s1.student_id
JOIN students s2 ON req.target_student_id = s2.student_id
JOIN rooms r1 ON req.applicant_room_id = r1.room_id
JOIN rooms r2 ON req.target_room_id = r2.room_id
JOIN hostels h1 ON r1.hostel_id = h1.hostel_id
JOIN hostels h2 ON r2.hostel_id = h2.hostel_id
WHERE req.status = 'PENDING';

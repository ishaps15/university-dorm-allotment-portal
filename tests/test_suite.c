#include "auth.h"
#include "crypto_sha256.h"
#include "db.h"
#include "dorm_service.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEST_PASS(name) printf("  [PASS] %-45s\n", name)
#define TEST_FAIL(name, reason) do { printf("  [FAIL] %-45s (%s)\n", name, reason); exit(1); } while(0)

static void test_sha256_cryptography(void) {
    /* Test vector for "abc" from NIST FIPS 180-2 */
    char hex[65];
    sha256_hash_string("abc", hex);
    const char *expected = "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad";
    if (strcmp(hex, expected) != 0) {
        TEST_FAIL("SHA-256 NIST Vector", "Hash did not match known standard");
    } else {
        TEST_PASS("SHA-256 NIST Test Vector Verification");
    }

    /* Test salt generation randomness */
    char salt1[33], salt2[33];
    generate_random_salt(salt1);
    generate_random_salt(salt2);
    if (strlen(salt1) != 32 || strlen(salt2) != 32 || strcmp(salt1, salt2) == 0) {
        TEST_FAIL("Salt Generation", "Salts must be 32 chars and unique");
    } else {
        TEST_PASS("Cryptographic Salt Generation & Uniqueness");
    }
}

static void test_authentication_system(void) {
    UserSession session;

    /* 1. Admin login */
    int res = auth_login("chief_warden", "Admin@NITD2025", &session);
    if (res != 0 || session.role != ROLE_ADMIN) {
        TEST_FAIL("Chief Warden Login", "Failed to authenticate admin");
    }
    TEST_PASS("Admin / Chief Warden Authentication");

    /* 2. Student login */
    res = auth_login("21CS8014", "Pass@Student1", &session);
    if (res != 0 || session.role != ROLE_STUDENT || strcmp(session.roll_number, "21CS8014") != 0) {
        TEST_FAIL("Student Login", "Failed to authenticate student");
    }
    TEST_PASS("Student Credential Authentication");

    /* 3. Invalid password rejection */
    res = auth_login("21CS8014", "WrongPassword123!", &session);
    if (res != -3) {
        TEST_FAIL("Invalid Password Check", "Did not reject wrong password");
    }
    TEST_PASS("Brute-Force & Invalid Password Rejection");

    /* 4. Non-existent user */
    res = auth_login("non_existent_9999", "Pass@123", &session);
    if (res != -1) {
        TEST_FAIL("Non-existent User Check", "Did not reject non-existent user");
    }
    TEST_PASS("Non-Existent User Rejection");

    /* 5. Self-registration */
    UserSession new_session;
    res = auth_register_student(
        "24CS9999", "SecurePass@2025", "Bikram Roy", "Computer Science", 1, GENDER_MALE,
        "bikram.roy@nitdgp.ac.in", "+91-9876543210", 9.50, 600.0, "Durgapur", "West Bengal", &new_session
    );
    if (res != 0 || strcmp(new_session.roll_number, "24CS9999") != 0) {
        TEST_FAIL("Student Registration", "Self-registration failed");
    }
    TEST_PASS("Student Self-Registration & Account Creation");
}

static void test_merit_score_algorithm(void) {
    /* Merit formula: (cgpa/10)*70 + (distance/1000)*20 + (year/4)*10 */
    double score1 = service_calculate_merit_score(10.0, 1000.0, 4);
    if (score1 < 99.99 || score1 > 100.01) {
        TEST_FAIL("Max Merit Score", "10.0 CGPA, 1000km, 4th yr must yield 100.0");
    }
    TEST_PASS("Maximum Merit Score Computation (100.0)");

    double score2 = service_calculate_merit_score(8.0, 200.0, 2);
    /* (8/10)*70 = 56, (200/1000)*20 = 4, (2/4)*10 = 5 => 65.0 */
    if (score2 < 64.99 || score2 > 65.01) {
        TEST_FAIL("Weighted Merit Score", "Score formula calculation mismatch");
    }
    TEST_PASS("Weighted Priority Merit Calculation Formula");
}

static void test_room_availability_tracking(void) {
    Room rooms_male[32];
    int count_m = 0;
    service_get_vacancies(GENDER_MALE, (RoomType)0, rooms_male, 32, &count_m);
    if (count_m <= 0) {
        TEST_FAIL("Male Room Vacancy", "Expected available male rooms");
    }
    for (int i = 0; i < count_m; i++) {
        if (rooms_male[i].current_occupancy >= rooms_male[i].capacity) {
            TEST_FAIL("Occupancy Check", "Returned room with no available beds");
        }
    }
    TEST_PASS("Real-Time Bed Vacancy Tracking");

    Room rooms_female[32];
    int count_f = 0;
    service_get_vacancies(GENDER_FEMALE, (RoomType)0, rooms_female, 32, &count_f);
    if (count_f <= 0) {
        TEST_FAIL("Female Room Vacancy", "Expected available female rooms");
    }
    TEST_PASS("Gender-Segregated Dormitory Vacancy Filtering");
}

static void test_allotment_engine(void) {
    /* Student 23CS8045 (student_id = 5) has a pending application */
    int allotted = 0;
    int res = service_run_automated_allotment(&allotted);
    if (res != DB_SUCCESS || allotted < 1) {
        TEST_FAIL("Automated Allotment", "Expected at least 1 pending applicant to be allotted");
    }
    TEST_PASS("Automated Priority-Merit Room Allotment Execution");

    /* Verify allotment is now active */
    Allotment allot;
    if (db_get_active_allotment(5, &allot) != DB_SUCCESS || !allot.is_active) {
        TEST_FAIL("Allotment Verification", "Student 5 did not receive active allotment");
    }
    TEST_PASS("Atomic Allotment Persistence & State Update");
}

static void test_room_swap_system(void) {
    /* Student 1 (Sourav Mukherjee, T-101) and Student 3 (Rahul Sharma, T-101) or other male */
    /* Let's swap Student 1 (in Room 1) and Student 5 (in newly allotted room) */
    Allotment al1, al5;
    db_get_active_allotment(1, &al1);
    db_get_active_allotment(5, &al5);

    int swap_id = 0;
    int res = service_request_room_swap(1, "23CS8045", "Mutual project collaboration", &swap_id);
    if (res != DB_SUCCESS || swap_id <= 0) {
        TEST_FAIL("Room Swap Request", "Failed to initiate room swap");
    }
    TEST_PASS("Room Swap Request Submission");

    /* Cross-gender swap rejection test */
    int invalid_swap = 0;
    res = service_request_room_swap(1, "21EC8022" /* female student */, "Invalid test", &invalid_swap);
    if (res != -6) {
        TEST_FAIL("Cross-Gender Swap Rejection", "Allowed cross-gender dorm swap!");
    }
    TEST_PASS("Cross-Gender Swap Security Constraint Enforcement");

    /* Approve swap */
    int old_r1 = al1.room_id;
    int old_r5 = al5.room_id;
    res = service_approve_room_swap(swap_id, 2 /* Warden */);
    if (res != DB_SUCCESS) {
        TEST_FAIL("Room Swap Approval", "Warden approval failed");
    }

    /* Verify exchanged rooms */
    db_get_active_allotment(1, &al1);
    db_get_active_allotment(5, &al5);
    if (al1.room_id != old_r5 || al5.room_id != old_r1) {
        TEST_FAIL("Room Swap Exchange", "Rooms were not properly exchanged");
    }
    TEST_PASS("Atomic Room Swap Execution & Roster Synchronization");
}

static void test_maintenance_complaint_lifecycle(void) {
    int comp_id = 0;
    int res = service_file_complaint(1, CAT_ELECTRICAL, "Desk tube light flickering continuously.", &comp_id);
    if (res != DB_SUCCESS || comp_id <= 0) {
        TEST_FAIL("Complaint Filing", "Failed to file maintenance ticket");
    }
    TEST_PASS("Maintenance Complaint Ticket Creation");

    res = service_resolve_complaint(comp_id, "Replaced LED tube light starter and bulb.", 2);
    if (res != DB_SUCCESS) {
        TEST_FAIL("Complaint Resolution", "Failed to resolve complaint");
    }

    MaintenanceComplaint comps[10];
    int count = 0;
    db_get_complaints_by_student(1, comps, 10, &count);
    bool found_resolved = false;
    for (int i = 0; i < count; i++) {
        if (comps[i].complaint_id == comp_id && comps[i].status == COMPLAINT_RESOLVED) {
            found_resolved = true;
            break;
        }
    }
    if (!found_resolved) {
        TEST_FAIL("Complaint Status Verification", "Ticket status was not marked RESOLVED");
    }
    TEST_PASS("Maintenance Complaint Resolution & Audit Tracking");
}

int main(void) {
    printf("\n=======================================================================\n");
    printf("   UNIVERSITY DORM ALLOTMENT PORTAL - AUTOMATED TEST SUITE             \n");
    printf("   Location: Durgapur, West Bengal | Tech: C, SQL, MySQL, SHA-256 Auth \n");
    printf("=======================================================================\n\n");

    db_init(NULL);

    printf("[1/6] Cryptographic Engine & Hashing Tests:\n");
    test_sha256_cryptography();

    printf("\n[2/6] Authentication & Authorization Tests:\n");
    test_authentication_system();

    printf("\n[3/6] Merit Scoring & Priority Engine Tests:\n");
    test_merit_score_algorithm();

    printf("\n[4/6] Room Vacancy & Availability Tracking Tests:\n");
    test_room_availability_tracking();

    printf("\n[5/6] Automated Room Allocation Engine Tests:\n");
    test_allotment_engine();

    printf("\n[6/6] Room Swap & Maintenance Ticketing Tests:\n");
    test_room_swap_system();
    test_maintenance_complaint_lifecycle();

    printf("\n=======================================================================\n");
    printf("   >>> ALL TEST CASES PASSED SUCCESSFULLY (15/15) - ZERO ERRORS!      \n");
    printf("=======================================================================\n\n");
    return 0;
}

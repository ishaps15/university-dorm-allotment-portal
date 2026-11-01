#include "dorm_service.h"
#include "db.h"
#include <stdio.h>
#include <string.h>

double service_calculate_merit_score(double cgpa, double home_distance_km, int year_of_study) {
    if (cgpa < 0.0) cgpa = 0.0;
    if (cgpa > 10.0) cgpa = 10.0;

    double cgpa_component = (cgpa / 10.0) * 70.0; /* 70% weight on academic excellence */

    double capped_dist = home_distance_km;
    if (capped_dist > 1000.0) capped_dist = 1000.0;
    if (capped_dist < 0.0) capped_dist = 0.0;
    double dist_component = (capped_dist / 1000.0) * 20.0; /* 20% weight on domicile distance */

    int yr = year_of_study;
    if (yr < 1) yr = 1;
    if (yr > 4) yr = 4;
    double year_component = ((double)yr / 4.0) * 10.0; /* 10% weight on seniority */

    return cgpa_component + dist_component + year_component;
}

int service_submit_application(int student_id, int preferred_hostel_id, RoomType preferred_type, int *out_app_id) {
    /* 1. Check if student already has active allotment */
    Allotment existing_allotment;
    if (db_get_active_allotment(student_id, &existing_allotment) == DB_SUCCESS && existing_allotment.is_active) {
        return -1; /* Student already holds active room allotment */
    }

    /* 2. Check if student already has pending application */
    AllotmentApplication existing_app;
    if (db_get_student_application(student_id, &existing_app) == DB_SUCCESS) {
        if (existing_app.status == APP_PENDING) {
            return -2; /* Already has pending application */
        }
    }

    /* 3. Get student profile to calculate merit score */
    Student student;
    if (db_get_student_by_id(student_id, &student) != DB_SUCCESS) {
        return -3;
    }

    double merit = service_calculate_merit_score(student.cgpa, student.home_distance_km, student.year_of_study);

    return db_create_application(student_id, preferred_hostel_id, preferred_type, merit, out_app_id);
}

int service_run_automated_allotment(int *out_allotted_count) {
    if (!out_allotted_count) return -1;
    *out_allotted_count = 0;

    AllotmentApplication pending_apps[100];
    int app_count = 0;

    if (db_get_pending_applications(pending_apps, 100, &app_count) != DB_SUCCESS || app_count == 0) {
        return 0; /* Nothing to allocate */
    }

    for (int i = 0; i < app_count; i++) {
        AllotmentApplication *app = &pending_apps[i];
        Student student;
        if (db_get_student_by_id(app->student_id, &student) != DB_SUCCESS) continue;

        /* Find candidate vacant rooms matching gender */
        Room available_rooms[100];
        int room_count = 0;
        if (db_get_available_rooms(student.gender, (RoomType)0 /* any */, available_rooms, 100, &room_count) != DB_SUCCESS || room_count == 0) {
            continue; /* No rooms currently vacant for this gender */
        }

        /* Preference matching: try preferred hostel first, then fallback to any eligible room */
        int chosen_room_id = -1;
        for (int r = 0; r < room_count; r++) {
            if (app->preferred_hostel_id > 0 && available_rooms[r].hostel_id == app->preferred_hostel_id) {
                if (app->preferred_room_type == 0 || available_rooms[r].room_type == app->preferred_room_type) {
                    chosen_room_id = available_rooms[r].room_id;
                    break;
                }
            }
        }

        /* Fallback 1: preferred hostel, any room type */
        if (chosen_room_id == -1 && app->preferred_hostel_id > 0) {
            for (int r = 0; r < room_count; r++) {
                if (available_rooms[r].hostel_id == app->preferred_hostel_id) {
                    chosen_room_id = available_rooms[r].room_id;
                    break;
                }
            }
        }

        /* Fallback 2: any room in any eligible hostel */
        if (chosen_room_id == -1 && room_count > 0) {
            chosen_room_id = available_rooms[0].room_id;
        }

        if (chosen_room_id != -1) {
            /* Execute atomic allotment transaction */
            db_begin_transaction();
            int allotment_id = 0;
            if (db_create_allotment(app->application_id, student.student_id, chosen_room_id, &allotment_id) == DB_SUCCESS) {
                db_commit();
                (*out_allotted_count)++;
                db_log_audit(0, "SYSTEM_ALLOC_ENGINE", "ROOM_AUTO_ALLOTTED", "ALLOTMENTS", allotment_id, "Automated merit allotment executed");
            } else {
                db_rollback();
            }
        }
    }

    return 0;
}

int service_get_vacancies(Gender student_gender, RoomType type_filter, Room *out_rooms, int max_rooms, int *out_count) {
    return db_get_available_rooms(student_gender, type_filter, out_rooms, max_rooms, out_count);
}

int service_get_campus_stats(CampusStats *stats) {
    if (!stats) return -1;
    memset(stats, 0, sizeof(CampusStats));

    Hostel hostels[32];
    int hostel_count = 0;
    if (db_get_hostels(hostels, 32, &hostel_count) == DB_SUCCESS) {
        stats->total_dorms = hostel_count;
        for (int i = 0; i < hostel_count; i++) {
            stats->total_rooms += hostels[i].total_rooms;
            stats->total_capacity += hostels[i].total_capacity;
            stats->total_occupied += hostels[i].current_occupancy;
        }
    }

    stats->total_available = stats->total_capacity - stats->total_occupied;
    if (stats->total_capacity > 0) {
        stats->overall_occupancy_rate = ((double)stats->total_occupied / stats->total_capacity) * 100.0;
    }

    AllotmentApplication apps[100];
    int app_count = 0;
    if (db_get_pending_applications(apps, 100, &app_count) == DB_SUCCESS) {
        stats->pending_applications = app_count;
    }

    MaintenanceComplaint complaints[100];
    int comp_count = 0;
    if (db_get_all_complaints(COMPLAINT_OPEN, complaints, 100, &comp_count) == DB_SUCCESS) {
        stats->open_complaints = comp_count;
    }

    return 0;
}

int service_request_room_swap(int applicant_student_id, const char *target_roll, const char *reason, int *out_swap_id) {
    if (!target_roll || !reason) return -1;

    /* 1. Verify applicant active allotment */
    Allotment allot1;
    if (db_get_active_allotment(applicant_student_id, &allot1) != DB_SUCCESS || !allot1.is_active) {
        return -2; /* Applicant has no active allotment */
    }

    /* 2. Find target student */
    Student target_student;
    if (db_get_student_by_roll(target_roll, &target_student) != DB_SUCCESS) {
        return -3; /* Target student not found */
    }

    if (target_student.student_id == applicant_student_id) {
        return -4; /* Cannot swap with oneself */
    }

    /* 3. Verify target active allotment */
    Allotment allot2;
    if (db_get_active_allotment(target_student.student_id, &allot2) != DB_SUCCESS || !allot2.is_active) {
        return -5; /* Target student has no active allotment */
    }

    /* 4. Gender compatibility check */
    Student applicant_student;
    db_get_student_by_id(applicant_student_id, &applicant_student);
    if (applicant_student.gender != target_student.gender) {
        return -6; /* Cannot swap between different gender dorms */
    }

    return db_create_swap_request(applicant_student_id, target_student.student_id, allot1.room_id, allot2.room_id, reason, out_swap_id);
}

int service_approve_room_swap(int swap_id, int warden_user_id) {
    int res = db_execute_room_swap(swap_id, warden_user_id);
    if (res == DB_SUCCESS) {
        db_log_audit(warden_user_id, "WARDEN", "ROOM_SWAP_APPROVED", "ROOM_SWAP", swap_id, "Warden approved room swap");
    }
    return res;
}

int service_vacate_student(int student_id, int warden_user_id) {
    Allotment allot;
    if (db_get_active_allotment(student_id, &allot) != DB_SUCCESS || !allot.is_active) {
        return -1;
    }

    int res = db_vacate_allotment(allot.allotment_id);
    if (res == DB_SUCCESS) {
        db_log_audit(warden_user_id, "WARDEN", "STUDENT_CHECKOUT", "ALLOTMENTS", allot.allotment_id, "Student vacated room");
    }
    return res;
}

int service_file_complaint(int student_id, ComplaintCategory category, const char *description, int *out_complaint_id) {
    Allotment allot;
    if (db_get_active_allotment(student_id, &allot) != DB_SUCCESS || !allot.is_active) {
        return -1; /* Must be residing in a room to file complaint */
    }

    return db_create_complaint(student_id, allot.room_id, category, description, out_complaint_id);
}

int service_resolve_complaint(int complaint_id, const char *resolution_notes, int warden_user_id) {
    int res = db_update_complaint_status(complaint_id, COMPLAINT_RESOLVED, resolution_notes);
    if (res == DB_SUCCESS) {
        db_log_audit(warden_user_id, "WARDEN", "COMPLAINT_RESOLVED", "MAINTENANCE", complaint_id, resolution_notes);
    }
    return res;
}

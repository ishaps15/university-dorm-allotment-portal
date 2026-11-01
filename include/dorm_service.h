#ifndef DORM_SERVICE_H
#define DORM_SERVICE_H

#include "models.h"

typedef struct {
    int total_dorms;
    int total_rooms;
    int total_capacity;
    int total_occupied;
    int total_available;
    double overall_occupancy_rate;
    int pending_applications;
    int open_complaints;
} CampusStats;

/* Allotment & Merit engine */
double service_calculate_merit_score(double cgpa, double home_distance_km, int year_of_study);
int    service_submit_application(int student_id, int preferred_hostel_id, RoomType preferred_type, int *out_app_id);
int    service_run_automated_allotment(int *out_allotted_count);

/* Room Availability & Catalog */
int    service_get_vacancies(Gender student_gender, RoomType type_filter, Room *out_rooms, int max_rooms, int *out_count);
int    service_get_campus_stats(CampusStats *stats);

/* Accommodations & Room Swap */
int    service_request_room_swap(int applicant_student_id, const char *target_roll, const char *reason, int *out_swap_id);
int    service_approve_room_swap(int swap_id, int warden_user_id);
int    service_vacate_student(int student_id, int warden_user_id);

/* Maintenance complaints */
int    service_file_complaint(int student_id, ComplaintCategory category, const char *description, int *out_complaint_id);
int    service_resolve_complaint(int complaint_id, const char *resolution_notes, int warden_user_id);

#endif /* DORM_SERVICE_H */

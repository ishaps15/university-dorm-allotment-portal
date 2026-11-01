#ifndef DB_H
#define DB_H

#include "models.h"
#include <stdbool.h>

#define DB_SUCCESS 0
#define DB_ERROR  -1

typedef struct {
    char host[MAX_STR_SM];
    int  port;
    char user[MAX_STR_SM];
    char password[MAX_STR_SM];
    char database[MAX_STR_SM];
    bool use_live_mysql;
    void *mysql_conn; /* void* to abstract MYSQL* without forcing header order */
} DBConfig;

/* Core DB lifecycle */
int  db_init(DBConfig *config);
int  db_connect(void);
void db_disconnect(void);
bool db_is_connected(void);
int  db_execute(const char *sql_format, ...);
int  db_begin_transaction(void);
int  db_commit(void);
int  db_rollback(void);

/* User & Authentication Repository */
int  db_get_user_by_username(const char *username, User *user);
int  db_get_user_by_id(int user_id, User *user);
int  db_create_user(const char *username, const char *password_hash, const char *salt, UserRole role, int *out_user_id);
int  db_update_password(int user_id, const char *new_hash, const char *new_salt);
int  db_update_last_login(int user_id);

/* Student Repository */
int  db_get_student_by_user_id(int user_id, Student *student);
int  db_get_student_by_roll(const char *roll, Student *student);
int  db_get_student_by_id(int student_id, Student *student);
int  db_create_student(Student *student);
int  db_get_all_students(Student *students, int max_count, int *out_count);

/* Hostels & Rooms Repository */
int  db_get_hostels(Hostel *hostels, int max_count, int *out_count);
int  db_get_hostel_by_id(int hostel_id, Hostel *hostel);
int  db_get_rooms_by_hostel(int hostel_id, Room *rooms, int max_count, int *out_count);
int  db_get_available_rooms(Gender gender, RoomType type, Room *rooms, int max_count, int *out_count);
int  db_get_room_by_id(int room_id, Room *room);
int  db_update_room_occupancy(int room_id, int delta);
int  db_update_room_status(int room_id, RoomStatus status);

/* Allotments & Applications */
int  db_create_application(int student_id, int preferred_hostel_id, RoomType preferred_type, double merit_score, int *out_app_id);
int  db_get_student_application(int student_id, AllotmentApplication *app);
int  db_get_pending_applications(AllotmentApplication *apps, int max_count, int *out_count);
int  db_create_allotment(int application_id, int student_id, int room_id, int *out_allotment_id);
int  db_get_active_allotment(int student_id, Allotment *allotment);
int  db_get_room_allotments(int room_id, Allotment *allotments, int max_count, int *out_count);
int  db_vacate_allotment(int allotment_id);

/* Room Swap */
int  db_create_swap_request(int student1_id, int student2_id, int room1_id, int room2_id, const char *reason, int *out_swap_id);
int  db_get_pending_swap_requests(RoomSwapRequest *swaps, int max_count, int *out_count);
int  db_execute_room_swap(int swap_id, int approved_by_user_id);
int  db_reject_room_swap(int swap_id, int rejected_by_user_id);

/* Complaints */
int  db_create_complaint(int student_id, int room_id, ComplaintCategory category, const char *description, int *out_complaint_id);
int  db_get_complaints_by_student(int student_id, MaintenanceComplaint *complaints, int max_count, int *out_count);
int  db_get_all_complaints(ComplaintStatus filter_status, MaintenanceComplaint *complaints, int max_count, int *out_count);
int  db_update_complaint_status(int complaint_id, ComplaintStatus status, const char *resolution_notes);

/* Audit Logs */
int  db_log_audit(int user_id, const char *username, const char *action, const char *entity, int entity_id, const char *details);
int  db_get_audit_logs(AuditLog *logs, int max_count, int *out_count);

#endif /* DB_H */

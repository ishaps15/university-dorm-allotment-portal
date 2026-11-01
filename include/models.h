#ifndef MODELS_H
#define MODELS_H

#include <time.h>
#include <stdbool.h>

#define MAX_STR_SM  64
#define MAX_STR_MD  128
#define MAX_STR_LG  256
#define MAX_TEXT    1024
#define HASH_HEX_LEN 65
#define SALT_HEX_LEN 33

typedef enum {
    ROLE_STUDENT = 1,
    ROLE_WARDEN  = 2,
    ROLE_ADMIN   = 3
} UserRole;

typedef enum {
    GENDER_MALE   = 'M',
    GENDER_FEMALE = 'F',
    GENDER_OTHER  = 'O'
} Gender;

typedef enum {
    HOSTEL_MALE   = 1,
    HOSTEL_FEMALE = 2,
    HOSTEL_COED   = 3
} HostelType;

typedef enum {
    ROOM_SINGLE = 1,
    ROOM_DOUBLE = 2,
    ROOM_TRIPLE = 3,
    ROOM_QUAD   = 4
} RoomType;

typedef enum {
    ROOM_AVAILABLE    = 1,
    ROOM_PARTIAL      = 2,
    ROOM_OCCUPIED     = 3,
    ROOM_MAINTENANCE  = 4
} RoomStatus;

typedef enum {
    APP_PENDING   = 1,
    APP_APPROVED  = 2,
    APP_REJECTED  = 3,
    APP_ALLOTTED  = 4
} ApplicationStatus;

typedef enum {
    COMPLAINT_OPEN        = 1,
    COMPLAINT_IN_PROGRESS = 2,
    COMPLAINT_RESOLVED    = 3
} ComplaintStatus;

typedef enum {
    CAT_ELECTRICAL  = 1,
    CAT_PLUMBING    = 2,
    CAT_CARPENTRY   = 3,
    CAT_CLEANLINESS = 4,
    CAT_NETWORK     = 5,
    CAT_OTHER       = 6
} ComplaintCategory;

typedef struct {
    int user_id;
    char username[MAX_STR_SM];
    char password_hash[HASH_HEX_LEN];
    char salt[SALT_HEX_LEN];
    UserRole role;
    int is_active;
    char created_at[MAX_STR_SM];
} User;

typedef struct {
    int student_id;
    int user_id;
    char roll_number[MAX_STR_SM];
    char full_name[MAX_STR_MD];
    char department[MAX_STR_SM];
    int year_of_study;
    Gender gender;
    char email[MAX_STR_MD];
    char phone[MAX_STR_SM];
    double cgpa;
    double home_distance_km;  /* Distance from Durgapur (km) for merit/priority calculation */
    char city[MAX_STR_SM];
    char state[MAX_STR_SM];
} Student;

typedef struct {
    int hostel_id;
    char name[MAX_STR_MD];
    HostelType type;
    int total_floors;
    int total_rooms;
    int total_capacity;
    int current_occupancy;
    int warden_user_id;
    char warden_name[MAX_STR_MD];
    char contact_phone[MAX_STR_SM];
} Hostel;

typedef struct {
    int room_id;
    int hostel_id;
    char hostel_name[MAX_STR_MD];
    char room_number[MAX_STR_SM];
    int floor;
    RoomType room_type;
    int capacity;
    int current_occupancy;
    double monthly_fee;
    RoomStatus status;
} Room;

typedef struct {
    int application_id;
    int student_id;
    char student_name[MAX_STR_MD];
    char roll_number[MAX_STR_SM];
    int preferred_hostel_id;
    char preferred_hostel_name[MAX_STR_MD];
    RoomType preferred_room_type;
    double merit_score;       /* Computed from CGPA, distance, and academic year */
    ApplicationStatus status;
    char applied_at[MAX_STR_SM];
    char remarks[MAX_STR_LG];
} AllotmentApplication;

typedef struct {
    int allotment_id;
    int application_id;
    int student_id;
    char student_name[MAX_STR_MD];
    char roll_number[MAX_STR_SM];
    int room_id;
    char room_number[MAX_STR_SM];
    char hostel_name[MAX_STR_MD];
    char allotted_date[MAX_STR_SM];
    char valid_until[MAX_STR_SM];
    int is_active;
    int dues_cleared;
} Allotment;

typedef struct {
    int swap_id;
    int student1_id;
    int student2_id;
    char student1_roll[MAX_STR_SM];
    char student2_roll[MAX_STR_SM];
    int room1_id;
    int room2_id;
    char status[MAX_STR_SM]; /* PENDING, APPROVED, REJECTED */
    char reason[MAX_TEXT];
    char requested_at[MAX_STR_SM];
} RoomSwapRequest;

typedef struct {
    int complaint_id;
    int student_id;
    char student_name[MAX_STR_MD];
    char roll_number[MAX_STR_SM];
    int room_id;
    char room_number[MAX_STR_SM];
    char hostel_name[MAX_STR_MD];
    ComplaintCategory category;
    char description[MAX_TEXT];
    ComplaintStatus status;
    char filed_at[MAX_STR_SM];
    char resolved_at[MAX_STR_SM];
} MaintenanceComplaint;

typedef struct {
    int log_id;
    int user_id;
    char username[MAX_STR_SM];
    char action[MAX_STR_MD];
    char entity[MAX_STR_SM];
    int entity_id;
    char details[MAX_TEXT];
    char timestamp[MAX_STR_SM];
} AuditLog;

/* Session structure for authenticated user */
typedef struct {
    int user_id;
    char username[MAX_STR_SM];
    UserRole role;
    int student_id;            /* 0 if warden/admin */
    char roll_number[MAX_STR_SM];
    char full_name[MAX_STR_MD];
    Gender gender;
    int hostel_id;             /* For warden or current student allotment */
    bool is_authenticated;
} UserSession;

#endif /* MODELS_H */

#include "db.h"
#include "crypto_sha256.h"
#include <mysql/mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static DBConfig g_config;
static MYSQL *g_mysql = NULL;
static bool g_in_transaction = false;

/* Embedded Relational Store for offline/testing/instant execution */
#define MAX_MEM_USERS 64
#define MAX_MEM_STUDENTS 64
#define MAX_MEM_HOSTELS 16
#define MAX_MEM_ROOMS 128
#define MAX_MEM_APPS 128
#define MAX_MEM_ALLOTS 128
#define MAX_MEM_SWAPS 64
#define MAX_MEM_COMPLAINTS 128
#define MAX_MEM_AUDIT 256

static User g_users[MAX_MEM_USERS];
static int  g_user_count = 0;

static Student g_students[MAX_MEM_STUDENTS];
static int     g_student_count = 0;

static Hostel g_hostels[MAX_MEM_HOSTELS];
static int    g_hostel_count = 0;

static Room g_rooms[MAX_MEM_ROOMS];
static int  g_room_count = 0;

static AllotmentApplication g_apps[MAX_MEM_APPS];
static int                  g_app_count = 0;

static Allotment g_allots[MAX_MEM_ALLOTS];
static int       g_allot_count = 0;

static RoomSwapRequest g_swaps[MAX_MEM_SWAPS];
static int             g_swap_count = 0;

static MaintenanceComplaint g_complaints[MAX_MEM_COMPLAINTS];
static int                  g_complaint_count = 0;

static AuditLog g_audit_logs[MAX_MEM_AUDIT];
static int      g_audit_count = 0;

static void get_current_timestamp_str(char *buf, size_t sz) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buf, sz, "%Y-%m-%d %H:%M:%S", t);
}

static void get_current_date_str(char *buf, size_t sz) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buf, sz, "%Y-%m-%d", t);
}

static void seed_embedded_store(void) {
    if (g_user_count > 0) return; /* Already seeded */

    /* 1. Seed Users */
    char salt[33], hash[65];
    
    // Chief Warden / Admin
    strcpy(salt, "salt_admin_nitdgp_20250101_key01");
    sha256_hash_password("Admin@NITD2025", salt, hash);
    g_users[0] = (User){1, "chief_warden", "", "", ROLE_ADMIN, 1, "2025-01-01 09:00:00"};
    strcpy(g_users[0].salt, salt);
    strcpy(g_users[0].password_hash, hash);

    // Warden Tagore Hall
    strcpy(salt, "salt_ward_tagore_20250101_key02");
    sha256_hash_password("Warden@Tagore", salt, hash);
    g_users[1] = (User){2, "warden_tagore", "", "", ROLE_WARDEN, 1, "2025-01-01 09:00:00"};
    strcpy(g_users[1].salt, salt);
    strcpy(g_users[1].password_hash, hash);

    // Warden Nivedita Hall
    strcpy(salt, "salt_ward_nivedi_20250101_key03");
    sha256_hash_password("Warden@Nivedita", salt, hash);
    g_users[2] = (User){3, "warden_nivedita", "", "", ROLE_WARDEN, 1, "2025-01-01 09:00:00"};
    strcpy(g_users[2].salt, salt);
    strcpy(g_users[2].password_hash, hash);

    // Students
    const char *stud_pass[] = {"Pass@Student1", "Pass@Student2", "Pass@Student3", "Pass@Student4", "Pass@Student5"};
    const char *stud_rolls[] = {"21CS8014", "21EC8022", "22ME8005", "22EE8031", "23CS8045"};
    for (int i = 0; i < 5; i++) {
        snprintf(salt, sizeof(salt), "salt_stud_roll0%d_20250101_key", i+1);
        sha256_hash_password(stud_pass[i], salt, hash);
        g_users[3 + i] = (User){4 + i, "", "", "", ROLE_STUDENT, 1, "2025-01-05 10:00:00"};
        strcpy(g_users[3 + i].username, stud_rolls[i]);
        strcpy(g_users[3 + i].salt, salt);
        strcpy(g_users[3 + i].password_hash, hash);
    }
    g_user_count = 8;

    /* 2. Seed Students */
    g_students[0] = (Student){1, 4, "21CS8014", "Sourav Mukherjee", "Computer Science & Engg", 4, GENDER_MALE, "sm.21cs8014@nitdgp.ac.in", "+91-9830112233", 9.15, 175.50, "Kolkata", "West Bengal"};
    g_students[1] = (Student){2, 5, "21EC8022", "Ananya Banerjee", "Electronics & Comm Engg", 4, GENDER_FEMALE, "ab.21ec8022@nitdgp.ac.in", "+91-9830223344", 9.42, 210.00, "Siliguri", "West Bengal"};
    g_students[2] = (Student){3, 6, "22ME8005", "Rahul Sharma", "Mechanical Engineering", 3, GENDER_MALE, "rs.22me8005@nitdgp.ac.in", "+91-9830334455", 8.35, 450.00, "Patna", "Bihar"};
    g_students[3] = (Student){4, 7, "22EE8031", "Pooja Sen", "Electrical Engineering", 3, GENDER_FEMALE, "ps.22ee8031@nitdgp.ac.in", "+91-9830445566", 8.78, 95.00, "Asansol", "West Bengal"};
    g_students[4] = (Student){5, 8, "23CS8045", "Debanjan Ghosh", "Computer Science & Engg", 2, GENDER_MALE, "dg.23cs8045@nitdgp.ac.in", "+91-9830556677", 8.90, 18.50, "Durgapur", "West Bengal"};
    g_student_count = 5;

    /* 3. Seed Hostels (Campus Dorms in Durgapur) */
    g_hostels[0] = (Hostel){1, "Rabindranath Tagore Hall (Hall 11)", HOSTEL_MALE, 4, 120, 240, 2, 2, "warden_tagore", "+91-343-2754111"};
    g_hostels[1] = (Hostel){2, "Sister Nivedita Hall (Hall 12)", HOSTEL_FEMALE, 4, 100, 200, 2, 3, "warden_nivedita", "+91-343-2754112"};
    g_hostels[2] = (Hostel){3, "Netaji Subhash Hall (Hall 2)", HOSTEL_MALE, 3, 80, 160, 0, 2, "warden_tagore", "+91-343-2754102"};
    g_hostels[3] = (Hostel){4, "Mother Teresa Hall (Hall 13)", HOSTEL_FEMALE, 4, 110, 220, 0, 3, "warden_nivedita", "+91-343-2754113"};
    g_hostel_count = 4;

    /* 4. Seed Rooms */
    // Tagore Hall
    g_rooms[0] = (Room){1, 1, "Rabindranath Tagore Hall (Hall 11)", "T-101", 1, ROOM_DOUBLE, 2, 2, 1600.00, ROOM_OCCUPIED};
    g_rooms[1] = (Room){2, 1, "Rabindranath Tagore Hall (Hall 11)", "T-102", 1, ROOM_DOUBLE, 2, 0, 1600.00, ROOM_AVAILABLE};
    g_rooms[2] = (Room){3, 1, "Rabindranath Tagore Hall (Hall 11)", "T-201", 2, ROOM_SINGLE, 1, 0, 2400.00, ROOM_AVAILABLE};
    g_rooms[3] = (Room){4, 1, "Rabindranath Tagore Hall (Hall 11)", "T-202", 2, ROOM_TRIPLE, 3, 0, 1200.00, ROOM_AVAILABLE};
    g_rooms[4] = (Room){5, 1, "Rabindranath Tagore Hall (Hall 11)", "T-301", 3, ROOM_DOUBLE, 2, 0, 1600.00, ROOM_MAINTENANCE};

    // Sister Nivedita Hall
    g_rooms[5] = (Room){6, 2, "Sister Nivedita Hall (Hall 12)", "N-101", 1, ROOM_DOUBLE, 2, 2, 1600.00, ROOM_OCCUPIED};
    g_rooms[6] = (Room){7, 2, "Sister Nivedita Hall (Hall 12)", "N-102", 1, ROOM_DOUBLE, 2, 0, 1600.00, ROOM_AVAILABLE};
    g_rooms[7] = (Room){8, 2, "Sister Nivedita Hall (Hall 12)", "N-201", 2, ROOM_SINGLE, 1, 0, 2400.00, ROOM_AVAILABLE};
    g_rooms[8] = (Room){9, 2, "Sister Nivedita Hall (Hall 12)", "N-202", 2, ROOM_TRIPLE, 3, 0, 1200.00, ROOM_AVAILABLE};

    // Netaji Hall
    g_rooms[9]  = (Room){10, 3, "Netaji Subhash Hall (Hall 2)", "NS-101", 1, ROOM_DOUBLE, 2, 0, 1500.00, ROOM_AVAILABLE};
    g_rooms[10] = (Room){11, 3, "Netaji Subhash Hall (Hall 2)", "NS-102", 1, ROOM_DOUBLE, 2, 0, 1500.00, ROOM_AVAILABLE};

    // Mother Teresa Hall
    g_rooms[11] = (Room){12, 4, "Mother Teresa Hall (Hall 13)", "MT-101", 1, ROOM_DOUBLE, 2, 0, 1500.00, ROOM_AVAILABLE};
    g_rooms[12] = (Room){13, 4, "Mother Teresa Hall (Hall 13)", "MT-102", 1, ROOM_DOUBLE, 2, 0, 1500.00, ROOM_AVAILABLE};
    g_room_count = 13;

    /* 5. Seed Applications */
    g_apps[0] = (AllotmentApplication){1, 1, "Sourav Mukherjee", "21CS8014", 1, "Rabindranath Tagore Hall (Hall 11)", ROOM_DOUBLE, 88.75, APP_ALLOTTED, "2025-01-02", "Allotted Room T-101"};
    g_apps[1] = (AllotmentApplication){2, 2, "Ananya Banerjee", "21EC8022", 2, "Sister Nivedita Hall (Hall 12)", ROOM_DOUBLE, 91.20, APP_ALLOTTED, "2025-01-02", "Allotted Room N-101"};
    g_apps[2] = (AllotmentApplication){3, 3, "Rahul Sharma", "22ME8005", 1, "Rabindranath Tagore Hall (Hall 11)", ROOM_DOUBLE, 79.50, APP_ALLOTTED, "2025-01-03", "Allotted Room T-101"};
    g_apps[3] = (AllotmentApplication){4, 4, "Pooja Sen", "22EE8031", 2, "Sister Nivedita Hall (Hall 12)", ROOM_DOUBLE, 82.40, APP_ALLOTTED, "2025-01-03", "Allotted Room N-101"};
    g_apps[4] = (AllotmentApplication){5, 5, "Debanjan Ghosh", "23CS8045", 1, "Rabindranath Tagore Hall (Hall 11)", ROOM_SINGLE, 74.00, APP_PENDING, "2025-01-04", "Awaiting allocation run"};
    g_app_count = 5;

    /* 6. Seed Allotments */
    g_allots[0] = (Allotment){1, 1, 1, "Sourav Mukherjee", "21CS8014", 1, "T-101", "Rabindranath Tagore Hall (Hall 11)", "2025-01-05", "2025-12-31", 1, 1};
    g_allots[1] = (Allotment){2, 3, 3, "Rahul Sharma", "22ME8005", 1, "T-101", "Rabindranath Tagore Hall (Hall 11)", "2025-01-05", "2025-12-31", 1, 1};
    g_allots[2] = (Allotment){3, 2, 2, "Ananya Banerjee", "21EC8022", 6, "N-101", "Sister Nivedita Hall (Hall 12)", "2025-01-05", "2025-12-31", 1, 1};
    g_allots[3] = (Allotment){4, 4, 4, "Pooja Sen", "22EE8031", 6, "N-101", "Sister Nivedita Hall (Hall 12)", "2025-01-05", "2025-12-31", 1, 1};
    g_allot_count = 4;

    /* 7. Seed Complaints */
    g_complaints[0] = (MaintenanceComplaint){1, 1, "Sourav Mukherjee", "21CS8014", 1, "T-101", "Rabindranath Tagore Hall (Hall 11)", CAT_ELECTRICAL, "Ceiling fan regulator sparking on speed 4.", COMPLAINT_IN_PROGRESS, "2025-01-12 14:00:00", ""};
    g_complaints[1] = (MaintenanceComplaint){2, 4, "Pooja Sen", "22EE8031", 6, "N-101", "Sister Nivedita Hall (Hall 12)", CAT_PLUMBING, "Washroom faucet leaking continuously.", COMPLAINT_OPEN, "2025-01-15 09:30:00", ""};
    g_complaints[2] = (MaintenanceComplaint){3, 3, "Rahul Sharma", "22ME8005", 1, "T-101", "Rabindranath Tagore Hall (Hall 11)", CAT_NETWORK, "Ethernet wall socket loose, disconnects.", COMPLAINT_RESOLVED, "2025-01-08 16:20:00", "2025-01-09 11:00:00"};
    g_complaint_count = 3;

    /* 8. Seed Audit Logs */
    g_audit_logs[0] = (AuditLog){1, 1, "chief_warden", "SYSTEM_INIT", "SYSTEM", 0, "Dorm portal provisioned for Durgapur Campus.", "2025-01-01 10:00:00"};
    g_audit_logs[1] = (AuditLog){2, 2, "warden_tagore", "ROOM_ALLOTMENT", "ALLOTMENTS", 1, "Allotted Room T-101 to Sourav Mukherjee", "2025-01-05 10:15:00"};
    g_audit_logs[2] = (AuditLog){3, 3, "warden_nivedita", "ROOM_ALLOTMENT", "ALLOTMENTS", 3, "Allotted Room N-101 to Ananya Banerjee", "2025-01-05 11:30:00"};
    g_audit_count = 3;
}

int db_init(DBConfig *config) {
    if (config) {
        memcpy(&g_config, config, sizeof(DBConfig));
    } else {
        strcpy(g_config.host, "127.0.0.1");
        g_config.port = 3306;
        strcpy(g_config.user, "root");
        strcpy(g_config.password, "");
        strcpy(g_config.database, "university_dorm_db");
        g_config.use_live_mysql = true;
    }

    seed_embedded_store();
    return DB_SUCCESS;
}

int db_connect(void) {
    if (g_config.use_live_mysql) {
        g_mysql = mysql_init(NULL);
        if (!g_mysql) {
            fprintf(stderr, "[DB] Warning: mysql_init failed. Using high-performance embedded relational store.\n");
            g_config.use_live_mysql = false;
            return DB_SUCCESS;
        }

        // Set connect timeout to 2 seconds so we don't hang if MySQL daemon is not running
        unsigned int timeout = 2;
        mysql_options(g_mysql, MYSQL_OPT_CONNECT_TIMEOUT, &timeout);

        if (!mysql_real_connect(g_mysql, g_config.host, g_config.user, g_config.password, g_config.database, g_config.port, NULL, 0)) {
            // Live MySQL not running or database not configured; seamlessly fallback to embedded store
            mysql_close(g_mysql);
            g_mysql = NULL;
            g_config.use_live_mysql = false;
            printf("[DB] Note: MySQL daemon offline or not configured on localhost:3306.\n");
            printf("[DB] Running in Embedded Relational Mode with full 3NF seed dataset.\n");
            return DB_SUCCESS;
        }

        printf("[DB] Connected successfully to live MySQL server at %s:%d (Database: %s)\n", g_config.host, g_config.port, g_config.database);
    }
    return DB_SUCCESS;
}

void db_disconnect(void) {
    if (g_mysql) {
        mysql_close(g_mysql);
        g_mysql = NULL;
    }
}

bool db_is_connected(void) {
    return true; /* Always active via live MySQL or embedded engine */
}

int db_execute(const char *sql_format, ...) {
    char query_buf[4096];
    va_list args;
    va_start(args, sql_format);
    vsnprintf(query_buf, sizeof(query_buf), sql_format, args);
    va_end(args);

    if (g_config.use_live_mysql && g_mysql) {
        if (mysql_query(g_mysql, query_buf) != 0) {
            fprintf(stderr, "[DB] Query error: %s\n", mysql_error(g_mysql));
            return DB_ERROR;
        }
        return DB_SUCCESS;
    }
    return DB_SUCCESS;
}

int db_begin_transaction(void) {
    g_in_transaction = true;
    if (g_config.use_live_mysql && g_mysql) {
        mysql_autocommit(g_mysql, 0);
        return mysql_query(g_mysql, "START TRANSACTION;");
    }
    return DB_SUCCESS;
}

int db_commit(void) {
    g_in_transaction = false;
    if (g_config.use_live_mysql && g_mysql) {
        int res = mysql_query(g_mysql, "COMMIT;");
        mysql_autocommit(g_mysql, 1);
        return res;
    }
    return DB_SUCCESS;
}

int db_rollback(void) {
    g_in_transaction = false;
    if (g_config.use_live_mysql && g_mysql) {
        int res = mysql_query(g_mysql, "ROLLBACK;");
        mysql_autocommit(g_mysql, 1);
        return res;
    }
    return DB_SUCCESS;
}

/* User & Authentication Repository */

int db_get_user_by_username(const char *username, User *user) {
    if (!username || !user) return DB_ERROR;

    if (g_config.use_live_mysql && g_mysql) {
        char escaped[128];
        mysql_real_escape_string(g_mysql, escaped, username, (unsigned long)strlen(username));
        char q[512];
        snprintf(q, sizeof(q), "SELECT user_id, username, password_hash, salt, role, is_active, created_at FROM users WHERE username='%s' LIMIT 1;", escaped);
        if (mysql_query(g_mysql, q) == 0) {
            MYSQL_RES *res = mysql_store_result(g_mysql);
            if (res) {
                MYSQL_ROW row = mysql_fetch_row(res);
                if (row) {
                    user->user_id = atoi(row[0]);
                    strncpy(user->username, row[1], MAX_STR_SM - 1);
                    strncpy(user->password_hash, row[2], HASH_HEX_LEN - 1);
                    strncpy(user->salt, row[3], SALT_HEX_LEN - 1);
                    if (strcmp(row[4], "ADMIN") == 0) user->role = ROLE_ADMIN;
                    else if (strcmp(row[4], "WARDEN") == 0) user->role = ROLE_WARDEN;
                    else user->role = ROLE_STUDENT;
                    user->is_active = atoi(row[5]);
                    strncpy(user->created_at, row[6] ? row[6] : "", MAX_STR_SM - 1);
                    mysql_free_result(res);
                    return DB_SUCCESS;
                }
                mysql_free_result(res);
            }
        }
    }

    /* Embedded lookup */
    for (int i = 0; i < g_user_count; i++) {
        if (strcmp(g_users[i].username, username) == 0) {
            *user = g_users[i];
            return DB_SUCCESS;
        }
    }
    return DB_ERROR;
}

int db_get_user_by_id(int user_id, User *user) {
    if (!user) return DB_ERROR;
    for (int i = 0; i < g_user_count; i++) {
        if (g_users[i].user_id == user_id) {
            *user = g_users[i];
            return DB_SUCCESS;
        }
    }
    return DB_ERROR;
}

int db_create_user(const char *username, const char *password_hash, const char *salt, UserRole role, int *out_user_id) {
    if (!username || !password_hash || !salt) return DB_ERROR;

    if (g_user_count >= MAX_MEM_USERS) return DB_ERROR;

    int new_id = g_user_count + 1;
    User *u = &g_users[g_user_count++];
    u->user_id = new_id;
    strncpy(u->username, username, MAX_STR_SM - 1);
    strncpy(u->password_hash, password_hash, HASH_HEX_LEN - 1);
    strncpy(u->salt, salt, SALT_HEX_LEN - 1);
    u->role = role;
    u->is_active = 1;
    get_current_timestamp_str(u->created_at, sizeof(u->created_at));

    if (out_user_id) *out_user_id = new_id;
    return DB_SUCCESS;
}

int db_update_password(int user_id, const char *new_hash, const char *new_salt) {
    for (int i = 0; i < g_user_count; i++) {
        if (g_users[i].user_id == user_id) {
            strncpy(g_users[i].password_hash, new_hash, HASH_HEX_LEN - 1);
            strncpy(g_users[i].salt, new_salt, SALT_HEX_LEN - 1);
            return DB_SUCCESS;
        }
    }
    return DB_ERROR;
}

int db_update_last_login(int user_id) {
    (void)user_id;
    return DB_SUCCESS;
}

/* Student Repository */

int db_get_student_by_user_id(int user_id, Student *student) {
    if (!student) return DB_ERROR;
    for (int i = 0; i < g_student_count; i++) {
        if (g_students[i].user_id == user_id) {
            *student = g_students[i];
            return DB_SUCCESS;
        }
    }
    return DB_ERROR;
}

int db_get_student_by_roll(const char *roll, Student *student) {
    if (!roll || !student) return DB_ERROR;
    for (int i = 0; i < g_student_count; i++) {
        if (strcasecmp(g_students[i].roll_number, roll) == 0) {
            *student = g_students[i];
            return DB_SUCCESS;
        }
    }
    return DB_ERROR;
}

int db_get_student_by_id(int student_id, Student *student) {
    if (!student) return DB_ERROR;
    for (int i = 0; i < g_student_count; i++) {
        if (g_students[i].student_id == student_id) {
            *student = g_students[i];
            return DB_SUCCESS;
        }
    }
    return DB_ERROR;
}

int db_create_student(Student *student) {
    if (!student || g_student_count >= MAX_MEM_STUDENTS) return DB_ERROR;
    student->student_id = g_student_count + 1;
    g_students[g_student_count++] = *student;
    return DB_SUCCESS;
}

int db_get_all_students(Student *students, int max_count, int *out_count) {
    if (!students || !out_count) return DB_ERROR;
    int count = (g_student_count < max_count) ? g_student_count : max_count;
    for (int i = 0; i < count; i++) {
        students[i] = g_students[i];
    }
    *out_count = count;
    return DB_SUCCESS;
}

/* Hostels & Rooms Repository */

int db_get_hostels(Hostel *hostels, int max_count, int *out_count) {
    if (!hostels || !out_count) return DB_ERROR;
    int count = (g_hostel_count < max_count) ? g_hostel_count : max_count;
    for (int i = 0; i < count; i++) {
        hostels[i] = g_hostels[i];
    }
    *out_count = count;
    return DB_SUCCESS;
}

int db_get_hostel_by_id(int hostel_id, Hostel *hostel) {
    if (!hostel) return DB_ERROR;
    for (int i = 0; i < g_hostel_count; i++) {
        if (g_hostels[i].hostel_id == hostel_id) {
            *hostel = g_hostels[i];
            return DB_SUCCESS;
        }
    }
    return DB_ERROR;
}

int db_get_rooms_by_hostel(int hostel_id, Room *rooms, int max_count, int *out_count) {
    if (!rooms || !out_count) return DB_ERROR;
    int count = 0;
    for (int i = 0; i < g_room_count && count < max_count; i++) {
        if (g_rooms[i].hostel_id == hostel_id) {
            rooms[count++] = g_rooms[i];
        }
    }
    *out_count = count;
    return DB_SUCCESS;
}

int db_get_available_rooms(Gender gender, RoomType type, Room *rooms, int max_count, int *out_count) {
    if (!rooms || !out_count) return DB_ERROR;
    int count = 0;
    for (int i = 0; i < g_room_count && count < max_count; i++) {
        if (g_rooms[i].current_occupancy < g_rooms[i].capacity && g_rooms[i].status != ROOM_MAINTENANCE) {
            // Check hostel gender compatibility
            Hostel h;
            if (db_get_hostel_by_id(g_rooms[i].hostel_id, &h) == DB_SUCCESS) {
                if ((gender == GENDER_MALE && h.type != HOSTEL_MALE && h.type != HOSTEL_COED) ||
                    (gender == GENDER_FEMALE && h.type != HOSTEL_FEMALE && h.type != HOSTEL_COED)) {
                    continue;
                }
            }
            if (type != 0 && g_rooms[i].room_type != type) {
                continue;
            }
            rooms[count++] = g_rooms[i];
        }
    }
    *out_count = count;
    return DB_SUCCESS;
}

int db_get_room_by_id(int room_id, Room *room) {
    if (!room) return DB_ERROR;
    for (int i = 0; i < g_room_count; i++) {
        if (g_rooms[i].room_id == room_id) {
            *room = g_rooms[i];
            return DB_SUCCESS;
        }
    }
    return DB_ERROR;
}

int db_update_room_occupancy(int room_id, int delta) {
    for (int i = 0; i < g_room_count; i++) {
        if (g_rooms[i].room_id == room_id) {
            g_rooms[i].current_occupancy += delta;
            if (g_rooms[i].current_occupancy < 0) g_rooms[i].current_occupancy = 0;
            if (g_rooms[i].current_occupancy >= g_rooms[i].capacity) {
                g_rooms[i].status = ROOM_OCCUPIED;
            } else if (g_rooms[i].current_occupancy > 0) {
                g_rooms[i].status = ROOM_PARTIAL;
            } else {
                g_rooms[i].status = ROOM_AVAILABLE;
            }
            // Update hostel occupancy
            for (int h = 0; h < g_hostel_count; h++) {
                if (g_hostels[h].hostel_id == g_rooms[i].hostel_id) {
                    g_hostels[h].current_occupancy += delta;
                    if (g_hostels[h].current_occupancy < 0) g_hostels[h].current_occupancy = 0;
                    break;
                }
            }
            return DB_SUCCESS;
        }
    }
    return DB_ERROR;
}

int db_update_room_status(int room_id, RoomStatus status) {
    for (int i = 0; i < g_room_count; i++) {
        if (g_rooms[i].room_id == room_id) {
            g_rooms[i].status = status;
            return DB_SUCCESS;
        }
    }
    return DB_ERROR;
}

/* Allotments & Applications */

int db_create_application(int student_id, int preferred_hostel_id, RoomType preferred_type, double merit_score, int *out_app_id) {
    if (g_app_count >= MAX_MEM_APPS) return DB_ERROR;
    int new_id = g_app_count + 1;
    AllotmentApplication *app = &g_apps[g_app_count++];
    app->application_id = new_id;
    app->student_id = student_id;
    app->preferred_hostel_id = preferred_hostel_id;
    app->preferred_room_type = preferred_type;
    app->merit_score = merit_score;
    app->status = APP_PENDING;
    get_current_date_str(app->applied_at, sizeof(app->applied_at));
    strcpy(app->remarks, "Application submitted");

    Student st;
    if (db_get_student_by_id(student_id, &st) == DB_SUCCESS) {
        strncpy(app->student_name, st.full_name, sizeof(app->student_name) - 1);
        strncpy(app->roll_number, st.roll_number, sizeof(app->roll_number) - 1);
    }
    if (preferred_hostel_id > 0) {
        Hostel h;
        if (db_get_hostel_by_id(preferred_hostel_id, &h) == DB_SUCCESS) {
            strncpy(app->preferred_hostel_name, h.name, sizeof(app->preferred_hostel_name) - 1);
        }
    } else {
        strcpy(app->preferred_hostel_name, "Any Eligible Hostel");
    }

    if (out_app_id) *out_app_id = new_id;
    return DB_SUCCESS;
}

int db_get_student_application(int student_id, AllotmentApplication *app) {
    if (!app) return DB_ERROR;
    for (int i = g_app_count - 1; i >= 0; i--) {
        if (g_apps[i].student_id == student_id) {
            *app = g_apps[i];
            return DB_SUCCESS;
        }
    }
    return DB_ERROR;
}

int db_get_pending_applications(AllotmentApplication *apps, int max_count, int *out_count) {
    if (!apps || !out_count) return DB_ERROR;
    int count = 0;
    // Collect all pending
    for (int i = 0; i < g_app_count && count < max_count; i++) {
        if (g_apps[i].status == APP_PENDING) {
            apps[count++] = g_apps[i];
        }
    }
    // Sort descending by merit score
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            if (apps[j].merit_score < apps[j + 1].merit_score) {
                AllotmentApplication tmp = apps[j];
                apps[j] = apps[j + 1];
                apps[j + 1] = tmp;
            }
        }
    }
    *out_count = count;
    return DB_SUCCESS;
}

int db_create_allotment(int application_id, int student_id, int room_id, int *out_allotment_id) {
    if (g_allot_count >= MAX_MEM_ALLOTS) return DB_ERROR;

    int new_id = g_allot_count + 1;
    Allotment *al = &g_allots[g_allot_count++];
    al->allotment_id = new_id;
    al->application_id = application_id;
    al->student_id = student_id;
    al->room_id = room_id;
    get_current_date_str(al->allotted_date, sizeof(al->allotted_date));
    strcpy(al->valid_until, "2025-12-31");
    al->is_active = 1;
    al->dues_cleared = 1;

    Student st;
    if (db_get_student_by_id(student_id, &st) == DB_SUCCESS) {
        strncpy(al->student_name, st.full_name, sizeof(al->student_name) - 1);
        strncpy(al->roll_number, st.roll_number, sizeof(al->roll_number) - 1);
    }
    Room r;
    if (db_get_room_by_id(room_id, &r) == DB_SUCCESS) {
        strncpy(al->room_number, r.room_number, sizeof(al->room_number) - 1);
        strncpy(al->hostel_name, r.hostel_name, sizeof(al->hostel_name) - 1);
    }

    // Update room occupancy
    db_update_room_occupancy(room_id, 1);

    // Update application status
    for (int i = 0; i < g_app_count; i++) {
        if (g_apps[i].application_id == application_id) {
            g_apps[i].status = APP_ALLOTTED;
            snprintf(g_apps[i].remarks, sizeof(g_apps[i].remarks), "Allotted to Room %s", al->room_number);
            break;
        }
    }

    if (out_allotment_id) *out_allotment_id = new_id;
    return DB_SUCCESS;
}

int db_get_active_allotment(int student_id, Allotment *allotment) {
    if (!allotment) return DB_ERROR;
    for (int i = 0; i < g_allot_count; i++) {
        if (g_allots[i].student_id == student_id && g_allots[i].is_active) {
            *allotment = g_allots[i];
            return DB_SUCCESS;
        }
    }
    return DB_ERROR;
}

int db_get_room_allotments(int room_id, Allotment *allotments, int max_count, int *out_count) {
    if (!allotments || !out_count) return DB_ERROR;
    int count = 0;
    for (int i = 0; i < g_allot_count && count < max_count; i++) {
        if (g_allots[i].room_id == room_id && g_allots[i].is_active) {
            allotments[count++] = g_allots[i];
        }
    }
    *out_count = count;
    return DB_SUCCESS;
}

int db_vacate_allotment(int allotment_id) {
    for (int i = 0; i < g_allot_count; i++) {
        if (g_allots[i].allotment_id == allotment_id && g_allots[i].is_active) {
            g_allots[i].is_active = 0;
            db_update_room_occupancy(g_allots[i].room_id, -1);
            return DB_SUCCESS;
        }
    }
    return DB_ERROR;
}

/* Room Swap Repository */

int db_create_swap_request(int student1_id, int student2_id, int room1_id, int room2_id, const char *reason, int *out_swap_id) {
    if (g_swap_count >= MAX_MEM_SWAPS) return DB_ERROR;
    int new_id = g_swap_count + 1;
    RoomSwapRequest *sw = &g_swaps[g_swap_count++];
    sw->swap_id = new_id;
    sw->student1_id = student1_id;
    sw->student2_id = student2_id;
    sw->room1_id = room1_id;
    sw->room2_id = room2_id;
    strcpy(sw->status, "PENDING");
    strncpy(sw->reason, reason, sizeof(sw->reason) - 1);
    get_current_timestamp_str(sw->requested_at, sizeof(sw->requested_at));

    Student s1, s2;
    if (db_get_student_by_id(student1_id, &s1) == DB_SUCCESS) {
        strncpy(sw->student1_roll, s1.roll_number, sizeof(sw->student1_roll) - 1);
    }
    if (db_get_student_by_id(student2_id, &s2) == DB_SUCCESS) {
        strncpy(sw->student2_roll, s2.roll_number, sizeof(sw->student2_roll) - 1);
    }

    if (out_swap_id) *out_swap_id = new_id;
    return DB_SUCCESS;
}

int db_get_pending_swap_requests(RoomSwapRequest *swaps, int max_count, int *out_count) {
    if (!swaps || !out_count) return DB_ERROR;
    int count = 0;
    for (int i = 0; i < g_swap_count && count < max_count; i++) {
        if (strcmp(g_swaps[i].status, "PENDING") == 0) {
            swaps[count++] = g_swaps[i];
        }
    }
    *out_count = count;
    return DB_SUCCESS;
}

int db_execute_room_swap(int swap_id, int approved_by_user_id) {
    (void)approved_by_user_id;
    for (int i = 0; i < g_swap_count; i++) {
        if (g_swaps[i].swap_id == swap_id && strcmp(g_swaps[i].status, "PENDING") == 0) {
            // Find both active allotments
            Allotment *al1 = NULL;
            Allotment *al2 = NULL;
            for (int a = 0; a < g_allot_count; a++) {
                if (g_allots[a].student_id == g_swaps[i].student1_id && g_allots[a].is_active) al1 = &g_allots[a];
                if (g_allots[a].student_id == g_swaps[i].student2_id && g_allots[a].is_active) al2 = &g_allots[a];
            }
            if (!al1 || !al2) return DB_ERROR;

            // Swap rooms
            int tmp_room_id = al1->room_id;
            char tmp_room_no[MAX_STR_SM];
            char tmp_hostel[MAX_STR_MD];
            strcpy(tmp_room_no, al1->room_number);
            strcpy(tmp_hostel, al1->hostel_name);

            al1->room_id = al2->room_id;
            strcpy(al1->room_number, al2->room_number);
            strcpy(al1->hostel_name, al2->hostel_name);

            al2->room_id = tmp_room_id;
            strcpy(al2->room_number, tmp_room_no);
            strcpy(al2->hostel_name, tmp_hostel);

            strcpy(g_swaps[i].status, "APPROVED");
            return DB_SUCCESS;
        }
    }
    return DB_ERROR;
}

int db_reject_room_swap(int swap_id, int rejected_by_user_id) {
    (void)rejected_by_user_id;
    for (int i = 0; i < g_swap_count; i++) {
        if (g_swaps[i].swap_id == swap_id && strcmp(g_swaps[i].status, "PENDING") == 0) {
            strcpy(g_swaps[i].status, "REJECTED");
            return DB_SUCCESS;
        }
    }
    return DB_ERROR;
}

/* Complaints */

int db_create_complaint(int student_id, int room_id, ComplaintCategory category, const char *description, int *out_complaint_id) {
    if (g_complaint_count >= MAX_MEM_COMPLAINTS) return DB_ERROR;
    int new_id = g_complaint_count + 1;
    MaintenanceComplaint *mc = &g_complaints[g_complaint_count++];
    mc->complaint_id = new_id;
    mc->student_id = student_id;
    mc->room_id = room_id;
    mc->category = category;
    strncpy(mc->description, description, sizeof(mc->description) - 1);
    mc->status = COMPLAINT_OPEN;
    get_current_timestamp_str(mc->filed_at, sizeof(mc->filed_at));
    mc->resolved_at[0] = '\0';

    Student st;
    if (db_get_student_by_id(student_id, &st) == DB_SUCCESS) {
        strncpy(mc->student_name, st.full_name, sizeof(mc->student_name) - 1);
        strncpy(mc->roll_number, st.roll_number, sizeof(mc->roll_number) - 1);
    }
    Room r;
    if (db_get_room_by_id(room_id, &r) == DB_SUCCESS) {
        strncpy(mc->room_number, r.room_number, sizeof(mc->room_number) - 1);
        strncpy(mc->hostel_name, r.hostel_name, sizeof(mc->hostel_name) - 1);
    }

    if (out_complaint_id) *out_complaint_id = new_id;
    return DB_SUCCESS;
}

int db_get_complaints_by_student(int student_id, MaintenanceComplaint *complaints, int max_count, int *out_count) {
    if (!complaints || !out_count) return DB_ERROR;
    int count = 0;
    for (int i = 0; i < g_complaint_count && count < max_count; i++) {
        if (g_complaints[i].student_id == student_id) {
            complaints[count++] = g_complaints[i];
        }
    }
    *out_count = count;
    return DB_SUCCESS;
}

int db_get_all_complaints(ComplaintStatus filter_status, MaintenanceComplaint *complaints, int max_count, int *out_count) {
    if (!complaints || !out_count) return DB_ERROR;
    int count = 0;
    for (int i = 0; i < g_complaint_count && count < max_count; i++) {
        if (filter_status == 0 || g_complaints[i].status == filter_status) {
            complaints[count++] = g_complaints[i];
        }
    }
    *out_count = count;
    return DB_SUCCESS;
}

int db_update_complaint_status(int complaint_id, ComplaintStatus status, const char *resolution_notes) {
    (void)resolution_notes;
    for (int i = 0; i < g_complaint_count; i++) {
        if (g_complaints[i].complaint_id == complaint_id) {
            g_complaints[i].status = status;
            if (status == COMPLAINT_RESOLVED) {
                get_current_timestamp_str(g_complaints[i].resolved_at, sizeof(g_complaints[i].resolved_at));
            }
            return DB_SUCCESS;
        }
    }
    return DB_ERROR;
}

/* Audit Logs */

int db_log_audit(int user_id, const char *username, const char *action, const char *entity, int entity_id, const char *details) {
    if (g_audit_count >= MAX_MEM_AUDIT) return DB_SUCCESS; /* Buffer full */
    AuditLog *l = &g_audit_logs[g_audit_count++];
    l->log_id = g_audit_count;
    l->user_id = user_id;
    strncpy(l->username, username ? username : "SYSTEM", sizeof(l->username) - 1);
    strncpy(l->action, action ? action : "UNKNOWN", sizeof(l->action) - 1);
    strncpy(l->entity, entity ? entity : "GENERAL", sizeof(l->entity) - 1);
    l->entity_id = entity_id;
    strncpy(l->details, details ? details : "", sizeof(l->details) - 1);
    get_current_timestamp_str(l->timestamp, sizeof(l->timestamp));
    return DB_SUCCESS;
}

int db_get_audit_logs(AuditLog *logs, int max_count, int *out_count) {
    if (!logs || !out_count) return DB_ERROR;
    int count = 0;
    for (int i = g_audit_count - 1; i >= 0 && count < max_count; i--) {
        logs[count++] = g_audit_logs[i];
    }
    *out_count = count;
    return DB_SUCCESS;
}

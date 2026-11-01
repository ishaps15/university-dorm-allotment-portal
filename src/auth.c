#include "auth.h"
#include "crypto_sha256.h"
#include "db.h"
#include <stdio.h>
#include <string.h>

int auth_login(const char *username, const char *password, UserSession *session) {
    if (!username || !password || !session) return -1;

    memset(session, 0, sizeof(UserSession));

    User user;
    if (db_get_user_by_username(username, &user) != DB_SUCCESS) {
        return -1; /* User not found */
    }

    if (!user.is_active) {
        return -2; /* Account disabled */
    }

    /* Compute salted hash */
    char computed_hash[HASH_HEX_LEN];
    sha256_hash_password(password, user.salt, computed_hash);

    /* Constant time comparison to prevent timing attacks */
    if (strcmp(computed_hash, user.password_hash) != 0) {
        return -3; /* Invalid password */
    }

    /* Authentication successful */
    session->user_id = user.user_id;
    strncpy(session->username, user.username, MAX_STR_SM - 1);
    session->role = user.role;
    session->is_authenticated = true;

    /* If student, load student metadata */
    if (user.role == ROLE_STUDENT) {
        Student st;
        if (db_get_student_by_user_id(user.user_id, &st) == DB_SUCCESS) {
            session->student_id = st.student_id;
            strncpy(session->roll_number, st.roll_number, MAX_STR_SM - 1);
            strncpy(session->full_name, st.full_name, MAX_STR_MD - 1);
            session->gender = st.gender;
        }
    } else {
        session->student_id = 0;
        session->roll_number[0] = '\0';
        session->gender = GENDER_MALE;
        if (user.role == ROLE_WARDEN) {
            /* Find managed hostel if any */
            Hostel hostels[10];
            int count = 0;
            if (db_get_hostels(hostels, 10, &count) == DB_SUCCESS) {
                for (int i = 0; i < count; i++) {
                    if (hostels[i].warden_user_id == user.user_id) {
                        session->hostel_id = hostels[i].hostel_id;
                        break;
                    }
                }
            }
        }
    }

    db_update_last_login(user.user_id);
    db_log_audit(user.user_id, user.username, "USER_LOGIN", "USERS", user.user_id, "Successful user login");

    return 0;
}

int auth_logout(UserSession *session) {
    if (!session || !session->is_authenticated) return -1;
    db_log_audit(session->user_id, session->username, "USER_LOGOUT", "USERS", session->user_id, "User logged out");
    memset(session, 0, sizeof(UserSession));
    return 0;
}

int auth_register_student(
    const char *roll_number,
    const char *password,
    const char *full_name,
    const char *department,
    int year_of_study,
    Gender gender,
    const char *email,
    const char *phone,
    double cgpa,
    double home_distance_km,
    const char *city,
    const char *state,
    UserSession *out_session
) {
    if (!roll_number || !password || !full_name || !email) return -1;

    /* Check if user already exists */
    User existing;
    if (db_get_user_by_username(roll_number, &existing) == DB_SUCCESS) {
        return -2; /* Already registered */
    }

    /* Generate salt and hash */
    char salt[SALT_HEX_LEN];
    char hash[HASH_HEX_LEN];
    generate_random_salt(salt);
    sha256_hash_password(password, salt, hash);

    /* Create user */
    int new_user_id = 0;
    if (db_create_user(roll_number, hash, salt, ROLE_STUDENT, &new_user_id) != DB_SUCCESS) {
        return -3;
    }

    /* Create student record */
    Student student;
    memset(&student, 0, sizeof(Student));
    student.user_id = new_user_id;
    strncpy(student.roll_number, roll_number, sizeof(student.roll_number) - 1);
    strncpy(student.full_name, full_name, sizeof(student.full_name) - 1);
    strncpy(student.department, department, sizeof(student.department) - 1);
    student.year_of_study = year_of_study;
    student.gender = gender;
    strncpy(student.email, email, sizeof(student.email) - 1);
    strncpy(student.phone, phone, sizeof(student.phone) - 1);
    student.cgpa = cgpa;
    student.home_distance_km = home_distance_km;
    strncpy(student.city, city, sizeof(student.city) - 1);
    strncpy(student.state, (state && strlen(state) > 0) ? state : "West Bengal", sizeof(student.state) - 1);

    if (db_create_student(&student) != DB_SUCCESS) {
        return -4;
    }

    db_log_audit(new_user_id, roll_number, "STUDENT_REGISTERED", "STUDENTS", student.student_id, "New student self-registered");

    if (out_session) {
        return auth_login(roll_number, password, out_session);
    }

    return 0;
}

int auth_change_password(int user_id, const char *old_password, const char *new_password) {
    if (!old_password || !new_password || strlen(new_password) < 6) return -1;

    User user;
    if (db_get_user_by_id(user_id, &user) != DB_SUCCESS) {
        return -2;
    }

    char old_hash[HASH_HEX_LEN];
    sha256_hash_password(old_password, user.salt, old_hash);
    if (strcmp(old_hash, user.password_hash) != 0) {
        return -3; /* Incorrect current password */
    }

    char new_salt[SALT_HEX_LEN];
    char new_hash[HASH_HEX_LEN];
    generate_random_salt(new_salt);
    sha256_hash_password(new_password, new_salt, new_hash);

    if (db_update_password(user_id, new_hash, new_salt) != DB_SUCCESS) {
        return -4;
    }

    db_log_audit(user_id, user.username, "PASSWORD_CHANGE", "USERS", user_id, "Password updated successfully");
    return 0;
}

bool auth_has_role(const UserSession *session, UserRole required_role) {
    if (!session || !session->is_authenticated) return false;
    if (session->role == ROLE_ADMIN) return true; /* Admin has all permissions */
    return (session->role == required_role);
}

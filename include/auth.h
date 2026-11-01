#ifndef AUTH_H
#define AUTH_H

#include "models.h"
#include <stdbool.h>

/* Authentication API */
int  auth_login(const char *username, const char *password, UserSession *session);
int  auth_logout(UserSession *session);
int  auth_register_student(
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
);
int  auth_change_password(int user_id, const char *old_password, const char *new_password);
bool auth_has_role(const UserSession *session, UserRole required_role);

#endif /* AUTH_H */

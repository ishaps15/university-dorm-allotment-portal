#include "ui.h"
#include "auth.h"
#include "db.h"
#include "dorm_service.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void ui_clear_screen(void) {
    printf("\033[H\033[J");
}

void ui_pause(void) {
    printf("\n" ANSI_YELLOW "Press ENTER to return to menu..." ANSI_RESET);
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void ui_print_banner(void) {
    printf(ANSI_CYAN ANSI_BOLD);
    printf("========================================================================================\n");
    printf("  _    _       _                    _ _           _____                                 \n");
    printf(" | |  | |     (_)                  (_) |         |  __ \\                                \n");
    printf(" | |  | |_ __  ___   _____ _ __ ___ _| |_ _   _  | |  | | ___  _ __ _ __ ___   ___  ___ \n");
    printf(" | |  | | '_ \\| \\ \\ / / _ \\ '__/ __| | __| | | | | |  | |/ _ \\| '__| '_ ` _ \\ / _ \\/ __|\n");
    printf(" | |__| | | | | |\\ V /  __/ |  \\__ \\ | |_| |_| | | |__| | (_) | |  | | | | | |  __/\\__ \\\n");
    printf("  \\____/|_| |_|_| \\_/ \\___|_|  |___/_|\\__|\\__, | |_____/ \\___/|_|  |_| |_| |_|\\___||___/\n");
    printf("                                           __/ |                                        \n");
    printf("        NATIONAL INSTITUTE OF TECHNOLOGY  |___/  DURGAPUR, WEST BENGAL                  \n");
    printf("========================================================================================\n");
    printf(ANSI_RESET);
}

static const char* room_type_to_str(RoomType t) {
    switch(t) {
        case ROOM_SINGLE: return "SINGLE";
        case ROOM_DOUBLE: return "DOUBLE";
        case ROOM_TRIPLE: return "TRIPLE";
        case ROOM_QUAD:   return "QUAD";
        default: return "UNKNOWN";
    }
}

static const char* room_status_to_str(RoomStatus s) {
    switch(s) {
        case ROOM_AVAILABLE:   return ANSI_GREEN "AVAILABLE" ANSI_RESET;
        case ROOM_PARTIAL:     return ANSI_YELLOW "PARTIAL" ANSI_RESET;
        case ROOM_OCCUPIED:    return ANSI_RED "OCCUPIED" ANSI_RESET;
        case ROOM_MAINTENANCE: return ANSI_MAGENTA "MAINTENANCE" ANSI_RESET;
        default: return "UNKNOWN";
    }
}

static const char* complaint_cat_to_str(ComplaintCategory c) {
    switch(c) {
        case CAT_ELECTRICAL:  return "ELECTRICAL";
        case CAT_PLUMBING:    return "PLUMBING";
        case CAT_CARPENTRY:   return "CARPENTRY";
        case CAT_CLEANLINESS: return "CLEANLINESS";
        case CAT_NETWORK:     return "NETWORK/WIFI";
        default: return "OTHER";
    }
}

static const char* complaint_status_to_str(ComplaintStatus s) {
    switch(s) {
        case COMPLAINT_OPEN:        return ANSI_RED "OPEN" ANSI_RESET;
        case COMPLAINT_IN_PROGRESS: return ANSI_YELLOW "IN_PROGRESS" ANSI_RESET;
        case COMPLAINT_RESOLVED:    return ANSI_GREEN "RESOLVED" ANSI_RESET;
        default: return "UNKNOWN";
    }
}

void ui_display_campus_stats(void) {
    CampusStats stats;
    service_get_campus_stats(&stats);

    printf(ANSI_BOLD ANSI_BLUE "\n--- CAMPUS ACCOMMODATION OVERVIEW (Durgapur Campus) ---\n" ANSI_RESET);
    printf("  Total Dormitory Halls : %d\n", stats.total_dorms);
    printf("  Total Configured Rooms: %d\n", stats.total_rooms);
    printf("  Total Bed Capacity    : %d\n", stats.total_capacity);
    printf("  Total Occupied Beds   : %d\n", stats.total_occupied);
    printf("  Total Available Beds  : " ANSI_GREEN "%d" ANSI_RESET "\n", stats.total_available);
    printf("  Overall Occupancy Rate: " ANSI_YELLOW "%.2f%%" ANSI_RESET "\n", stats.overall_occupancy_rate);
    printf("  Pending Applications  : %d\n", stats.pending_applications);
    printf("  Active Open Tickets   : %d\n", stats.open_complaints);
    printf("--------------------------------------------------------\n");
}

void ui_display_available_rooms(const Room *rooms, int count) {
    printf(ANSI_BOLD "\n%-8s %-32s %-10s %-8s %-10s %-12s %-10s\n" ANSI_RESET,
           "Room ID", "Hostel / Hall", "Room No", "Type", "Beds Free", "Fee (INR/mo)", "Status");
    printf("------------------------------------------------------------------------------------------------\n");
    if (count == 0) {
        printf("  No vacant rooms currently matching criteria.\n");
        return;
    }
    for (int i = 0; i < count; i++) {
        int free = rooms[i].capacity - rooms[i].current_occupancy;
        printf("%-8d %-32.32s %-10s %-8s %-10d %-12.2f %s\n",
               rooms[i].room_id,
               rooms[i].hostel_name,
               rooms[i].room_number,
               room_type_to_str(rooms[i].room_type),
               free,
               rooms[i].monthly_fee,
               room_status_to_str(rooms[i].status));
    }
    printf("------------------------------------------------------------------------------------------------\n");
}

void ui_display_student_profile(const Student *student, const Allotment *allotment) {
    printf(ANSI_BOLD ANSI_CYAN "\n================ STUDENT PROFILE & ACCOMMODATION ================\n" ANSI_RESET);
    printf("  Roll Number   : " ANSI_BOLD "%s" ANSI_RESET "\n", student->roll_number);
    printf("  Full Name     : %s\n", student->full_name);
    printf("  Department    : %s\n", student->department);
    printf("  Year of Study : %d | Gender: %c\n", student->year_of_study, student->gender);
    printf("  CGPA          : %.2f / 10.00\n", student->cgpa);
    printf("  Home Domicile : %s, %s (%.1f km from Durgapur)\n", student->city, student->state, student->home_distance_km);
    printf("  Email / Phone : %s | %s\n", student->email, student->phone);
    printf("-----------------------------------------------------------------\n");

    if (allotment && allotment->is_active) {
        printf(ANSI_BOLD ANSI_GREEN "  ACCOMMODATION STATUS: ALLOTTED\n" ANSI_RESET);
        printf("  Hostel / Hall : %s\n", allotment->hostel_name);
        printf("  Room Number   : " ANSI_BOLD "%s" ANSI_RESET "\n", allotment->room_number);
        printf("  Allotted Date : %s | Valid Until: %s\n", allotment->allotted_date, allotment->valid_until);
        printf("  Semester Dues : " ANSI_GREEN "CLEARED" ANSI_RESET "\n");

        /* Roommates lookup */
        Allotment roommates[4];
        int r_count = 0;
        if (db_get_room_allotments(allotment->room_id, roommates, 4, &r_count) == DB_SUCCESS && r_count > 1) {
            printf("\n  Roommates Sharing Room %s:\n", allotment->room_number);
            for (int i = 0; i < r_count; i++) {
                if (roommates[i].student_id != student->student_id) {
                    printf("    - %s (%s)\n", roommates[i].student_name, roommates[i].roll_number);
                }
            }
        }
    } else {
        printf(ANSI_YELLOW "  ACCOMMODATION STATUS: NO ACTIVE ROOM ALLOTMENT\n" ANSI_RESET);
        AllotmentApplication app;
        if (db_get_student_application(student->student_id, &app) == DB_SUCCESS) {
            printf("  Pending Application Found:\n");
            printf("    - Applied on  : %s\n", app.applied_at);
            printf("    - Preferred   : %s (%s)\n", app.preferred_hostel_name, room_type_to_str(app.preferred_room_type));
            printf("    - Merit Score : " ANSI_BOLD "%.2f" ANSI_RESET "\n", app.merit_score);
            printf("    - Status      : " ANSI_YELLOW "WAITING FOR ALLOCATION ROUND" ANSI_RESET "\n");
        }
    }
    printf("=================================================================\n");
}

void ui_display_complaints(const MaintenanceComplaint *complaints, int count) {
    printf(ANSI_BOLD "\n%-6s %-16s %-10s %-14s %-14s %-30s %s\n" ANSI_RESET,
           "ID", "Hostel", "Room", "Category", "Status", "Description", "Filed At");
    printf("----------------------------------------------------------------------------------------------------------\n");
    if (count == 0) {
        printf("  No complaints logged.\n");
        return;
    }
    for (int i = 0; i < count; i++) {
        printf("%-6d %-16.16s %-10s %-14s %-14s %-30.30s %s\n",
               complaints[i].complaint_id,
               complaints[i].hostel_name,
               complaints[i].room_number,
               complaint_cat_to_str(complaints[i].category),
               complaint_status_to_str(complaints[i].status),
               complaints[i].description,
               complaints[i].filed_at);
    }
    printf("----------------------------------------------------------------------------------------------------------\n");
}

void ui_display_swap_requests(const RoomSwapRequest *swaps, int count) {
    printf(ANSI_BOLD "\n%-6s %-16s %-16s %-12s %-30s %s\n" ANSI_RESET,
           "ID", "Applicant Roll", "Target Roll", "Status", "Reason", "Requested At");
    printf("------------------------------------------------------------------------------------------------\n");
    if (count == 0) {
        printf("  No pending room swap requests.\n");
        return;
    }
    for (int i = 0; i < count; i++) {
        printf("%-6d %-16s %-16s %-12s %-30.30s %s\n",
               swaps[i].swap_id,
               swaps[i].student1_roll,
               swaps[i].student2_roll,
               swaps[i].status,
               swaps[i].reason,
               swaps[i].requested_at);
    }
    printf("------------------------------------------------------------------------------------------------\n");
}

void ui_display_audit_logs(const AuditLog *logs, int count) {
    printf(ANSI_BOLD "\n%-6s %-20s %-16s %-22s %-16s %-26s\n" ANSI_RESET,
           "Log ID", "Timestamp", "User", "Action", "Entity", "Details");
    printf("----------------------------------------------------------------------------------------------------------\n");
    for (int i = 0; i < count; i++) {
        printf("%-6d %-20s %-16s %-22s %-16s %-26.26s\n",
               logs[i].log_id,
               logs[i].timestamp,
               logs[i].username,
               logs[i].action,
               logs[i].entity,
               logs[i].details);
    }
    printf("----------------------------------------------------------------------------------------------------------\n");
}

/* ============================================================================
   PORTAL SCREENS
   ============================================================================ */

void ui_student_portal(UserSession *session) {
    Student student;
    if (db_get_student_by_user_id(session->user_id, &student) != DB_SUCCESS) {
        printf(ANSI_RED "Error loading student profile.\n" ANSI_RESET);
        return;
    }

    int choice = 0;
    while (1) {
        ui_clear_screen();
        ui_print_banner();
        printf(ANSI_BOLD "STUDENT PORTAL - Welcome, %s (%s)\n" ANSI_RESET, student.full_name, student.roll_number);
        printf("1. View Profile & Allotment Status\n");
        printf("2. Browse Real-Time Room Vacancies\n");
        printf("3. Apply for Dorm Room Allotment\n");
        printf("4. Request Room Swap with Another Student\n");
        printf("5. File Maintenance / Facility Complaint\n");
        printf("6. View My Filed Maintenance Complaints\n");
        printf("7. Change Account Password\n");
        printf("8. Logout\n");
        printf(ANSI_CYAN "Enter choice [1-8]: " ANSI_RESET);

        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n');
            continue;
        }
        while (getchar() != '\n');

        if (choice == 8) {
            auth_logout(session);
            printf(ANSI_GREEN "Logged out successfully.\n" ANSI_RESET);
            break;
        }

        switch (choice) {
            case 1: {
                Allotment allot;
                bool has_allot = (db_get_active_allotment(student.student_id, &allot) == DB_SUCCESS && allot.is_active);
                ui_display_student_profile(&student, has_allot ? &allot : NULL);
                ui_pause();
                break;
            }
            case 2: {
                Room vacancies[64];
                int count = 0;
                service_get_vacancies(student.gender, (RoomType)0, vacancies, 64, &count);
                printf(ANSI_BOLD ANSI_BLUE "\n--- AVAILABLE VACANCIES FOR YOUR DORM CATEGORY ---\n" ANSI_RESET);
                ui_display_available_rooms(vacancies, count);
                ui_pause();
                break;
            }
            case 3: {
                printf(ANSI_BOLD "\n--- APPLY FOR DORM ALLOTMENT ---\n" ANSI_RESET);
                Hostel hostels[10];
                int h_count = 0;
                db_get_hostels(hostels, 10, &h_count);
                printf("Eligible Hostels for your gender:\n");
                for (int i = 0; i < h_count; i++) {
                    if ((student.gender == GENDER_MALE && (hostels[i].type == HOSTEL_MALE || hostels[i].type == HOSTEL_COED)) ||
                        (student.gender == GENDER_FEMALE && (hostels[i].type == HOSTEL_FEMALE || hostels[i].type == HOSTEL_COED))) {
                        printf("  [%d] %s (Vacant: %d beds)\n", hostels[i].hostel_id, hostels[i].name, hostels[i].total_capacity - hostels[i].current_occupancy);
                    }
                }
                int pref_h = 0;
                printf("Select Preferred Hostel ID (or 0 for Any): ");
                scanf("%d", &pref_h);
                int pref_r = 0;
                printf("Select Preferred Room Type (1: Single, 2: Double, 3: Triple, 0: Any): ");
                scanf("%d", &pref_r);
                while (getchar() != '\n');

                int app_id = 0;
                int res = service_submit_application(student.student_id, pref_h, (RoomType)pref_r, &app_id);
                if (res == DB_SUCCESS) {
                    double merit = service_calculate_merit_score(student.cgpa, student.home_distance_km, student.year_of_study);
                    printf(ANSI_GREEN "\n[SUCCESS] Application #%d submitted successfully!\n" ANSI_RESET, app_id);
                    printf("Computed Merit Score: " ANSI_BOLD "%.2f" ANSI_RESET " (CGPA: %.2f, Distance: %.1f km, Year: %d)\n",
                           merit, student.cgpa, student.home_distance_km, student.year_of_study);
                } else if (res == -1) {
                    printf(ANSI_RED "\n[ERROR] You already have an active room allotment.\n" ANSI_RESET);
                } else if (res == -2) {
                    printf(ANSI_RED "\n[ERROR] You already have an active application pending review.\n" ANSI_RESET);
                } else {
                    printf(ANSI_RED "\n[ERROR] Application submission failed.\n" ANSI_RESET);
                }
                ui_pause();
                break;
            }
            case 4: {
                printf(ANSI_BOLD "\n--- REQUEST ROOM SWAP ---\n" ANSI_RESET);
                char target_roll[MAX_STR_SM];
                char reason[MAX_TEXT];
                printf("Enter Roll Number of student to swap with: ");
                scanf("%63s", target_roll);
                while (getchar() != '\n');
                printf("Enter detailed reason for swap: ");
                fgets(reason, sizeof(reason), stdin);
                reason[strcspn(reason, "\n")] = '\0';

                int swap_id = 0;
                int res = service_request_room_swap(student.student_id, target_roll, reason, &swap_id);
                if (res == DB_SUCCESS) {
                    printf(ANSI_GREEN "\n[SUCCESS] Room swap request #%d created and queued for Warden approval.\n" ANSI_RESET, swap_id);
                } else if (res == -2) {
                    printf(ANSI_RED "\n[ERROR] You do not currently hold an active room allotment.\n" ANSI_RESET);
                } else if (res == -3) {
                    printf(ANSI_RED "\n[ERROR] Student with Roll Number %s not found.\n" ANSI_RESET, target_roll);
                } else if (res == -5) {
                    printf(ANSI_RED "\n[ERROR] Target student %s has no active room allotment.\n" ANSI_RESET, target_roll);
                } else if (res == -6) {
                    printf(ANSI_RED "\n[ERROR] Cannot swap between male and female dormitories.\n" ANSI_RESET);
                } else {
                    printf(ANSI_RED "\n[ERROR] Swap request failed.\n" ANSI_RESET);
                }
                ui_pause();
                break;
            }
            case 5: {
                printf(ANSI_BOLD "\n--- FILE MAINTENANCE COMPLAINT ---\n" ANSI_RESET);
                printf("Select Category:\n");
                printf("  1. Electrical (Lights, Fans, Sockets)\n");
                printf("  2. Plumbing (Taps, Showers, Drainage)\n");
                printf("  3. Carpentry (Bed, Desk, Door, Lock)\n");
                printf("  4. Cleanliness & Sanitation\n");
                printf("  5. Campus WiFi / LAN Network\n");
                printf("  6. Other\n");
                int cat = 0;
                printf("Choice [1-6]: ");
                scanf("%d", &cat);
                while (getchar() != '\n');
                char desc[MAX_TEXT];
                printf("Describe the issue: ");
                fgets(desc, sizeof(desc), stdin);
                desc[strcspn(desc, "\n")] = '\0';

                int comp_id = 0;
                int res = service_file_complaint(student.student_id, (ComplaintCategory)cat, desc, &comp_id);
                if (res == DB_SUCCESS) {
                    printf(ANSI_GREEN "\n[SUCCESS] Maintenance ticket #%d created. Warden notified.\n" ANSI_RESET, comp_id);
                } else {
                    printf(ANSI_RED "\n[ERROR] Failed to file complaint. (Are you residing in an active room?)\n" ANSI_RESET);
                }
                ui_pause();
                break;
            }
            case 6: {
                MaintenanceComplaint comps[32];
                int count = 0;
                db_get_complaints_by_student(student.student_id, comps, 32, &count);
                ui_display_complaints(comps, count);
                ui_pause();
                break;
            }
            case 7: {
                printf(ANSI_BOLD "\n--- CHANGE PASSWORD ---\n" ANSI_RESET);
                char old_p[MAX_STR_SM], new_p[MAX_STR_SM];
                printf("Enter current password: ");
                scanf("%63s", old_p);
                printf("Enter new password (min 6 chars): ");
                scanf("%63s", new_p);
                while (getchar() != '\n');

                int res = auth_change_password(session->user_id, old_p, new_p);
                if (res == DB_SUCCESS) {
                    printf(ANSI_GREEN "\n[SUCCESS] Password updated successfully.\n" ANSI_RESET);
                } else if (res == -3) {
                    printf(ANSI_RED "\n[ERROR] Current password incorrect.\n" ANSI_RESET);
                } else {
                    printf(ANSI_RED "\n[ERROR] Password update failed.\n" ANSI_RESET);
                }
                ui_pause();
                break;
            }
        }
    }
}

void ui_warden_portal(UserSession *session) {
    int choice = 0;
    while (1) {
        ui_clear_screen();
        ui_print_banner();
        printf(ANSI_BOLD "HALL WARDEN PORTAL - Logged in as %s\n" ANSI_RESET, session->username);
        printf("1. View Hostel Rooms & Real-Time Bed Vacancies\n");
        printf("2. Review & Approve Room Swap Requests\n");
        printf("3. Review & Resolve Maintenance Tickets\n");
        printf("4. View All Registered Students\n");
        printf("5. Process Student Check-Out / Vacate Room\n");
        printf("6. Logout\n");
        printf(ANSI_CYAN "Enter choice [1-6]: " ANSI_RESET);

        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n');
            continue;
        }
        while (getchar() != '\n');

        if (choice == 6) {
            auth_logout(session);
            printf(ANSI_GREEN "Logged out successfully.\n" ANSI_RESET);
            break;
        }

        switch (choice) {
            case 1: {
                int h_id = session->hostel_id > 0 ? session->hostel_id : 1;
                Room rooms[64];
                int count = 0;
                db_get_rooms_by_hostel(h_id, rooms, 64, &count);
                Hostel h;
                db_get_hostel_by_id(h_id, &h);
                printf(ANSI_BOLD ANSI_BLUE "\n--- HOSTEL ROOM MATRIX: %s ---\n" ANSI_RESET, h.name);
                ui_display_available_rooms(rooms, count);
                ui_pause();
                break;
            }
            case 2: {
                RoomSwapRequest swaps[32];
                int count = 0;
                db_get_pending_swap_requests(swaps, 32, &count);
                ui_display_swap_requests(swaps, count);
                if (count > 0) {
                    int s_id = 0;
                    printf("\nEnter Swap ID to act on (0 to skip): ");
                    scanf("%d", &s_id);
                    if (s_id > 0) {
                        int act = 0;
                        printf("Select Action (1: Approve, 2: Reject): ");
                        scanf("%d", &act);
                        while (getchar() != '\n');
                        if (act == 1) {
                            if (service_approve_room_swap(s_id, session->user_id) == DB_SUCCESS) {
                                printf(ANSI_GREEN "[SUCCESS] Swap #%d approved. Room rosters updated.\n" ANSI_RESET, s_id);
                            } else {
                                printf(ANSI_RED "[ERROR] Failed to execute swap.\n" ANSI_RESET);
                            }
                        } else if (act == 2) {
                            db_reject_room_swap(s_id, session->user_id);
                            printf(ANSI_YELLOW "[INFO] Swap request #%d rejected.\n" ANSI_RESET, s_id);
                        }
                    } else {
                        while (getchar() != '\n');
                    }
                }
                ui_pause();
                break;
            }
            case 3: {
                MaintenanceComplaint comps[64];
                int count = 0;
                db_get_all_complaints((ComplaintStatus)0, comps, 64, &count);
                ui_display_complaints(comps, count);
                if (count > 0) {
                    int c_id = 0;
                    printf("\nEnter Complaint ID to resolve (0 to skip): ");
                    scanf("%d", &c_id);
                    while (getchar() != '\n');
                    if (c_id > 0) {
                        char notes[MAX_TEXT];
                        printf("Enter resolution work done notes: ");
                        fgets(notes, sizeof(notes), stdin);
                        notes[strcspn(notes, "\n")] = '\0';
                        if (service_resolve_complaint(c_id, notes, session->user_id) == DB_SUCCESS) {
                            printf(ANSI_GREEN "[SUCCESS] Complaint #%d marked RESOLVED.\n" ANSI_RESET, c_id);
                        } else {
                            printf(ANSI_RED "[ERROR] Could not resolve complaint.\n" ANSI_RESET);
                        }
                    }
                }
                ui_pause();
                break;
            }
            case 4: {
                Student students[64];
                int count = 0;
                db_get_all_students(students, 64, &count);
                printf(ANSI_BOLD "\n%-12s %-22s %-25s %-6s %-6s %-10s\n" ANSI_RESET,
                       "Roll No", "Full Name", "Department", "Year", "CGPA", "City");
                printf("----------------------------------------------------------------------------------------\n");
                for (int i = 0; i < count; i++) {
                    printf("%-12s %-22.22s %-25.25s %-6d %-6.2f %-10s\n",
                           students[i].roll_number,
                           students[i].full_name,
                           students[i].department,
                           students[i].year_of_study,
                           students[i].cgpa,
                           students[i].city);
                }
                printf("----------------------------------------------------------------------------------------\n");
                ui_pause();
                break;
            }
            case 5: {
                printf(ANSI_BOLD "\n--- STUDENT CHECK-OUT & VACATE ROOM ---\n" ANSI_RESET);
                char roll[MAX_STR_SM];
                printf("Enter Student Roll Number to check out: ");
                scanf("%63s", roll);
                while (getchar() != '\n');

                Student st;
                if (db_get_student_by_roll(roll, &st) == DB_SUCCESS) {
                    if (service_vacate_student(st.student_id, session->user_id) == DB_SUCCESS) {
                        printf(ANSI_GREEN "[SUCCESS] Student %s checked out. Bed vacancy restored.\n" ANSI_RESET, roll);
                    } else {
                        printf(ANSI_RED "[ERROR] Student %s does not have an active allotment.\n" ANSI_RESET, roll);
                    }
                } else {
                    printf(ANSI_RED "[ERROR] Student not found.\n" ANSI_RESET);
                }
                ui_pause();
                break;
            }
        }
    }
}

void ui_admin_portal(UserSession *session) {
    int choice = 0;
    while (1) {
        ui_clear_screen();
        ui_print_banner();
        printf(ANSI_BOLD "CHIEF WARDEN & SYSTEM ADMIN DASHBOARD\n" ANSI_RESET);
        printf("1. View Campus-Wide Dorm Occupancy Analytics\n");
        printf("2. Execute Automated Priority-Merit Room Allotment Engine\n");
        printf("3. View Pending Allotment Applications\n");
        printf("4. View All Hostels & Infrastructure Details\n");
        printf("5. View Security Audit Trail Logs\n");
        printf("6. Logout\n");
        printf(ANSI_CYAN "Enter choice [1-6]: " ANSI_RESET);

        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n');
            continue;
        }
        while (getchar() != '\n');

        if (choice == 6) {
            auth_logout(session);
            printf(ANSI_GREEN "Logged out successfully.\n" ANSI_RESET);
            break;
        }

        switch (choice) {
            case 1: {
                ui_display_campus_stats();
                Hostel hostels[10];
                int h_count = 0;
                db_get_hostels(hostels, 10, &h_count);
                printf(ANSI_BOLD "\n%-8s %-36s %-10s %-12s %-12s %-10s\n" ANSI_RESET,
                       "ID", "Hostel / Hall Name", "Type", "Capacity", "Occupancy", "Occupancy %");
                printf("-------------------------------------------------------------------------------------------\n");
                for (int i = 0; i < h_count; i++) {
                    double rate = hostels[i].total_capacity > 0 ? ((double)hostels[i].current_occupancy / hostels[i].total_capacity) * 100.0 : 0.0;
                    printf("%-8d %-36.36s %-10s %-12d %-12d %-10.1f%%\n",
                           hostels[i].hostel_id,
                           hostels[i].name,
                           hostels[i].type == HOSTEL_MALE ? "MALE" : "FEMALE",
                           hostels[i].total_capacity,
                           hostels[i].current_occupancy,
                           rate);
                }
                printf("-------------------------------------------------------------------------------------------\n");
                ui_pause();
                break;
            }
            case 2: {
                printf(ANSI_BOLD ANSI_CYAN "\n--- RUNNING ATOMIC MERIT ALLOCATION ALGORITHM ---\n" ANSI_RESET);
                int allotted = 0;
                int res = service_run_automated_allotment(&allotted);
                if (res == DB_SUCCESS) {
                    printf(ANSI_GREEN "[SUCCESS] Automated allocation round completed! %d students successfully allotted beds.\n" ANSI_RESET, allotted);
                } else {
                    printf(ANSI_RED "[ERROR] Allocation engine failed.\n" ANSI_RESET);
                }
                ui_pause();
                break;
            }
            case 3: {
                AllotmentApplication apps[64];
                int count = 0;
                db_get_pending_applications(apps, 64, &count);
                printf(ANSI_BOLD "\n%-6s %-14s %-22s %-12s %-32s %-8s\n" ANSI_RESET,
                       "App ID", "Roll No", "Student Name", "Merit Score", "Preferred Hostel", "Status");
                printf("--------------------------------------------------------------------------------------------------\n");
                for (int i = 0; i < count; i++) {
                    printf("%-6d %-14s %-22.22s %-12.2f %-32.32s %-8s\n",
                           apps[i].application_id,
                           apps[i].roll_number,
                           apps[i].student_name,
                           apps[i].merit_score,
                           apps[i].preferred_hostel_name,
                           "PENDING");
                }
                printf("--------------------------------------------------------------------------------------------------\n");
                ui_pause();
                break;
            }
            case 4: {
                Hostel hostels[10];
                int count = 0;
                db_get_hostels(hostels, 10, &count);
                printf(ANSI_BOLD "\n%-4s %-34s %-8s %-8s %-10s %-16s %-16s\n" ANSI_RESET,
                       "ID", "Name", "Type", "Floors", "Capacity", "Warden", "Contact");
                printf("----------------------------------------------------------------------------------------------------------\n");
                for (int i = 0; i < count; i++) {
                    printf("%-4d %-34.34s %-8s %-8d %-10d %-16s %-16s\n",
                           hostels[i].hostel_id,
                           hostels[i].name,
                           hostels[i].type == HOSTEL_MALE ? "MALE" : "FEMALE",
                           hostels[i].total_floors,
                           hostels[i].total_capacity,
                           hostels[i].warden_name,
                           hostels[i].contact_phone);
                }
                printf("----------------------------------------------------------------------------------------------------------\n");
                ui_pause();
                break;
            }
            case 5: {
                AuditLog logs[64];
                int count = 0;
                db_get_audit_logs(logs, 64, &count);
                ui_display_audit_logs(logs, count);
                ui_pause();
                break;
            }
        }
    }
}

void ui_main_menu(void) {
    int choice = 0;
    while (1) {
        ui_clear_screen();
        ui_print_banner();
        printf(ANSI_BOLD "\nMAIN ENTRY MENU\n" ANSI_RESET);
        printf("1. Sign In (Student / Warden / Admin)\n");
        printf("2. Student Registration (New Admission)\n");
        printf("3. View Public Campus Hostel Vacancy Statistics\n");
        printf("4. View System Demonstration Credentials\n");
        printf("5. Exit Application\n");
        printf(ANSI_CYAN "Enter selection [1-5]: " ANSI_RESET);

        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n');
            continue;
        }
        while (getchar() != '\n');

        if (choice == 5) {
            printf("\nThank you for using NIT Durgapur Dorm Allotment Portal. Goodbye!\n");
            break;
        }

        switch (choice) {
            case 1: {
                char username[MAX_STR_SM];
                char password[MAX_STR_SM];
                printf(ANSI_BOLD "\n--- SECURE AUTHENTICATION ---\n" ANSI_RESET);
                printf("Username / Roll Number: ");
                scanf("%63s", username);
                printf("Password              : ");
                scanf("%63s", password);
                while (getchar() != '\n');

                UserSession session;
                int res = auth_login(username, password, &session);
                if (res == 0) {
                    printf(ANSI_GREEN "\n[SUCCESS] Authentication verified for %s (Role: %s)\n" ANSI_RESET,
                           session.username,
                           session.role == ROLE_ADMIN ? "ADMIN" : (session.role == ROLE_WARDEN ? "WARDEN" : "STUDENT"));
                    ui_pause();
                    if (session.role == ROLE_STUDENT) {
                        ui_student_portal(&session);
                    } else if (session.role == ROLE_WARDEN) {
                        ui_warden_portal(&session);
                    } else if (session.role == ROLE_ADMIN) {
                        ui_admin_portal(&session);
                    }
                } else if (res == -3) {
                    printf(ANSI_RED "\n[FAILED] Invalid password entered. Please try again.\n" ANSI_RESET);
                    ui_pause();
                } else {
                    printf(ANSI_RED "\n[FAILED] Username '%s' not registered or inactive.\n" ANSI_RESET, username);
                    ui_pause();
                }
                break;
            }
            case 2: {
                printf(ANSI_BOLD "\n--- NEW STUDENT REGISTRATION ---\n" ANSI_RESET);
                char roll[MAX_STR_SM], name[MAX_STR_MD], dept[MAX_STR_SM], email[MAX_STR_MD], phone[MAX_STR_SM];
                char city[MAX_STR_SM], state[MAX_STR_SM], pass[MAX_STR_SM];
                int year = 1;
                char gender_ch = 'M';
                double cgpa = 8.0, dist = 100.0;

                printf("Roll Number (e.g. 24CS8001): ");
                scanf("%63s", roll);
                while (getchar() != '\n');
                printf("Full Name: ");
                fgets(name, sizeof(name), stdin);
                name[strcspn(name, "\n")] = '\0';
                printf("Department: ");
                fgets(dept, sizeof(dept), stdin);
                dept[strcspn(dept, "\n")] = '\0';
                printf("Year of Study (1-4): ");
                scanf("%d", &year);
                printf("Gender (M/F): ");
                scanf(" %c", &gender_ch);
                printf("CGPA (0.00 - 10.00): ");
                scanf("%lf", &cgpa);
                printf("Home Distance from Durgapur in KM: ");
                scanf("%lf", &dist);
                printf("Home City: ");
                scanf("%63s", city);
                printf("Home State: ");
                scanf("%63s", state);
                printf("Email Address: ");
                scanf("%127s", email);
                printf("Phone Number: ");
                scanf("%63s", phone);
                printf("Create Password (min 6 chars): ");
                scanf("%63s", pass);
                while (getchar() != '\n');

                UserSession new_session;
                int res = auth_register_student(
                    roll, pass, name, dept, year, (gender_ch == 'F' || gender_ch == 'f') ? GENDER_FEMALE : GENDER_MALE,
                    email, phone, cgpa, dist, city, state, &new_session
                );
                if (res == DB_SUCCESS) {
                    printf(ANSI_GREEN "\n[SUCCESS] Student profile and user account created!\n" ANSI_RESET);
                    ui_pause();
                    ui_student_portal(&new_session);
                } else if (res == -2) {
                    printf(ANSI_RED "\n[ERROR] Roll number %s is already registered.\n" ANSI_RESET, roll);
                    ui_pause();
                } else {
                    printf(ANSI_RED "\n[ERROR] Registration failed.\n" ANSI_RESET);
                    ui_pause();
                }
                break;
            }
            case 3: {
                ui_display_campus_stats();
                ui_pause();
                break;
            }
            case 4: {
                printf(ANSI_BOLD "\n--- SYSTEM DEMONSTRATION CREDENTIALS ---\n" ANSI_RESET);
                printf("  [Admin / Chief Warden]\n");
                printf("    Username: " ANSI_BOLD "chief_warden" ANSI_RESET "     | Password: " ANSI_YELLOW "Admin@NITD2025" ANSI_RESET "\n\n");
                printf("  [Hall Wardens]\n");
                printf("    Username: " ANSI_BOLD "warden_tagore" ANSI_RESET "   | Password: " ANSI_YELLOW "Warden@Tagore" ANSI_RESET "   (Male Dorms)\n");
                printf("    Username: " ANSI_BOLD "warden_nivedita" ANSI_RESET " | Password: " ANSI_YELLOW "Warden@Nivedita" ANSI_RESET " (Female Dorms)\n\n");
                printf("  [Sample Students]\n");
                printf("    Username: " ANSI_BOLD "21CS8014" ANSI_RESET "        | Password: " ANSI_YELLOW "Pass@Student1" ANSI_RESET " (Sourav, CSE 4th Yr)\n");
                printf("    Username: " ANSI_BOLD "21EC8022" ANSI_RESET "        | Password: " ANSI_YELLOW "Pass@Student2" ANSI_RESET " (Ananya, ECE 4th Yr)\n");
                printf("    Username: " ANSI_BOLD "22ME8005" ANSI_RESET "        | Password: " ANSI_YELLOW "Pass@Student3" ANSI_RESET " (Rahul, ME 3rd Yr)\n");
                printf("    Username: " ANSI_BOLD "22EE8031" ANSI_RESET "        | Password: " ANSI_YELLOW "Pass@Student4" ANSI_RESET " (Pooja, EE 3rd Yr)\n");
                printf("    Username: " ANSI_BOLD "23CS8045" ANSI_RESET "        | Password: " ANSI_YELLOW "Pass@Student5" ANSI_RESET " (Debanjan, CSE 2nd Yr)\n");
                printf("--------------------------------------------------------\n");
                ui_pause();
                break;
            }
        }
    }
}

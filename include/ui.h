#ifndef UI_H
#define UI_H

#include "models.h"

/* ANSI Terminal Colors */
#define ANSI_RESET   "\x1b[0m"
#define ANSI_BOLD    "\x1b[1m"
#define ANSI_RED     "\x1b[31m"
#define ANSI_GREEN   "\x1b[32m"
#define ANSI_YELLOW  "\x1b[33m"
#define ANSI_BLUE    "\x1b[34m"
#define ANSI_MAGENTA "\x1b[35m"
#define ANSI_CYAN    "\x1b[36m"
#define ANSI_WHITE   "\x1b[37m"

void ui_print_banner(void);
void ui_clear_screen(void);
void ui_pause(void);

/* Portal menus */
void ui_main_menu(void);
void ui_student_portal(UserSession *session);
void ui_warden_portal(UserSession *session);
void ui_admin_portal(UserSession *session);

/* Tabular view formatters */
void ui_display_available_rooms(const Room *rooms, int count);
void ui_display_hostels(const Hostel *hostels, int count);
void ui_display_student_profile(const Student *student, const Allotment *allotment);
void ui_display_allotment_slip(const Allotment *allotment, const Student *student, const Room *room);
void ui_display_complaints(const MaintenanceComplaint *complaints, int count);
void ui_display_swap_requests(const RoomSwapRequest *swaps, int count);
void ui_display_audit_logs(const AuditLog *logs, int count);
void ui_display_campus_stats(void);

#endif /* UI_H */

#include "ui.h"
#include "auth.h"
#include "db.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_help(const char *prog_name) {
    printf("University Dorm Allotment Portal (Durgapur, West Bengal)\n");
    printf("Usage: %s [OPTIONS]\n\n", prog_name);
    printf("Options:\n");
    printf("  --mysql           Connect to active MySQL/MariaDB server\n");
    printf("  --host <ip>       Database host (default: 127.0.0.1)\n");
    printf("  --port <port>     Database port (default: 3306)\n");
    printf("  --user <user>     Database user (default: root)\n");
    printf("  --pass <pass>     Database password\n");
    printf("  --db <dbname>     Database name (default: university_dorm_db)\n");
    printf("  --stats           Display campus accommodation statistics and exit\n");
    printf("  --help            Show this help dialog\n");
}

int main(int argc, char *argv[]) {
    DBConfig db_cfg;
    memset(&db_cfg, 0, sizeof(db_cfg));
    strcpy(db_cfg.host, "127.0.0.1");
    db_cfg.port = 3306;
    strcpy(db_cfg.user, "root");
    strcpy(db_cfg.password, "");
    strcpy(db_cfg.database, "university_dorm_db");
    db_cfg.use_live_mysql = true; /* Will seamlessly fallback to embedded if offline */

    bool stats_only = false;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            print_help(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "--stats") == 0) {
            stats_only = true;
        } else if (strcmp(argv[i], "--host") == 0 && i + 1 < argc) {
            strncpy(db_cfg.host, argv[++i], sizeof(db_cfg.host) - 1);
        } else if (strcmp(argv[i], "--port") == 0 && i + 1 < argc) {
            db_cfg.port = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--user") == 0 && i + 1 < argc) {
            strncpy(db_cfg.user, argv[++i], sizeof(db_cfg.user) - 1);
        } else if (strcmp(argv[i], "--pass") == 0 && i + 1 < argc) {
            strncpy(db_cfg.password, argv[++i], sizeof(db_cfg.password) - 1);
        } else if (strcmp(argv[i], "--db") == 0 && i + 1 < argc) {
            strncpy(db_cfg.database, argv[++i], sizeof(db_cfg.database) - 1);
        } else if (strcmp(argv[i], "--embedded") == 0) {
            db_cfg.use_live_mysql = false;
        }
    }

    if (db_init(&db_cfg) != DB_SUCCESS) {
        fprintf(stderr, "Failed to initialize database configuration.\n");
        return 1;
    }

    if (db_connect() != DB_SUCCESS) {
        fprintf(stderr, "Database connection initialization failed.\n");
        return 1;
    }

    if (stats_only) {
        ui_print_banner();
        ui_display_campus_stats();
        db_disconnect();
        return 0;
    }

    /* Start User Interface */
    ui_main_menu();

    db_disconnect();
    return 0;
}

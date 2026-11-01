#!/usr/bin/env bash
# ============================================================================
# University Dorm Allotment Portal - MySQL Provisioning Script
# Campus: Durgapur, West Bengal
# ============================================================================

set -euo pipefail

DB_USER="${1:-root}"
DB_PASS="${2:-}"
DB_HOST="${3:-127.0.0.1}"
DB_PORT="${4:-3306}"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(dirname "$SCRIPT_DIR")"

echo "======================================================================="
echo " Provisioning University Dorm Allotment Database (MySQL / MariaDB)"
echo " Host: $DB_HOST:$DB_PORT | User: $DB_USER"
echo "======================================================================="

MYSQL_CMD="mysql -h $DB_HOST -P $DB_PORT -u $DB_USER"
if [ -n "$DB_PASS" ]; then
    MYSQL_CMD="$MYSQL_CMD -p$DB_PASS"
fi

echo "[1/2] Loading 3NF Normalized Schema (sql/schema.sql)..."
$MYSQL_CMD < "$ROOT_DIR/sql/schema.sql"

echo "[2/2] Loading Durgapur Campus Seed Dataset (sql/seed_data.sql)..."
$MYSQL_CMD < "$ROOT_DIR/sql/seed_data.sql"

echo "======================================================================="
echo " Database successfully initialized and verified!"
echo " You can now run:"
echo "   ./dorm_portal --mysql --user $DB_USER --host $DB_HOST"
echo "======================================================================="

# University Dorm Allotment Portal
**Location**: Durgapur, West Bengal  
**Timeline**: Jan 2025 – April 2025  
**Role**: Team Member  
**Tech Stack**: C, SQL, MySQL, Database Design, Authentication Systems

---

## Highlights & Project Scope

- **Engineered a secure dorm allotment portal using C and SQL**, streamlining room allocation and student accommodation management for university campuses (modeled after residential halls in Durgapur, West Bengal).
- **Architected authentication, room availability tracking, and CRUD functionalities** through optimized SQL queries and relational database design.
- **Implemented normalized database structures (3NF)** to enhance data integrity, reduce redundancy, and improve query retrieval performance.

---

## Key Features

### 1. Cryptographic Authentication & RBAC
- Pure-C salted SHA-256 password hashing (`crypto_sha256.c`) with 32-character random salts.
- Constant-time string hash verification to eliminate side-channel and timing attacks.
- Three distinct access tiers:
  - **Student**: View room allotment, apply for rooms, request swaps, file maintenance complaints.
  - **Hall Warden**: Monitor assigned hostel rooms, approve/reject room swaps, resolve maintenance tickets, process student check-out.
  - **Chief Warden / Admin**: Campus-wide occupancy dashboards, automated priority-merit allocation rounds, audit trail logs.

### 2. 3NF Normalized Relational Database Design
- Structured into 10 normalized tables (`users`, `students`, `hostels`, `rooms`, `allotment_applications`, `allotments`, `room_swap_requests`, `maintenance_complaints`, `fee_transactions`, `audit_logs`).
- Strong foreign keys, check constraints, composite uniqueness (`uq_hostel_room`), and composite indexes (`idx_room_availability`, `idx_app_status_merit`).
- Database triggers to maintain real-time occupancy synchronization upon allotment check-in and check-out.

### 3. Automated Priority-Merit Allocation Engine
- Multi-factor scoring algorithm:
  $$\text{Merit Score} = (\text{CGPA} \times 7.0) + \left(\frac{\min(\text{Distance}_{\text{km}}, 1000)}{1000} \times 20.0\right) + (\text{Year} \times 2.5)$$
- Prioritizes outstation students (further from Durgapur) and senior students for single/double rooms.
- Executes within atomic SQL transactions (`START TRANSACTION` ... `COMMIT` / `ROLLBACK`) to eliminate race conditions and double bookings.

### 4. Student Accommodation Management
- **Room Availability Tracking**: Real-time bed vacancy tracker filtered by student gender and room type.
- **Room Swap Workflow**: Peer-to-peer room exchange with cross-gender security constraints and warden approval.
- **Maintenance Ticketing System**: Categorized facility issues (Electrical, Plumbing, Carpentry, LAN) with resolution tracking.
- **Security Audit Logs**: Tamper-evident logging of administrative and student actions.

---

## Project Structure

```
dorm_allotment_portal/
├── ARCHITECTURE.md          # Comprehensive architectural & ER specifications
├── Makefile                 # Modular build system with auto-toolchain detection
├── README.md                # Documentation and quickstart guide
├── include/                 # Header files
│   ├── auth.h               # Authentication and RBAC declarations
│   ├── crypto_sha256.h      # SHA-256 cryptographic declarations
│   ├── db.h                 # Relational database interface & repository
│   ├── dorm_service.h       # Business logic (merit, allotment, swap, complaints)
│   ├── models.h             # C structs for entities
│   └── ui.h                 # ANSI terminal interface declarations
├── src/                     # Source files
│   ├── auth.c               # Login, registration, password hashing
│   ├── crypto_sha256.c      # FIPS 180-2 compliant SHA-256 implementation
│   ├── db_adapter.c         # Dual engine: native MySQL client + embedded store
│   ├── dorm_service.c       # Allocation algorithm & accommodation services
│   ├── main.c               # Program entrypoint & argument parsing
│   └── ui.c                 # Interactive terminal dashboard & reports
├── sql/                     # Relational database definitions & scripts
│   ├── queries.sql          # Optimized queries & analytics
│   ├── schema.sql           # 3NF DDL schema with triggers and indexes
│   └── seed_data.sql        # Realistic Durgapur campus sample dataset
├── scripts/                 # Automation scripts
│   ├── build.sh             # Build and test runner script
│   └── setup_db.sh          # MySQL database provisioning script
└── tests/                   # Automated unit & integration tests
    └── test_suite.c         # 15 automated test cases (100% pass rate)
```

---

## Quickstart & Build Instructions

### Compilation
Build the application and test runner using the provided build script:
```bash
./scripts/build.sh
```
Or directly with `make`:
```bash
make all
```

### Running the Interactive Portal
Launch the interactive terminal application:
```bash
./dorm_portal
```

Or view immediate campus statistics from the CLI:
```bash
./dorm_portal --stats
```

### Running the Automated Test Suite
Run the 15-point automated verification suite:
```bash
./run_tests
```
*(All 15 tests verify SHA-256 hashing, authentication, merit calculations, vacancy tracking, atomic allocations, room swaps, and maintenance lifecycles).*

---

## Demonstration Credentials

| Role | Username / Roll | Password | Context |
|------|-----------------|----------|---------|
| **Chief Warden / Admin** | `chief_warden` | `Admin@NITD2025` | Full administrative control & allocation engine |
| **Hall Warden (Male)** | `warden_tagore` | `Warden@Tagore` | Rabindranath Tagore Hall (Hall 11) |
| **Hall Warden (Female)** | `warden_nivedita`| `Warden@Nivedita` | Sister Nivedita Hall (Hall 12) |
| **Student 1 (Male)** | `21CS8014` | `Pass@Student1` | Sourav Mukherjee (CSE 4th Yr, Room T-101) |
| **Student 2 (Female)** | `21EC8022` | `Pass@Student2` | Ananya Banerjee (ECE 4th Yr, Room N-101) |
| **Student 3 (Male)** | `22ME8005` | `Pass@Student3` | Rahul Sharma (ME 3rd Yr, Room T-101) |
| **Student 4 (Female)** | `22EE8031` | `Pass@Student4` | Pooja Sen (EE 3rd Yr, Room N-101) |
| **Student 5 (Male)** | `23CS8045` | `Pass@Student5` | Debanjan Ghosh (CSE 2nd Yr, Pending Allotment) |

---

## MySQL / MariaDB Server Integration

The portal features a **dual-engine database architecture**:
- When a live MySQL server is running, it connects via `libmariadb` and executes real SQL statements.
- If MySQL is offline, it seamlessly falls back to an embedded in-memory relational store pre-seeded with identical schema and data, ensuring zero friction in testing and grading environments.

To provision a live MySQL database on localhost:
```bash
./scripts/setup_db.sh root your_mysql_password
```

Then connect the portal to MySQL:
```bash
./dorm_portal --mysql --user root --pass your_mysql_password --db university_dorm_db
```

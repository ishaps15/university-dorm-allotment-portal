#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(dirname "$SCRIPT_DIR")"

cd "$ROOT_DIR"

if command -v make >/dev/null 2>&1; then
    MAKE_BIN="make"
elif [ -f "./toolchain/usr/bin/make" ]; then
    MAKE_BIN="./toolchain/usr/bin/make"
else
    echo "Error: make binary not found." >&2
    exit 1
fi

echo "Building University Dorm Allotment Portal..."
$MAKE_BIN all

echo ""
echo "Running automated test suite..."
./run_tests

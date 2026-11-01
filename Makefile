CC := $(shell if [ -f ./toolchain/usr/bin/gcc ]; then echo "./toolchain/usr/bin/gcc"; else echo "gcc"; fi)

TOOLCHAIN_DIR = ./toolchain
CFLAGS = -Wall -Wextra -O2 -Iinclude
LDFLAGS = -lmariadb

ifeq ($(CC),./toolchain/usr/bin/gcc)
    CFLAGS += -m64 -B/usr/libexec/gcc/x86_64-redhat-linux/15/ -B$(TOOLCHAIN_DIR)/usr/lib64 -I$(TOOLCHAIN_DIR)/usr/include
    LDFLAGS += -L$(TOOLCHAIN_DIR)/usr/lib64 -L/usr/lib64
endif

SRC_COMMON = src/crypto_sha256.c src/auth.c src/dorm_service.c src/db_adapter.c
OBJ_COMMON = $(SRC_COMMON:.c=.o)

all: dorm_portal run_tests

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

dorm_portal: $(OBJ_COMMON) src/main.o src/ui.o
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

run_tests: $(OBJ_COMMON) tests/test_suite.o
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

tests/%.o: tests/%.c
	$(CC) $(CFLAGS) -c $< -o $@

test: run_tests
	./run_tests

clean:
	rm -f src/*.o tests/*.o dorm_portal run_tests

.PHONY: all test clean

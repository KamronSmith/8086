CC := gcc
BIN_DIR := bin/
TARGET := $(BIN_DIR)/8086
SRC := $(wildcard src/*.c)

TESTS := $(wildcard tests/*.c)
TEST_EXE_NAME := test
TEST_SRC_INC := $(filter-out src/8086.c, $(SRC))
TEST_DIR := tests

BUILD ?= debug

CFLAGS_COMMON = -Wall -Wextra -Werror -Wshadow -Wformat=2
CFLAGS_DEBUG = $(CFLAGS_COMMON) -Wpedantic -pedantic-errors -Wno-unused -g -O0 -DDEBUG -fsanitize=address
CFLAGS_RELEASE = $(CFLAGS_COMMON) -O3 -DNDEBUG

ifeq ($(BUILD),debug)
	ifeq ($(shell uname -s),Linux)
		CFLAGS := $(CFLAGS_DEBUG) -lasan
	endif
else
	CFLAGS := $(CFLAGS_RELEASE)
endif

default:
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

compile-tests:
	$(CC) $(TESTS) $(TEST_SRC_INC) -Isrc -lcriterion -o $(TEST_DIR)/$(TEST_EXE_NAME)

.PHONY: run test clean

run: default
	./$(TARGET)

test: compile-tests
	./$(TEST_DIR)/$(TEST_EXE_NAME)

clean:
	rm -f $(TARGET)

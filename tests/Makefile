# tests/Makefile - Graylib Unit Tests
#
# Copyright 2025 Zach Podbielniak
# SPDX-License-Identifier: AGPL-3.0-or-later

# Include configuration from parent
include ../config.mk
include ../rules.mk

# Project root directory
PROJROOT := $(abspath ..)

# Override paths to use parent's directories
LIBOUTDIR := $(PROJROOT)/build/lib
RAYLIB_LIBDIR := $(PROJROOT)/deps/raylib/src

# Test sources
TEST_SOURCES := \
	test-vector2.c \
	test-color.c \
	test-rectangle.c

# Test executables
TEST_PROGRAMS := $(patsubst %.c,%,$(TEST_SOURCES))

# Object directory
TEST_OBJDIR := $(PROJROOT)/build/tests

# Compiler flags for tests
TEST_CFLAGS := \
	$(CFLAGS) \
	$(GLIB_CFLAGS) \
	-I$(PROJROOT) \
	-I$(PROJROOT)/graylib \
	-DGRAYLIB_COMPILATION

# Linker flags for tests
TEST_LDFLAGS := \
	-L$(LIBOUTDIR) \
	-lgraylib \
	$(GLIB_LIBS) \
	$(RAYLIB_LIBDIR)/libraylib.a \
	-lGL -lm -lpthread -ldl -lrt -lX11

# Default target
all: $(TEST_PROGRAMS)

# Run all tests
run: all
	@echo "Running tests..."
	@for test in $(TEST_PROGRAMS); do \
		echo ""; \
		echo "=== Running $$test ==="; \
		LD_LIBRARY_PATH=$(LIBOUTDIR):$$LD_LIBRARY_PATH ./$$test || exit 1; \
	done
	@echo ""
	@echo "All tests passed!"

# Build test programs
test-vector2: test-vector2.c | $(LIBOUTDIR)/$(LIB_SHARED)
	$(CC) $(TEST_CFLAGS) -o $@ $< $(TEST_LDFLAGS)

test-color: test-color.c | $(LIBOUTDIR)/$(LIB_SHARED)
	$(CC) $(TEST_CFLAGS) -o $@ $< $(TEST_LDFLAGS)

test-rectangle: test-rectangle.c | $(LIBOUTDIR)/$(LIB_SHARED)
	$(CC) $(TEST_CFLAGS) -o $@ $< $(TEST_LDFLAGS)

# Clean
clean:
	$(RM) $(TEST_PROGRAMS)
	$(RMDIR) $(TEST_OBJDIR)

.PHONY: all run clean

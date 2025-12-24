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
TESTOUTDIR := $(PROJROOT)/build/tests
RAYLIB_LIBDIR := $(PROJROOT)/deps/raylib/src

# Test sources
TEST_SOURCES := \
	test-vector2.c \
	test-color.c \
	test-rectangle.c

# Test executables (with output directory)
TEST_PROGRAMS := $(addprefix $(TESTOUTDIR)/,$(patsubst %.c,%,$(TEST_SOURCES)))

# Compiler flags for tests
TEST_CFLAGS := \
	$(CFLAGS) \
	$(GLIB_CFLAGS) \
	-I$(PROJROOT) \
	-I$(PROJROOT)/src \
	-DGRAYLIB_COMPILATION

# Linker flags for tests
TEST_LDFLAGS := \
	-L$(LIBOUTDIR) \
	-lgraylib \
	$(GLIB_LIBS) \
	$(RAYLIB_LIBDIR)/libraylib.a \
	-lGL -lm -lpthread -ldl -lrt -lX11

# Default target
all: $(TESTOUTDIR) $(TEST_PROGRAMS)

# Create output directory
$(TESTOUTDIR):
	$(MKDIR_P) $@

# Run all tests
run: all
	@echo "Running tests..."
	@for test in $(TEST_PROGRAMS); do \
		echo ""; \
		echo "=== Running $$(basename $$test) ==="; \
		LD_LIBRARY_PATH=$(LIBOUTDIR):$$LD_LIBRARY_PATH $$test || exit 1; \
	done
	@echo ""
	@echo "All tests passed!"

# Build test programs
$(TESTOUTDIR)/test-vector2: test-vector2.c | $(LIBOUTDIR)/$(LIB_SHARED) $(TESTOUTDIR)
	$(CC) $(TEST_CFLAGS) -o $@ $< $(TEST_LDFLAGS)

$(TESTOUTDIR)/test-color: test-color.c | $(LIBOUTDIR)/$(LIB_SHARED) $(TESTOUTDIR)
	$(CC) $(TEST_CFLAGS) -o $@ $< $(TEST_LDFLAGS)

$(TESTOUTDIR)/test-rectangle: test-rectangle.c | $(LIBOUTDIR)/$(LIB_SHARED) $(TESTOUTDIR)
	$(CC) $(TEST_CFLAGS) -o $@ $< $(TEST_LDFLAGS)

# Clean
clean:
	$(RM) -r $(TESTOUTDIR)

.PHONY: all run clean

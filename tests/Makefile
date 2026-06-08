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

# Test sources - all test files
TEST_SOURCES := \
	test-vector2.c \
	test-vector3.c \
	test-vector4.c \
	test-matrix.c \
	test-quaternion.c \
	test-bounding-box.c \
	test-color.c \
	test-rectangle.c \
	test-collision.c \
	test-camera2d.c \
	test-camera3d.c \
	test-enums.c \
	test-font.c \
	test-entity.c \
	test-scene.c \
	test-interfaces.c \
	test-math-utils.c \
	test-wave.c \
	test-audio-stream.c \
	test-ui-control.c \
	test-ui-button.c \
	test-png.c \
	test-resource-pack.c \
	test-image-draw.c \
	test-image-accumulator.c \
	test-gif-quantize.c \
	test-image-temporal.c \
	test-path.c \
	test-image-font.c

# Test executables (with output directory and platform-specific extension)
TEST_PROGRAMS := $(addprefix $(TESTOUTDIR)/,$(patsubst %.c,%$(EXE_EXT),$(TEST_SOURCES)))

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
	$(PLATFORM_LIBS)

# Default target
all: $(TESTOUTDIR) $(TEST_PROGRAMS)

# Create output directory
$(TESTOUTDIR):
	$(MKDIR_P) $@

# Run all tests
run: all
	@echo "Running tests..."
ifeq ($(TARGET_PLATFORM),windows)
	@for test in $(TEST_PROGRAMS); do \
		echo ""; \
		echo "=== Running $$(basename $$test) ==="; \
		wine $$test || exit 1; \
	done
else
	@for test in $(TEST_PROGRAMS); do \
		echo ""; \
		echo "=== Running $$(basename $$test) ==="; \
		LD_LIBRARY_PATH=$(LIBOUTDIR):$$LD_LIBRARY_PATH $$test || exit 1; \
	done
endif
	@echo ""
	@echo "All tests passed!"

# Pattern rule for building all tests
$(TESTOUTDIR)/test-%$(EXE_EXT): test-%.c | $(LIBOUTDIR)/$(LIB_SHARED) $(TESTOUTDIR)
	$(CC) $(TEST_CFLAGS) -o $@ $< $(TEST_LDFLAGS)

# Clean
clean:
	$(RM) -r $(TESTOUTDIR)

.PHONY: all run clean

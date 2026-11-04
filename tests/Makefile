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
	test-input-event-mods.c \
	test-frame-sync.c \
	test-rectangle.c \
	test-collision.c \
	test-camera2d.c \
	test-camera3d.c \
	test-enums.c \
	test-font.c \
	test-entity.c \
	test-interpolation.c \
	test-scene.c \
	test-interfaces.c \
	test-math-utils.c \
	test-wave.c \
	test-audio-stream.c \
	test-audio-processor.c \
	test-audio-backend.c \
	test-audio-queue.c \
	test-music.c \
	test-ui-control.c \
	test-ui-button.c \
	test-png.c \
	test-resource-pack.c \
	test-image-draw.c \
	test-image-accumulator.c \
	test-gif-quantize.c \
	test-image-temporal.c \
	test-path.c \
	test-image-font.c \
	test-svg.c \
	test-color-oklab.c \
	test-image-noise-overlay.c \
	test-image-bloom.c \
	test-image-stamp-path.c \
	test-image-layer.c \
	test-bone-info.c \
	test-transform.c \
	test-fs.c \
	test-raylib62.c \
	test-text.c \
	test-software-renderer.c \
	test-sound-pan.c \
	test-model-skinning.c \
	test-text-layout.c \
	test-texture-mipmaps.c \
	test-fast2d.c
TEST_SOURCES += test-audio-owned.c

# Executable-owned public SDK adapters, never a device or private SDK layout.
$(TESTOUTDIR)/test-audio-owned: TEST_LDFLAGS += -Wl,--export-dynamic

# The fast 2D path test reads an internal counter (hidden in the shared
# library): link it against the static one.
$(TESTOUTDIR)/test-fast2d$(EXE_EXT): TEST_LDFLAGS := $(LIBOUTDIR)/$(LIB_STATIC) \
	$(GLIB_LIBS) $(RAYLIB_LIBDIR)/libraylib.a $(PLATFORM_LIBS)
$(TESTOUTDIR)/test-fast2d$(EXE_EXT): $(LIBOUTDIR)/$(LIB_STATIC)

# Test executables (with output directory and platform-specific extension)
TEST_PROGRAMS := $(addprefix $(TESTOUTDIR)/,$(patsubst %.c,%$(EXE_EXT),$(TEST_SOURCES)))

# Compiler flags for tests
TEST_CFLAGS := \
	$(CFLAGS) \
	$(GLIB_CFLAGS) \
	-I$(PROJROOT) \
	-I$(PROJROOT)/src \
	-I$(RAYLIB_LIBDIR) \
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

# Metadata tests load only the owned build's full typelib and open no devices.
# Keep this separate from the legacy display/audio suite.
$(TESTOUTDIR)/test-introspection: test-introspection.c | $(TESTOUTDIR)
	$(CC) $(EXE_CFLAGS) $(TEST_CFLAGS) $(shell $(PKG_CONFIG) --cflags girepository-2.0) -Werror \
		-o $@ $< $(shell $(PKG_CONFIG) --libs girepository-2.0)

test-introspection: $(TESTOUTDIR)/test-introspection
	$(MAKE) -C .. gir
	GRL_TEST_TYPELIB_DIR=$(PROJROOT)/build/gir $(TESTOUTDIR)/test-introspection --no-sound

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

# ThreadSanitizer run of the audio thread (make -C tests tsan): graylib's
# audio sources and test-audio-queue built with -fsanitize=thread into
# their own binary (the rest of graylib from the shared library, raylib
# uninstrumented: its pthread locks are seen through TSan's interceptors).
# Needs an audio server for the device tests (GRL_TEST_AUDIO_DEVICE=1).
# TSAN_CC picks the compiler: gcc needs libtsan (Fedora package libtsan);
# clang (e.g. TSAN_CC=clang) brings its own runtime.
TSAN_CC ?= $(CC)
TSAN_OUTDIR := $(PROJROOT)/build/tsan
TSAN_SOURCES := \
	$(PROJROOT)/src/audio/grl-audio-queue.c \
	$(PROJROOT)/src/audio/grl-audio-device.c \
	$(PROJROOT)/src/audio/grl-sound.c \
	$(PROJROOT)/src/audio/grl-music.c \
	$(PROJROOT)/src/audio/grl-audio-stream.c \
	$(PROJROOT)/src/audio/grl-audio-processor.c \
	$(PROJROOT)/src/audio/grl-wave.c \
	test-audio-queue.c \
	test-audio-backend.c \
	test-music.c \
	test-audio-stream.c
TSAN_CFLAGS := $(TEST_CFLAGS) -g -O1 -fsanitize=thread -fno-omit-frame-pointer

tsan: $(TSAN_OUTDIR)/test-audio-queue $(TSAN_OUTDIR)/test-audio-backend $(TSAN_OUTDIR)/test-music $(TSAN_OUTDIR)/test-audio-stream
	TSAN_OPTIONS="halt_on_error=1 second_deadlock_stack=1 suppressions=$(CURDIR)/tsan.supp $$TSAN_OPTIONS" \
		LD_LIBRARY_PATH=$(LIBOUTDIR):$$LD_LIBRARY_PATH $(TSAN_OUTDIR)/test-audio-queue
	TSAN_OPTIONS="halt_on_error=1 second_deadlock_stack=1 suppressions=$(CURDIR)/tsan.supp $$TSAN_OPTIONS" \
		LD_LIBRARY_PATH=$(LIBOUTDIR):$$LD_LIBRARY_PATH $(TSAN_OUTDIR)/test-audio-backend
	TSAN_OPTIONS="halt_on_error=1 second_deadlock_stack=1 suppressions=$(CURDIR)/tsan.supp $$TSAN_OPTIONS" \
		LD_LIBRARY_PATH=$(LIBOUTDIR):$$LD_LIBRARY_PATH $(TSAN_OUTDIR)/test-music
	TSAN_OPTIONS="halt_on_error=1 second_deadlock_stack=1 suppressions=$(CURDIR)/tsan.supp $$TSAN_OPTIONS" \
		LD_LIBRARY_PATH=$(LIBOUTDIR):$$LD_LIBRARY_PATH $(TSAN_OUTDIR)/test-audio-stream

$(TSAN_OUTDIR)/test-%: test-%.c $(filter-out test-%.c,$(TSAN_SOURCES)) | $(LIBOUTDIR)/$(LIB_SHARED)
	@$(MKDIR_P) $(TSAN_OUTDIR)
	$(TSAN_CC) $(TSAN_CFLAGS) -I$(PROJROOT)/src/audio -o $@ $< \
		$(filter $(PROJROOT)/src/%,$(TSAN_SOURCES)) $(TEST_LDFLAGS) -fsanitize=thread

# Clean
clean:
	$(RM) -r $(TESTOUTDIR)

.PHONY: all run clean test-introspection tsan

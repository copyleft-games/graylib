# tools/preview/Makefile - Graylib Live-Preview Host
#
# Copyright 2025 Zach Podbielniak
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Builds the grl-preview-host executable.
#
# LINUX-ONLY: this tool uses inotify and dlopen; macOS and Windows
# are not supported.  The Makefile will refuse to build on non-Linux
# platforms.
#
# Usage (from the graylib repo root):
#   make BUILD_PREVIEW=1 preview-tools
#
# Or directly:
#   cd tools/preview && make
#
# Run:
#   LD_LIBRARY_PATH=../../build/lib:$LD_LIBRARY_PATH \
#       ./../../build/preview/grl-preview-host my-sketch.c

# Set default goal before including other files
.DEFAULT_GOAL := all

# Include configuration from parent
include ../../config.mk
include ../../rules.mk

# =============================================================================
# Platform guard — Linux only
# =============================================================================

ifneq ($(PLATFORM),linux)
all:
	@echo "grl-preview-host is Linux-only (requires inotify + dlopen)."
	@echo "Skipping build on platform: $(PLATFORM)"

clean:
	@true

.PHONY: all clean
else

# =============================================================================
# Paths
# =============================================================================

PROJROOT    := $(abspath ../..)
LIBOUTDIR   := $(PROJROOT)/build/lib
PREVIEWDIR  := $(PROJROOT)/build/preview
RAYLIB_LIBDIR := $(PROJROOT)/deps/raylib/src

# =============================================================================
# Compiler flags
# =============================================================================

PREVIEW_CFLAGS := \
	$(CFLAGS) \
	$(GLIB_CFLAGS) \
	-I$(PROJROOT) \
	-I$(PROJROOT)/src \
	-DGRAYLIB_COMPILATION \
	-DGRAYLIB_ROOT_DEFAULT=\"$(PROJROOT)\"

# Link flags: mirror examples/Makefile, add -ldl for dlopen
PREVIEW_LDFLAGS := \
	-L$(LIBOUTDIR) \
	-lgraylib \
	$(GLIB_LIBS) \
	$(RAYLIB_LIBDIR)/libraylib.a \
	$(PLATFORM_LIBS) \
	-ldl

HOST_BIN := $(PREVIEWDIR)/grl-preview-host

# =============================================================================
# Targets
# =============================================================================

all: $(PREVIEWDIR) $(HOST_BIN)

$(PREVIEWDIR):
	$(MKDIR_P) $@

$(HOST_BIN): grl-preview-host.c | $(LIBOUTDIR)/$(LIB_SHARED) $(PREVIEWDIR)
	$(CC) $(PREVIEW_CFLAGS) -o $@ $< $(PREVIEW_LDFLAGS)
	@echo "Built: $@"

# Run the host against the bundled sketch template (for quick smoke tests)
run-template: $(HOST_BIN)
	LD_LIBRARY_PATH=$(LIBOUTDIR):$$LD_LIBRARY_PATH \
		$(HOST_BIN) $(PROJROOT)/tools/preview/sketch-template.c

clean:
	$(RM) -r $(PREVIEWDIR)

.PHONY: all clean run-template

endif # PLATFORM == linux

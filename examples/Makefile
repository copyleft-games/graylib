# examples/Makefile - Graylib Examples
#
# Copyright 2025 Zach Podbielniak
# SPDX-License-Identifier: AGPL-3.0-or-later

# Set default goal before including other files
.DEFAULT_GOAL := all

# Include configuration from parent
include ../config.mk
include ../rules.mk

# Project root directory
PROJROOT := $(abspath ..)

# Override paths to use parent's directories
LIBOUTDIR := $(PROJROOT)/build/lib
RAYLIB_LIBDIR := $(PROJROOT)/deps/raylib/src

# Example sources
EXAMPLE_SOURCES := \
	basic-window.c \
	shapes-demo.c \
	textures-text.c \
	input-demo.c \
	audio-demo.c \
	entity-demo.c \
	scene-demo.c \
	camera-demo.c \
	model-demo.c \
	collision-demo.c

# Example executables
EXAMPLE_PROGRAMS := $(patsubst %.c,%,$(EXAMPLE_SOURCES))

# Compiler flags for examples
EXAMPLE_CFLAGS := \
	$(CFLAGS) \
	$(GLIB_CFLAGS) \
	-I$(PROJROOT) \
	-I$(PROJROOT)/graylib \
	-DGRAYLIB_COMPILATION

# Linker flags for examples
EXAMPLE_LDFLAGS := \
	-L$(LIBOUTDIR) \
	-lgraylib \
	$(GLIB_LIBS) \
	$(RAYLIB_LIBDIR)/libraylib.a \
	-lGL -lm -lpthread -ldl -lrt -lX11

# Default target
all: $(EXAMPLE_PROGRAMS)

# Build example programs
basic-window: basic-window.c | $(LIBOUTDIR)/$(LIB_SHARED)
	$(CC) $(EXAMPLE_CFLAGS) -o $@ $< $(EXAMPLE_LDFLAGS)

shapes-demo: shapes-demo.c | $(LIBOUTDIR)/$(LIB_SHARED)
	$(CC) $(EXAMPLE_CFLAGS) -o $@ $< $(EXAMPLE_LDFLAGS)

textures-text: textures-text.c | $(LIBOUTDIR)/$(LIB_SHARED)
	$(CC) $(EXAMPLE_CFLAGS) -o $@ $< $(EXAMPLE_LDFLAGS)

input-demo: input-demo.c | $(LIBOUTDIR)/$(LIB_SHARED)
	$(CC) $(EXAMPLE_CFLAGS) -o $@ $< $(EXAMPLE_LDFLAGS)

audio-demo: audio-demo.c | $(LIBOUTDIR)/$(LIB_SHARED)
	$(CC) $(EXAMPLE_CFLAGS) -o $@ $< $(EXAMPLE_LDFLAGS)

entity-demo: entity-demo.c | $(LIBOUTDIR)/$(LIB_SHARED)
	$(CC) $(EXAMPLE_CFLAGS) -o $@ $< $(EXAMPLE_LDFLAGS)

scene-demo: scene-demo.c | $(LIBOUTDIR)/$(LIB_SHARED)
	$(CC) $(EXAMPLE_CFLAGS) -o $@ $< $(EXAMPLE_LDFLAGS)

camera-demo: camera-demo.c | $(LIBOUTDIR)/$(LIB_SHARED)
	$(CC) $(EXAMPLE_CFLAGS) -o $@ $< $(EXAMPLE_LDFLAGS)

model-demo: model-demo.c | $(LIBOUTDIR)/$(LIB_SHARED)
	$(CC) $(EXAMPLE_CFLAGS) -o $@ $< $(EXAMPLE_LDFLAGS)

collision-demo: collision-demo.c | $(LIBOUTDIR)/$(LIB_SHARED)
	$(CC) $(EXAMPLE_CFLAGS) -o $@ $< $(EXAMPLE_LDFLAGS)

# Run an example (usage: make run-basic-window)
run-%: %
	LD_LIBRARY_PATH=$(LIBOUTDIR):$$LD_LIBRARY_PATH ./$<

# Clean
clean:
	$(RM) $(EXAMPLE_PROGRAMS)

.PHONY: all clean

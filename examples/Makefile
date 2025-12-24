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
EXAMPLEOUTDIR := $(PROJROOT)/build/examples
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
	collision-demo.c \
	pong.c \
	pong3d.c \
	snake3d.c \
	tetris3d.c \
	flappybird3d.c \
	doodlejump3d.c \
	asteroids3d.c \
	firstperson-demo.c \
	roguelike3d.c \
	voxelcraft3d.c

# Example executables (with output directory)
EXAMPLE_PROGRAMS := $(addprefix $(EXAMPLEOUTDIR)/,$(patsubst %.c,%,$(EXAMPLE_SOURCES)))

# Compiler flags for examples
EXAMPLE_CFLAGS := \
	$(CFLAGS) \
	$(GLIB_CFLAGS) \
	-I$(PROJROOT) \
	-I$(PROJROOT)/src \
	-DGRAYLIB_COMPILATION

# Linker flags for examples
EXAMPLE_LDFLAGS := \
	-L$(LIBOUTDIR) \
	-lgraylib \
	$(GLIB_LIBS) \
	$(RAYLIB_LIBDIR)/libraylib.a \
	-lGL -lm -lpthread -ldl -lrt -lX11

# Default target
all: $(EXAMPLEOUTDIR) $(EXAMPLE_PROGRAMS)

# Create output directory
$(EXAMPLEOUTDIR):
	$(MKDIR_P) $@

# Build example programs
$(EXAMPLEOUTDIR)/basic-window: basic-window.c | $(LIBOUTDIR)/$(LIB_SHARED) $(EXAMPLEOUTDIR)
	$(CC) $(EXAMPLE_CFLAGS) -o $@ $< $(EXAMPLE_LDFLAGS)

$(EXAMPLEOUTDIR)/shapes-demo: shapes-demo.c | $(LIBOUTDIR)/$(LIB_SHARED) $(EXAMPLEOUTDIR)
	$(CC) $(EXAMPLE_CFLAGS) -o $@ $< $(EXAMPLE_LDFLAGS)

$(EXAMPLEOUTDIR)/textures-text: textures-text.c | $(LIBOUTDIR)/$(LIB_SHARED) $(EXAMPLEOUTDIR)
	$(CC) $(EXAMPLE_CFLAGS) -o $@ $< $(EXAMPLE_LDFLAGS)

$(EXAMPLEOUTDIR)/input-demo: input-demo.c | $(LIBOUTDIR)/$(LIB_SHARED) $(EXAMPLEOUTDIR)
	$(CC) $(EXAMPLE_CFLAGS) -o $@ $< $(EXAMPLE_LDFLAGS)

$(EXAMPLEOUTDIR)/audio-demo: audio-demo.c | $(LIBOUTDIR)/$(LIB_SHARED) $(EXAMPLEOUTDIR)
	$(CC) $(EXAMPLE_CFLAGS) -o $@ $< $(EXAMPLE_LDFLAGS)

$(EXAMPLEOUTDIR)/entity-demo: entity-demo.c | $(LIBOUTDIR)/$(LIB_SHARED) $(EXAMPLEOUTDIR)
	$(CC) $(EXAMPLE_CFLAGS) -o $@ $< $(EXAMPLE_LDFLAGS)

$(EXAMPLEOUTDIR)/scene-demo: scene-demo.c | $(LIBOUTDIR)/$(LIB_SHARED) $(EXAMPLEOUTDIR)
	$(CC) $(EXAMPLE_CFLAGS) -o $@ $< $(EXAMPLE_LDFLAGS)

$(EXAMPLEOUTDIR)/camera-demo: camera-demo.c | $(LIBOUTDIR)/$(LIB_SHARED) $(EXAMPLEOUTDIR)
	$(CC) $(EXAMPLE_CFLAGS) -o $@ $< $(EXAMPLE_LDFLAGS)

$(EXAMPLEOUTDIR)/model-demo: model-demo.c | $(LIBOUTDIR)/$(LIB_SHARED) $(EXAMPLEOUTDIR)
	$(CC) $(EXAMPLE_CFLAGS) -o $@ $< $(EXAMPLE_LDFLAGS)

$(EXAMPLEOUTDIR)/collision-demo: collision-demo.c | $(LIBOUTDIR)/$(LIB_SHARED) $(EXAMPLEOUTDIR)
	$(CC) $(EXAMPLE_CFLAGS) -o $@ $< $(EXAMPLE_LDFLAGS)

$(EXAMPLEOUTDIR)/pong: pong.c | $(LIBOUTDIR)/$(LIB_SHARED) $(EXAMPLEOUTDIR)
	$(CC) $(EXAMPLE_CFLAGS) -o $@ $< $(EXAMPLE_LDFLAGS)

$(EXAMPLEOUTDIR)/pong3d: pong3d.c | $(LIBOUTDIR)/$(LIB_SHARED) $(EXAMPLEOUTDIR)
	$(CC) $(EXAMPLE_CFLAGS) -o $@ $< $(EXAMPLE_LDFLAGS)

$(EXAMPLEOUTDIR)/snake3d: snake3d.c | $(LIBOUTDIR)/$(LIB_SHARED) $(EXAMPLEOUTDIR)
	$(CC) $(EXAMPLE_CFLAGS) -o $@ $< $(EXAMPLE_LDFLAGS)

$(EXAMPLEOUTDIR)/tetris3d: tetris3d.c | $(LIBOUTDIR)/$(LIB_SHARED) $(EXAMPLEOUTDIR)
	$(CC) $(EXAMPLE_CFLAGS) -o $@ $< $(EXAMPLE_LDFLAGS)

$(EXAMPLEOUTDIR)/flappybird3d: flappybird3d.c | $(LIBOUTDIR)/$(LIB_SHARED) $(EXAMPLEOUTDIR)
	$(CC) $(EXAMPLE_CFLAGS) -o $@ $< $(EXAMPLE_LDFLAGS)

$(EXAMPLEOUTDIR)/doodlejump3d: doodlejump3d.c | $(LIBOUTDIR)/$(LIB_SHARED) $(EXAMPLEOUTDIR)
	$(CC) $(EXAMPLE_CFLAGS) -o $@ $< $(EXAMPLE_LDFLAGS)

$(EXAMPLEOUTDIR)/asteroids3d: asteroids3d.c | $(LIBOUTDIR)/$(LIB_SHARED) $(EXAMPLEOUTDIR)
	$(CC) $(EXAMPLE_CFLAGS) -o $@ $< $(EXAMPLE_LDFLAGS)

$(EXAMPLEOUTDIR)/firstperson-demo: firstperson-demo.c | $(LIBOUTDIR)/$(LIB_SHARED) $(EXAMPLEOUTDIR)
	$(CC) $(EXAMPLE_CFLAGS) -o $@ $< $(EXAMPLE_LDFLAGS)

$(EXAMPLEOUTDIR)/roguelike3d: roguelike3d.c | $(LIBOUTDIR)/$(LIB_SHARED) $(EXAMPLEOUTDIR)
	$(CC) $(EXAMPLE_CFLAGS) -o $@ $< $(EXAMPLE_LDFLAGS)

$(EXAMPLEOUTDIR)/voxelcraft3d: voxelcraft3d.c | $(LIBOUTDIR)/$(LIB_SHARED) $(EXAMPLEOUTDIR)
	$(CC) $(EXAMPLE_CFLAGS) -o $@ $< $(EXAMPLE_LDFLAGS)

# Run an example (usage: make run-basic-window)
run-%: $(EXAMPLEOUTDIR)/%
	LD_LIBRARY_PATH=$(LIBOUTDIR):$$LD_LIBRARY_PATH $<

# Clean
clean:
	$(RM) -r $(EXAMPLEOUTDIR)

.PHONY: all clean

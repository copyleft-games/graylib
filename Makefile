# Makefile - Graylib Root Build File
#
# Copyright 2025 Zach Podbielniak
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Main build orchestration for Graylib - a GObject wrapper around raylib.
#
# Usage:
#   make                     - Build the library
#   make DEBUG=1             - Build with debug symbols
#   make RAYLIB_SHARED=1     - Link against system raylib
#   make test                - Run unit tests
#   make install             - Install to PREFIX
#   make help                - Show all targets

# Include configuration
include config.mk
include rules.mk

# =============================================================================
# Source Files
# =============================================================================

# Public headers (for installation and GIR)
PUBLIC_HEADERS := \
	graylib/graylib.h \
	graylib/grl-version.h \
	graylib/grl-types.h \
	graylib/grl-enums.h \
	graylib/math/grl-vector2.h \
	graylib/math/grl-vector3.h \
	graylib/math/grl-vector4.h \
	graylib/math/grl-matrix.h \
	graylib/math/grl-bounding-box.h \
	graylib/math/grl-color.h \
	graylib/math/grl-rectangle.h \
	graylib/core/grl-application.h \
	graylib/core/grl-window.h \
	graylib/core/grl-input.h \
	graylib/graphics/grl-image.h \
	graylib/graphics/grl-texture.h \
	graylib/graphics/grl-font.h \
	graylib/graphics/grl-camera2d.h \
	graylib/graphics/grl-camera3d.h \
	graylib/graphics/grl-shader.h \
	graylib/graphics/grl-render-texture.h \
	graylib/graphics/grl-mesh.h \
	graylib/graphics/grl-material.h \
	graylib/graphics/grl-model.h \
	graylib/graphics/grl-model-animation.h \
	graylib/drawing/grl-draw.h \
	graylib/audio/grl-audio-device.h \
	graylib/audio/grl-sound.h \
	graylib/audio/grl-music.h \
	graylib/scene/grl-drawable.h \
	graylib/scene/grl-updatable.h \
	graylib/scene/grl-collidable.h \
	graylib/scene/grl-entity.h \
	graylib/scene/grl-sprite.h \
	graylib/scene/grl-animated-texture.h \
	graylib/scene/grl-scene.h \
	graylib/scene/grl-scene-manager.h \
	graylib/collision/grl-collision.h

# Source files
SOURCES := \
	graylib/grl-enums.c \
	graylib/math/grl-vector2.c \
	graylib/math/grl-vector3.c \
	graylib/math/grl-vector4.c \
	graylib/math/grl-matrix.c \
	graylib/math/grl-bounding-box.c \
	graylib/math/grl-color.c \
	graylib/math/grl-rectangle.c \
	graylib/core/grl-application.c \
	graylib/core/grl-window.c \
	graylib/core/grl-input.c \
	graylib/graphics/grl-image.c \
	graylib/graphics/grl-texture.c \
	graylib/graphics/grl-font.c \
	graylib/graphics/grl-camera2d.c \
	graylib/graphics/grl-camera3d.c \
	graylib/graphics/grl-shader.c \
	graylib/graphics/grl-render-texture.c \
	graylib/graphics/grl-mesh.c \
	graylib/graphics/grl-material.c \
	graylib/graphics/grl-model.c \
	graylib/graphics/grl-model-animation.c \
	graylib/drawing/grl-draw-shapes.c \
	graylib/drawing/grl-draw-textures.c \
	graylib/drawing/grl-draw-text.c \
	graylib/drawing/grl-draw-3d.c \
	graylib/audio/grl-audio-device.c \
	graylib/audio/grl-sound.c \
	graylib/audio/grl-music.c \
	graylib/scene/grl-drawable.c \
	graylib/scene/grl-updatable.c \
	graylib/scene/grl-collidable.c \
	graylib/scene/grl-entity.c \
	graylib/scene/grl-sprite.c \
	graylib/scene/grl-animated-texture.c \
	graylib/scene/grl-scene.c \
	graylib/scene/grl-scene-manager.c \
	graylib/collision/grl-collision.c

# Object files
OBJECTS := $(patsubst %.c,$(OBJDIR)/%.o,$(SOURCES))
DEPENDS := $(patsubst %.c,$(OBJDIR)/%.d,$(SOURCES))

# =============================================================================
# Default Target
# =============================================================================

all: raylib-check lib
ifeq ($(BUILD_GIR),1)
all: gir
endif

# =============================================================================
# Raylib Dependency
# =============================================================================

# Check if raylib needs to be built
raylib-check:
ifeq ($(RAYLIB_SHARED),0)
	@if [ ! -f "$(RAYLIB_SRC)/libraylib.a" ]; then \
		echo "Building raylib from deps/raylib..."; \
		$(MAKE) raylib; \
	fi
endif

# Build raylib from source
raylib:
ifeq ($(RAYLIB_SHARED),0)
	$(call print_status,"Building raylib...")
	cd $(RAYLIB_SRC) && $(MAKE) PLATFORM=PLATFORM_DESKTOP RAYLIB_LIBTYPE=STATIC
else
	$(call print_warning,"RAYLIB_SHARED=1 - using system raylib, skipping build")
endif

# Clean raylib build
raylib-clean:
ifeq ($(RAYLIB_SHARED),0)
	cd $(RAYLIB_SRC) && $(MAKE) clean
endif

# =============================================================================
# Library Targets
# =============================================================================

lib: lib-static lib-shared

lib-static: $(LIBOUTDIR)/$(LIB_STATIC)

lib-shared: $(LIBOUTDIR)/$(LIB_SHARED)

# Static library
$(LIBOUTDIR)/$(LIB_STATIC): $(OBJECTS) | $(LIBOUTDIR)
	$(call print_status,"Creating static library: $(LIB_STATIC)")
	$(AR) rcs $@ $(OBJECTS)
	$(RANLIB) $@

# Shared library
$(LIBOUTDIR)/$(LIB_SHARED): $(OBJECTS) | $(LIBOUTDIR)
	$(call print_status,"Creating shared library: $(LIB_SHARED)")
	$(CC) $(LIB_LDFLAGS) -o $(LIBOUTDIR)/$(LIB_SHARED_VERSION) $(OBJECTS) $(ALL_LIBS)
	cd $(LIBOUTDIR) && ln -sf $(LIB_SHARED_VERSION) $(LIB_SHARED_SONAME)
	cd $(LIBOUTDIR) && ln -sf $(LIB_SHARED_SONAME) $(LIB_SHARED)

# =============================================================================
# Generated Files
# =============================================================================

# Generate version header from template
graylib/grl-version.h: graylib/grl-version.h.in config.mk
	$(call print_status,"Generating grl-version.h")
	$(SED) -e 's/@VERSION_MAJOR@/$(VERSION_MAJOR)/g' \
	       -e 's/@VERSION_MINOR@/$(VERSION_MINOR)/g' \
	       -e 's/@VERSION_MICRO@/$(VERSION_MICRO)/g' \
	       -e 's/@VERSION@/$(VERSION)/g' \
	       -e 's/@API_VERSION@/$(API_VERSION)/g' \
	       $< > $@

# Generate config header from template
graylib/config.h: graylib/config.h.in config.mk
	$(call print_status,"Generating config.h")
	$(SED) -e 's/@VERSION_MAJOR@/$(VERSION_MAJOR)/g' \
	       -e 's/@VERSION_MINOR@/$(VERSION_MINOR)/g' \
	       -e 's/@VERSION_MICRO@/$(VERSION_MICRO)/g' \
	       -e 's/@VERSION@/$(VERSION)/g' \
	       $< > $@

# Generate pkg-config file
$(BUILDDIR)/$(PC_FILE): graylib.pc.in config.mk | $(BUILDDIR)
	$(call print_status,"Generating pkg-config file")
	$(SED) -e 's|@PREFIX@|$(PREFIX)|g' \
	       -e 's|@EXEC_PREFIX@|$(EXEC_PREFIX)|g' \
	       -e 's|@LIBDIR@|$(LIBDIR)|g' \
	       -e 's|@INCLUDEDIR@|$(INCLUDEDIR)|g' \
	       -e 's|@VERSION@|$(VERSION)|g' \
	       -e 's|@API_VERSION@|$(API_VERSION)|g' \
	       $< > $@

# =============================================================================
# GObject Introspection
# =============================================================================

gir: $(GIROUTDIR)/$(GIR_NAME) $(GIROUTDIR)/$(TYPELIB_NAME)

$(GIROUTDIR)/$(GIR_NAME): $(LIBOUTDIR)/$(LIB_SHARED) $(PUBLIC_HEADERS) $(SOURCES) | $(GIROUTDIR)
	$(call print_status,"Generating GIR: $(GIR_NAME)")
	$(GIR_SCANNER) $(GIR_SCANNER_FLAGS) \
		--library=$(LIB_NAME) \
		--library-path=$(LIBOUTDIR) \
		--output=$@ \
		$(PUBLIC_HEADERS) $(SOURCES)

$(GIROUTDIR)/$(TYPELIB_NAME): $(GIROUTDIR)/$(GIR_NAME)
	$(call print_status,"Generating typelib: $(TYPELIB_NAME)")
	$(GIR_COMPILER) $(GIR_COMPILER_FLAGS) \
		--output=$@ $<

# =============================================================================
# Tests
# =============================================================================

test tests check: lib
ifeq ($(BUILD_TESTS),1)
	$(call print_status,"Building and running tests...")
	$(MAKE) -C tests run
else
	$(call print_warning,"Tests disabled (BUILD_TESTS=0)")
endif

# =============================================================================
# Examples
# =============================================================================

examples: lib
ifeq ($(BUILD_EXAMPLES),1)
	$(call print_status,"Building examples...")
	$(MAKE) -C examples
else
	$(call print_warning,"Examples disabled (BUILD_EXAMPLES=0)")
endif

# =============================================================================
# Documentation
# =============================================================================

docs:
ifeq ($(BUILD_DOCS),1)
	$(call print_status,"Building documentation...")
	$(GI_DOCGEN) generate --config docs/graylib.toml $(GIROUTDIR)/$(GIR_NAME)
else
	$(call print_warning,"Documentation build disabled (BUILD_DOCS=0)")
endif

# =============================================================================
# Installation
# =============================================================================

install: lib $(BUILDDIR)/$(PC_FILE)
	$(call print_status,"Installing to $(PREFIX)...")
	# Create directories
	$(MKDIR_P) $(DESTDIR)$(LIBDIR)
	$(MKDIR_P) $(DESTDIR)$(INCLUDEDIR)/graylib
	$(MKDIR_P) $(DESTDIR)$(INCLUDEDIR)/graylib/math
	$(MKDIR_P) $(DESTDIR)$(INCLUDEDIR)/graylib/core
	$(MKDIR_P) $(DESTDIR)$(INCLUDEDIR)/graylib/graphics
	$(MKDIR_P) $(DESTDIR)$(INCLUDEDIR)/graylib/drawing
	$(MKDIR_P) $(DESTDIR)$(INCLUDEDIR)/graylib/audio
	$(MKDIR_P) $(DESTDIR)$(INCLUDEDIR)/graylib/scene
	$(MKDIR_P) $(DESTDIR)$(INCLUDEDIR)/graylib/collision
	$(MKDIR_P) $(DESTDIR)$(PKGCONFIGDIR)
ifeq ($(BUILD_GIR),1)
	$(MKDIR_P) $(DESTDIR)$(GIRDIR)
	$(MKDIR_P) $(DESTDIR)$(TYPELIBDIR)
endif
	# Install libraries
ifeq ($(BUILD_STATIC),1)
	$(INSTALL_DATA) $(LIBOUTDIR)/$(LIB_STATIC) $(DESTDIR)$(LIBDIR)/
endif
ifeq ($(BUILD_SHARED),1)
	$(INSTALL_PROGRAM) $(LIBOUTDIR)/$(LIB_SHARED_VERSION) $(DESTDIR)$(LIBDIR)/
	cd $(DESTDIR)$(LIBDIR) && ln -sf $(LIB_SHARED_VERSION) $(LIB_SHARED_SONAME)
	cd $(DESTDIR)$(LIBDIR) && ln -sf $(LIB_SHARED_SONAME) $(LIB_SHARED)
endif
	# Install headers
	$(INSTALL_DATA) graylib/graylib.h $(DESTDIR)$(INCLUDEDIR)/graylib/
	$(INSTALL_DATA) graylib/grl-version.h $(DESTDIR)$(INCLUDEDIR)/graylib/
	$(INSTALL_DATA) graylib/grl-types.h $(DESTDIR)$(INCLUDEDIR)/graylib/
	$(INSTALL_DATA) graylib/grl-enums.h $(DESTDIR)$(INCLUDEDIR)/graylib/
	$(INSTALL_DATA) graylib/math/grl-vector2.h $(DESTDIR)$(INCLUDEDIR)/graylib/math/
	$(INSTALL_DATA) graylib/math/grl-vector3.h $(DESTDIR)$(INCLUDEDIR)/graylib/math/
	$(INSTALL_DATA) graylib/math/grl-vector4.h $(DESTDIR)$(INCLUDEDIR)/graylib/math/
	$(INSTALL_DATA) graylib/math/grl-matrix.h $(DESTDIR)$(INCLUDEDIR)/graylib/math/
	$(INSTALL_DATA) graylib/math/grl-bounding-box.h $(DESTDIR)$(INCLUDEDIR)/graylib/math/
	$(INSTALL_DATA) graylib/math/grl-color.h $(DESTDIR)$(INCLUDEDIR)/graylib/math/
	$(INSTALL_DATA) graylib/math/grl-rectangle.h $(DESTDIR)$(INCLUDEDIR)/graylib/math/
	$(INSTALL_DATA) graylib/core/grl-application.h $(DESTDIR)$(INCLUDEDIR)/graylib/core/
	$(INSTALL_DATA) graylib/core/grl-window.h $(DESTDIR)$(INCLUDEDIR)/graylib/core/
	$(INSTALL_DATA) graylib/core/grl-input.h $(DESTDIR)$(INCLUDEDIR)/graylib/core/
	$(INSTALL_DATA) graylib/graphics/grl-image.h $(DESTDIR)$(INCLUDEDIR)/graylib/graphics/
	$(INSTALL_DATA) graylib/graphics/grl-texture.h $(DESTDIR)$(INCLUDEDIR)/graylib/graphics/
	$(INSTALL_DATA) graylib/graphics/grl-font.h $(DESTDIR)$(INCLUDEDIR)/graylib/graphics/
	$(INSTALL_DATA) graylib/graphics/grl-camera2d.h $(DESTDIR)$(INCLUDEDIR)/graylib/graphics/
	$(INSTALL_DATA) graylib/graphics/grl-camera3d.h $(DESTDIR)$(INCLUDEDIR)/graylib/graphics/
	$(INSTALL_DATA) graylib/graphics/grl-shader.h $(DESTDIR)$(INCLUDEDIR)/graylib/graphics/
	$(INSTALL_DATA) graylib/graphics/grl-render-texture.h $(DESTDIR)$(INCLUDEDIR)/graylib/graphics/
	$(INSTALL_DATA) graylib/graphics/grl-mesh.h $(DESTDIR)$(INCLUDEDIR)/graylib/graphics/
	$(INSTALL_DATA) graylib/graphics/grl-material.h $(DESTDIR)$(INCLUDEDIR)/graylib/graphics/
	$(INSTALL_DATA) graylib/graphics/grl-model.h $(DESTDIR)$(INCLUDEDIR)/graylib/graphics/
	$(INSTALL_DATA) graylib/graphics/grl-model-animation.h $(DESTDIR)$(INCLUDEDIR)/graylib/graphics/
	$(INSTALL_DATA) graylib/drawing/grl-draw.h $(DESTDIR)$(INCLUDEDIR)/graylib/drawing/
	$(INSTALL_DATA) graylib/audio/grl-audio-device.h $(DESTDIR)$(INCLUDEDIR)/graylib/audio/
	$(INSTALL_DATA) graylib/audio/grl-sound.h $(DESTDIR)$(INCLUDEDIR)/graylib/audio/
	$(INSTALL_DATA) graylib/audio/grl-music.h $(DESTDIR)$(INCLUDEDIR)/graylib/audio/
	$(INSTALL_DATA) graylib/scene/grl-drawable.h $(DESTDIR)$(INCLUDEDIR)/graylib/scene/
	$(INSTALL_DATA) graylib/scene/grl-updatable.h $(DESTDIR)$(INCLUDEDIR)/graylib/scene/
	$(INSTALL_DATA) graylib/scene/grl-collidable.h $(DESTDIR)$(INCLUDEDIR)/graylib/scene/
	$(INSTALL_DATA) graylib/scene/grl-entity.h $(DESTDIR)$(INCLUDEDIR)/graylib/scene/
	$(INSTALL_DATA) graylib/scene/grl-sprite.h $(DESTDIR)$(INCLUDEDIR)/graylib/scene/
	$(INSTALL_DATA) graylib/scene/grl-animated-texture.h $(DESTDIR)$(INCLUDEDIR)/graylib/scene/
	$(INSTALL_DATA) graylib/scene/grl-scene.h $(DESTDIR)$(INCLUDEDIR)/graylib/scene/
	$(INSTALL_DATA) graylib/scene/grl-scene-manager.h $(DESTDIR)$(INCLUDEDIR)/graylib/scene/
	$(INSTALL_DATA) graylib/collision/grl-collision.h $(DESTDIR)$(INCLUDEDIR)/graylib/collision/
	# Install pkg-config
	$(INSTALL_DATA) $(BUILDDIR)/$(PC_FILE) $(DESTDIR)$(PKGCONFIGDIR)/
	# Install GIR
ifeq ($(BUILD_GIR),1)
	$(INSTALL_DATA) $(GIROUTDIR)/$(GIR_NAME) $(DESTDIR)$(GIRDIR)/
	$(INSTALL_DATA) $(GIROUTDIR)/$(TYPELIB_NAME) $(DESTDIR)$(TYPELIBDIR)/
endif
	$(call print_status,"Installation complete!")

uninstall:
	$(call print_status,"Uninstalling from $(PREFIX)...")
	$(RM) $(DESTDIR)$(LIBDIR)/$(LIB_STATIC)
	$(RM) $(DESTDIR)$(LIBDIR)/$(LIB_SHARED)
	$(RM) $(DESTDIR)$(LIBDIR)/$(LIB_SHARED_SONAME)
	$(RM) $(DESTDIR)$(LIBDIR)/$(LIB_SHARED_VERSION)
	$(RMDIR) $(DESTDIR)$(INCLUDEDIR)/graylib
	$(RM) $(DESTDIR)$(PKGCONFIGDIR)/$(PC_FILE)
ifeq ($(BUILD_GIR),1)
	$(RM) $(DESTDIR)$(GIRDIR)/$(GIR_NAME)
	$(RM) $(DESTDIR)$(TYPELIBDIR)/$(TYPELIB_NAME)
endif

# =============================================================================
# Cleanup
# =============================================================================

clean:
	$(call print_status,"Cleaning build artifacts...")
	$(RMDIR) $(BUILDDIR)
	$(RM) graylib/grl-version.h
	$(RM) graylib/config.h

distclean: clean raylib-clean
	$(call print_status,"Cleaning all generated files...")
	$(RM) graylib/grl-enums.c
	$(RM) graylib/grl-enums.h

# =============================================================================
# Debug Target (shorthand for DEBUG=1)
# =============================================================================

debug-build:
	$(MAKE) DEBUG=1 all

# =============================================================================
# Build Directories
# =============================================================================

$(BUILDDIR):
	$(MKDIR_P) $@

# =============================================================================
# Dependencies
# =============================================================================

# Source files depend on generated headers
$(OBJECTS): graylib/grl-version.h graylib/config.h

# Include auto-generated dependencies (if they exist)
-include $(DEPENDS)

# =============================================================================
# Object File Rules
# =============================================================================

# Math module
$(OBJDIR)/graylib/math/grl-vector2.o: graylib/math/grl-vector2.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/graylib/math/grl-color.o: graylib/math/grl-color.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/graylib/math/grl-rectangle.o: graylib/math/grl-rectangle.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/graylib/math/grl-vector3.o: graylib/math/grl-vector3.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/graylib/math/grl-vector4.o: graylib/math/grl-vector4.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/graylib/math/grl-matrix.o: graylib/math/grl-matrix.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/graylib/math/grl-bounding-box.o: graylib/math/grl-bounding-box.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

# Core module
$(OBJDIR)/graylib/core/grl-application.o: graylib/core/grl-application.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/graylib/core/grl-window.o: graylib/core/grl-window.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/graylib/core/grl-input.o: graylib/core/grl-input.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

# Graphics module
$(OBJDIR)/graylib/graphics/grl-image.o: graylib/graphics/grl-image.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/graylib/graphics/grl-texture.o: graylib/graphics/grl-texture.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/graylib/graphics/grl-font.o: graylib/graphics/grl-font.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/graylib/graphics/grl-camera2d.o: graylib/graphics/grl-camera2d.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/graylib/graphics/grl-camera3d.o: graylib/graphics/grl-camera3d.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/graylib/graphics/grl-shader.o: graylib/graphics/grl-shader.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/graylib/graphics/grl-render-texture.o: graylib/graphics/grl-render-texture.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/graylib/graphics/grl-mesh.o: graylib/graphics/grl-mesh.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/graylib/graphics/grl-material.o: graylib/graphics/grl-material.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/graylib/graphics/grl-model.o: graylib/graphics/grl-model.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/graylib/graphics/grl-model-animation.o: graylib/graphics/grl-model-animation.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

# Drawing module
$(OBJDIR)/graylib/drawing/grl-draw-shapes.o: graylib/drawing/grl-draw-shapes.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/graylib/drawing/grl-draw-textures.o: graylib/drawing/grl-draw-textures.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/graylib/drawing/grl-draw-text.o: graylib/drawing/grl-draw-text.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/graylib/drawing/grl-draw-3d.o: graylib/drawing/grl-draw-3d.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

# Audio module
$(OBJDIR)/graylib/audio/grl-audio-device.o: graylib/audio/grl-audio-device.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/graylib/audio/grl-sound.o: graylib/audio/grl-sound.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/graylib/audio/grl-music.o: graylib/audio/grl-music.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

# Scene module
$(OBJDIR)/graylib/scene/grl-drawable.o: graylib/scene/grl-drawable.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/graylib/scene/grl-updatable.o: graylib/scene/grl-updatable.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/graylib/scene/grl-collidable.o: graylib/scene/grl-collidable.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/graylib/scene/grl-entity.o: graylib/scene/grl-entity.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/graylib/scene/grl-sprite.o: graylib/scene/grl-sprite.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/graylib/scene/grl-animated-texture.o: graylib/scene/grl-animated-texture.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/graylib/scene/grl-scene.o: graylib/scene/grl-scene.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/graylib/scene/grl-scene-manager.o: graylib/scene/grl-scene-manager.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

# Collision
$(OBJDIR)/graylib/collision/grl-collision.o: graylib/collision/grl-collision.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

# Enums
$(OBJDIR)/graylib/grl-enums.o: graylib/grl-enums.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

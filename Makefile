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

# Set default goal before including other files
.DEFAULT_GOAL := all

# Include configuration
include config.mk
include rules.mk

# =============================================================================
# Source Files
# =============================================================================

# Public headers (for installation and GIR)
PUBLIC_HEADERS := \
	src/graylib.h \
	src/grl-version.h \
	src/grl-types.h \
	src/grl-enums.h \
	src/math/grl-vector2.h \
	src/math/grl-vector3.h \
	src/math/grl-vector4.h \
	src/math/grl-matrix.h \
	src/math/grl-quaternion.h \
	src/math/grl-bounding-box.h \
	src/math/grl-color.h \
	src/math/grl-rectangle.h \
	src/math/grl-math-utils.h \
	src/core/grl-application.h \
	src/core/grl-window.h \
	src/core/grl-input.h \
	src/graphics/grl-image.h \
	src/graphics/grl-texture.h \
	src/graphics/grl-font.h \
	src/graphics/grl-camera2d.h \
	src/graphics/grl-camera3d.h \
	src/graphics/grl-shader.h \
	src/graphics/grl-render-texture.h \
	src/graphics/grl-mesh.h \
	src/graphics/grl-material.h \
	src/graphics/grl-model.h \
	src/graphics/grl-model-animation.h \
	src/graphics/grl-png.h \
	src/drawing/grl-draw.h \
	src/audio/grl-audio-device.h \
	src/audio/grl-wave.h \
	src/audio/grl-sound.h \
	src/audio/grl-music.h \
	src/audio/grl-audio-stream.h \
	src/scene/grl-drawable.h \
	src/scene/grl-updatable.h \
	src/scene/grl-collidable.h \
	src/scene/grl-entity.h \
	src/scene/grl-sprite.h \
	src/scene/grl-animated-texture.h \
	src/scene/grl-scene.h \
	src/scene/grl-scene-manager.h \
	src/collision/grl-collision.h \
	src/ui/grl-ui-enums.h \
	src/ui/grl-ui-control.h \
	src/ui/grl-ui-label.h \
	src/ui/grl-ui-button.h \
	src/ui/grl-ui-checkbox.h \
	src/ui/grl-ui-slider.h \
	src/ui/grl-ui-textbox.h \
	src/ui/grl-ui-toggle.h \
	src/ui/grl-ui-progressbar.h \
	src/ui/grl-ui-spinner.h \
	src/ui/grl-ui-valuebox.h \
	src/ui/grl-ui-combobox.h \
	src/ui/grl-ui-dropdownbox.h \
	src/ui/grl-ui-togglegroup.h \
	src/ui/grl-ui-listview.h \
	src/ui/grl-ui-colorpicker.h \
	src/ui/grl-ui-panel.h \
	src/ui/grl-ui-groupbox.h \
	src/ui/grl-ui-windowbox.h \
	src/ui/grl-ui-style.h \
	src/resources/grl-resource-enums.h \
	src/resources/grl-resource-chunk-info.h \
	src/resources/grl-resource-pack.h \
	src/rlgl/grl-rlgl.h

# Source files
SOURCES := \
	src/grl-enums.c \
	src/math/grl-vector2.c \
	src/math/grl-vector3.c \
	src/math/grl-vector4.c \
	src/math/grl-matrix.c \
	src/math/grl-quaternion.c \
	src/math/grl-bounding-box.c \
	src/math/grl-color.c \
	src/math/grl-rectangle.c \
	src/math/grl-math-utils.c \
	src/core/grl-application.c \
	src/core/grl-window.c \
	src/core/grl-input.c \
	src/graphics/grl-image.c \
	src/graphics/grl-texture.c \
	src/graphics/grl-font.c \
	src/graphics/grl-camera2d.c \
	src/graphics/grl-camera3d.c \
	src/graphics/grl-shader.c \
	src/graphics/grl-render-texture.c \
	src/graphics/grl-mesh.c \
	src/graphics/grl-material.c \
	src/graphics/grl-model.c \
	src/graphics/grl-model-animation.c \
	src/graphics/grl-png.c \
	src/drawing/grl-draw-shapes.c \
	src/drawing/grl-draw-textures.c \
	src/drawing/grl-draw-text.c \
	src/drawing/grl-draw-3d.c \
	src/audio/grl-audio-device.c \
	src/audio/grl-wave.c \
	src/audio/grl-sound.c \
	src/audio/grl-music.c \
	src/audio/grl-audio-stream.c \
	src/scene/grl-drawable.c \
	src/scene/grl-updatable.c \
	src/scene/grl-collidable.c \
	src/scene/grl-entity.c \
	src/scene/grl-sprite.c \
	src/scene/grl-animated-texture.c \
	src/scene/grl-scene.c \
	src/scene/grl-scene-manager.c \
	src/collision/grl-collision.c \
	src/ui/grl-ui-enums.c \
	src/ui/grl-ui-control.c \
	src/ui/grl-ui-label.c \
	src/ui/grl-ui-button.c \
	src/ui/grl-ui-checkbox.c \
	src/ui/grl-ui-slider.c \
	src/ui/grl-ui-textbox.c \
	src/ui/grl-ui-toggle.c \
	src/ui/grl-ui-progressbar.c \
	src/ui/grl-ui-spinner.c \
	src/ui/grl-ui-valuebox.c \
	src/ui/grl-ui-combobox.c \
	src/ui/grl-ui-dropdownbox.c \
	src/ui/grl-ui-togglegroup.c \
	src/ui/grl-ui-listview.c \
	src/ui/grl-ui-colorpicker.c \
	src/ui/grl-ui-panel.c \
	src/ui/grl-ui-groupbox.c \
	src/ui/grl-ui-windowbox.c \
	src/ui/grl-ui-style.c \
	src/resources/grl-resource-enums.c \
	src/resources/grl-resource-chunk-info.c \
	src/resources/grl-resource-pack.c \
	src/rlgl/grl-rlgl-core.c \
	src/rlgl/grl-rlgl-matrix.c \
	src/rlgl/grl-rlgl-vertex.c \
	src/rlgl/grl-rlgl-state.c \
	src/rlgl/grl-rlgl-texture.c \
	src/rlgl/grl-rlgl-buffer.c \
	src/rlgl/grl-rlgl-framebuffer.c \
	src/rlgl/grl-rlgl-shader.c

# Object files
OBJECTS := $(patsubst %.c,$(OBJDIR)/%.o,$(SOURCES))
DEPENDS := $(patsubst %.c,$(OBJDIR)/%.d,$(SOURCES))

# =============================================================================
# Generated Headers (must be built first)
# =============================================================================

GENERATED_HEADERS := src/grl-version.h src/config.h

.PHONY: generate
generate: $(GENERATED_HEADERS)

# =============================================================================
# Build Platform Tracking
# =============================================================================

# Platform marker file to detect when graylib needs rebuild for different platform
GRAYLIB_PLATFORM_MARKER := $(BUILDDIR)/.graylib-platform

# Check if build directory needs to be cleaned for a different platform
platform-check:
	@if [ -f "$(GRAYLIB_PLATFORM_MARKER)" ] && [ "$$(cat $(GRAYLIB_PLATFORM_MARKER))" != "$(TARGET_PLATFORM)" ]; then \
		echo "Graylib platform mismatch (need $(TARGET_PLATFORM)), cleaning build..."; \
		$(RMDIR) $(OBJDIR); \
		$(RMDIR) $(LIBOUTDIR); \
	fi
	@$(MKDIR_P) $(BUILDDIR)
	@echo "$(TARGET_PLATFORM)" > $(GRAYLIB_PLATFORM_MARKER)

# =============================================================================
# Default Target
# =============================================================================

# Use recursive make to ensure generate completes before lib starts
all: raylib-check platform-check generate
	@$(MAKE) --no-print-directory _lib
ifeq ($(BUILD_GIR),1)
	@$(MAKE) --no-print-directory gir
endif

# Internal target for actual library build (called after generate)
.PHONY: _lib
_lib: lib-static lib-shared

# =============================================================================
# Raylib Dependency
# =============================================================================

# Platform marker file to detect when raylib needs rebuild for different platform
RAYLIB_PLATFORM_MARKER := $(RAYLIB_SRC)/.graylib-platform

# Check if raylib needs to be built
raylib-check:
ifeq ($(RAYLIB_SHARED),0)
	@if [ ! -f "$(RAYLIB_SRC)/libraylib.a" ]; then \
		echo "Building raylib from deps/raylib..."; \
		$(MAKE) WINDOWS=$(WINDOWS) CROSS=$(CROSS) raylib; \
	elif [ ! -f "$(RAYLIB_PLATFORM_MARKER)" ] || [ "$$(cat $(RAYLIB_PLATFORM_MARKER))" != "$(TARGET_PLATFORM)" ]; then \
		echo "Raylib platform mismatch (need $(TARGET_PLATFORM)), rebuilding..."; \
		$(MAKE) raylib-clean; \
		$(MAKE) WINDOWS=$(WINDOWS) CROSS=$(CROSS) raylib; \
	fi
endif

# Build raylib from source
raylib:
ifeq ($(RAYLIB_SHARED),0)
	$(call print_status,"Building raylib...")
ifeq ($(TARGET_PLATFORM),windows)
	cd $(RAYLIB_SRC) && $(MAKE) CC=$(CC) AR=$(AR) \
		PLATFORM=PLATFORM_DESKTOP PLATFORM_OS=WINDOWS RAYLIB_LIBTYPE=STATIC
else
	cd $(RAYLIB_SRC) && $(MAKE) PLATFORM=PLATFORM_DESKTOP RAYLIB_LIBTYPE=STATIC
endif
	@echo "$(TARGET_PLATFORM)" > $(RAYLIB_PLATFORM_MARKER)
else
	$(call print_warning,"RAYLIB_SHARED=1 - using system raylib, skipping build")
endif

# Clean raylib build
raylib-clean:
ifeq ($(RAYLIB_SHARED),0)
	cd $(RAYLIB_SRC) && $(MAKE) clean
	@$(RM) $(RAYLIB_PLATFORM_MARKER)
endif

# =============================================================================
# Library Targets
# =============================================================================

# Note: 'lib' target uses recursive make to ensure generate runs first
lib: raylib-check platform-check generate
	@$(MAKE) --no-print-directory _lib

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
ifeq ($(TARGET_PLATFORM),windows)
	$(CC) $(LIB_LDFLAGS) -Wl,--out-implib,$(LIBOUTDIR)/$(LIB_IMPORT) \
		-o $(LIBOUTDIR)/$(LIB_SHARED) $(OBJECTS) $(ALL_LIBS)
else
	$(CC) $(LIB_LDFLAGS) -o $(LIBOUTDIR)/$(LIB_SHARED_VERSION) $(OBJECTS) $(ALL_LIBS)
	cd $(LIBOUTDIR) && ln -sf $(LIB_SHARED_VERSION) $(LIB_SHARED_SONAME)
	cd $(LIBOUTDIR) && ln -sf $(LIB_SHARED_SONAME) $(LIB_SHARED)
endif

# =============================================================================
# Generated Files
# =============================================================================

# Generate version header from template
src/grl-version.h: src/grl-version.h.in config.mk
	$(call print_status,"Generating grl-version.h")
	$(SED) -e 's/@VERSION_MAJOR@/$(VERSION_MAJOR)/g' \
	       -e 's/@VERSION_MINOR@/$(VERSION_MINOR)/g' \
	       -e 's/@VERSION_MICRO@/$(VERSION_MICRO)/g' \
	       -e 's/@VERSION@/$(VERSION)/g' \
	       -e 's/@API_VERSION@/$(API_VERSION)/g' \
	       $< > $@

# Generate config header from template
src/config.h: src/config.h.in config.mk
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
	$(MKDIR_P) $(DESTDIR)$(INCLUDEDIR)/graylib/ui
	$(MKDIR_P) $(DESTDIR)$(INCLUDEDIR)/graylib/rlgl
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
	$(INSTALL_DATA) src/graylib.h $(DESTDIR)$(INCLUDEDIR)/graylib/
	$(INSTALL_DATA) src/grl-version.h $(DESTDIR)$(INCLUDEDIR)/graylib/
	$(INSTALL_DATA) src/grl-types.h $(DESTDIR)$(INCLUDEDIR)/graylib/
	$(INSTALL_DATA) src/grl-enums.h $(DESTDIR)$(INCLUDEDIR)/graylib/
	$(INSTALL_DATA) src/math/grl-vector2.h $(DESTDIR)$(INCLUDEDIR)/graylib/math/
	$(INSTALL_DATA) src/math/grl-vector3.h $(DESTDIR)$(INCLUDEDIR)/graylib/math/
	$(INSTALL_DATA) src/math/grl-vector4.h $(DESTDIR)$(INCLUDEDIR)/graylib/math/
	$(INSTALL_DATA) src/math/grl-matrix.h $(DESTDIR)$(INCLUDEDIR)/graylib/math/
	$(INSTALL_DATA) src/math/grl-bounding-box.h $(DESTDIR)$(INCLUDEDIR)/graylib/math/
	$(INSTALL_DATA) src/math/grl-color.h $(DESTDIR)$(INCLUDEDIR)/graylib/math/
	$(INSTALL_DATA) src/math/grl-rectangle.h $(DESTDIR)$(INCLUDEDIR)/graylib/math/
	$(INSTALL_DATA) src/math/grl-quaternion.h $(DESTDIR)$(INCLUDEDIR)/graylib/math/
	$(INSTALL_DATA) src/math/grl-math-utils.h $(DESTDIR)$(INCLUDEDIR)/graylib/math/
	$(INSTALL_DATA) src/core/grl-application.h $(DESTDIR)$(INCLUDEDIR)/graylib/core/
	$(INSTALL_DATA) src/core/grl-window.h $(DESTDIR)$(INCLUDEDIR)/graylib/core/
	$(INSTALL_DATA) src/core/grl-input.h $(DESTDIR)$(INCLUDEDIR)/graylib/core/
	$(INSTALL_DATA) src/graphics/grl-image.h $(DESTDIR)$(INCLUDEDIR)/graylib/graphics/
	$(INSTALL_DATA) src/graphics/grl-texture.h $(DESTDIR)$(INCLUDEDIR)/graylib/graphics/
	$(INSTALL_DATA) src/graphics/grl-font.h $(DESTDIR)$(INCLUDEDIR)/graylib/graphics/
	$(INSTALL_DATA) src/graphics/grl-camera2d.h $(DESTDIR)$(INCLUDEDIR)/graylib/graphics/
	$(INSTALL_DATA) src/graphics/grl-camera3d.h $(DESTDIR)$(INCLUDEDIR)/graylib/graphics/
	$(INSTALL_DATA) src/graphics/grl-shader.h $(DESTDIR)$(INCLUDEDIR)/graylib/graphics/
	$(INSTALL_DATA) src/graphics/grl-render-texture.h $(DESTDIR)$(INCLUDEDIR)/graylib/graphics/
	$(INSTALL_DATA) src/graphics/grl-mesh.h $(DESTDIR)$(INCLUDEDIR)/graylib/graphics/
	$(INSTALL_DATA) src/graphics/grl-material.h $(DESTDIR)$(INCLUDEDIR)/graylib/graphics/
	$(INSTALL_DATA) src/graphics/grl-model.h $(DESTDIR)$(INCLUDEDIR)/graylib/graphics/
	$(INSTALL_DATA) src/graphics/grl-model-animation.h $(DESTDIR)$(INCLUDEDIR)/graylib/graphics/
	$(INSTALL_DATA) src/graphics/grl-png.h $(DESTDIR)$(INCLUDEDIR)/graylib/graphics/
	$(INSTALL_DATA) src/drawing/grl-draw.h $(DESTDIR)$(INCLUDEDIR)/graylib/drawing/
	$(INSTALL_DATA) src/audio/grl-audio-device.h $(DESTDIR)$(INCLUDEDIR)/graylib/audio/
	$(INSTALL_DATA) src/audio/grl-wave.h $(DESTDIR)$(INCLUDEDIR)/graylib/audio/
	$(INSTALL_DATA) src/audio/grl-sound.h $(DESTDIR)$(INCLUDEDIR)/graylib/audio/
	$(INSTALL_DATA) src/audio/grl-music.h $(DESTDIR)$(INCLUDEDIR)/graylib/audio/
	$(INSTALL_DATA) src/audio/grl-audio-stream.h $(DESTDIR)$(INCLUDEDIR)/graylib/audio/
	$(INSTALL_DATA) src/scene/grl-drawable.h $(DESTDIR)$(INCLUDEDIR)/graylib/scene/
	$(INSTALL_DATA) src/scene/grl-updatable.h $(DESTDIR)$(INCLUDEDIR)/graylib/scene/
	$(INSTALL_DATA) src/scene/grl-collidable.h $(DESTDIR)$(INCLUDEDIR)/graylib/scene/
	$(INSTALL_DATA) src/scene/grl-entity.h $(DESTDIR)$(INCLUDEDIR)/graylib/scene/
	$(INSTALL_DATA) src/scene/grl-sprite.h $(DESTDIR)$(INCLUDEDIR)/graylib/scene/
	$(INSTALL_DATA) src/scene/grl-animated-texture.h $(DESTDIR)$(INCLUDEDIR)/graylib/scene/
	$(INSTALL_DATA) src/scene/grl-scene.h $(DESTDIR)$(INCLUDEDIR)/graylib/scene/
	$(INSTALL_DATA) src/scene/grl-scene-manager.h $(DESTDIR)$(INCLUDEDIR)/graylib/scene/
	$(INSTALL_DATA) src/collision/grl-collision.h $(DESTDIR)$(INCLUDEDIR)/graylib/collision/
	$(INSTALL_DATA) src/ui/grl-ui-enums.h $(DESTDIR)$(INCLUDEDIR)/graylib/ui/
	$(INSTALL_DATA) src/ui/grl-ui-control.h $(DESTDIR)$(INCLUDEDIR)/graylib/ui/
	$(INSTALL_DATA) src/ui/grl-ui-label.h $(DESTDIR)$(INCLUDEDIR)/graylib/ui/
	$(INSTALL_DATA) src/ui/grl-ui-button.h $(DESTDIR)$(INCLUDEDIR)/graylib/ui/
	$(INSTALL_DATA) src/ui/grl-ui-checkbox.h $(DESTDIR)$(INCLUDEDIR)/graylib/ui/
	$(INSTALL_DATA) src/ui/grl-ui-slider.h $(DESTDIR)$(INCLUDEDIR)/graylib/ui/
	$(INSTALL_DATA) src/ui/grl-ui-textbox.h $(DESTDIR)$(INCLUDEDIR)/graylib/ui/
	$(INSTALL_DATA) src/ui/grl-ui-toggle.h $(DESTDIR)$(INCLUDEDIR)/graylib/ui/
	$(INSTALL_DATA) src/ui/grl-ui-progressbar.h $(DESTDIR)$(INCLUDEDIR)/graylib/ui/
	$(INSTALL_DATA) src/ui/grl-ui-spinner.h $(DESTDIR)$(INCLUDEDIR)/graylib/ui/
	$(INSTALL_DATA) src/ui/grl-ui-valuebox.h $(DESTDIR)$(INCLUDEDIR)/graylib/ui/
	$(INSTALL_DATA) src/ui/grl-ui-combobox.h $(DESTDIR)$(INCLUDEDIR)/graylib/ui/
	$(INSTALL_DATA) src/ui/grl-ui-dropdownbox.h $(DESTDIR)$(INCLUDEDIR)/graylib/ui/
	$(INSTALL_DATA) src/ui/grl-ui-togglegroup.h $(DESTDIR)$(INCLUDEDIR)/graylib/ui/
	$(INSTALL_DATA) src/ui/grl-ui-listview.h $(DESTDIR)$(INCLUDEDIR)/graylib/ui/
	$(INSTALL_DATA) src/ui/grl-ui-colorpicker.h $(DESTDIR)$(INCLUDEDIR)/graylib/ui/
	$(INSTALL_DATA) src/ui/grl-ui-panel.h $(DESTDIR)$(INCLUDEDIR)/graylib/ui/
	$(INSTALL_DATA) src/ui/grl-ui-groupbox.h $(DESTDIR)$(INCLUDEDIR)/graylib/ui/
	$(INSTALL_DATA) src/ui/grl-ui-windowbox.h $(DESTDIR)$(INCLUDEDIR)/graylib/ui/
	$(INSTALL_DATA) src/rlgl/grl-rlgl.h $(DESTDIR)$(INCLUDEDIR)/graylib/rlgl/
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
	$(RM) src/grl-version.h
	$(RM) src/config.h
	$(RM) deps/raylib/src/.graylib-platform

distclean: clean raylib-clean
	$(call print_status,"Cleaning all generated files...")
	$(RM) src/grl-enums.c
	$(RM) src/grl-enums.h

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
$(OBJECTS): src/grl-version.h src/config.h

# Auto-generated dependency tracking disabled to avoid build ordering issues
# TODO: Re-enable once a proper solution is found
# -include $(DEPENDS)

# =============================================================================
# Object File Rules
# =============================================================================

# Math module
$(OBJDIR)/src/math/grl-vector2.o: src/math/grl-vector2.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/src/math/grl-color.o: src/math/grl-color.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/src/math/grl-rectangle.o: src/math/grl-rectangle.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/src/math/grl-vector3.o: src/math/grl-vector3.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/src/math/grl-vector4.o: src/math/grl-vector4.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/src/math/grl-matrix.o: src/math/grl-matrix.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/src/math/grl-quaternion.o: src/math/grl-quaternion.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/src/math/grl-bounding-box.o: src/math/grl-bounding-box.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/src/math/grl-math-utils.o: src/math/grl-math-utils.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

# Core module
$(OBJDIR)/src/core/grl-application.o: src/core/grl-application.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/src/core/grl-window.o: src/core/grl-window.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/src/core/grl-input.o: src/core/grl-input.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

# Graphics module
$(OBJDIR)/src/graphics/grl-image.o: src/graphics/grl-image.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/src/graphics/grl-texture.o: src/graphics/grl-texture.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/src/graphics/grl-font.o: src/graphics/grl-font.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/src/graphics/grl-camera2d.o: src/graphics/grl-camera2d.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/src/graphics/grl-camera3d.o: src/graphics/grl-camera3d.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/src/graphics/grl-shader.o: src/graphics/grl-shader.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/src/graphics/grl-render-texture.o: src/graphics/grl-render-texture.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/src/graphics/grl-mesh.o: src/graphics/grl-mesh.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/src/graphics/grl-material.o: src/graphics/grl-material.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/src/graphics/grl-model.o: src/graphics/grl-model.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/src/graphics/grl-model-animation.o: src/graphics/grl-model-animation.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

# Drawing module
$(OBJDIR)/src/drawing/grl-draw-shapes.o: src/drawing/grl-draw-shapes.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/src/drawing/grl-draw-textures.o: src/drawing/grl-draw-textures.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/src/drawing/grl-draw-text.o: src/drawing/grl-draw-text.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/src/drawing/grl-draw-3d.o: src/drawing/grl-draw-3d.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

# Audio module
$(OBJDIR)/src/audio/grl-audio-device.o: src/audio/grl-audio-device.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/src/audio/grl-wave.o: src/audio/grl-wave.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/src/audio/grl-sound.o: src/audio/grl-sound.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/src/audio/grl-music.o: src/audio/grl-music.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/src/audio/grl-audio-stream.o: src/audio/grl-audio-stream.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

# Scene module
$(OBJDIR)/src/scene/grl-drawable.o: src/scene/grl-drawable.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/src/scene/grl-updatable.o: src/scene/grl-updatable.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/src/scene/grl-collidable.o: src/scene/grl-collidable.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/src/scene/grl-entity.o: src/scene/grl-entity.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/src/scene/grl-sprite.o: src/scene/grl-sprite.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/src/scene/grl-animated-texture.o: src/scene/grl-animated-texture.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/src/scene/grl-scene.o: src/scene/grl-scene.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/src/scene/grl-scene-manager.o: src/scene/grl-scene-manager.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

# Collision
$(OBJDIR)/src/collision/grl-collision.o: src/collision/grl-collision.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

# UI module
$(OBJDIR)/src/ui/grl-ui-enums.o: src/ui/grl-ui-enums.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/src/ui/grl-ui-control.o: src/ui/grl-ui-control.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

# rpng needs C99 and its include path
# grl-png.c contains RPNG_IMPLEMENTATION (not DEFLATE, raylib already has sdefl/sinfl)
RPNG_SRC := $(CURDIR)/deps/rpng/src
RPNG_CFLAGS := $(subst -std=gnu89,-std=gnu99,$(LIB_CFLAGS)) -isystem $(RPNG_SRC)

$(OBJDIR)/src/graphics/grl-png.o: src/graphics/grl-png.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(RPNG_CFLAGS) -c -o $@ $<

# grl-image.c also uses rpng for indexed PNG functions
$(OBJDIR)/src/graphics/grl-image.o: src/graphics/grl-image.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(RPNG_CFLAGS) -c -o $@ $<

# rres needs C99 and its include path
# grl-resource-pack.c contains RRES_IMPLEMENTATION
RRES_SRC := $(CURDIR)/deps/rres/src
RRES_CFLAGS := $(subst -std=gnu89,-std=gnu99,$(LIB_CFLAGS)) -isystem $(RRES_SRC)

$(OBJDIR)/src/resources/grl-resource-pack.o: src/resources/grl-resource-pack.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(RRES_CFLAGS) -c -o $@ $<

# UI controls need raygui include path and C99 (raygui uses C99 features)
# grl-ui-label.c contains RAYGUI_IMPLEMENTATION
RAYGUI_SRC := $(CURDIR)/deps/raygui/src
UI_CFLAGS := $(subst -std=gnu89,-std=gnu99,$(LIB_CFLAGS)) -isystem $(RAYGUI_SRC)

$(OBJDIR)/src/ui/grl-ui-label.o: src/ui/grl-ui-label.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(UI_CFLAGS) -c -o $@ $<

$(OBJDIR)/src/ui/grl-ui-button.o: src/ui/grl-ui-button.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(UI_CFLAGS) -c -o $@ $<

$(OBJDIR)/src/ui/grl-ui-checkbox.o: src/ui/grl-ui-checkbox.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(UI_CFLAGS) -c -o $@ $<

$(OBJDIR)/src/ui/grl-ui-slider.o: src/ui/grl-ui-slider.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(UI_CFLAGS) -c -o $@ $<

$(OBJDIR)/src/ui/grl-ui-textbox.o: src/ui/grl-ui-textbox.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(UI_CFLAGS) -c -o $@ $<

$(OBJDIR)/src/ui/grl-ui-toggle.o: src/ui/grl-ui-toggle.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(UI_CFLAGS) -c -o $@ $<

$(OBJDIR)/src/ui/grl-ui-progressbar.o: src/ui/grl-ui-progressbar.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(UI_CFLAGS) -c -o $@ $<

$(OBJDIR)/src/ui/grl-ui-spinner.o: src/ui/grl-ui-spinner.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(UI_CFLAGS) -c -o $@ $<

$(OBJDIR)/src/ui/grl-ui-valuebox.o: src/ui/grl-ui-valuebox.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(UI_CFLAGS) -c -o $@ $<

$(OBJDIR)/src/ui/grl-ui-combobox.o: src/ui/grl-ui-combobox.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(UI_CFLAGS) -c -o $@ $<

$(OBJDIR)/src/ui/grl-ui-dropdownbox.o: src/ui/grl-ui-dropdownbox.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(UI_CFLAGS) -c -o $@ $<

$(OBJDIR)/src/ui/grl-ui-togglegroup.o: src/ui/grl-ui-togglegroup.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(UI_CFLAGS) -c -o $@ $<

$(OBJDIR)/src/ui/grl-ui-listview.o: src/ui/grl-ui-listview.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(UI_CFLAGS) -c -o $@ $<

$(OBJDIR)/src/ui/grl-ui-colorpicker.o: src/ui/grl-ui-colorpicker.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(UI_CFLAGS) -c -o $@ $<

$(OBJDIR)/src/ui/grl-ui-panel.o: src/ui/grl-ui-panel.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(UI_CFLAGS) -c -o $@ $<

$(OBJDIR)/src/ui/grl-ui-groupbox.o: src/ui/grl-ui-groupbox.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(UI_CFLAGS) -c -o $@ $<

$(OBJDIR)/src/ui/grl-ui-windowbox.o: src/ui/grl-ui-windowbox.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(UI_CFLAGS) -c -o $@ $<

$(OBJDIR)/src/ui/grl-ui-style.o: src/ui/grl-ui-style.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(UI_CFLAGS) -c -o $@ $<

# Enums
$(OBJDIR)/src/grl-enums.o: src/grl-enums.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

# RLGL module
$(OBJDIR)/src/rlgl/grl-rlgl-core.o: src/rlgl/grl-rlgl-core.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/src/rlgl/grl-rlgl-matrix.o: src/rlgl/grl-rlgl-matrix.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/src/rlgl/grl-rlgl-vertex.o: src/rlgl/grl-rlgl-vertex.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/src/rlgl/grl-rlgl-state.o: src/rlgl/grl-rlgl-state.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/src/rlgl/grl-rlgl-texture.o: src/rlgl/grl-rlgl-texture.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/src/rlgl/grl-rlgl-buffer.o: src/rlgl/grl-rlgl-buffer.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/src/rlgl/grl-rlgl-framebuffer.o: src/rlgl/grl-rlgl-framebuffer.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(OBJDIR)/src/rlgl/grl-rlgl-shader.o: src/rlgl/grl-rlgl-shader.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

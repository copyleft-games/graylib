# rules.mk - Common Build Rules for Graylib
#
# Copyright 2025 Zach Podbielniak
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# This file contains common build rules and flag configurations.
# Include this after config.mk in all Makefiles.

# =============================================================================
# Dependency Resolution
# =============================================================================

# GLib/GObject dependencies (required)
# Use -isystem instead of -I to suppress warnings from system headers
GLIB_CFLAGS := $(shell $(PKG_CONFIG) --cflags glib-2.0 gobject-2.0 gio-2.0 | sed 's/-I/-isystem /g')
GLIB_LIBS := $(shell $(PKG_CONFIG) --libs glib-2.0 gobject-2.0 gio-2.0)
GLIB_VERSION := $(shell $(PKG_CONFIG) --modversion glib-2.0)

# Raylib dependency
ifeq ($(RAYLIB_SHARED),1)
    # Link against system raylib
    RAYLIB_CFLAGS := $(shell $(PKG_CONFIG) --cflags raylib 2>/dev/null)
    RAYLIB_LIBS := $(shell $(PKG_CONFIG) --libs raylib 2>/dev/null)
    RAYLIB_VERSION := $(shell $(PKG_CONFIG) --modversion raylib 2>/dev/null)
    ifeq ($(RAYLIB_LIBS),)
        $(error System raylib not found. Install raylib or use RAYLIB_SHARED=0)
    endif
else
    # Use embedded raylib from deps/
    # Use -isystem to suppress warnings from raylib headers (raymath.h has C99 code)
    RAYLIB_CFLAGS := -isystem $(RAYLIB_SRC)
    RAYLIB_LIBS := $(RAYLIB_SRC)/libraylib.a
    RAYLIB_VERSION := 6.0
endif

# The extra WAV-context free is valid ONLY for this unmodified bundled SDK.
# Unknown/system SDKs keep their native ownership contract (no extra free).
# Checking both revision and bytes prevents a future fixed decoder from being
# freed twice merely because it still advertises raylib 6.0.
override GRL_SDK_WAV_EXTRA_FREE := 0
ifeq ($(RAYLIB_SHARED),0)
override _GRL_SDK_REV := $(shell git -C "$(RAYLIB_SRC)" rev-parse HEAD 2>/dev/null)
override _GRL_SDK_CLEAN := $(shell git -C "$(RAYLIB_SRC)/.." diff --quiet HEAD -- . 2>/dev/null && printf clean)
override _GRL_SDK_AUDIO_SHA := $(shell sha256sum "$(RAYLIB_SRC)/raudio.c" 2>/dev/null | sed 's/ .*//')
override _GRL_SDK_HEADER_SHA := $(shell sha256sum "$(RAYLIB_SRC)/raylib.h" 2>/dev/null | sed 's/ .*//')
ifeq ($(_GRL_SDK_REV),dbc56a87da87d973a9c5baa4e7438a9d20121d28)
ifeq ($(_GRL_SDK_CLEAN),clean)
ifeq ($(_GRL_SDK_AUDIO_SHA),2a5b42a701fd872dbf6a9715964344ce44b891bdcd7643122774d5cac9589812)
ifeq ($(_GRL_SDK_HEADER_SHA),047e7255f93f8c34039cab906ad76136706b5c7b4c5b5b065d84141963ee9b6b)
override GRL_SDK_WAV_EXTRA_FREE := 1
endif
endif
endif
endif
endif

# =============================================================================
# GLFW Wayland backend (see WAYLAND in config.mk)
# =============================================================================

GRAYLIB_WAYLAND_PKGS := wayland-client wayland-cursor wayland-egl xkbcommon
GRAYLIB_WAYLAND := 0
ifeq ($(TARGET_PLATFORM),linux)
ifeq ($(RAYLIB_SHARED),0)
    ifeq ($(WAYLAND),auto)
        GRAYLIB_WAYLAND := $(shell $(PKG_CONFIG) --exists $(GRAYLIB_WAYLAND_PKGS) && \
                                  command -v wayland-scanner >/dev/null 2>&1 && echo 1 || echo 0)
    else ifeq ($(WAYLAND),1)
        GRAYLIB_WAYLAND := 1
    endif
endif
endif

# raylib's own switch, and the tag that forces a rebuild when it changes
ifeq ($(GRAYLIB_WAYLAND),1)
    RAYLIB_GLFW_FLAGS := GLFW_LINUX_ENABLE_WAYLAND=TRUE
    RAYLIB_PLATFORM_TAG := $(TARGET_PLATFORM)+wayland
else
    RAYLIB_GLFW_FLAGS :=
    RAYLIB_PLATFORM_TAG := $(TARGET_PLATFORM)
endif

# raylib's own Makefile builds the desktop RELEASE configuration at -O1.
# CUSTOM_CFLAGS is appended after its optimisation flag, so -O2 here wins
# (raylib's sources are untouched). The tag change forces one rebuild.
#
# -fPIC: the static raylib is linked into libgraylib.so, so it must be
# position-independent. raylib's Makefile adds -fPIC only for shared
# builds; this worked where gcc defaults to PIE (Fedora) and failed where it
# does not (FreeBSD's gcc: "relocation R_X86_64_32 ... recompile with
# -fPIC"). It lives here, not as a second CUSTOM_CFLAGS= on raylib's command
# line, because the last CUSTOM_CFLAGS= given wins. Windows (MinGW) builds
# a static archive only and has no use for it.
ifeq ($(TARGET_PLATFORM),windows)
RAYLIB_OPT_FLAGS := CUSTOM_CFLAGS=-O2
RAYLIB_PLATFORM_TAG := $(RAYLIB_PLATFORM_TAG)+O2
else
RAYLIB_OPT_FLAGS := CUSTOM_CFLAGS="-O2 -fPIC"
RAYLIB_PLATFORM_TAG := $(RAYLIB_PLATFORM_TAG)+O2+pic
endif

# External SDK configuration is an actual build input, not a feature policy
# of graylib. Distinguish configured archives from vanilla archives and let
# consumers name included headers whose content must invalidate that archive.
RAYLIB_CONFIG_INPUTS ?=
ifneq ($(strip $(RAYLIB_CONFIG_FLAGS) $(RAYLIB_CONFIG_INPUTS)),)
_RAYLIB_MISSING_CONFIG_INPUTS := $(filter-out $(wildcard $(RAYLIB_CONFIG_INPUTS)),$(RAYLIB_CONFIG_INPUTS))
ifneq ($(_RAYLIB_MISSING_CONFIG_INPUTS),)
$(error Missing explicit RAYLIB_CONFIG_INPUTS: $(_RAYLIB_MISSING_CONFIG_INPUTS))
endif
_RAYLIB_CONFIG_DIGEST := $(shell printf '%s\n' '$(RAYLIB_CONFIG_FLAGS)' '$(RAYLIB_CONFIG_INPUTS)' '$(foreach input,$(RAYLIB_CONFIG_INPUTS),$(shell sha256sum "$(input)"))' | sha256sum | sed 's/ .*//')
RAYLIB_PLATFORM_TAG := $(RAYLIB_PLATFORM_TAG)+cfg$(_RAYLIB_CONFIG_DIGEST)
endif

# =============================================================================
# Platform-Specific System Libraries
# =============================================================================
# These are the system libraries required by raylib on each platform.
# Exported as PLATFORM_LIBS for use by tests/examples Makefiles.

ifeq ($(TARGET_PLATFORM),windows)
    # Windows libraries (MinGW)
    PLATFORM_LIBS := -lopengl32 -lgdi32 -lwinmm -lshell32
else ifeq ($(TARGET_PLATFORM),linux)
    # Linux libraries
    PLATFORM_LIBS := -lGL -lm -lpthread -ldl -lrt -lX11
    ifeq ($(GRAYLIB_WAYLAND),1)
        PLATFORM_LIBS += $(shell $(PKG_CONFIG) --libs $(GRAYLIB_WAYLAND_PKGS))
    endif
else ifeq ($(TARGET_PLATFORM),macos)
    # macOS libraries (untested)
    PLATFORM_LIBS := -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
else
    # Fallback
    PLATFORM_LIBS := -lGL -lm
endif

# Add platform libs to raylib when using embedded build
ifeq ($(RAYLIB_SHARED),0)
    RAYLIB_LIBS += $(PLATFORM_LIBS)
endif

# Math library
MATH_LIBS := -lm

# =============================================================================
# Compiler Flag Assembly
# =============================================================================

# Base CFLAGS
BASE_CFLAGS := -std=$(CSTD)
BASE_CFLAGS += $(WARN_CFLAGS)
BASE_CFLAGS += $(FEATURE_CFLAGS)
BASE_CFLAGS += $(GLIB_CFLAGS)
BASE_CFLAGS += $(RAYLIB_CFLAGS)

# Debug vs Release flags
ifeq ($(DEBUG),1)
    OPT_CFLAGS := -g3 -O0 -DGRL_DEBUG=1
    OPT_CFLAGS += -fno-omit-frame-pointer
    OPT_CFLAGS += -fno-inline
    # Sanitizers (optional)
    ifeq ($(ASAN),1)
        OPT_CFLAGS += -fsanitize=address
        OPT_LDFLAGS += -fsanitize=address
    endif
    ifeq ($(UBSAN),1)
        OPT_CFLAGS += -fsanitize=undefined
        OPT_LDFLAGS += -fsanitize=undefined
    endif
else
    OPT_CFLAGS := -O2 -DNDEBUG
    # Calls between graylib's own functions may be inlined and bound
    # locally even though the library is built -fPIC.
    OPT_CFLAGS += -fno-semantic-interposition
    # GObject cast macros (GRL_ENTITY (x) ...) compile to plain casts in
    # release builds; g_return_if_fail() argument checks stay on.
    OPT_CFLAGS += -DG_DISABLE_CAST_CHECKS
    # Link-time optimization (optional, can be slow)
    # OPT_CFLAGS += -flto
    # OPT_LDFLAGS += -flto
endif

# Library compilation flags
LIB_CFLAGS := $(BASE_CFLAGS) $(OPT_CFLAGS) $(PIC_CFLAGS) $(VISIBILITY_CFLAGS)
LIB_CFLAGS += -DGRAYLIB_COMPILATION

# Include paths for library source
LIB_CFLAGS += -I$(CURDIR)
LIB_CFLAGS += -I$(CURDIR)/src

# Executable compilation flags (tests, examples)
EXE_CFLAGS := $(BASE_CFLAGS) $(OPT_CFLAGS)
EXE_CFLAGS += -I$(CURDIR)
EXE_CFLAGS += -I$(CURDIR)/src

# =============================================================================
# Linker Flag Assembly
# =============================================================================

# Base LDFLAGS
BASE_LDFLAGS := $(OPT_LDFLAGS)

# Library linking
LIB_LDFLAGS := $(BASE_LDFLAGS)
LIB_LDFLAGS += -shared
ifeq ($(TARGET_PLATFORM),windows)
    # Windows: export all symbols (including embedded raylib/rlgl)
    LIB_LDFLAGS += -Wl,--export-all-symbols
else
    # Unix: use soname for versioning
    LIB_LDFLAGS += -Wl,-soname,$(LIB_SHARED_SONAME)
endif
# Note: Windows import library (--out-implib) is added in Makefile rule
# because LIBOUTDIR is not yet defined at this point

# All libraries to link
ALL_LIBS := $(GLIB_LIBS) $(RAYLIB_LIBS) $(MATH_LIBS)

# =============================================================================
# GObject Introspection Configuration
# =============================================================================

ifeq ($(BUILD_GIR),1)
    GIR_SCANNER_FLAGS := --warn-all --warn-error
    GIR_SCANNER_FLAGS += --namespace=Graylib
    GIR_SCANNER_FLAGS += --nsversion=$(API_VERSION)
    GIR_SCANNER_FLAGS += --identifier-prefix=Grl
    GIR_SCANNER_FLAGS += --symbol-prefix=grl
    GIR_SCANNER_FLAGS += --include=GObject-2.0
    GIR_SCANNER_FLAGS += --include=Gio-2.0
    GIR_SCANNER_FLAGS += --pkg=glib-2.0
    GIR_SCANNER_FLAGS += --pkg=gobject-2.0
    GIR_SCANNER_FLAGS += --pkg=gio-2.0
    GIR_SCANNER_FLAGS += -I$(CURDIR)
    GIR_SCANNER_FLAGS += -I$(CURDIR)/src
    GIR_SCANNER_FLAGS += -DGRAYLIB_COMPILATION

    GIR_COMPILER_FLAGS := --includedir=$(CURDIR)
endif

# =============================================================================
# Build Output Directories
# =============================================================================

BUILDDIR := $(CURDIR)/build
OBJDIR := $(BUILDDIR)/obj
LIBOUTDIR := $(BUILDDIR)/lib
BINDIR_BUILD := $(BUILDDIR)/bin
EXAMPLEOUTDIR := $(BUILDDIR)/examples
GIROUTDIR := $(BUILDDIR)/gir

# Create build directories
$(OBJDIR) $(LIBOUTDIR) $(BINDIR_BUILD) $(EXAMPLEOUTDIR) $(GIROUTDIR):
	$(MKDIR_P) $@

# =============================================================================
# Common Pattern Rules
# =============================================================================

# Compile C source to object file
$(OBJDIR)/%.o: %.c | $(OBJDIR)
	@$(MKDIR_P) $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

# Dependency generation (auto-dependency tracking)
$(OBJDIR)/%.d: %.c | $(OBJDIR)
	@$(MKDIR_P) $(dir $@)
	@$(CC) $(LIB_CFLAGS) -MM -MT '$(OBJDIR)/$*.o $(OBJDIR)/$*.d' $< > $@

# =============================================================================
# Utility Functions
# =============================================================================

# Print a colored status message
define print_status
	@printf "\033[1;32m==>\033[0m \033[1m%s\033[0m\n" $(1)
endef

# Print a warning message
define print_warning
	@printf "\033[1;33mWarning:\033[0m %s\n" $(1)
endef

# Print an error message
define print_error
	@printf "\033[1;31mError:\033[0m %s\n" $(1)
endef

# =============================================================================
# Phony Targets
# =============================================================================

.PHONY: all clean distclean install uninstall
.PHONY: lib lib-static lib-shared
.PHONY: gir typelib
.PHONY: test tests check
.PHONY: examples
.PHONY: docs
.PHONY: raylib raylib-check raylib-clean
.PHONY: platform-check
.PHONY: help info debug-build

# =============================================================================
# Help Target
# =============================================================================

help:
	@echo "Graylib Build System"
	@echo "===================="
	@echo ""
	@echo "Main targets:"
	@echo "  all          - Build library (default)"
	@echo "  install      - Install to PREFIX ($(PREFIX))"
	@echo "  uninstall    - Remove installed files"
	@echo "  clean        - Remove build artifacts"
	@echo "  distclean    - Remove all generated files"
	@echo ""
	@echo "Build targets:"
	@echo "  lib          - Build both static and shared libraries"
	@echo "  lib-static   - Build static library only"
	@echo "  lib-shared   - Build shared library only"
	@echo "  gir          - Generate GObject Introspection files"
	@echo "  test         - Build and run unit tests"
	@echo "  examples     - Build example programs"
	@echo "  docs         - Build documentation"
	@echo "  raylib       - Build raylib from deps/"
	@echo ""
	@echo "Build options (set on command line):"
	@echo "  DEBUG=1          - Build with debug symbols for gdb"
	@echo "  RAYLIB_SHARED=1  - Link against system raylib"
	@echo "  ASAN=1           - Enable AddressSanitizer (requires DEBUG=1)"
	@echo "  UBSAN=1          - Enable UBSan (requires DEBUG=1)"
	@echo "  PREFIX=/path     - Installation prefix (default: /usr/local)"
	@echo ""
	@echo "Examples:"
	@echo "  make                     - Build release library"
	@echo "  make DEBUG=1             - Build debug library"
	@echo "  make RAYLIB_SHARED=1     - Use system raylib"
	@echo "  make install PREFIX=/usr - Install to /usr"
	@echo "  make test                - Run unit tests"

# =============================================================================
# Info Target (Debug Build Configuration)
# =============================================================================

info:
	@echo "Graylib Build Configuration"
	@echo "==========================="
	@echo ""
	@echo "Version:        $(VERSION)"
	@echo "API Version:    $(API_VERSION)"
	@echo "Platform:       $(PLATFORM) ($(UNAME_M))"
	@echo ""
	@echo "Directories:"
	@echo "  PREFIX:       $(PREFIX)"
	@echo "  LIBDIR:       $(LIBDIR)"
	@echo "  INCLUDEDIR:   $(INCLUDEDIR)"
	@echo ""
	@echo "Build Options:"
	@echo "  DEBUG:        $(DEBUG)"
	@echo "  RAYLIB_SHARED:$(RAYLIB_SHARED)"
	@echo "  BUILD_SHARED: $(BUILD_SHARED)"
	@echo "  BUILD_STATIC: $(BUILD_STATIC)"
	@echo "  BUILD_GIR:    $(BUILD_GIR)"
	@echo ""
	@echo "Dependencies:"
	@echo "  GLib:         $(GLIB_VERSION)"
	@echo "  Raylib:       $(RAYLIB_VERSION)"
	@echo ""
	@echo "Compiler:"
	@echo "  CC:           $(CC)"
	@echo "  CFLAGS:       $(LIB_CFLAGS)"

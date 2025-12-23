# config.mk - Graylib Build Configuration
#
# Copyright 2025 Zach Podbielniak
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# This file contains all configurable build options for Graylib.
# Override any option on the command line: make DEBUG=1 RAYLIB_SHARED=1

# =============================================================================
# Version Information
# =============================================================================

VERSION_MAJOR := 0
VERSION_MINOR := 1
VERSION_MICRO := 0
VERSION := $(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_MICRO)

# API version for parallel installability (bump on ABI breaks)
API_VERSION := 1

# Shared library versioning (libtool-style: current:revision:age)
# See: https://www.gnu.org/software/libtool/manual/html_node/Updating-version-info.html
SO_VERSION := 0
SO_MINOR := 0
SO_RELEASE := 0

# =============================================================================
# Installation Directories
# =============================================================================

PREFIX ?= /usr/local
EXEC_PREFIX ?= $(PREFIX)
BINDIR ?= $(EXEC_PREFIX)/bin
LIBDIR ?= $(EXEC_PREFIX)/lib
INCLUDEDIR ?= $(PREFIX)/include
DATADIR ?= $(PREFIX)/share
DOCDIR ?= $(DATADIR)/doc/graylib
GIRDIR ?= $(DATADIR)/gir-1.0
TYPELIBDIR ?= $(LIBDIR)/girepository-1.0
PKGCONFIGDIR ?= $(LIBDIR)/pkgconfig

# =============================================================================
# Build Options
# =============================================================================

# Build shared library (libgraylib.so)
BUILD_SHARED ?= 1

# Build static library (libgraylib.a)
BUILD_STATIC ?= 1

# Build GObject Introspection data (.gir and .typelib)
BUILD_GIR ?= 1

# Build unit tests
BUILD_TESTS ?= 1

# Build example programs
BUILD_EXAMPLES ?= 1

# Build documentation (requires gi-docgen)
BUILD_DOCS ?= 0

# =============================================================================
# Raylib Configuration
# =============================================================================

# Raylib linking mode:
#   0 = Static embed (default) - build raylib from deps/ and link statically
#   1 = Shared library - link against system-installed raylib
RAYLIB_SHARED ?= 0

# Path to raylib source (used when RAYLIB_SHARED=0)
RAYLIB_SRC ?= $(CURDIR)/deps/raylib/src

# =============================================================================
# Debug Configuration
# =============================================================================

# Debug build mode:
#   0 = Release build (-O2, no debug symbols)
#   1 = Debug build (-g3 -O0, full debug info for gdb)
DEBUG ?= 0

# Enable AddressSanitizer (requires DEBUG=1)
ASAN ?= 0

# Enable UndefinedBehaviorSanitizer (requires DEBUG=1)
UBSAN ?= 0

# =============================================================================
# Compiler and Tools
# =============================================================================

CC ?= gcc
AR ?= ar
RANLIB ?= ranlib
PKG_CONFIG ?= pkg-config
INSTALL ?= install
INSTALL_DATA ?= $(INSTALL) -m 644
INSTALL_PROGRAM ?= $(INSTALL) -m 755
SED ?= sed
MKDIR_P ?= mkdir -p
RM ?= rm -f
RMDIR ?= rm -rf

# GObject Introspection tools
GIR_SCANNER ?= g-ir-scanner
GIR_COMPILER ?= g-ir-compiler
GLIB_MKENUMS ?= glib-mkenums

# Documentation tools
GI_DOCGEN ?= gi-docgen

# =============================================================================
# Compiler Flags
# =============================================================================

# C standard (gnu89 as per user preference)
CSTD ?= gnu89

# Warning flags
WARN_CFLAGS := -Wall -Wextra -Wpedantic
WARN_CFLAGS += -Wformat=2 -Wformat-security
WARN_CFLAGS += -Wnull-dereference
WARN_CFLAGS += -Wstack-protector
WARN_CFLAGS += -Wstrict-prototypes
WARN_CFLAGS += -Wmissing-prototypes
WARN_CFLAGS += -Wold-style-definition
WARN_CFLAGS += -Wdeclaration-after-statement
WARN_CFLAGS += -Wno-unused-parameter

# Feature test macros
FEATURE_CFLAGS := -D_GNU_SOURCE

# Visibility for shared libraries
VISIBILITY_CFLAGS := -fvisibility=hidden

# Position independent code (required for shared libraries)
PIC_CFLAGS := -fPIC

# =============================================================================
# Platform Detection
# =============================================================================

UNAME_S := $(shell uname -s)
UNAME_M := $(shell uname -m)

ifeq ($(UNAME_S),Linux)
    PLATFORM := linux
    PLATFORM_LINUX := 1
else ifeq ($(UNAME_S),FreeBSD)
    PLATFORM := freebsd
    PLATFORM_FREEBSD := 1
else ifeq ($(UNAME_S),Darwin)
    PLATFORM := macos
    PLATFORM_MACOS := 1
else ifneq (,$(findstring MINGW,$(UNAME_S)))
    PLATFORM := windows
    PLATFORM_WINDOWS := 1
else ifneq (,$(findstring MSYS,$(UNAME_S)))
    PLATFORM := windows
    PLATFORM_WINDOWS := 1
else
    PLATFORM := unknown
endif

# =============================================================================
# Library Names
# =============================================================================

LIB_NAME := graylib
LIB_STATIC := lib$(LIB_NAME).a
LIB_SHARED := lib$(LIB_NAME).so
LIB_SHARED_VERSION := $(LIB_SHARED).$(SO_VERSION).$(SO_MINOR).$(SO_RELEASE)
LIB_SHARED_SONAME := $(LIB_SHARED).$(SO_VERSION)

# GIR output names
GIR_NAME := Graylib-$(API_VERSION).gir
TYPELIB_NAME := Graylib-$(API_VERSION).typelib

# pkg-config file
PC_FILE := graylib-$(API_VERSION).pc

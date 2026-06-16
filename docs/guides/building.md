# Building Graylib

This guide covers all aspects of building Graylib from source.

## Requirements

### Compiler

- GCC (GNU Compiler Collection)
- GNU Make

### Libraries

- GLib 2.56+
- GObject Introspection (for GIR generation)
- pkg-config

### For raylib (bundled)

When building raylib from the bundled submodule:

- OpenGL development headers
- X11 development libraries (Linux)
- Mesa libraries

## Installing Dependencies

### Fedora

```bash
sudo dnf install gcc make glib2-devel gobject-introspection-devel \
    mesa-libGL-devel libX11-devel libXrandr-devel \
    libXi-devel libXcursor-devel libXinerama-devel
```

### Ubuntu/Debian

```bash
sudo apt install build-essential libglib2.0-dev \
    gobject-introspection libgirepository1.0-dev \
    libgl1-mesa-dev libx11-dev libxrandr-dev \
    libxi-dev libxcursor-dev libxinerama-dev
```

### Arch Linux

```bash
sudo pacman -S gcc make glib2 gobject-introspection \
    mesa libx11 libxrandr libxi libxcursor libxinerama
```

## Build Configuration

Build options are configured in `config.mk`. You can override them on the command line:

### Version Settings

| Variable | Default | Description |
|----------|---------|-------------|
| `VERSION_MAJOR` | 0 | Major version number |
| `VERSION_MINOR` | 1 | Minor version number |
| `VERSION_MICRO` | 0 | Micro version number |
| `API_VERSION` | 1 | API version for GIR |

### Directory Settings

| Variable | Default | Description |
|----------|---------|-------------|
| `PREFIX` | `/usr/local` | Installation prefix |
| `LIBDIR` | `$(PREFIX)/lib` | Library installation directory |
| `INCLUDEDIR` | `$(PREFIX)/include` | Header installation directory |
| `PKGCONFIGDIR` | `$(LIBDIR)/pkgconfig` | pkg-config file directory |
| `GIRDIR` | `$(DATADIR)/gir-1.0` | GIR file directory |
| `TYPELIBDIR` | `$(LIBDIR)/girepository-1.0` | Typelib directory |

### Build Options

| Variable | Default | Description |
|----------|---------|-------------|
| `BUILD_SHARED` | 1 | Build shared library |
| `BUILD_STATIC` | 1 | Build static library |
| `BUILD_GIR` | 1 | Generate GObject Introspection files |
| `BUILD_TESTS` | 1 | Build unit tests |
| `BUILD_EXAMPLES` | 1 | Build example programs |
| `BUILD_DOCS` | 0 | Generate API documentation |
| `RAYLIB_SHARED` | 0 | Link against system raylib |
| `DEBUG` | 0 | Enable debug build |
| `WINDOWS` | 0 | Cross-compile for 64-bit Windows |
| `CROSS` | (empty) | Cross-compilation toolchain prefix |

## Build Targets

### Building

```bash
# Build everything (library + GIR)
make

# Build only the library
make lib

# Build static library only
make lib-static

# Build shared library only
make lib-shared

# Build with debug symbols
make DEBUG=1

# Build raylib from submodule
make raylib
```

### Testing

```bash
# Build and run tests
make test

# Build tests only
make -C tests

# Run specific test
./tests/test-vector2
```

### Examples

```bash
# Build examples
make examples

# Run an example
make -C examples run-basic-window
```

### GObject Introspection

```bash
# Generate GIR and typelib
make gir
```

This generates:
- `build/gir/Graylib-1.gir` - GIR XML file
- `build/gir/Graylib-1.typelib` - Compiled typelib

### Installation

```bash
# Install to PREFIX (default /usr/local)
sudo make install

# Install to custom location
make install PREFIX=/opt/graylib

# Install to staging directory (for packaging)
make install DESTDIR=/path/to/staging

# Uninstall
sudo make uninstall
```

### Cleaning

```bash
# Clean build artifacts
make clean

# Clean everything including raylib
make distclean

# Clean raylib only
make raylib-clean
```

## Debug Builds

For debugging with gdb, build with `DEBUG=1`:

```bash
make DEBUG=1
```

This enables:
- `-g3` for maximum debug information
- `-O0` to disable optimizations
- `-fno-omit-frame-pointer` for better stack traces
- `GRL_DEBUG` macro defined

Example debugging session:

```bash
# Build with debug symbols
make DEBUG=1

# Run example under gdb
gdb ./examples/basic-window
(gdb) run
(gdb) bt   # backtrace on crash
```

## Using System raylib

By default, Graylib builds raylib from the bundled submodule. To use system raylib:

```bash
# Ensure raylib is installed
sudo dnf install raylib-devel

# Build with system raylib
make RAYLIB_SHARED=1
```

When using system raylib:
- pkg-config is used to find raylib
- The bundled raylib is not built
- Make sure raylib 6.0+ is installed

## Cross-Platform Support

### Linux (Primary Platform)

Full native support. All features work.

### Windows (Cross-Compilation)

Build Windows binaries from a Linux host using MinGW-w64.

#### Installing Cross-Compilation Dependencies

**Fedora:**

```bash
sudo dnf install mingw64-gcc mingw64-glib2 mingw64-pkg-config
```

**Ubuntu/Debian:**

```bash
sudo apt install gcc-mingw-w64-x86-64 mingw-w64-tools
# Note: GLib cross-compilation packages may need manual setup
```

#### Building for Windows

```bash
# Build everything for 64-bit Windows
make WINDOWS=1

# Build just the library
make WINDOWS=1 lib

# Build examples
make WINDOWS=1 examples

# Build tests
make WINDOWS=1 test

# Alternative: use CROSS directly (for advanced users)
make CROSS=x86_64-w64-mingw32
```

#### Output Files

```
build/lib/
├── graylib.dll        # Shared library (DLL)
├── libgraylib.dll.a   # Import library for linking
└── libgraylib.a       # Static library

build/examples/
├── basic-window.exe
├── shapes-demo.exe
└── ...

build/tests/
├── test-vector2.exe
└── ...
```

#### Running Windows Binaries on Linux

Use Wine to test Windows executables:

```bash
# Run an example
wine build/examples/basic-window.exe

# Run a test
wine build/tests/test-vector2.exe

# Run tests via make (uses Wine automatically)
make WINDOWS=1 test
```

#### Limitations

- GObject Introspection (.gir/.typelib) is disabled when cross-compiling
- Running Windows executables requires Wine or a Windows machine
- Python bindings are not available for cross-compiled builds

### macOS

Not yet tested. Should work with:
- Xcode command line tools
- Homebrew packages for dependencies

## Troubleshooting

### pkg-config not finding GLib

Ensure pkg-config can find glib:

```bash
pkg-config --cflags --libs glib-2.0
```

If not found, check that `PKG_CONFIG_PATH` includes the glib .pc file location.

### raylib build fails

Ensure all X11 development packages are installed:

```bash
# Fedora
sudo dnf install libX11-devel libXrandr-devel libXi-devel \
    libXcursor-devel libXinerama-devel

# Check OpenGL
glxinfo | head
```

### GIR scanner fails

Ensure gobject-introspection is installed:

```bash
which g-ir-scanner
g-ir-scanner --version
```

### Library not found at runtime

Set `LD_LIBRARY_PATH`:

```bash
export LD_LIBRARY_PATH=/path/to/graylib/build/lib:$LD_LIBRARY_PATH
```

Or install the library system-wide:

```bash
sudo make install
sudo ldconfig
```

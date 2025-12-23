# Getting Started with Graylib

This guide will help you get started with Graylib, a GObject wrapper for raylib.

## Prerequisites

Before building Graylib, ensure you have the following installed:

### Required

- GCC compiler
- GNU Make
- GLib 2.56 or later
- GObject Introspection (for language bindings)
- pkg-config

### For raylib (when building from submodule)

- OpenGL development libraries
- X11 development libraries (Linux)

### Fedora Installation

```bash
sudo dnf install gcc make glib2-devel gobject-introspection-devel \
    mesa-libGL-devel libX11-devel libXrandr-devel \
    libXi-devel libXcursor-devel libXinerama-devel
```

## Building Graylib

1. Clone the repository with submodules:

```bash
git clone --recursive https://github.com/your-repo/graylib.git
cd graylib
```

2. Build the library:

```bash
make
```

This will:
- Build raylib from the submodule (if not already built)
- Compile the Graylib library (static and shared)
- Generate GObject Introspection files

### Build Options

```bash
# Debug build (for gdb debugging)
make DEBUG=1

# Use system raylib instead of bundled
make RAYLIB_SHARED=1

# Build only the static library
make lib-static

# Build only the shared library
make lib-shared
```

## Your First Graylib Program

Create a file called `hello.c`:

```c
#include <graylib.h>

int
main (int argc, char *argv[])
{
    g_autoptr(GrlWindow) window = NULL;
    g_autoptr(GrlColor) bg = NULL;

    /* Create window */
    window = grl_window_new (800, 600, "Hello Graylib!");

    /* Set background color */
    bg = grl_color_new_raywhite ();

    /* Set target FPS */
    grl_window_set_target_fps (window, 60);

    /* Main loop */
    while (!grl_window_should_close (window))
    {
        grl_window_begin_drawing (window);
        grl_window_clear_background (window, bg);
        grl_window_end_drawing (window);
    }

    return 0;
}
```

## Compiling Your Program

```bash
gcc hello.c -o hello \
    $(pkg-config --cflags --libs glib-2.0 gobject-2.0) \
    -I/path/to/graylib \
    -L/path/to/graylib/build/lib -lgraylib \
    -lraylib -lm
```

Or if Graylib is installed:

```bash
gcc hello.c -o hello $(pkg-config --cflags --libs graylib-1)
```

## Running Your Program

```bash
# If Graylib is not installed, set the library path
export LD_LIBRARY_PATH=/path/to/graylib/build/lib:$LD_LIBRARY_PATH

./hello
```

## Basic Concepts

### Memory Management

Graylib uses GLib's automatic memory management. Use `g_autoptr()` for automatic cleanup:

```c
g_autoptr(GrlColor) color = grl_color_new_red ();
/* color is automatically freed when it goes out of scope */
```

For manual management:

```c
GrlColor *color = grl_color_new_red ();
/* ... use color ... */
grl_color_free (color);
```

### GBoxed vs GObject Types

Graylib has two types of types:

**GBoxed types** (simple value types):
- `GrlVector2` - 2D vector
- `GrlColor` - RGBA color
- `GrlRectangle` - Axis-aligned rectangle

These are lightweight and use copy/free semantics.

**GObject types** (reference-counted objects):
- `GrlApplication` - Application singleton
- `GrlWindow` - Window management

These use `g_object_ref()` and `g_object_unref()`.

### The Game Loop

A typical Graylib game loop:

```c
while (!grl_window_should_close (window))
{
    /* Get delta time for frame-rate independent movement */
    gfloat dt = grl_window_get_frame_time (window);

    /* Update game state */
    player_x += speed * dt;

    /* Drawing */
    grl_window_begin_drawing (window);
    grl_window_clear_background (window, bg_color);

    /* Draw game objects */
    grl_draw_rectangle (player_x, player_y, 50, 50, player_color);

    grl_window_end_drawing (window);
}
```

## Next Steps

- Explore the [examples](../examples/) directory
- Read the [API documentation](api/overview.md)
- Learn about [drawing shapes](api/drawing.md)
- Create [custom entities](guides/custom-entities.md) (coming in Phase 5)

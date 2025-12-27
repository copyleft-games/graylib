# Graylib

**Graylib** is a GObject/GLib wrapper around the [raylib](https://www.raylib.com/) game programming library. It provides:

- **GObject types** with automatic memory management via reference counting
- **GObject Introspection** support for language bindings (Python, Lua, Vala, JavaScript, etc.)
- **Signal-based event handling** for input and window events
- **Property system** for data binding
- **Derivable types** for creating custom game entities

## Features

- Full wrapper around raylib 5.5 functionality
- Both static and shared library builds
- Debug build support for gdb
- Comprehensive documentation
- Unit tests for all types

## Getting Started

See the [Getting Started Guide](getting-started.md) for installation and basic usage.

## Building

See the [Building Guide](guides/building.md) for build instructions.

## Best Practices

See [Best Practices](best_practices.md) for common pitfalls and solutions when working with graylib (depth buffer clearing, z-fighting, color operations, etc.).

## API Reference

- [Math Types](api/math-types.md) - Vector2, Vector3, Vector4, Color, Rectangle, Matrix, BoundingBox
- [Core](api/core.md) - Application, Window
- [Input](api/input.md) - Keyboard, mouse, and gamepad input
- [Graphics](api/graphics.md) - Images, textures, fonts, shaders, and render textures
- [Drawing](api/drawing.md) - Shape, texture, and text drawing functions
- [Audio](api/audio.md) - Sound effects and music
- [Scene](api/scene.md) - Entity system, sprites, and animations
- [Cameras](api/cameras.md) - 2D and 3D camera systems
- [Models](api/models.md) - 3D meshes, materials, models, and animations
- [Collision](api/collision.md) - 2D and 3D collision detection
- [RLGL](api/rlgl.md) - Low-level OpenGL abstraction layer
- [Enumerations](api/enums.md) - Key codes, blend modes, and other constants

## Examples

Example programs are in the `examples/` directory:

### C Examples
- `basic-window.c` - Simple window with basic drawing
- `shapes-demo.c` - Demonstration of shape drawing functions
- `input-demo.c` - Keyboard and mouse input handling
- `textures-text.c` - Texture loading and text rendering
- `audio-demo.c` - Sound effects and music playback
- `entity-demo.c` - Entity system and collision detection
- `scene-demo.c` - Scene management demonstration
- `camera-demo.c` - 2D/3D camera controls
- `model-demo.c` - 3D model rendering
- `collision-demo.c` - 2D collision detection

### Python Examples
- `basic-window.py` - Python GI bindings demonstration

Run Python examples with:
```bash
GI_TYPELIB_PATH=../build/gir LD_LIBRARY_PATH=../build/lib python3 basic-window.py
```

## License

Graylib is licensed under the GNU Affero General Public License v3.0 or later.
See `LICENSE` for the full license text.

Note: raylib (bundled in deps/) is licensed under the Zlib license.

## Dependencies

- GLib 2.56+
- GObject Introspection
- raylib 5.5 (included as submodule)

### Fedora

```bash
sudo dnf install gcc glib2-devel gobject-introspection-devel \
    mesa-libGL-devel libX11-devel libXrandr-devel \
    libXi-devel libXcursor-devel libXinerama-devel
```

## Quick Example

```c
#include <graylib.h>

int
main (int argc, char *argv[])
{
    g_autoptr(GrlWindow) window = NULL;
    g_autoptr(GrlColor) bg_color = NULL;
    g_autoptr(GrlColor) circle_color = NULL;
    g_autoptr(GrlVector2) center = NULL;

    window = grl_window_new (800, 600, "My Game");
    bg_color = grl_color_new (40, 40, 60, 255);
    circle_color = grl_color_new_red ();
    center = grl_vector2_new (400.0f, 300.0f);

    grl_window_set_target_fps (window, 60);

    while (!grl_window_should_close (window))
    {
        grl_window_begin_drawing (window);
        grl_window_clear_background (window, bg_color);
        grl_draw_circle_v (center, 50.0f, circle_color);
        grl_window_end_drawing (window);
    }

    return 0;
}
```

## Project Structure

```
graylib/
├── src/                    # Library source code
│   ├── math/              # Math types (GBoxed): Vector2, Color, Rectangle
│   ├── core/              # Core types: Application, Window, Input
│   ├── graphics/          # Graphics types: Image, Texture, Font
│   ├── drawing/           # Drawing functions
│   ├── audio/             # Audio types: AudioDevice, Sound, Music
│   ├── scene/             # Entity system: Entity, Sprite, AnimatedTexture
│   └── rlgl/              # Low-level OpenGL abstraction layer
├── deps/                   # Dependencies (raylib submodule)
├── tests/                  # Unit tests
├── examples/               # Example source files
├── docs/                   # Documentation
└── build/                  # Build output
    ├── lib/               # Libraries (libgraylib.a, libgraylib.so)
    ├── gir/               # GObject Introspection files
    ├── examples/          # Compiled example binaries
    └── obj/               # Object files
```

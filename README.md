# Graylib

**Graylib** is a GObject/GLib wrapper around the [raylib](https://www.raylib.com/) 5.5 game programming library.

## Features

- **GObject types** with automatic memory management via reference counting
- **GObject Introspection** support for language bindings (Python, Lua, Vala, JavaScript, etc.)
- **Signal-based event handling** for input and window events
- **Property system** for data binding
- **Derivable types** for creating custom game entities
- Both static (`libgraylib.a`) and shared (`libgraylib.so`) library builds
- Debug build support for gdb
- Comprehensive documentation and unit tests

## Dependencies (Fedora)

```bash
sudo dnf install gcc make glib2-devel gobject-introspection-devel \
    mesa-libGL-devel libX11-devel libXrandr-devel \
    libXi-devel libXcursor-devel libXinerama-devel
```

## Getting Started

```bash
# Clone the repository
git clone https://gitlab.com/your-username/graylib.git
cd graylib

# Initialize submodules (raylib)
git submodule update --init --recursive

# Build the library
make

# Build examples
make examples

# Run an example
LD_LIBRARY_PATH=build/lib:$LD_LIBRARY_PATH ./build/examples/basic-window
```

## Build Options

| Command | Description |
|---------|-------------|
| `make` | Build static library, shared library, and GIR files |
| `make DEBUG=1` | Build with debug symbols for gdb |
| `make static` | Build only static library (`libgraylib.a`) |
| `make shared` | Build only shared library (`libgraylib.so`) |
| `make gir` | Generate GObject Introspection files |
| `make examples` | Build all example programs |
| `make test` | Run unit tests |
| `make docs` | Generate API documentation (requires gi-docgen) |
| `make install` | Install to PREFIX (default: `/usr/local`) |
| `make clean` | Remove build artifacts |
| `make help` | Show all available targets |

### Build Variables

| Variable | Default | Description |
|----------|---------|-------------|
| `DEBUG` | 0 | Enable debug build with `-g -O0` |
| `PREFIX` | `/usr/local` | Installation prefix |
| `LIBDIR` | `$(PREFIX)/lib64` | Library installation directory |

## Quick Example

```c
#include <graylib.h>

int
main (int argc, char *argv[])
{
    g_autoptr(GrlWindow) window = NULL;
    g_autoptr(GrlColor) bg = NULL;
    g_autoptr(GrlColor) red = NULL;
    g_autoptr(GrlVector2) center = NULL;

    window = grl_window_new (800, 600, "Hello Graylib");
    bg = grl_color_new_raywhite ();
    red = grl_color_new_red ();
    center = grl_vector2_new (400.0f, 300.0f);

    grl_window_set_target_fps (window, 60);

    while (!grl_window_should_close (window))
    {
        grl_window_begin_drawing (window);
        grl_window_clear_background (window, bg);
        grl_draw_circle_v (center, 50.0f, red);
        grl_window_end_drawing (window);
    }

    return 0;
}
```

Compile with:

```bash
gcc -o myapp myapp.c $(pkg-config --cflags --libs graylib)
```

Or manually:

```bash
gcc -o myapp myapp.c -I/path/to/graylib -L/path/to/build/lib -lgraylib -lraylib $(pkg-config --cflags --libs glib-2.0 gobject-2.0) -lGL -lm -lpthread -ldl
```

## Running Examples

After building:

```bash
# Set library path and run (from project root)
LD_LIBRARY_PATH=build/lib:$LD_LIBRARY_PATH ./build/examples/basic-window
LD_LIBRARY_PATH=build/lib:$LD_LIBRARY_PATH ./build/examples/shapes-demo
LD_LIBRARY_PATH=build/lib:$LD_LIBRARY_PATH ./build/examples/input-demo
LD_LIBRARY_PATH=build/lib:$LD_LIBRARY_PATH ./build/examples/audio-demo
LD_LIBRARY_PATH=build/lib:$LD_LIBRARY_PATH ./build/examples/entity-demo
LD_LIBRARY_PATH=build/lib:$LD_LIBRARY_PATH ./build/examples/collision-demo
LD_LIBRARY_PATH=build/lib:$LD_LIBRARY_PATH ./build/examples/model-demo
```

## Python Bindings

Graylib supports Python via GObject Introspection (PyGObject).

### Requirements

```bash
sudo dnf install python3-gobject
```

### Usage

```python
#!/usr/bin/env python3
import gi
gi.require_version('Graylib', '1')
from gi.repository import Graylib as Grl

# Create window
window = Grl.Window.new(800, 600, "Python Example")
window.set_target_fps(60)

# Create colors
bg = Grl.Color.new(40, 40, 60, 255)
red = Grl.Color.new(230, 41, 55, 255)

# Main loop
while not window.should_close():
    window.begin_drawing()
    Grl.Draw.clear_background(bg)
    Grl.Draw.circle(400, 300, 50.0, red)
    window.end_drawing()
```

### Running Python Examples

```bash
# From project root
GI_TYPELIB_PATH=build/gir LD_LIBRARY_PATH=build/lib python3 examples/basic-window.py

# Or from examples directory
cd examples
GI_TYPELIB_PATH=../build/gir LD_LIBRARY_PATH=../build/lib python3 basic-window.py
```

## Project Structure

```
graylib/
├── src/                    # Library source code
│   ├── math/              # Math types (GBoxed): Vector2, Color, Rectangle
│   ├── core/              # Core types: Application, Window
│   ├── input/             # Input handling: Keyboard, Mouse, Gamepad
│   ├── graphics/          # Graphics types: Image, Texture, Font, Camera, Shader
│   ├── drawing/           # Drawing functions
│   ├── audio/             # Audio types: AudioDevice, Sound, Music
│   └── scene/             # Entity system: Entity, Sprite, Scene, SceneManager
├── deps/                   # Dependencies (raylib submodule)
├── tests/                  # Unit tests
├── examples/               # Example source files (C and Python)
├── docs/                   # Documentation
└── build/                  # Build output
    ├── lib/               # Libraries (libgraylib.a, libgraylib.so)
    ├── gir/               # GObject Introspection files
    ├── examples/          # Compiled example binaries
    └── obj/               # Object files
```

## Documentation

- [Getting Started](docs/getting-started.md)
- [Building Guide](docs/guides/building.md)
- [API Reference](docs/api/)
  - [Math Types](docs/api/math-types.md) - Vector2, Vector3, Vector4, Color, Rectangle, Matrix, BoundingBox
  - [Core Types](docs/api/core.md) - Application, Window
  - [Input System](docs/api/input.md) - Keyboard, Mouse, Gamepad
  - [Graphics Types](docs/api/graphics.md) - Image, Texture, Font, Shader, RenderTexture
  - [Drawing Functions](docs/api/drawing.md) - Shapes, textures, text
  - [Audio System](docs/api/audio.md) - Sound effects and music
  - [Scene System](docs/api/scene.md) - Entity, Sprite, Scene, SceneManager
  - [Cameras](docs/api/cameras.md) - Camera2D, Camera3D
  - [Models](docs/api/models.md) - Mesh, Material, Model, ModelAnimation
  - [Collision](docs/api/collision.md) - 2D and 3D collision detection
  - [Enumerations](docs/api/enums.md) - Key codes, blend modes, and constants

## Examples

| Example | Description |
|---------|-------------|
| `basic-window.c` | Simple window with basic drawing |
| `shapes-demo.c` | Shape drawing functions |
| `input-demo.c` | Keyboard and mouse input handling |
| `textures-text.c` | Texture loading and text rendering |
| `audio-demo.c` | Sound effects and music playback |
| `entity-demo.c` | Entity system and sprites |
| `scene-demo.c` | Scene management |
| `camera-demo.c` | 2D/3D camera controls |
| `model-demo.c` | 3D model rendering |
| `collision-demo.c` | 2D collision detection |
| `basic-window.py` | Python GI bindings demonstration |

## Installation

```bash
# Install to /usr/local (default)
sudo make install

# Install to custom prefix
make PREFIX=/opt/graylib install

# After installation, configure pkg-config
export PKG_CONFIG_PATH=/usr/local/lib64/pkgconfig:$PKG_CONFIG_PATH
```

## Requirements

- GCC
- GNU Make
- GLib 2.56+
- GObject Introspection
- raylib 5.5 (included as submodule)

## License

Graylib is licensed under the **GNU Affero General Public License v3.0** (AGPLv3) or later.
See [LICENSE](LICENSE) for the full license text.

Note: raylib (bundled in deps/) is licensed under the Zlib license.

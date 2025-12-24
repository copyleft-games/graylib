# Graylib - Claude Code Instructions

## Project Overview

**Graylib** is a GObject/GLib wrapper around the [raylib](https://www.raylib.com/) 5.5 game programming library. It provides:

- GObject types with automatic memory management via reference counting
- GObject Introspection (GIR) support for language bindings (Python, Lua, Vala, JS)
- Signal-based event handling for input and window events
- Property system for data binding
- Derivable types for creating custom game entities

**License**: AGPL-3.0-or-later (see `LICENSE`)

## Tech Stack

| Component | Technology |
|-----------|------------|
| Language | C (gnu89 standard) |
| Compiler | GCC |
| Build System | GNU Make |
| Dependencies | GLib 2.56+, GObject Introspection, raylib 5.5 |
| Testing | GLib GTest framework |
| Documentation | gtk-doc comments, gi-docgen |

## Build Commands

```bash
make                # Build library (static + shared + GIR)
make DEBUG=1        # Debug build with gdb symbols (-g3 -O0)
make test           # Build and run unit tests
make examples       # Build example programs
make gir            # Generate GObject Introspection files
make docs           # Generate API documentation (requires gi-docgen)
make install        # Install to PREFIX (default: /usr/local)
make clean          # Remove build artifacts
make help           # Show all targets
```

**Build Variables:**
- `DEBUG=1` - Enable debug build
- `RAYLIB_SHARED=1` - Link against system raylib instead of bundled
- `PREFIX=/path` - Installation prefix
- `ASAN=1` - Enable AddressSanitizer (requires DEBUG=1)
- `UBSAN=1` - Enable UndefinedBehaviorSanitizer (requires DEBUG=1)

## Naming Conventions

| Element | Convention | Example |
|---------|------------|---------|
| Namespace | `Grl` | `GrlTexture`, `GrlSprite` |
| GType macro | `GRL_TYPE_*` | `GRL_TYPE_TEXTURE` |
| Instance check | `GRL_IS_*` | `GRL_IS_TEXTURE(obj)` |
| Class cast | `GRL_*_CLASS` | `GRL_TEXTURE_CLASS(klass)` |
| Get class | `GRL_*_GET_CLASS` | `GRL_TEXTURE_GET_CLASS(obj)` |
| Private data | `*Private` | `GrlTexturePrivate` |
| Functions | `grl_*` | `grl_texture_new_from_file()` |
| Signals | `kebab-case` | `"key-pressed"`, `"frame-tick"` |
| Properties | `kebab-case` | `"position"`, `"frame-count"` |
| Constants | `GRL_*` | `GRL_KEY_SPACE`, `GRL_BLEND_ALPHA` |
| Error domain | `GRL_*_ERROR` | `GRL_RESOURCE_ERROR` |

## Code Style

### Warning-Free Builds
The build system is configured for **zero warnings**:
- Uses `-Wall -Wextra` but not `-Wpedantic` (incompatible with GLib patterns)
- System headers (GLib, raylib) use `-isystem` to suppress their warnings
- All graylib code must compile without warnings

### C89/gnu89 Compliance
Since we use `gnu89` standard, follow these rules to avoid warnings:
- **Declare all variables at the top of blocks** - no mixed declarations and code
- **No C99 features** like designated initializers in variable declarations
- **Remove unused variables** - don't leave dead code
- **Prototype all non-static functions** - include the appropriate private header

### Comments
- **Always use** `/* comment */` style, never `//`
- All public functions must have gtk-doc comments with GIR annotations
- Break complex functions into logical chunks with comments

### Memory Management
```c
/* Use g_autoptr for automatic cleanup */
g_autoptr(GrlVector2) pos = grl_vector2_new(100.0f, 200.0f);

/* Use g_steal_pointer when transferring ownership */
return (GrlTexture *)g_steal_pointer(&texture);

/* Use g_clear_pointer/g_clear_object in finalize */
g_clear_pointer(&priv->name, g_free);
g_clear_object(&priv->texture);
```

### Public API Validation
```c
void
grl_sprite_set_texture(GrlSprite  *self,
                       GrlTexture *texture)
{
    g_return_if_fail(GRL_IS_SPRITE(self));
    g_return_if_fail(texture == NULL || GRL_IS_TEXTURE(texture));
    /* ... */
}
```

### GIR Annotations
All public API must include proper annotations:

```c
/**
 * grl_texture_new_from_file:
 * @path: (type filename): Path to the image file
 * @error: (nullable): Return location for error, or %NULL
 *
 * Loads a texture from a file.
 *
 * Returns: (transfer full) (nullable): A new #GrlTexture, or %NULL on error
 */
GrlTexture *
grl_texture_new_from_file(const gchar  *path,
                          GError      **error);
```

Common annotations:
- `(transfer full)` - Caller owns the returned value
- `(transfer none)` - Caller does not own the value
- `(nullable)` - Value can be NULL
- `(element-type X)` - For container types
- `(scope call)` - Callback only valid during function call
- `(closure func)` - User data for callback

## Type System

### GBoxed Types (Value Types)
Stack-allocatable, copyable value types:

| Type | Description |
|------|-------------|
| `GrlVector2` | 2D vector (x, y) |
| `GrlVector3` | 3D vector (x, y, z) |
| `GrlVector4` | 4D vector (x, y, z, w) |
| `GrlColor` | RGBA color (r, g, b, a as guint8) |
| `GrlRectangle` | Rectangle (x, y, width, height) |
| `GrlMatrix` | 4x4 transformation matrix |
| `GrlBoundingBox` | 3D bounding box (min, max) |

### GObject Types (Reference Types)
Heap-allocated, reference-counted objects:

**Core:** GrlApplication, GrlWindow, GrlInput

**Graphics:** GrlImage, GrlTexture, GrlFont, GrlCamera2D, GrlCamera3D, GrlShader, GrlRenderTexture, GrlMesh, GrlMaterial, GrlModel, GrlModelAnimation

**Audio:** GrlAudioDevice, GrlSound, GrlMusic

**Scene:** GrlEntity, GrlSprite, GrlAnimatedTexture, GrlScene, GrlSceneManager

### Interfaces
- `GrlDrawable` - Objects that can be drawn (`draw()`)
- `GrlUpdatable` - Objects that update each frame (`update(delta)`)
- `GrlCollidable` - Objects participating in collision (`get_bounds()`, `on_collision()`)

## Project Structure

```
graylib/
├── graylib/                # Library source code
│   ├── graylib.h          # Main public header (includes all)
│   ├── grl-version.h.in   # Version macros (generated)
│   ├── grl-types.h        # Forward declarations
│   ├── grl-enums.h/c      # Enumerations
│   ├── math/              # GBoxed math types
│   ├── core/              # Application, Window, Input
│   ├── graphics/          # Image, Texture, Font, Camera, Shader, Model
│   ├── drawing/           # Drawing functions (shapes, textures, text, 3D)
│   ├── audio/             # AudioDevice, Sound, Music
│   ├── scene/             # Entity, Sprite, Scene, SceneManager, interfaces
│   └── collision/         # Collision detection functions
├── deps/                   # Dependencies (raylib submodule)
├── tests/                  # Unit tests (GTest)
├── examples/               # Example programs (C and Python)
├── docs/                   # Documentation
├── build/                  # Build output
│   ├── lib/               # libgraylib.a, libgraylib.so
│   ├── gir/               # Graylib-1.gir, Graylib-1.typelib
│   └── obj/               # Object files
├── Makefile               # Main build file
├── config.mk              # Build configuration
├── rules.mk               # Build rules
└── plan.md                # Design specification
```

## Templates

### Header File Template
```c
/* grl-example.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Brief description of the type.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "../grl-version.h"
#include "../grl-types.h"

G_BEGIN_DECLS

#define GRL_TYPE_EXAMPLE (grl_example_get_type ())

GRL_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (GrlExample, grl_example, GRL, EXAMPLE, GObject)

/**
 * GrlExampleClass:
 * @parent_class: The parent class
 * @do_something: Virtual method description
 *
 * The class structure for #GrlExample.
 */
struct _GrlExampleClass
{
    GObjectClass parent_class;

    /* Virtual methods */
    void (*do_something) (GrlExample *self,
                          gint        value);

    /*< private >*/
    gpointer _reserved[8];
};

/*
 * Constructors
 */

GRL_AVAILABLE_IN_ALL
GrlExample *    grl_example_new         (void);

/*
 * Methods
 */

GRL_AVAILABLE_IN_ALL
void            grl_example_do_something (GrlExample *self,
                                          gint        value);

G_END_DECLS
```

### Source File Template
```c
/* grl-example.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "config.h"
#include "grl-example.h"
#include <raylib.h>

typedef struct
{
    gint   value;
    gchar *name;
} GrlExamplePrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GrlExample, grl_example, G_TYPE_OBJECT)

enum
{
    PROP_0,
    PROP_VALUE,
    N_PROPS
};

static GParamSpec *properties[N_PROPS];

static void
grl_example_finalize (GObject *object)
{
    GrlExample        *self = GRL_EXAMPLE (object);
    GrlExamplePrivate *priv = grl_example_get_instance_private (self);

    g_clear_pointer (&priv->name, g_free);

    G_OBJECT_CLASS (grl_example_parent_class)->finalize (object);
}

static void
grl_example_get_property (GObject    *object,
                          guint       prop_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
    GrlExample        *self = GRL_EXAMPLE (object);
    GrlExamplePrivate *priv = grl_example_get_instance_private (self);

    switch (prop_id)
    {
    case PROP_VALUE:
        g_value_set_int (value, priv->value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
grl_example_set_property (GObject      *object,
                          guint         prop_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
    GrlExample        *self = GRL_EXAMPLE (object);
    GrlExamplePrivate *priv = grl_example_get_instance_private (self);

    switch (prop_id)
    {
    case PROP_VALUE:
        priv->value = g_value_get_int (value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
grl_example_class_init (GrlExampleClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = grl_example_finalize;
    object_class->get_property = grl_example_get_property;
    object_class->set_property = grl_example_set_property;

    /**
     * GrlExample:value:
     *
     * An example property.
     */
    properties[PROP_VALUE] =
        g_param_spec_int ("value",
                          "Value",
                          "An example value",
                          G_MININT, G_MAXINT, 0,
                          G_PARAM_READWRITE |
                          G_PARAM_STATIC_STRINGS |
                          G_PARAM_EXPLICIT_NOTIFY);

    g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
grl_example_init (GrlExample *self)
{
    GrlExamplePrivate *priv = grl_example_get_instance_private (self);

    priv->value = 0;
    priv->name = NULL;
}

/**
 * grl_example_new:
 *
 * Creates a new #GrlExample.
 *
 * Returns: (transfer full): A new #GrlExample
 */
GrlExample *
grl_example_new (void)
{
    return g_object_new (GRL_TYPE_EXAMPLE, NULL);
}
```

### GBoxed Type Template
```c
/* In header */
struct _GrlVector2
{
    gfloat x;
    gfloat y;
};

GRL_AVAILABLE_IN_ALL
GType        grl_vector2_get_type (void) G_GNUC_CONST;

GRL_AVAILABLE_IN_ALL
GrlVector2 * grl_vector2_new      (gfloat x, gfloat y);

GRL_AVAILABLE_IN_ALL
GrlVector2 * grl_vector2_copy     (const GrlVector2 *self);

GRL_AVAILABLE_IN_ALL
void         grl_vector2_free     (GrlVector2 *self);

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GrlVector2, grl_vector2_free)

/* In source */
G_DEFINE_BOXED_TYPE (GrlVector2, grl_vector2,
                     grl_vector2_copy, grl_vector2_free)
```

## Documentation Requirements

**IMPORTANT:** When adding new types or changing APIs, documentation must be updated:

1. **gtk-doc comments** on all public functions with GIR annotations
2. **Update `docs/api/*.md`** for the relevant module
3. **Update `docs/index.md`** if adding new examples
4. **Update `README.md`** if adding major features

## Running Examples

```bash
# Build examples
make examples

# Run C examples
cd examples
LD_LIBRARY_PATH=../build/lib:$LD_LIBRARY_PATH ./basic-window

# Run Python examples
GI_TYPELIB_PATH=../build/gir LD_LIBRARY_PATH=../build/lib python3 basic-window.py
```

## Testing

Tests use the GLib GTest framework:

```bash
# Run all tests
make test

# Tests are in tests/ directory
# Each type should have corresponding test-*.c file
```

Test file structure:
```c
#include <graylib.h>

static void
test_vector2_new (void)
{
    g_autoptr(GrlVector2) v = grl_vector2_new (1.0f, 2.0f);

    g_assert_nonnull (v);
    g_assert_cmpfloat_with_epsilon (v->x, 1.0f, 0.0001f);
}

int
main (int   argc,
      char *argv[])
{
    g_test_init (&argc, &argv, NULL);

    g_test_add_func ("/vector2/new", test_vector2_new);

    return g_test_run ();
}
```

## Adding New Types

When adding a new type:

1. **Add AGPLv3 license header** to all new files (see Header File Template above)
2. Create `grl-typename.h` and `grl-typename.c` in appropriate module
3. Add forward declaration to `graylib/grl-types.h`
4. Add include to `graylib/graylib.h`
5. Add to `PUBLIC_HEADERS` and `SOURCES` in `Makefile`
6. Add object file rule to `Makefile`
7. Add `GRL_AVAILABLE_IN_ALL` before `G_DECLARE_*_TYPE` for GIR visibility
8. Write unit tests in `tests/test-typename.c`
9. Update `docs/api/` documentation
10. Create example if appropriate

## Common Pitfalls

1. **GBoxed vs GObject properties**: Use `g_param_spec_boxed()` for GBoxed types (GrlColor, GrlRectangle, etc.), not `g_param_spec_object()`

2. **GIR symbol visibility**: All `G_DECLARE_*_TYPE` macros must be preceded by `GRL_AVAILABLE_IN_ALL` for symbols to be exported

3. **Header guards**: Use `#pragma once` and the `GRAYLIB_INSIDE` check

4. **Include order**: Always include `"config.h"` first in source files

5. **Memory ownership**: Document transfer semantics with `(transfer full)` or `(transfer none)` annotations

6. **bool vs gboolean ABI mismatch**: When wrapping raylib functions that return C99 `bool` (1 byte), you MUST use an `unsigned char` intermediate variable before returning `gboolean` (4 bytes). The compiler may otherwise read garbage from upper register bytes.

   **WRONG** (garbage in upper 24 bits):
   ```c
   gboolean
   grl_input_is_key_down (GrlKey key)
   {
       return IsKeyDown ((int)key);  /* BAD: tail-call leaves upper bytes dirty */
   }

   gboolean
   grl_input_is_key_down (GrlKey key)
   {
       return IsKeyDown ((int)key) ? TRUE : FALSE;  /* BAD: compiler tests full %eax */
   }
   ```

   **CORRECT** (forces 8-bit read, then zero-extends):
   ```c
   gboolean
   grl_input_is_key_down (GrlKey key)
   {
       unsigned char raw = IsKeyDown ((int)key);
       return raw != 0;
   }
   ```

   This generates proper assembly: `test %al,%al` + `movzbl %al,%eax` instead of `test %eax,%eax`.

   **Affected functions**: All raylib functions returning `bool` - input functions (`IsKeyDown`, `IsMouseButtonPressed`, etc.), collision detection (`CheckCollision*`), and any other boolean-returning APIs. See `docs/api/input.md` for the full list.

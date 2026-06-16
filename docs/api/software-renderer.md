# Software Renderer (rlsw)

`GrlSoftwareRenderer` is a final GObject wrapping raylib 6.0's standalone,
header-only software rasterizer (`rlsw`). It rasterizes immediate-mode,
OpenGL-1.1-style geometry entirely on the CPU into an internal framebuffer —
with **no GPU, no GL context, and no window required**.

This makes it usable in genuinely headless environments (CI, asset bakers,
servers) where `GrlTexture` / `GrlRenderTexture` cannot run, since those need a
live GL context. Rendered output is read back into a `GrlImage` for capture.

**Warning:** Like the [RLGL](rlgl.md) module, this is a low-level rendering API.
Most applications should prefer the higher-level drawing functions. Reach for
the software renderer specifically when you need rasterized output without a GPU.

## Singleton Constraint

`rlsw` maintains a single, process-global rasterizer context — its
init/shutdown entry points take no handle. Therefore **at most one
`GrlSoftwareRenderer` instance may be alive at a time.**

`grl_software_renderer_new()` returns `NULL` (and emits a `GWarning`) if another
instance is already live. The global context is released when the live instance
is finalized, after which a new one may be created.

```c
g_autoptr(GrlSoftwareRenderer) sw = grl_software_renderer_new (640, 480);
if (sw == NULL)
{
    g_printerr ("Could not create software renderer\n");
    return;
}
```

## Constructors & Lifecycle

```c
GrlSoftwareRenderer * grl_software_renderer_new        (gint width,
                                                        gint height);
gboolean              grl_software_renderer_resize     (GrlSoftwareRenderer *self,
                                                        gint width,
                                                        gint height);
gint                  grl_software_renderer_get_width  (GrlSoftwareRenderer *self);
gint                  grl_software_renderer_get_height (GrlSoftwareRenderer *self);
```

| Function | Description |
|----------|-------------|
| `new(width, height)` | Create the renderer and initialize the rasterizer with a framebuffer of the given size (both must be `> 0`). Returns `NULL` if an instance is already live or init failed. |
| `resize(width, height)` | Resize the internal framebuffer. Returns `TRUE` on success. |
| `get_width()` / `get_height()` | Get the current framebuffer dimensions in pixels. |

## Framebuffer & State

State and clear functions configure the rasterizer pipeline. Capabilities are
toggled with `enable()` / `disable()` using `GrlSwState`.

```c
void grl_software_renderer_enable       (GrlSoftwareRenderer *self, GrlSwState state);
void grl_software_renderer_disable      (GrlSoftwareRenderer *self, GrlSwState state);

void grl_software_renderer_viewport     (GrlSoftwareRenderer *self,
                                         gint x, gint y, gint width, gint height);
void grl_software_renderer_scissor      (GrlSoftwareRenderer *self,
                                         gint x, gint y, gint width, gint height);

void grl_software_renderer_clear_color  (GrlSoftwareRenderer *self,
                                         gfloat r, gfloat g, gfloat b, gfloat a);
void grl_software_renderer_clear_depth  (GrlSoftwareRenderer *self, gfloat depth);
void grl_software_renderer_clear        (GrlSoftwareRenderer *self, GrlSwBuffer mask);

void grl_software_renderer_blend_func   (GrlSoftwareRenderer *self,
                                         GrlSwFactor src_factor,
                                         GrlSwFactor dst_factor);
void grl_software_renderer_polygon_mode (GrlSoftwareRenderer *self, GrlSwPolygonMode mode);
void grl_software_renderer_cull_face    (GrlSoftwareRenderer *self, GrlSwFace face);
void grl_software_renderer_point_size   (GrlSoftwareRenderer *self, gfloat size);
void grl_software_renderer_line_width   (GrlSoftwareRenderer *self, gfloat width);
```

| Function | Description |
|----------|-------------|
| `enable(state)` / `disable(state)` | Toggle a `GrlSwState` capability (scissor test, texturing, depth test, face culling, blending) |
| `viewport(x, y, w, h)` | Map normalized device coordinates to framebuffer pixels |
| `scissor(x, y, w, h)` | Set the scissor rectangle (pixels outside are discarded when the scissor test is on) |
| `clear_color(r, g, b, a)` | Set the color the framebuffer is cleared to (components in `[0, 1]`) |
| `clear_depth(depth)` | Set the depth value the depth buffer is cleared to (`[0, 1]`) |
| `clear(mask)` | Clear the buffers selected by a `GrlSwBuffer` bitmask |
| `blend_func(src, dst)` | Set the source/destination `GrlSwFactor` blend factors |
| `polygon_mode(mode)` | Set polygon rasterization (points, wireframe, filled) |
| `cull_face(face)` | Select which faces are culled when culling is enabled |
| `point_size(size)` / `line_width(width)` | Set rasterized point/line size in pixels |

See [Enumerations](enums.md#software-renderer-rlsw) for `GrlSwState`,
`GrlSwBuffer`, `GrlSwFactor`, `GrlSwPolygonMode`, and `GrlSwFace`.

## Matrix Stack

Classic fixed-function-style matrix stacks, one per `GrlSwMatrixMode`.

```c
void grl_software_renderer_matrix_mode   (GrlSoftwareRenderer *self, GrlSwMatrixMode mode);
void grl_software_renderer_push_matrix   (GrlSoftwareRenderer *self);
void grl_software_renderer_pop_matrix    (GrlSoftwareRenderer *self);
void grl_software_renderer_load_identity (GrlSoftwareRenderer *self);

void grl_software_renderer_translate     (GrlSoftwareRenderer *self,
                                          gfloat x, gfloat y, gfloat z);
void grl_software_renderer_rotate        (GrlSoftwareRenderer *self,
                                          gfloat angle, gfloat x, gfloat y, gfloat z);
void grl_software_renderer_scale         (GrlSoftwareRenderer *self,
                                          gfloat x, gfloat y, gfloat z);
void grl_software_renderer_mult_matrix   (GrlSoftwareRenderer *self,
                                          const gfloat *matrix);  /* 16 floats, column-major */

void grl_software_renderer_frustum       (GrlSoftwareRenderer *self,
                                          gdouble left, gdouble right,
                                          gdouble bottom, gdouble top,
                                          gdouble znear, gdouble zfar);
void grl_software_renderer_ortho         (GrlSoftwareRenderer *self,
                                          gdouble left, gdouble right,
                                          gdouble bottom, gdouble top,
                                          gdouble znear, gdouble zfar);
```

| Function | Description |
|----------|-------------|
| `matrix_mode(mode)` | Select which `GrlSwMatrixMode` stack subsequent operations affect |
| `push_matrix()` / `pop_matrix()` | Save / restore the current matrix on the active stack |
| `load_identity()` | Replace the current matrix with identity |
| `translate()` / `rotate()` / `scale()` | Multiply by translation / rotation (angle in degrees) / scaling matrices |
| `mult_matrix(matrix)` | Multiply by a caller-supplied 4×4 matrix (16 floats, column-major) |
| `frustum()` / `ortho()` | Multiply by a perspective frustum / orthographic projection |

## Immediate-Mode Geometry

Vertices specified between `begin()` and `end()` are assembled into primitives
of the given `GrlSwDrawMode`. Per-vertex color and texture coordinate are set by
the `color*` / `tex_coord2f` calls preceding each vertex.

```c
void grl_software_renderer_begin       (GrlSoftwareRenderer *self, GrlSwDrawMode mode);
void grl_software_renderer_end         (GrlSoftwareRenderer *self);

void grl_software_renderer_vertex2f    (GrlSoftwareRenderer *self, gfloat x, gfloat y);
void grl_software_renderer_vertex3f    (GrlSoftwareRenderer *self,
                                        gfloat x, gfloat y, gfloat z);

void grl_software_renderer_color3ub    (GrlSoftwareRenderer *self,
                                        guint8 r, guint8 g, guint8 b);
void grl_software_renderer_color4ub    (GrlSoftwareRenderer *self,
                                        guint8 r, guint8 g, guint8 b, guint8 a);
void grl_software_renderer_color3f     (GrlSoftwareRenderer *self,
                                        gfloat r, gfloat g, gfloat b);
void grl_software_renderer_color4f     (GrlSoftwareRenderer *self,
                                        gfloat r, gfloat g, gfloat b, gfloat a);

void grl_software_renderer_tex_coord2f (GrlSoftwareRenderer *self, gfloat u, gfloat v);
```

| Function | Description |
|----------|-------------|
| `begin(mode)` / `end()` | Begin/end a primitive of the given `GrlSwDrawMode` |
| `vertex2f()` / `vertex3f()` | Emit a 2D (`z = 0`) or 3D vertex |
| `color3ub()` / `color4ub()` | Set the current vertex color from 8-bit components |
| `color3f()` / `color4f()` | Set the current vertex color from float `[0, 1]` components |
| `tex_coord2f(u, v)` | Set the current vertex texture coordinate |

## Textures

OpenGL-1.1-style texture objects, identified by integer names (ids).

```c
guint32 grl_software_renderer_gen_texture     (GrlSoftwareRenderer *self);
void    grl_software_renderer_delete_texture  (GrlSoftwareRenderer *self, guint32 texture);
void    grl_software_renderer_bind_texture    (GrlSoftwareRenderer *self, guint32 texture);

void    grl_software_renderer_tex_image_2d    (GrlSoftwareRenderer *self,
                                               gint width, gint height,
                                               GrlSwFormat format,
                                               GrlSwType type,
                                               const guint8 *data);
void    grl_software_renderer_tex_parameteri  (GrlSoftwareRenderer *self,
                                               GrlSwTexParam param,
                                               gint value);
```

| Function | Description |
|----------|-------------|
| `gen_texture()` | Generate a texture object and return its name (id) |
| `delete_texture(texture)` | Delete a texture object |
| `bind_texture(texture)` | Bind a texture as the active 2D texture (`0` unbinds) |
| `tex_image_2d(...)` | Upload pixel data (`GrlSwFormat` layout + `GrlSwType` component type) to the bound texture; `data` may be `NULL` |
| `tex_parameteri(param, value)` | Set an integer `GrlSwTexParam` on the bound texture |

## Reading Back the Framebuffer

`read_to_image()` is the primary way to obtain rendered output: it reads an RGBA
region of the framebuffer into a new CPU-side `GrlImage` in
`GRL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8`. The returned `GrlImage` can then be
exported to a file, drawn into with the `GrlImage` software primitives, or
encoded as a frame in a `GrlGifWriter`.

```c
GrlImage * grl_software_renderer_read_to_image (GrlSoftwareRenderer *self,
                                                gint x, gint y,
                                                gint width, gint height);
```

| Function | Description |
|----------|-------------|
| `read_to_image(x, y, w, h)` | Read back an RGBA region into a new `GrlImage` (transfer full, nullable on error) |

## Example: Headless Triangle Capture

```c
#include <graylib.h>

int
main (int argc, char *argv[])
{
    g_autoptr(GrlSoftwareRenderer) sw = NULL;
    g_autoptr(GrlImage) image = NULL;

    /* No window or GL context needed */
    sw = grl_software_renderer_new (320, 240);
    if (sw == NULL)
        return 1;

    /* Clear the framebuffer to dark blue */
    grl_software_renderer_clear_color (sw, 0.1f, 0.1f, 0.2f, 1.0f);
    grl_software_renderer_clear (sw, GRL_SW_BUFFER_COLOR | GRL_SW_BUFFER_DEPTH);

    /* Orthographic projection covering the framebuffer */
    grl_software_renderer_matrix_mode (sw, GRL_SW_MATRIX_MODE_PROJECTION);
    grl_software_renderer_load_identity (sw);
    grl_software_renderer_ortho (sw, 0, 320, 0, 240, -1.0, 1.0);
    grl_software_renderer_matrix_mode (sw, GRL_SW_MATRIX_MODE_MODELVIEW);
    grl_software_renderer_load_identity (sw);

    /* Draw a Gouraud-shaded triangle in immediate mode */
    grl_software_renderer_begin (sw, GRL_SW_DRAW_MODE_TRIANGLES);
        grl_software_renderer_color3ub (sw, 255, 0, 0);
        grl_software_renderer_vertex2f (sw, 60.0f, 40.0f);

        grl_software_renderer_color3ub (sw, 0, 255, 0);
        grl_software_renderer_vertex2f (sw, 260.0f, 40.0f);

        grl_software_renderer_color3ub (sw, 0, 0, 255);
        grl_software_renderer_vertex2f (sw, 160.0f, 200.0f);
    grl_software_renderer_end (sw);

    /* Read the framebuffer back into a GrlImage and save it */
    image = grl_software_renderer_read_to_image (sw, 0, 0, 320, 240);
    if (image != NULL)
        grl_image_export (image, "triangle.png");

    return 0;
}
```

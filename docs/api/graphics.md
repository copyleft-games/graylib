# Graphics Types

This document covers the graphics resource types added in Phase 2: images, textures, and fonts.

## GrlImage

CPU-side image type for loading and manipulating pixel data before uploading to the GPU.

### Loading Images

```c
/* Load from file */
g_autoptr(GrlImage) img = grl_image_new_from_file ("sprite.png");

/* Load from memory */
g_autoptr(GrlImage) img = grl_image_new_from_memory (".png", data, data_size);

/* Capture screenshot */
g_autoptr(GrlImage) screenshot = grl_image_new_from_screen ();
```

### Generating Images

```c
/* Solid color */
g_autoptr(GrlColor) red = grl_color_new_red ();
g_autoptr(GrlImage) img = grl_image_new_color (256, 256, red);

/* Gradients */
g_autoptr(GrlImage) linear = grl_image_new_gradient_linear (256, 256, 0, start, end);
g_autoptr(GrlImage) radial = grl_image_new_gradient_radial (256, 256, 0.5f, inner, outer);

/* Patterns */
g_autoptr(GrlImage) checker = grl_image_new_checked (256, 256, 8, 8, color1, color2);
g_autoptr(GrlImage) noise = grl_image_new_white_noise (256, 256, 1.0f);
g_autoptr(GrlImage) perlin = grl_image_new_perlin_noise (256, 256, 0, 0, 4.0f);
g_autoptr(GrlImage) cells = grl_image_new_cellular (256, 256, 16);
```

### Image Transformations

```c
/* Resize (bilinear interpolation) */
grl_image_resize (img, 128, 128);

/* Resize (nearest neighbor - preserves pixels) */
grl_image_resize_nearest (img, 128, 128);

/* Resize canvas (add padding) */
grl_image_resize_canvas (img, 300, 300, 50, 50, bg_color);

/* Crop to region */
grl_image_crop (img, rect);

/* Flip (mirror) */
grl_image_flip_vertical (img);
grl_image_flip_horizontal (img);

/* Rotate by any angle, clockwise, -359..359 (90/180/270 are exact/lossless) */
grl_image_rotate (img, 45);
grl_image_rotate_cw (img);    /* 90 degrees clockwise */
grl_image_rotate_ccw (img);   /* 90 degrees counter-clockwise */

/* Scale to a NEW image, leaving the original untouched */
g_autoptr(GrlImage) big = grl_image_resized (img, 256, 256);          /* bicubic */
g_autoptr(GrlImage) px  = grl_image_scaled_nearest (img, 64, 64);     /* pixel-perfect */
```

### Color Operations

```c
/* Apply color tint */
grl_image_color_tint (img, tint_color);

/* Invert colors */
grl_image_color_invert (img);

/* Convert to grayscale */
grl_image_color_grayscale (img);

/* Adjust contrast (-100 to 100) */
grl_image_color_contrast (img, 20.0f);

/* Adjust brightness (-255 to 255) */
grl_image_color_brightness (img, 40);

/* Replace a specific color */
grl_image_color_replace (img, old_color, new_color);
```

### Alpha Operations

```c
/* Premultiply alpha */
grl_image_alpha_premultiply (img);

/* Crop to non-transparent region */
grl_image_alpha_crop (img, 0.0f);

/* Clear pixels matching color to transparent */
grl_image_alpha_clear (img, color, 0.1f);

/* Apply alpha mask from another image */
grl_image_alpha_mask (img, mask_img);
```

### Drawing on Images

```c
/* Clear with background color */
grl_image_clear_background (img, color);

/* Basic primitives (raylib-backed, overwrite the destination) */
grl_image_draw_pixel (img, x, y, color);
grl_image_draw_line (img, x1, y1, x2, y2, color);
grl_image_draw_circle (img, cx, cy, radius, color);
grl_image_draw_rectangle (img, rect, color);
grl_image_draw_rectangle_lines (img, rect, thickness, color);

/* Composite another image. tint is nullable: pass NULL for "no tint" (white). */
grl_image_draw_image (img, src, src_rect, dst_rect, NULL);

/* Read a pixel back */
g_autoptr(GrlColor) c = grl_image_get_pixel (img, x, y);
```

### Additional Drawing Primitives

These primitives honour the image's [drawing state](#drawing-state-blend-modes-clipping-anti-aliasing)
(blend mode, clip rectangle and anti-aliasing). Point arguments are `GrlVector2`.

```c
g_autoptr(GrlVector2) a = grl_vector2_new (10.0f, 10.0f);
g_autoptr(GrlVector2) b = grl_vector2_new (90.0f, 70.0f);

/* Thick lines, with round caps */
grl_image_draw_line_ex (img, a, b, 3, color);

/* Circle and ellipse: filled and outlined (outlines take a thickness) */
grl_image_draw_circle_lines (img, cx, cy, radius, thickness, color);
grl_image_draw_ellipse (img, cx, cy, rx, ry, color);
grl_image_draw_ellipse_lines (img, cx, cy, rx, ry, thickness, color);

/* Triangles: filled and outlined */
grl_image_draw_triangle (img, v1, v2, v3, color);
grl_image_draw_triangle_lines (img, v1, v2, v3, thickness, color);

/* Polygons (even-odd fill, supports concave) and polylines */
grl_image_draw_polygon (img, points, n_points, color);
grl_image_draw_polyline (img, points, n_points, /*closed*/ TRUE, thickness, color);

/* Cubic Bezier curve as a thick stroke */
grl_image_draw_bezier (img, p0, p1, p2, p3, thickness, color);

/* Gradients drawn ONTO the existing image (cf. grl_image_new_gradient_*) */
grl_image_draw_gradient_rect (img, rect, color_a, color_b, GRL_GRADIENT_AXIS_VERTICAL);
grl_image_draw_gradient_radial (img, cx, cy, radius, inner, outer);   /* glows / halos */

/* Flood fill a contiguous region matching the seed pixel within a tolerance */
grl_image_flood_fill (img, x, y, color, /*tolerance*/ 0);
```

#### Sub-pixel thin strokes

`grl_image_draw_line_ex()` takes an integer thickness (minimum 1px).
`grl_image_draw_line_thin()` takes a **fractional** thickness and positions the
stroke with sub-pixel precision, so thin diagonals stay smooth and shift
continuously as the endpoints move by fractions of a pixel — instead of snapping
to the integer grid and shimmering between frames. Widths below one pixel are
rendered as faded hairlines (the peak coverage is scaled by the requested
fraction) rather than a hard 1px line.

```c
g_autoptr(GrlVector2) a = grl_vector2_new (1.5f, 0.0f);
g_autoptr(GrlVector2) b = grl_vector2_new (1.5f, 64.0f);

grl_image_set_antialias (img, TRUE);          /* required for sub-pixel widths */
grl_image_draw_line_thin (img, a, b, 0.75f, color);
```

Sub-pixel widths only show up on the coverage-based (anti-aliased) path; enable
anti-aliasing first. For thickness >= 1 this behaves exactly like
`grl_image_draw_line_ex()`.

### Drawing State (Blend Modes, Clipping, Anti-aliasing)

Per-image state controls how every `grl_image_draw_*` primitive combines with the
canvas. The default blend mode, `GRL_IMAGE_BLEND_REPLACE`, is byte-for-byte
identical to drawing with no state set (source overwrites destination).

```c
/* Blend modes. Modes other than REPLACE require an R8G8B8A8 image; on other
 * formats drawing silently falls back to REPLACE. */
grl_image_set_blend_mode (img, GRL_IMAGE_BLEND_OVER);      /* alpha compositing  */
grl_image_set_blend_mode (img, GRL_IMAGE_BLEND_ADD);       /* additive (glow)    */
grl_image_set_blend_mode (img, GRL_IMAGE_BLEND_MULTIPLY);
grl_image_set_blend_mode (img, GRL_IMAGE_BLEND_SUBTRACT);
grl_image_set_blend_mode (img, GRL_IMAGE_BLEND_REPLACE);   /* default            */

/* Clip subsequent drawing to a rectangle (NULL clears) */
grl_image_set_clip_rect (img, clip_rect);
grl_image_set_clip_rect (img, NULL);

/* Anti-alias the edges of curved/outline primitives (requires R8G8B8A8) */
grl_image_set_antialias (img, TRUE);
```

A proper additive glow that saturates to white at the core (instead of the
over-blend that an alpha stack produces):

```c
g_autoptr(GrlColor) hot  = grl_color_new (255, 240, 200, 255);
g_autoptr(GrlColor) edge = grl_color_new (255, 120,  30, 255);

grl_image_set_blend_mode (img, GRL_IMAGE_BLEND_ADD);
grl_image_draw_gradient_radial (img, cx, cy, radius, hot, edge);
grl_image_set_blend_mode (img, GRL_IMAGE_BLEND_REPLACE);
```

#### Linear-light (sRGB-correct) blending

By default, blending math runs directly on 8-bit sRGB values
(`GRL_IMAGE_COLOR_SPACE_GAMMA`). That is fast and historically compatible, but
it is not physically correct: blending and anti-aliased coverage in gamma space
makes edges look too dark and shifts hues at partial coverage (a 50%-covered
white edge over black comes out at sRGB 128, perceptually much darker than true
half-brightness).

Switch to `GRL_IMAGE_COLOR_SPACE_LINEAR` to composite in linear light — pixels
are decoded sRGB→linear, blended, then re-encoded — so `OVER` / `ADD` /
`MULTIPLY` / `SUBTRACT` and anti-aliased edges look correct:

```c
/* Correct, non-darkening anti-aliased compositing. */
grl_image_set_blend_color_space (img, GRL_IMAGE_COLOR_SPACE_LINEAR);
grl_image_set_antialias (img, TRUE);
grl_image_set_blend_mode (img, GRL_IMAGE_BLEND_OVER);
grl_image_draw_text_bitmap (img, "Crisp", 8, 8, 16, white);

grl_image_set_blend_color_space (img, GRL_IMAGE_COLOR_SPACE_GAMMA); /* restore */
```

That same white(α=128)-over-black draw now resolves to sRGB ≈ 188 — perceptual
half-brightness — instead of 128.

Linear blending requires an `R8G8B8A8` image (it reads the destination back); on
any other format the draw silently falls back to gamma blending. It never
affects `GRL_IMAGE_BLEND_REPLACE`, which always overwrites and is unconditionally
byte-identical. The conversion uses lookup tables, so there is no per-pixel
`pow()` cost; only the slow (non-REPLACE, non-opaque) path pays for it at all.

#### Compositing (Porter-Duff)

`grl_image_composite()` applies one of the twelve standard Porter-Duff operators
from the original 1984 paper to composite a source image onto a destination at a
given pixel offset. All math is performed on **premultiplied alpha** (graylib
images store straight alpha and the conversion is applied internally). The
result is stored back as straight alpha.

```c
/* Composite a semi-transparent sprite over a background */
grl_image_composite (background, sprite, GRL_PORTER_DUFF_SRC_OVER, x, y);
```

**Contract:**
- Both images must be `R8G8B8A8`; if either is not, the call returns silently (no crash).
- Pixels of `self` that lie outside the mapped area of `src` are left **unchanged**.
- `self`'s clip rectangle is honoured.
- If `self`'s blend colour space is `GRL_IMAGE_COLOR_SPACE_LINEAR`, compositing
  is performed in linear light using the sRGB LUTs.

**Operator quick reference** (`sa` = source alpha, `da` = destination alpha):

| Operator | Fa | Fb | Effect |
|----------|----|----|--------|
| `CLEAR` | 0 | 0 | Transparent black everywhere |
| `SRC` | 1 | 0 | Source replaces destination |
| `DST` | 0 | 1 | Destination unchanged (no-op) |
| `SRC_OVER` | 1 | 1−sa | Standard alpha compositing |
| `DST_OVER` | 1−da | 1 | Source placed *behind* destination |
| `SRC_IN` | da | 0 | Source inside destination's shape |
| `DST_IN` | 0 | sa | Destination inside source's shape |
| `SRC_OUT` | 1−da | 0 | Source outside destination's shape |
| `DST_OUT` | 0 | 1−sa | Destination outside source's shape |
| `SRC_ATOP` | da | 1−sa | Source on top, destination shows elsewhere |
| `DST_ATOP` | 1−da | sa | Destination on top, source shows elsewhere |
| `XOR` | 1−da | 1−sa | Exclusive OR of shapes |

> **Note on naming:** `DST_OVER` is sometimes called `DEST_OVER` in other APIs
> (e.g. Canvas 2D); they name the same operation.

#### Masks

`grl_image_new_mask()` creates a single-channel `GRAYSCALE` (1 byte/pixel) image
initialised to zero (fully transparent coverage). Use the normal draw primitives
to paint coverage into the mask — white (255) = fully opaque, black (0) = fully
transparent.

`grl_image_apply_mask()` multiplies the alpha channel of each destination pixel
by the corresponding coverage value in the mask.

**Outside-mask contract:** pixels of `self` that map *outside* the mask's
bounds have their alpha **set to zero** (cut). The mask defines exactly which
region survives — this is the standard stencil / clipping contract, consistent
with raylib's `ImageAlphaMask`.

```c
/* Create a circular mask */
g_autoptr(GrlImage) mask = grl_image_new_mask (128, 128);
grl_image_draw_ellipse (mask, 64, 64, 60, 60,
                        grl_color_new (255, 255, 255, 255));

/* Apply mask: pixels outside the ellipse get alpha = 0 */
grl_image_apply_mask (img, mask, 0, 0);
```

#### Drop Shadows and Glow

Soft drop shadows and glow effects combine `grl_image_new_mask()`,
`grl_image_blur_box()`, and `grl_image_composite()`:

```c
/* Step 1: Create a silhouette of the source in a new RGBA image */
g_autoptr(GrlImage) shadow = grl_image_new_color (
    grl_image_get_width (sprite), grl_image_get_height (sprite),
    grl_color_new (0, 0, 0, 0));  /* fully transparent canvas */

/* Draw the shadow solid black at full source alpha */
{
    g_autoptr(GrlImage) src_copy = grl_image_copy (sprite);
    grl_image_color_tint (src_copy, grl_color_new (0, 0, 0, 255));
    grl_image_composite (shadow, src_copy,
                         GRL_PORTER_DUFF_SRC_OVER, 0, 0);
}

/* Step 2: Blur the silhouette to get a soft halo */
grl_image_blur_box (shadow, 6);

/* Step 3: Composite the blurred shadow *behind* the background:
 *   DST_OVER puts shadow behind whatever is already in the canvas. */
grl_image_composite (canvas, shadow, GRL_PORTER_DUFF_DST_OVER,
                     sprite_x + 4, sprite_y + 4);  /* shadow offset */

/* Step 4: Draw the sprite on top */
grl_image_composite (canvas, sprite, GRL_PORTER_DUFF_SRC_OVER,
                     sprite_x, sprite_y);
```

For a **glow** instead of a shadow: tint the silhouette in the glow colour
(e.g. bright cyan), blur it, and composite it *behind* the sprite with
`DST_OVER`. The sprite itself is always drawn last on top with `SRC_OVER`.

**Box blur** (`grl_image_blur_box()`) applies a separable (horizontal then
vertical) box blur of the given pixel radius. All four channels (R, G, B, A)
are blurred, so shadow alphas get soft edges correctly. `radius <= 0` is a
no-op. The image must be `R8G8B8A8`; on other formats returns silently.

```c
/* Blur a shadow silhouette with a radius of 8 pixels */
grl_image_blur_box (shadow, 8);
```

#### Transform stack

A 2D affine transform stack lets you author at a natural scale and position
without baking coordinates by hand. The current transform is applied to the
geometry of the blend-aware primitives at draw time.

```c
grl_image_push_matrix (img);
grl_image_translate (img, 100.0f, 80.0f);
grl_image_rotate_matrix (img, G_PI_4);     /* radians */
grl_image_scale (img, 2.0f, 2.0f);
grl_image_draw_polygon (img, points, n, color);  /* drawn transformed */
grl_image_pop_matrix (img);                 /* restore previous transform */

/* Also: grl_image_reset_matrix(), grl_image_set_matrix(img, m_or_NULL),
 * grl_image_get_matrix(img) -> (transfer full) GrlMatrix. */
```

The transform affects the point-list and centre/radius primitives that route
through the software rasterizer: `draw_line_ex`, `draw_line_thin`,
`draw_circle_lines`, `draw_ellipse`, `draw_ellipse_lines`, `draw_triangle`,
`draw_triangle_lines`, `draw_polygon`, `draw_polyline`, `draw_bezier`. Stroke
thickness and circle/ellipse radii are scaled by the transform's mean
(area-preserving) factor.

**Limitations** (by design — these favour speed and simplicity):

- Point-list primitives (polygons, triangles, lines, beziers) transform fully,
  including rotation and shear. Centre/radius primitives (circles, ellipses)
  transform their centre and scale their radius by the mean factor, so a
  **non-uniform scale keeps a circle circular** rather than turning it into an
  ellipse — use `grl_image_draw_ellipse()` or a polygon for that, and note that
  circle/ellipse outlines do not represent rotation/shear of the ring.
- The legacy raylib-backed overwrite primitives (`grl_image_draw_pixel` /
  `_line` / `_circle` / `_rectangle`) and the gradient fills ignore the
  transform. The default transform is the identity, so output is byte-identical
  unless you set a transform.
- The stack depth is capped at 256; pushing past it or popping an empty stack
  warns and is a no-op.

### Drawing Text on Images

```c
/* Uses raylib's default font when a window exists; falls back to graylib's
 * embedded CPU bitmap font when headless (no InitWindow). Always safe. */
grl_image_draw_text (img, "Hello", x, y, 20, color);

/* Always the embedded CPU bitmap font: identical headless or windowed, never
 * touches a GL context, and honours blend mode / clip / anti-aliasing. */
grl_image_draw_text_bitmap (img, "Hello", x, y, 20, color);

/* Measure text laid out by grl_image_draw_text_bitmap() (x = width, y = height) */
g_autoptr(GrlVector2) size = grl_image_measure_text_bitmap ("Hello", 20);
```

> **Headless note:** `grl_image_draw_text()` previously crashed when called
> without a window because raylib's default-font loader lays out its glyph atlas
> against a GL texture that does not exist yet. graylib now detects the headless
> case and renders with an embedded bitmap font, so image text drawing works in
> asset bakers, tests and CLI tools.

### Exporting Images

```c
/* Save to file (format determined by extension) */
grl_image_export (img, "output.png");

/* Export to memory buffer */
gsize size;
guint8 *data = grl_image_export_to_memory (img, ".png", &size);
g_free (data);
```

## GrlGifWriter

A streaming, multi-frame animated GIF writer. Open the file, append frames (each
with its own delay), then close. Frames are converted to RGBA and scaled to the
canvas size if needed, then quantised to a 6×6×6 web-safe palette. Works fully
headless.

```c
g_autoptr(GError) error = NULL;

/* width, height, loop_count (0 = loop forever) */
GrlGifWriter *gif = grl_gif_writer_new ("anim.gif", 320, 240, 0, &error);

for (gint i = 0; i < frame_count; i++)
{
    g_autoptr(GrlImage) frame = render_frame (i);

    /* delay is in centiseconds (1/100 s); 5 => 20 fps */
    grl_gif_writer_add_frame (gif, frame, 5, &error);
}

grl_gif_writer_close (gif, &error);
g_object_unref (gif);
```

> **Note:** the encoder is intentionally simple (uncompressed GIF LZW), trading
> file size for broad compatibility. The output decodes in every conformant GIF
> reader.

### Quantization & dithering

By default `GrlGifWriter` uses a fixed 6×6×6 web-safe palette (216 colours),
producing output byte-for-byte identical to the original encoder. For better
colour fidelity, switch to the adaptive **median-cut** quantizer:

```c
GrlGifWriter *w = grl_gif_writer_new ("out.gif", 320, 240, 0, &err);

grl_image_set... /* configure before the first frame: */
grl_gif_writer_set_quantizer     (w, GRL_GIF_QUANTIZER_MEDIAN_CUT);
grl_gif_writer_set_max_colors    (w, 128);
grl_gif_writer_set_dither        (w, GRL_GIF_DITHER_FLOYD_STEINBERG);
grl_gif_writer_set_palette_scope (w, GRL_GIF_PALETTE_SCOPE_GLOBAL);

grl_gif_writer_add_frame (w, frame, 4, &err);
grl_gif_writer_close (w, &err);
```

| Setter | Default | Notes |
|--------|---------|-------|
| `grl_gif_writer_set_quantizer()` | `WEB_SAFE` | `MEDIAN_CUT` for an adaptive palette |
| `grl_gif_writer_set_max_colors()` | 256 | Clamped to [2, 256] |
| `grl_gif_writer_set_dither()` | `NONE` | `FLOYD_STEINBERG` reduces banding on gradients |
| `grl_gif_writer_set_palette_scope()` | `GLOBAL` | `PER_FRAME` gives each frame its own local colour table |
| `grl_gif_writer_set_transparency()` | off, 128 | Reserves one palette slot; sets the GCE transparency flag |

**Palette scope:** `GLOBAL` builds the palette from the first frame and reuses it
(smaller files); `PER_FRAME` quantises each frame independently with a GIF Local
Color Table (larger files, best per-frame accuracy).

**Backward compatibility:** with no setters called, all defaults are in effect
and the output is byte-for-byte identical to the previous encoder.

## GrlImageAccumulator

`GrlImageAccumulator` sums multiple `GrlImage` sub-frames in float precision and
resolves the weighted average back to an 8-bit RGBA image. Typical uses:

- **Motion blur** — render N evenly-spaced sub-frames of an animation and average
  them with equal weights.
- **Temporal anti-aliasing** — accumulate a few jittered samples per output frame.

Pass `linear = TRUE` to convert sRGB samples to linear light before accumulating
and back on resolve. Linear accumulation avoids the perceptual darkening that
straight-gamma averaging produces (the midpoint of black and white in linear
light resolves to sRGB ≈ 188, not 128).

```c
GrlImageAccumulator *acc = grl_image_accumulator_new (320, 240, /*linear*/ TRUE);
g_autoptr(GrlGifWriter) gif = grl_gif_writer_new ("blur.gif", 320, 240, 0, NULL);
gint frame, n;

for (frame = 0; frame < n_frames; frame++)
  {
    grl_image_accumulator_reset (acc);

    /* Sample the animation at sub-frame times within this frame's shutter. */
    for (n = 0; n < n_samples; n++)
      {
        gdouble t = (frame + (n + 0.5) / n_samples) / (gdouble) n_frames;
        g_autoptr(GrlImage) sub = render_sub_frame (t);
        grl_image_accumulator_add (acc, sub, 1.0f);
      }

    {
      g_autoptr(GrlImage) avg = grl_image_accumulator_resolve (acc);
      grl_gif_writer_add_frame (gif, avg, 4, NULL);
    }
  }

grl_gif_writer_close (gif, NULL);
g_object_unref (acc);
```

With `n_samples == 1` the output is identical to a single render (no blur).
Sub-frames whose dimensions differ from the canvas are resampled
(nearest-neighbour). `resolve()` returns `NULL` if no samples were added.

## Onion-skinning & temporal AA

Two higher-level helpers in `grl-image-temporal.h` sit on top of the drawing
and accumulation primitives.

### Onion-skinning (`grl_image_onion_skin`)

Composites semi-transparent "ghost" frames of the previous and/or next
animation frame onto a **current** frame in-place (OVER blending) — the standard
visualisation in frame-by-frame animation tools.

```c
g_autoptr(GrlColor) red  = grl_color_new (255, 0,   0, 255);
g_autoptr(GrlColor) blue = grl_color_new (0,   0, 255, 255);

/* 50% red ghost of the previous frame, 30% blue ghost of the next frame. */
grl_image_onion_skin (current, prev_frame, next_frame,
                      0.5f, 0.3f, red, blue);
```

`@prev`/`@next` and the tints are nullable; opacity ≤ 0 skips that ghost. The
tint's alpha is overridden by the opacity, so you can pass a pure hue. The
function saves and restores `@current`'s blend mode. `@current` must be
`R8G8B8A8`; on other formats it logs a debug message and returns unchanged.

### Temporal AA (`grl_image_temporal_aa`)

A thin helper over `GrlImageAccumulator` for jittered super-sampling: render N
samples each with a small sub-pixel offset, then average them.

```c
/* Jitter recommendation: Halton(2,3) or a rotated grid in [-0.5, +0.5] px. */
GrlImage *out = grl_image_temporal_aa (acc, samples, n_samples);
```

The accumulator is reset on each call (reuse it across frames). With
`n_samples == 1` the result equals the single sample; `n_samples == 0` or a
`NULL` accumulator returns `NULL`.

## Vector paths (`GrlPath`)

`GrlPath` is a retained 2D vector path type.  A path is built from subpaths
using SVG-like builder verbs, then rasterized onto a `GrlImage` using
`grl_image_fill_path` or `grl_image_stroke_path`.

### Building a path

```c
g_autoptr(GrlPath) path = grl_path_new ();

/* Move / line */
grl_path_move_to (path, 10.0f, 10.0f);
grl_path_line_to (path, 90.0f, 10.0f);
grl_path_line_to (path, 90.0f, 90.0f);
grl_path_line_to (path, 10.0f, 90.0f);
grl_path_close (path);

/* Cubic Bézier (two control points) */
grl_path_move_to  (path,   0.0f,  50.0f);
grl_path_cubic_to (path,  25.0f,   0.0f,   /* CP1 */
                           75.0f, 100.0f,   /* CP2 */
                          100.0f,  50.0f);  /* end */

/* Quadratic Bézier (one control point) */
grl_path_move_to (path,   0.0f, 50.0f);
grl_path_quad_to (path,  50.0f,  0.0f,   /* CP */
                         100.0f, 50.0f);  /* end */
```

### Shape helpers

```c
GrlRectangle rect = { 10.0f, 10.0f, 80.0f, 50.0f };
grl_path_add_rect    (path, &rect);

grl_path_add_circle  (path, cx, cy, radius);
grl_path_add_ellipse (path, cx, cy, rx, ry);
```

### Introspection

```c
if (grl_path_is_empty (path))
    return;

guint n = grl_path_get_subpath_count (path);

GrlRectangle bounds;
if (grl_path_get_bounds (path, &bounds))
    g_print ("%.1f × %.1f\n", bounds.width, bounds.height);
```

### Transforms

All transforms are applied **in-place** to the path's control points.

```c
grl_path_translate (path, dx, dy);
grl_path_scale     (path, sx, sy);
grl_path_rotate    (path, degrees);   /* clockwise */

/* Arbitrary affine matrix */
GrlMatrix *m = grl_matrix_new_rotate_z (G_PI / 6.0f);
grl_path_transform (path, m);
grl_matrix_free (m);
```

### Flattening (to polylines)

```c
guint       n_sub, total;
guint      *lengths;
GrlVector2 *pts = grl_path_get_flattened (path, 0.5f /* tolerance */,
                                           &lengths, &n_sub, &total);
/* pts[0..lengths[0]-1]  = subpath 0 */
/* pts[lengths[0].....]  = subpath 1, etc. */
g_free (pts);
g_free (lengths);
```

### Copy

```c
g_autoptr(GrlPath) copy = grl_path_copy (path);
grl_path_translate (copy, 100.0f, 0.0f);   /* original unaffected */
```

### Filling and stroking onto `GrlImage`

```c
g_autoptr(GrlColor) red  = grl_color_new (220, 50, 50, 255);
g_autoptr(GrlColor) blue = grl_color_new (50, 100, 220, 255);

/* Fill — winding rules control how self-intersecting paths are filled */
grl_image_fill_path (img, path, GRL_FILL_RULE_NONZERO,  red);
grl_image_fill_path (img, path, GRL_FILL_RULE_EVEN_ODD, blue);

/* Stroke — thickness in image pixels (scaled by the current CTM) */
grl_image_stroke_path (img, path, 3, blue);
```

The image's current **blend mode**, **clip rectangle**, **anti-alias flag**, and
**transform matrix** are all honoured by both calls.

### Winding rules

| Rule | Fills when… |
|------|-------------|
| `GRL_FILL_RULE_NONZERO` | winding count ≠ 0 (default SVG / PDF behaviour) |
| `GRL_FILL_RULE_EVEN_ODD` | crossing count is odd (alternate fill) |

A CCW inner contour inside a CW outer contour produces a "donut" hole under
`NONZERO`; under `EVEN_ODD` any enclosed region toggles in/out.

### Raster-approximate boolean operations

Boolean ops rasterize both operands to a coverage mask, combine them
pixel-by-pixel, and convert the result back to a scanline-rectangle path.
They are approximate (precision controlled by the 4× internal raster
resolution) but always produce a valid, fillable `GrlPath`.

```c
g_autoptr(GrlPath) a = build_shape_a ();
g_autoptr(GrlPath) b = build_shape_b ();

g_autoptr(GrlPath) u   = grl_path_union     (a, b);
g_autoptr(GrlPath) ix  = grl_path_intersect (a, b);
g_autoptr(GrlPath) sub = grl_path_subtract  (a, b);  /* a minus b */
g_autoptr(GrlPath) xr  = grl_path_xor       (a, b);  /* symmetric difference */

grl_image_fill_path (img, u, GRL_FILL_RULE_NONZERO, fill_color);
```

Passing an empty path as an operand is safe; e.g. `union(∅, b) ≡ b`.

## Headless TTF/OTF text

`GrlImageFont` (`src/graphics/grl-image-font.h`) wraps
[stb_truetype](https://github.com/nothings/stb) to render scalable TrueType and
OpenType fonts onto `GrlImage` buffers without a GL context — ideal for
asset baking and CLI tools. (The 8×8 bitmap API, `grl_image_draw_text_bitmap()`,
remains the zero-dependency fallback for pixel-art aesthetics.)

```c
g_autoptr(GError) error = NULL;
g_autoptr(GrlImageFont) font =
    grl_image_font_new_from_file ("/usr/share/fonts/dejavu-sans-fonts/DejaVuSans.ttf",
                                  &error);
if (font == NULL)
  {
    g_warning ("font load failed: %s", error->message);
    return;
  }

g_autoptr(GrlColor) black = grl_color_new (0, 0, 0, 255);
grl_image_draw_text_ttf (image, font, "Hello, world!", 10, 10, 32.0f, black);

/* Measure (advance width in .x, total height in .y) and query metrics. */
g_autoptr(GrlVector2) size = grl_image_measure_text_ttf (font, "Hello", 32.0f);
gfloat ascent, descent, line_gap;
grl_image_font_get_v_metrics (font, 32.0f, &ascent, &descent, &line_gap);
```

`grl_image_font_new_from_memory()` loads from a byte buffer (copied). `(x, y)`
is the top-left of the first line's em box; the colour's alpha is modulated by
the per-pixel anti-aliasing coverage. The text is composited
`GRL_PORTER_DUFF_SRC_OVER`, honouring the destination's clip rectangle and blend
colour space (the per-image *blend mode* is not consulted for this operation).

**Limitations (v1):** stb provides minimal hinting, so small sizes (< ~14 px)
look slightly soft — prefer ≥ 14 px. Layout is left-to-right, one codepoint per
glyph (advance + kerning); RTL/bidi, combining marks, ligatures, and vertical
text are out of scope (HarfBuzz-class shaping is future work).

## GrlTexture

GPU texture for efficient rendering. Textures are created from images or loaded directly.

### Creating Textures

```c
/* Load directly from file */
g_autoptr(GrlTexture) tex = grl_texture_new_from_file ("sprite.png");

/* Create from image (recommended for manipulation) */
g_autoptr(GrlImage) img = grl_image_new_from_file ("sprite.png");
grl_image_resize (img, 64, 64);
g_autoptr(GrlTexture) tex = grl_texture_new_from_image (img);

/* Load from memory */
g_autoptr(GrlTexture) tex = grl_texture_new_from_memory (".png", data, size);
```

### Texture Properties

```c
gint width = grl_texture_get_width (tex);
gint height = grl_texture_get_height (tex);
gint mipmaps = grl_texture_get_mipmaps (tex);
GrlPixelFormat format = grl_texture_get_format (tex);
gboolean valid = grl_texture_is_valid (tex);
```

### Texture Configuration

```c
/* Generate mipmaps for smooth scaling */
grl_texture_gen_mipmaps (tex);

/* Set filtering mode */
grl_texture_set_filter (tex, GRL_TEXTURE_FILTER_POINT);     /* Pixelated */
grl_texture_set_filter (tex, GRL_TEXTURE_FILTER_BILINEAR);  /* Smooth */
grl_texture_set_filter (tex, GRL_TEXTURE_FILTER_TRILINEAR); /* Smooth + mipmaps */

/* Set wrapping mode */
grl_texture_set_wrap (tex, GRL_TEXTURE_WRAP_REPEAT);        /* Tile */
grl_texture_set_wrap (tex, GRL_TEXTURE_WRAP_CLAMP);         /* Extend edges */
grl_texture_set_wrap (tex, GRL_TEXTURE_WRAP_MIRROR_REPEAT); /* Mirror + tile */
```

### Updating Textures

```c
/* Update entire texture from image */
grl_texture_update (tex, new_image);

/* Update rectangular region */
grl_texture_update_rec (tex, rect, pixel_data);
```

### Downloading Texture Data

```c
/* Download from GPU to create image */
g_autoptr(GrlImage) img = grl_texture_to_image (tex);
grl_image_export (img, "screenshot.png");
```

## GrlFont

Font type for text rendering. Supports TTF, OTF, FNT, and sprite fonts.

### Loading Fonts

```c
/* Use default raylib font */
g_autoptr(GrlFont) font = grl_font_new_default ();

/* Load from file */
g_autoptr(GrlFont) font = grl_font_new_from_file ("fonts/arial.ttf");

/* Load with specific size and characters */
g_autoptr(GrlFont) font = grl_font_new_from_file_ex ("fonts/arial.ttf", 32, NULL, 0);

/* Load from sprite image (XNA-style) */
g_autoptr(GrlFont) font = grl_font_new_from_image (img, magenta_key, 32);

/* Load from memory */
g_autoptr(GrlFont) font = grl_font_new_from_memory (".ttf", data, size, 24, NULL, 0);
```

### Font Properties

```c
gint base_size = grl_font_get_base_size (font);
gint glyph_count = grl_font_get_glyph_count (font);
gint padding = grl_font_get_glyph_padding (font);
gboolean valid = grl_font_is_valid (font);
```

### Text Measurement

```c
/* Measure text dimensions */
g_autoptr(GrlVector2) size = grl_font_measure_text (font, "Hello World", 24.0f, 2.0f);
g_print ("Text size: %.0f x %.0f\n", size->x, size->y);

/* Measure with default font */
gint width = grl_measure_text ("Hello", 20);
```

### Glyph Information

```c
/* Get glyph index for a character */
gint index = grl_font_get_glyph_index (font, 'A');

/* Get glyph rectangle in font atlas */
g_autoptr(GrlRectangle) rec = grl_font_get_glyph_atlas_rec (font, 'A');
```

### Texture Filtering

```c
/* Enable smooth anti-aliased text */
grl_font_set_filter (font, GRL_TEXTURE_FILTER_BILINEAR);

/* Use pixel-perfect crisp text (default) */
grl_font_set_filter (font, GRL_TEXTURE_FILTER_POINT);
```

## Drawing Textures

All drawing must occur between `grl_window_begin_drawing()` and `grl_window_end_drawing()`.

```c
/* Simple draw at position */
grl_draw_texture (tex, 100, 100, white);
grl_draw_texture_v (tex, position, white);

/* Draw with rotation and scale */
grl_draw_texture_ex (tex, position, 45.0f, 2.0f, white);

/* Draw portion of texture (sprite sheets) */
grl_draw_texture_rec (tex, source_rect, position, white);

/* Full control: source, dest, origin, rotation */
grl_draw_texture_pro (tex, source, dest, origin, rotation, white);
```

## Drawing Text

```c
/* Draw with default font */
grl_draw_text ("Hello World!", 100, 100, 20, white);

/* Draw FPS counter */
grl_draw_fps (10, 10);

/* Draw with custom font */
grl_draw_text_ex (font, "Hello", position, 24.0f, 2.0f, white);

/* Draw with rotation */
grl_draw_text_pro (font, "Rotated", position, origin, 45.0f, 24.0f, 2.0f, white);

/* Draw single Unicode character */
grl_draw_text_codepoint (font, 0x263A, position, 32.0f, white);  /* Smiley face */
```

## GrlShader

Custom shader programs for advanced rendering effects.

### Creating Shaders

```c
/* Load from files */
g_autoptr(GError) error = NULL;
g_autoptr(GrlShader) shader = grl_shader_new_from_file (
    "shaders/vertex.vs",
    "shaders/fragment.fs",
    &error
);

/* Use default vertex shader with custom fragment */
g_autoptr(GrlShader) shader = grl_shader_new_from_file (
    NULL,                    /* Use default vertex shader */
    "shaders/fragment.fs",
    &error
);

/* Create from code strings */
const gchar *fs_code =
    "#version 330\n"
    "in vec2 fragTexCoord;\n"
    "in vec4 fragColor;\n"
    "uniform sampler2D texture0;\n"
    "uniform float time;\n"
    "out vec4 finalColor;\n"
    "void main() {\n"
    "    vec4 texColor = texture(texture0, fragTexCoord);\n"
    "    finalColor = texColor * fragColor;\n"
    "}\n";

g_autoptr(GrlShader) shader = grl_shader_new_from_memory (
    NULL,      /* Default vertex shader */
    fs_code,
    &error
);
```

### Using Shaders

```c
/* Check if valid */
if (!grl_shader_is_valid (shader))
{
    g_printerr ("Shader failed to compile\n");
    return;
}

/* Begin shader mode */
grl_shader_begin (shader);

/* Draw with shader applied */
grl_draw_texture (texture, x, y, white);

/* End shader mode */
grl_shader_end (shader);
```

### Setting Uniform Values

```c
/* Get uniform location (cache this for performance) */
gint time_loc = grl_shader_get_location (shader, "time");
gint resolution_loc = grl_shader_get_location (shader, "resolution");
gint color_loc = grl_shader_get_location (shader, "tintColor");

/* Set values */
grl_shader_set_value_float (shader, time_loc, elapsed_time);
grl_shader_set_value_vec2 (shader, resolution_loc, 800.0f, 600.0f);
grl_shader_set_value_vec4 (shader, color_loc, 1.0f, 0.5f, 0.2f, 1.0f);

/* Set texture uniform */
gint tex_loc = grl_shader_get_location (shader, "noiseTexture");
grl_shader_set_value_texture (shader, tex_loc, noise_texture);
```

### Shader Methods

| Method | Description |
|--------|-------------|
| `is_valid()` | Check if shader compiled successfully |
| `begin()` | Begin using this shader for drawing |
| `end()` | Return to default shader |
| `get_location(name)` | Get uniform location by name |
| `get_location_attrib(name)` | Get attribute location by name |
| `set_value_float(loc, value)` | Set float uniform |
| `set_value_int(loc, value)` | Set integer uniform |
| `set_value_vec2(loc, x, y)` | Set vec2 uniform |
| `set_value_vec3(loc, x, y, z)` | Set vec3 uniform |
| `set_value_vec4(loc, x, y, z, w)` | Set vec4 uniform |
| `set_value_texture(loc, texture)` | Set sampler2D uniform |

### Example: Grayscale Shader

```c
const gchar *grayscale_fs =
    "#version 330\n"
    "in vec2 fragTexCoord;\n"
    "in vec4 fragColor;\n"
    "uniform sampler2D texture0;\n"
    "out vec4 finalColor;\n"
    "void main() {\n"
    "    vec4 texColor = texture(texture0, fragTexCoord);\n"
    "    float gray = dot(texColor.rgb, vec3(0.299, 0.587, 0.114));\n"
    "    finalColor = vec4(gray, gray, gray, texColor.a) * fragColor;\n"
    "}\n";

g_autoptr(GrlShader) grayscale = grl_shader_new_from_memory (NULL, grayscale_fs, NULL);

/* In render loop */
grl_shader_begin (grayscale);
grl_draw_texture (sprite, x, y, white);
grl_shader_end (grayscale);
```

## GrlRenderTexture

Off-screen rendering target (framebuffer). Useful for post-processing, minimap rendering, or caching complex scenes.

### Creating Render Textures

```c
/* Create render texture matching window size */
g_autoptr(GrlRenderTexture) target = grl_render_texture_new (800, 600);

/* Check if valid */
if (!grl_render_texture_is_valid (target))
{
    g_printerr ("Failed to create render texture\n");
}

/* Get dimensions */
gint width = grl_render_texture_get_width (target);
gint height = grl_render_texture_get_height (target);
```

### Rendering to Texture

```c
/* Begin rendering to texture */
grl_render_texture_begin (target);

/* Clear the render texture */
g_autoptr(GrlColor) bg = grl_color_new (0, 0, 0, 255);
grl_window_clear_background (window, bg);

/* Draw scene to render texture */
grl_draw_circle (400, 300, 100, red);
grl_draw_rectangle (100, 100, 200, 150, blue);

/* End rendering to texture */
grl_render_texture_end (target);
```

### Drawing Render Texture to Screen

```c
/* Get the texture from render target */
g_autoptr(GrlTexture) tex = grl_render_texture_get_texture (target);

/* Draw to screen (note: Y is flipped in OpenGL framebuffers) */
g_autoptr(GrlRectangle) source = grl_rectangle_new (0, 600, 800, -600);  /* Flip Y */
g_autoptr(GrlRectangle) dest = grl_rectangle_new (0, 0, 800, 600);
g_autoptr(GrlVector2) origin = grl_vector2_new (0, 0);
g_autoptr(GrlColor) white = grl_color_new_white ();

grl_draw_texture_pro (tex, source, dest, origin, 0.0f, white);
```

### Methods

| Method | Description |
|--------|-------------|
| `is_valid()` | Check if render texture is valid |
| `get_width()` | Get width in pixels |
| `get_height()` | Get height in pixels |
| `begin()` | Begin rendering to this texture |
| `end()` | End rendering and return to screen |
| `get_texture()` | Get the color buffer as a texture |

### Example: Post-Processing Effect

```c
#include <graylib.h>

int
main (int argc, char *argv[])
{
    g_autoptr(GrlWindow) window = grl_window_new (800, 600, "Post-Processing");
    grl_window_set_target_fps (window, 60);

    /* Create render texture for scene */
    g_autoptr(GrlRenderTexture) scene_target = grl_render_texture_new (800, 600);

    /* Load post-processing shader */
    g_autoptr(GrlShader) blur_shader = grl_shader_new_from_file (
        NULL, "shaders/blur.fs", NULL);
    gint resolution_loc = grl_shader_get_location (blur_shader, "resolution");

    g_autoptr(GrlColor) bg = grl_color_new (30, 30, 45, 255);
    g_autoptr(GrlColor) white = grl_color_new_white ();
    g_autoptr(GrlColor) red = grl_color_new_red ();

    while (!grl_window_should_close (window))
    {
        /* 1. Render scene to texture */
        grl_render_texture_begin (scene_target);
        grl_window_clear_background (window, bg);
        grl_draw_circle (400, 300, 100, red);
        grl_render_texture_end (scene_target);

        /* 2. Draw scene texture to screen with shader */
        grl_window_begin_drawing (window);

        grl_shader_begin (blur_shader);
        grl_shader_set_value_vec2 (blur_shader, resolution_loc, 800.0f, 600.0f);

        g_autoptr(GrlTexture) scene_tex = grl_render_texture_get_texture (scene_target);
        g_autoptr(GrlRectangle) src = grl_rectangle_new (0, 600, 800, -600);
        g_autoptr(GrlRectangle) dst = grl_rectangle_new (0, 0, 800, 600);
        g_autoptr(GrlVector2) origin = grl_vector2_new (0, 0);
        grl_draw_texture_pro (scene_tex, src, dst, origin, 0.0f, white);

        grl_shader_end (blur_shader);

        grl_draw_fps (10, 10);
        grl_window_end_drawing (window);
    }

    return 0;
}
```

## Complete Example

```c
#include <graylib.h>

int
main (int argc, char *argv[])
{
    g_autoptr(GrlApplication) app = grl_application_new ();
    g_autoptr(GrlWindow) window = grl_window_new (800, 600, "Graphics Demo");
    g_autoptr(GrlColor) bg = grl_color_new (40, 40, 60, 255);
    g_autoptr(GrlColor) white = grl_color_new_white ();

    /* Load and configure texture */
    g_autoptr(GrlTexture) tex = grl_texture_new_from_file ("sprite.png");
    grl_texture_set_filter (tex, GRL_TEXTURE_FILTER_POINT);

    /* Load custom font */
    g_autoptr(GrlFont) font = grl_font_new_from_file ("fonts/pixel.ttf");

    g_autoptr(GrlVector2) pos = grl_vector2_new (400.0f, 300.0f);

    grl_window_set_target_fps (window, 60);

    while (!grl_window_should_close (window))
    {
        grl_window_begin_drawing (window);
        grl_draw_clear_background (bg);

        /* Draw texture centered */
        grl_draw_texture_v (tex, pos, white);

        /* Draw text with custom font */
        grl_draw_text_ex (font, "Graylib Graphics!", pos, 32.0f, 2.0f, white);

        grl_draw_fps (10, 10);

        grl_window_end_drawing (window);
    }

    return 0;
}
```

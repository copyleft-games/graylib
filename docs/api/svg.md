# SVG Import / Export

`grl-svg.h` provides SVG import and export backed by [`GrlPath`](graphics.md#vector-paths-grlpath).
The surface is deliberately small: parse to a flat list of `GrlVectorShape`
(geometry + paint), render onto a `GrlImage`, and serialise back to SVG. No
external SVG library is used — the parser is self-contained.

## GrlVectorShape

A `GBoxed` type pairing a `GrlPath` with paint:

| Field | Type | Description |
|-------|------|-------------|
| `path` | `GrlPath *` | Owned reference. |
| `has_fill` | `gboolean` | Whether fill is active. |
| `fill` | `GrlColor` | Fill colour (by value). |
| `has_stroke` | `gboolean` | Whether stroke is active. |
| `stroke` | `GrlColor` | Stroke colour (by value). |
| `stroke_width` | `gfloat` | Stroke width in user units. |
| `fill_rule` | `GrlFillRule` | Nonzero or even-odd. |

`grl_vector_shape_new()` returns `(transfer full)`. Free a shape array returned
by the loaders like this:

```c
for (guint i = 0; i < n; i++)
    grl_vector_shape_free (shapes[i]);
g_free (shapes);
```

## API

```c
/* Import */
GrlVectorShape **grl_svg_load_from_file   (const gchar *filename, gfloat dpi,
                                           guint *n_shapes, GError **error);
GrlVectorShape **grl_svg_load_from_memory (const gchar *data, gsize len, gfloat dpi,
                                           guint *n_shapes, GError **error);
GrlPath         *grl_path_new_from_svg_file (const gchar *filename, gfloat dpi,
                                             GError **error);  /* merged geometry */

/* Render */
void grl_image_draw_svg_shapes (GrlImage *self,
                                GrlVectorShape * const *shapes, guint n_shapes);

/* Export */
gchar   *grl_svg_to_string   (GrlVectorShape * const *shapes, guint n_shapes,
                              gint width, gint height);            /* (transfer full) */
gboolean grl_svg_save_to_file (GrlVectorShape * const *shapes, guint n_shapes,
                              gint width, gint height,
                              const gchar *filename, GError **error);
gchar   *grl_path_to_svg_string (GrlPath *self);                  /* bare d= */
```

## Example

```c
guint n = 0;
g_autoptr(GError) error = NULL;
GrlVectorShape **shapes = grl_svg_load_from_file ("icon.svg", 96.0f, &n, &error);

g_autoptr(GrlColor) bg  = grl_color_new (255, 255, 255, 255);
g_autoptr(GrlImage) img = grl_image_new_color (128, 128, bg);
grl_image_set_antialias (img, TRUE);
grl_image_draw_svg_shapes (img, (GrlVectorShape * const *) shapes, n);

grl_svg_save_to_file ((GrlVectorShape * const *) shapes, n, 128, 128, "out.svg", NULL);

for (guint i = 0; i < n; i++)
    grl_vector_shape_free (shapes[i]);
g_free (shapes);
```

## Supported subset (import + export)

- **Elements:** `<svg>`, `<g>`, `<path>`, `<rect>` (incl. `rx`/`ry`), `<circle>`,
  `<ellipse>`, `<line>`, `<polyline>`, `<polygon>`.
- **Path data:** `M m L l H h V v C c S s Q q T t Z z`. Arcs (`A a`) are
  approximated with cubic-bezier segments (SVG spec parametric conversion).
- **Paint:** `fill`, `stroke` as `#rgb`, `#rrggbb`, `rgb(r,g,b)`, a basic set of
  named colours, or `none`; plus `fill-opacity`, `stroke-opacity`, `opacity`,
  `stroke-width`, `fill-rule` (nonzero/evenodd).
- **Transforms** (baked into coordinates): `translate`, `scale`, `rotate`,
  `matrix`, `skewX`, `skewY`. `<g>` composes/inherits transform + paint onto its
  children; export emits a flat list of `<path>` with no `transform` attribute.

## Round-trip

Round-trip is defined over **baked geometry + paint**, not source bytes. Export
emits only `M/L/C/Q/Z` (curves flattened to the path's tolerance) plus
`fill`/`fill-opacity`/`stroke`/`stroke-width`/`fill-rule`, so re-import is
geometry-lossless within the flatness tolerance. Opacity may drift by ±1/255.

## Out of scope (skipped without error)

Gradients, patterns, filters, masks/clip-paths, `<text>`/`<tspan>`, `<use>`/
`<defs>`/symbols, CSS `<style>` / inline `style=`, animation, raster `<image>`,
and stroke `linecap`/`linejoin`/`dasharray`. Unknown elements and malformed
tags are skipped; data with no XML tags at all yields `GRL_SVG_ERROR_PARSE`.

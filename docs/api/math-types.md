# Math Types

Graylib provides several GBoxed math types for working with 2D graphics.

## GrlVector2

A 2D vector with float components.

### Structure

```c
struct _GrlVector2
{
    gfloat x;
    gfloat y;
};
```

### Creating Vectors

```c
/* Heap allocated (use g_autoptr for automatic cleanup) */
g_autoptr(GrlVector2) v1 = grl_vector2_new (10.0f, 20.0f);
g_autoptr(GrlVector2) zero = grl_vector2_new_zero ();
g_autoptr(GrlVector2) one = grl_vector2_new_one ();

/* Stack allocated (no cleanup needed) */
GrlVector2 v2 = grl_vector2_init (30.0f, 40.0f);
```

### Operations

```c
g_autoptr(GrlVector2) a = grl_vector2_new (1.0f, 2.0f);
g_autoptr(GrlVector2) b = grl_vector2_new (3.0f, 4.0f);

/* Arithmetic */
g_autoptr(GrlVector2) sum = grl_vector2_add (a, b);
g_autoptr(GrlVector2) diff = grl_vector2_subtract (a, b);
g_autoptr(GrlVector2) scaled = grl_vector2_scale (a, 2.0f);
g_autoptr(GrlVector2) neg = grl_vector2_negate (a);

/* Scalar operations */
gfloat len = grl_vector2_length (a);
gfloat len_sq = grl_vector2_length_sqr (a);
gfloat dot = grl_vector2_dot (a, b);
gfloat dist = grl_vector2_distance (a, b);
gfloat angle = grl_vector2_angle (a, b);

/* Normalization */
g_autoptr(GrlVector2) norm = grl_vector2_normalize (a);

/* Interpolation */
g_autoptr(GrlVector2) lerped = grl_vector2_lerp (a, b, 0.5f);

/* Rotation (around origin, angle in radians) */
g_autoptr(GrlVector2) rotated = grl_vector2_rotate (a, 1.57f);

/* Comparison */
gboolean eq = grl_vector2_equal (a, b);
```

### Accessors

```c
gfloat x = grl_vector2_get_x (v);
gfloat y = grl_vector2_get_y (v);
grl_vector2_set (v, new_x, new_y);
```

## GrlColor

An RGBA color with 8-bit components.

### Structure

```c
struct _GrlColor
{
    guint8 r;   /* Red (0-255) */
    guint8 g;   /* Green (0-255) */
    guint8 b;   /* Blue (0-255) */
    guint8 a;   /* Alpha (0-255, 255 = opaque) */
};
```

### Creating Colors

```c
/* From components */
g_autoptr(GrlColor) c1 = grl_color_new (255, 128, 64, 255);
g_autoptr(GrlColor) c2 = grl_color_new_rgb (255, 128, 64);  /* Alpha = 255 */

/* From HSV */
g_autoptr(GrlColor) c3 = grl_color_new_from_hsv (0.0f, 1.0f, 1.0f);  /* Red */

/* From hex integer (0xRRGGBBAA) */
g_autoptr(GrlColor) c4 = grl_color_new_from_int (0xFF8040FF);

/* Predefined colors */
g_autoptr(GrlColor) white = grl_color_new_white ();
g_autoptr(GrlColor) black = grl_color_new_black ();
g_autoptr(GrlColor) blank = grl_color_new_blank ();  /* Transparent */
g_autoptr(GrlColor) red = grl_color_new_red ();
g_autoptr(GrlColor) green = grl_color_new_green ();
g_autoptr(GrlColor) blue = grl_color_new_blue ();
g_autoptr(GrlColor) yellow = grl_color_new_yellow ();
g_autoptr(GrlColor) magenta = grl_color_new_magenta ();
g_autoptr(GrlColor) cyan = grl_color_new_cyan ();
g_autoptr(GrlColor) gray = grl_color_new_gray ();
g_autoptr(GrlColor) raywhite = grl_color_new_raywhite ();  /* Off-white */

/* Stack allocated */
GrlColor c5 = grl_color_init (100, 150, 200, 255);
```

### Operations

```c
/* Modify alpha (0.0 - 1.0) */
g_autoptr(GrlColor) faded = grl_color_fade (color, 0.5f);
g_autoptr(GrlColor) alpha_mod = grl_color_alpha (color, 0.25f);

/* Tint (multiply RGB by tint color) */
g_autoptr(GrlColor) tinted = grl_color_tint (color, tint_color);

/* Brightness adjustment (factor > 1 = brighter) */
g_autoptr(GrlColor) brighter = grl_color_brightness (color, 1.5f);

/* Contrast adjustment */
g_autoptr(GrlColor) high_contrast = grl_color_contrast (color, 1.2f);

/* Alpha blending */
g_autoptr(GrlColor) blended = grl_color_alpha_blend (dst, src);

/* Interpolation */
g_autoptr(GrlColor) lerped = grl_color_lerp (color1, color2, 0.5f);
```

### Conversions

```c
/* To integer (0xRRGGBBAA) */
guint32 hex = grl_color_to_int (color);

/* To HSV */
gfloat h, s, v;
grl_color_to_hsv (color, &h, &s, &v);

/* To normalized floats (0.0 - 1.0) */
gfloat r, g, b, a;
grl_color_normalize (color, &r, &g, &b, &a);

/* To string */
g_autofree gchar *str = grl_color_to_string (color);  /* "GrlColor(r, g, b, a)" */
g_autofree gchar *hex_str = grl_color_to_hex (color); /* "#RRGGBBAA" */
```

## GrlRectangle

A 2D axis-aligned rectangle.

### Structure

```c
struct _GrlRectangle
{
    gfloat x;       /* X position of top-left corner */
    gfloat y;       /* Y position of top-left corner */
    gfloat width;   /* Width */
    gfloat height;  /* Height */
};
```

### Creating Rectangles

```c
/* From position and size */
g_autoptr(GrlRectangle) r1 = grl_rectangle_new (10.0f, 20.0f, 100.0f, 50.0f);
g_autoptr(GrlRectangle) empty = grl_rectangle_new_empty ();

/* From vectors */
g_autoptr(GrlVector2) pos = grl_vector2_new (10.0f, 20.0f);
g_autoptr(GrlVector2) size = grl_vector2_new (100.0f, 50.0f);
g_autoptr(GrlRectangle) r2 = grl_rectangle_new_from_vectors (pos, size);

/* From corners */
g_autoptr(GrlVector2) top_left = grl_vector2_new (10.0f, 20.0f);
g_autoptr(GrlVector2) bottom_right = grl_vector2_new (110.0f, 70.0f);
g_autoptr(GrlRectangle) r3 = grl_rectangle_new_from_corners (top_left, bottom_right);

/* Stack allocated */
GrlRectangle r4 = grl_rectangle_init (10.0f, 20.0f, 100.0f, 50.0f);
```

### Derived Properties

```c
/* Edges */
gfloat left = grl_rectangle_get_left (rect);    /* x */
gfloat right = grl_rectangle_get_right (rect);  /* x + width */
gfloat top = grl_rectangle_get_top (rect);      /* y */
gfloat bottom = grl_rectangle_get_bottom (rect); /* y + height */

/* Position and size as vectors */
g_autoptr(GrlVector2) pos = grl_rectangle_get_position (rect);
g_autoptr(GrlVector2) size = grl_rectangle_get_size (rect);
g_autoptr(GrlVector2) center = grl_rectangle_get_center (rect);

/* Measurements */
gfloat area = grl_rectangle_get_area (rect);
gfloat perimeter = grl_rectangle_get_perimeter (rect);
```

### Operations

```c
/* Expand (grow by amount on all sides) */
g_autoptr(GrlRectangle) expanded = grl_rectangle_expand (rect, 5.0f);
g_autoptr(GrlRectangle) expanded_xy = grl_rectangle_expand_xy (rect, 10.0f, 5.0f);

/* Translate (move) */
g_autoptr(GrlRectangle) moved = grl_rectangle_translate (rect, dx, dy);
g_autoptr(GrlVector2) offset = grl_vector2_new (10.0f, 20.0f);
g_autoptr(GrlRectangle) moved_v = grl_rectangle_translate_v (rect, offset);

/* Scale */
g_autoptr(GrlRectangle) scaled = grl_rectangle_scale (rect, 2.0f, 2.0f);
```

### Collision Detection

```c
/* Point containment */
gboolean contains = grl_rectangle_contains_point (rect, x, y);
gboolean contains_v = grl_rectangle_contains_point_v (rect, point);

/* Rectangle containment */
gboolean contains_rect = grl_rectangle_contains_rect (outer, inner);

/* Intersection test */
gboolean intersects = grl_rectangle_intersects (rect1, rect2);

/* Get intersection rectangle */
g_autoptr(GrlRectangle) inter = grl_rectangle_intersection (rect1, rect2);

/* Get union (bounding rectangle) */
g_autoptr(GrlRectangle) united = grl_rectangle_union (rect1, rect2);
```

### Comparison

```c
gboolean eq = grl_rectangle_equal (rect1, rect2);
gboolean is_empty = grl_rectangle_is_empty (rect);  /* width or height <= 0 */
```

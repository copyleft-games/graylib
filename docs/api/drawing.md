# Drawing Functions

All drawing functions must be called between `grl_window_begin_drawing()` and `grl_window_end_drawing()`.

## Background

```c
/* Clear screen with color */
grl_draw_clear_background (color);
```

## Blend Modes

```c
/* Begin blend mode */
grl_draw_begin_blend_mode (GRL_BLEND_MODE_ADDITIVE);

/* Draw with blend mode */
grl_draw_circle_v (pos, 50.0f, color);

/* End blend mode (return to default alpha) */
grl_draw_end_blend_mode ();
```

Available blend modes:
- `GRL_BLEND_MODE_ALPHA` - Default alpha blending
- `GRL_BLEND_MODE_ADDITIVE` - Colors add together
- `GRL_BLEND_MODE_MULTIPLIED` - Colors multiply
- `GRL_BLEND_MODE_ADD_COLORS` - Add source to destination
- `GRL_BLEND_MODE_SUBTRACT_COLORS` - Subtract source from destination
- `GRL_BLEND_MODE_ALPHA_PREMULTIPLY` - Premultiplied alpha
- `GRL_BLEND_MODE_CUSTOM` - Custom OpenGL blending

## Scissor Mode

Restrict drawing to a rectangular region.

```c
grl_draw_begin_scissor_mode (x, y, width, height);
/* Only draws within the rectangle */
grl_draw_rectangle (0, 0, 1000, 1000, color);
grl_draw_end_scissor_mode ();
```

## Pixels

```c
grl_draw_pixel (x, y, color);
grl_draw_pixel_v (position, color);
```

Note: Pixel drawing is slow. Use sparingly.

## Lines

```c
/* Basic line */
grl_draw_line (start_x, start_y, end_x, end_y, color);
grl_draw_line_v (start_vec, end_vec, color);

/* Line with thickness */
grl_draw_line_ex (start_vec, end_vec, thickness, color);

/* Connected lines */
GrlVector2 points[] = { ... };
grl_draw_line_strip (points, count, color);

/* Bezier curve */
grl_draw_line_bezier (start_vec, end_vec, thickness, color);
```

## Circles

```c
/* Filled circle */
grl_draw_circle (center_x, center_y, radius, color);
grl_draw_circle_v (center_vec, radius, color);

/* Circle outline */
grl_draw_circle_lines (center_x, center_y, radius, color);
grl_draw_circle_lines_v (center_vec, radius, color);

/* Circle sector (pie slice) */
grl_draw_circle_sector (center_vec, radius, start_angle, end_angle, segments, color);
grl_draw_circle_sector_lines (center_vec, radius, start_angle, end_angle, segments, color);

/* Gradient circle */
grl_draw_circle_gradient (center_x, center_y, radius, inner_color, outer_color);
```

## Ellipses

```c
grl_draw_ellipse (center_x, center_y, radius_h, radius_v, color);
grl_draw_ellipse_lines (center_x, center_y, radius_h, radius_v, color);
```

## Rings

```c
grl_draw_ring (center_vec, inner_radius, outer_radius, start_angle, end_angle, segments, color);
grl_draw_ring_lines (center_vec, inner_radius, outer_radius, start_angle, end_angle, segments, color);
```

## Rectangles

### Basic Rectangles

```c
/* Position and size */
grl_draw_rectangle (x, y, width, height, color);
grl_draw_rectangle_v (position_vec, size_vec, color);

/* From GrlRectangle */
grl_draw_rectangle_rec (rect, color);

/* With rotation */
grl_draw_rectangle_pro (rect, origin_vec, rotation_degrees, color);

/* Outline */
grl_draw_rectangle_lines (x, y, width, height, color);
grl_draw_rectangle_lines_ex (rect, line_thickness, color);
```

### Rounded Rectangles

```c
/* Filled */
grl_draw_rectangle_rounded (rect, roundness, segments, color);

/* Outline */
grl_draw_rectangle_rounded_lines (rect, roundness, segments, color);
grl_draw_rectangle_rounded_lines_ex (rect, roundness, segments, line_thickness, color);
```

Parameters:
- `roundness`: 0.0 (square) to 1.0 (maximum rounding)
- `segments`: Number of segments per corner (more = smoother)

### Gradient Rectangles

```c
/* Vertical gradient */
grl_draw_rectangle_gradient_v (x, y, width, height, top_color, bottom_color);

/* Horizontal gradient */
grl_draw_rectangle_gradient_h (x, y, width, height, left_color, right_color);

/* Four-corner gradient */
grl_draw_rectangle_gradient_ex (rect, top_left, bottom_left, top_right, bottom_right);
```

## Triangles

```c
/* Filled triangle (counter-clockwise vertices) */
grl_draw_triangle (v1, v2, v3, color);

/* Triangle outline */
grl_draw_triangle_lines (v1, v2, v3, color);

/* Triangle fan (first point is center) */
GrlVector2 points[] = { center, p1, p2, p3, ... };
grl_draw_triangle_fan (points, count, color);

/* Triangle strip */
grl_draw_triangle_strip (points, count, color);
```

## Polygons

```c
/* Regular polygon (filled) */
grl_draw_poly (center_vec, sides, radius, rotation_degrees, color);

/* Polygon outline */
grl_draw_poly_lines (center_vec, sides, radius, rotation_degrees, color);
grl_draw_poly_lines_ex (center_vec, sides, radius, rotation_degrees, line_thickness, color);
```

Examples:
- `sides = 3`: Triangle
- `sides = 4`: Square (rotated 45 degrees = diamond)
- `sides = 5`: Pentagon
- `sides = 6`: Hexagon

## Splines

```c
/* Linear spline (minimum 2 points) */
grl_draw_spline_linear (points, count, thickness, color);

/* B-spline (minimum 4 points) */
grl_draw_spline_basis (points, count, thickness, color);

/* Catmull-Rom spline (minimum 4 points) */
grl_draw_spline_catmull_rom (points, count, thickness, color);

/* Quadratic Bezier (minimum 3 points: start, control, end) */
grl_draw_spline_bezier_quadratic (points, count, thickness, color);

/* Cubic Bezier (minimum 4 points: start, control1, control2, end) */
grl_draw_spline_bezier_cubic (points, count, thickness, color);
```

## Example: Drawing Shapes

```c
g_autoptr(GrlWindow) window = grl_window_new (800, 600, "Shapes");
g_autoptr(GrlColor) bg = grl_color_new (40, 40, 60, 255);
g_autoptr(GrlColor) red = grl_color_new_red ();
g_autoptr(GrlColor) blue = grl_color_new_blue ();
g_autoptr(GrlVector2) center = grl_vector2_new (400.0f, 300.0f);
g_autoptr(GrlRectangle) rect = grl_rectangle_new (100.0f, 100.0f, 200.0f, 150.0f);

while (!grl_window_should_close (window))
{
    grl_window_begin_drawing (window);
    grl_draw_clear_background (bg);

    /* Draw a circle */
    grl_draw_circle_v (center, 100.0f, red);

    /* Draw a rounded rectangle */
    grl_draw_rectangle_rounded (rect, 0.3f, 8, blue);

    /* Draw a rotating hexagon */
    gfloat rotation = grl_window_get_time (window) * 30.0f;
    grl_draw_poly (center, 6, 50.0f, rotation, red);

    grl_window_end_drawing (window);
}
```

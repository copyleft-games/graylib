/* grl-draw-shapes.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Basic shapes drawing functions implementation.
 */

#include "config.h"
#include "grl-draw.h"
#include <raylib.h>

/**
 * SECTION:grl-draw
 * @title: Drawing Functions
 * @short_description: Functions for drawing shapes and primitives
 *
 * This module provides functions for drawing basic shapes like lines,
 * rectangles, circles, and polygons. These functions wrap the corresponding
 * raylib drawing functions and use Graylib types.
 *
 * All drawing functions must be called between grl_window_begin_drawing()
 * and grl_window_end_drawing().
 *
 * Example:
 * |[<!-- language="C" -->
 * g_autoptr(GrlColor) red = grl_color_new_red ();
 * g_autoptr(GrlVector2) pos = grl_vector2_new (100.0f, 100.0f);
 *
 * grl_window_begin_drawing (window);
 * grl_draw_clear_background (bg_color);
 * grl_draw_circle_v (pos, 50.0f, red);
 * grl_window_end_drawing (window);
 * ]|
 */

/*
 * Helper macros for converting Graylib types to raylib types
 */

#define GRL_TO_RAYLIB_COLOR(c) \
    ((Color){ .r = (c)->r, .g = (c)->g, .b = (c)->b, .a = (c)->a })

#define GRL_TO_RAYLIB_VECTOR2(v) \
    ((Vector2){ .x = (v)->x, .y = (v)->y })

#define GRL_TO_RAYLIB_RECTANGLE(r) \
    ((Rectangle){ .x = (r)->x, .y = (r)->y, .width = (r)->width, .height = (r)->height })

/*
 * Background
 */

/**
 * grl_draw_clear_background:
 * @color: The color to clear the screen with.
 *
 * Clears the screen with the specified color. This should be called
 * at the beginning of each frame.
 */
void
grl_draw_clear_background (const GrlColor *color)
{
    g_return_if_fail (color != NULL);

    ClearBackground (GRL_TO_RAYLIB_COLOR (color));
}

/*
 * Blend modes
 */

/**
 * grl_draw_begin_blend_mode:
 * @mode: The blend mode to use.
 *
 * Begins a blending mode. All drawing operations after this call
 * will use the specified blend mode until grl_draw_end_blend_mode()
 * is called.
 */
void
grl_draw_begin_blend_mode (GrlBlendMode mode)
{
    BeginBlendMode ((int)mode);
}

/**
 * grl_draw_end_blend_mode:
 *
 * Ends the current blending mode and resets to default alpha blending.
 */
void
grl_draw_end_blend_mode (void)
{
    EndBlendMode ();
}

/*
 * Scissor mode
 */

/**
 * grl_draw_begin_scissor_mode:
 * @x: X position of the scissor rectangle.
 * @y: Y position of the scissor rectangle.
 * @width: Width of the scissor rectangle.
 * @height: Height of the scissor rectangle.
 *
 * Begins scissor mode. Only pixels within the specified rectangle
 * will be drawn.
 */
void
grl_draw_begin_scissor_mode (gint x,
                             gint y,
                             gint width,
                             gint height)
{
    BeginScissorMode (x, y, width, height);
}

/**
 * grl_draw_end_scissor_mode:
 *
 * Ends scissor mode.
 */
void
grl_draw_end_scissor_mode (void)
{
    EndScissorMode ();
}

/*
 * Pixel drawing
 */

/**
 * grl_draw_pixel:
 * @x: X position.
 * @y: Y position.
 * @color: Pixel color.
 *
 * Draws a single pixel. Note: This can be slow, use with care.
 */
void
grl_draw_pixel (gint            x,
                gint            y,
                const GrlColor *color)
{
    g_return_if_fail (color != NULL);

    DrawPixel (x, y, GRL_TO_RAYLIB_COLOR (color));
}

/**
 * grl_draw_pixel_v:
 * @position: Pixel position.
 * @color: Pixel color.
 *
 * Draws a single pixel using vector position.
 */
void
grl_draw_pixel_v (const GrlVector2 *position,
                  const GrlColor   *color)
{
    g_return_if_fail (position != NULL);
    g_return_if_fail (color != NULL);

    DrawPixelV (GRL_TO_RAYLIB_VECTOR2 (position), GRL_TO_RAYLIB_COLOR (color));
}

/*
 * Line drawing
 */

/**
 * grl_draw_line:
 * @start_x: Starting X position.
 * @start_y: Starting Y position.
 * @end_x: Ending X position.
 * @end_y: Ending Y position.
 * @color: Line color.
 *
 * Draws a line between two points.
 */
void
grl_draw_line (gint            start_x,
               gint            start_y,
               gint            end_x,
               gint            end_y,
               const GrlColor *color)
{
    g_return_if_fail (color != NULL);

    DrawLine (start_x, start_y, end_x, end_y, GRL_TO_RAYLIB_COLOR (color));
}

/**
 * grl_draw_line_v:
 * @start: Starting position.
 * @end: Ending position.
 * @color: Line color.
 *
 * Draws a line between two points using vectors.
 */
void
grl_draw_line_v (const GrlVector2 *start,
                 const GrlVector2 *end,
                 const GrlColor   *color)
{
    g_return_if_fail (start != NULL);
    g_return_if_fail (end != NULL);
    g_return_if_fail (color != NULL);

    DrawLineV (GRL_TO_RAYLIB_VECTOR2 (start),
               GRL_TO_RAYLIB_VECTOR2 (end),
               GRL_TO_RAYLIB_COLOR (color));
}

/**
 * grl_draw_line_ex:
 * @start: Starting position.
 * @end: Ending position.
 * @thickness: Line thickness.
 * @color: Line color.
 *
 * Draws a line with specified thickness.
 */
void
grl_draw_line_ex (const GrlVector2 *start,
                  const GrlVector2 *end,
                  gfloat            thickness,
                  const GrlColor   *color)
{
    g_return_if_fail (start != NULL);
    g_return_if_fail (end != NULL);
    g_return_if_fail (color != NULL);

    DrawLineEx (GRL_TO_RAYLIB_VECTOR2 (start),
                GRL_TO_RAYLIB_VECTOR2 (end),
                thickness,
                GRL_TO_RAYLIB_COLOR (color));
}

/**
 * grl_draw_line_strip:
 * @points: (array length=point_count): Array of points.
 * @point_count: Number of points.
 * @color: Line color.
 *
 * Draws a sequence of connected lines.
 */
void
grl_draw_line_strip (const GrlVector2 *points,
                     gint              point_count,
                     const GrlColor   *color)
{
    Vector2 *raylib_points;
    gint i;

    g_return_if_fail (points != NULL);
    g_return_if_fail (point_count > 0);
    g_return_if_fail (color != NULL);

    raylib_points = g_alloca (sizeof (Vector2) * point_count);
    for (i = 0; i < point_count; i++)
    {
        raylib_points[i] = GRL_TO_RAYLIB_VECTOR2 (&points[i]);
    }

    DrawLineStrip (raylib_points, point_count, GRL_TO_RAYLIB_COLOR (color));
}

/**
 * grl_draw_line_bezier:
 * @start: Starting position.
 * @end: Ending position.
 * @thickness: Line thickness.
 * @color: Line color.
 *
 * Draws a cubic bezier line.
 */
void
grl_draw_line_bezier (const GrlVector2 *start,
                      const GrlVector2 *end,
                      gfloat            thickness,
                      const GrlColor   *color)
{
    g_return_if_fail (start != NULL);
    g_return_if_fail (end != NULL);
    g_return_if_fail (color != NULL);

    DrawLineBezier (GRL_TO_RAYLIB_VECTOR2 (start),
                    GRL_TO_RAYLIB_VECTOR2 (end),
                    thickness,
                    GRL_TO_RAYLIB_COLOR (color));
}

/*
 * Circle drawing
 */

/**
 * grl_draw_circle:
 * @center_x: Center X position.
 * @center_y: Center Y position.
 * @radius: Circle radius.
 * @color: Fill color.
 *
 * Draws a filled circle.
 */
void
grl_draw_circle (gint            center_x,
                 gint            center_y,
                 gfloat          radius,
                 const GrlColor *color)
{
    g_return_if_fail (color != NULL);

    DrawCircle (center_x, center_y, radius, GRL_TO_RAYLIB_COLOR (color));
}

/**
 * grl_draw_circle_v:
 * @center: Center position.
 * @radius: Circle radius.
 * @color: Fill color.
 *
 * Draws a filled circle using vector position.
 */
void
grl_draw_circle_v (const GrlVector2 *center,
                   gfloat            radius,
                   const GrlColor   *color)
{
    g_return_if_fail (center != NULL);
    g_return_if_fail (color != NULL);

    DrawCircleV (GRL_TO_RAYLIB_VECTOR2 (center), radius, GRL_TO_RAYLIB_COLOR (color));
}

/**
 * grl_draw_circle_lines:
 * @center_x: Center X position.
 * @center_y: Center Y position.
 * @radius: Circle radius.
 * @color: Outline color.
 *
 * Draws a circle outline.
 */
void
grl_draw_circle_lines (gint            center_x,
                       gint            center_y,
                       gfloat          radius,
                       const GrlColor *color)
{
    g_return_if_fail (color != NULL);

    DrawCircleLines (center_x, center_y, radius, GRL_TO_RAYLIB_COLOR (color));
}

/**
 * grl_draw_circle_lines_v:
 * @center: Center position.
 * @radius: Circle radius.
 * @color: Outline color.
 *
 * Draws a circle outline using vector position.
 */
void
grl_draw_circle_lines_v (const GrlVector2 *center,
                         gfloat            radius,
                         const GrlColor   *color)
{
    g_return_if_fail (center != NULL);
    g_return_if_fail (color != NULL);

    DrawCircleLinesV (GRL_TO_RAYLIB_VECTOR2 (center), radius, GRL_TO_RAYLIB_COLOR (color));
}

/**
 * grl_draw_circle_sector:
 * @center: Center position.
 * @radius: Circle radius.
 * @start_angle: Start angle in degrees.
 * @end_angle: End angle in degrees.
 * @segments: Number of segments.
 * @color: Fill color.
 *
 * Draws a filled circle sector (pie slice).
 */
void
grl_draw_circle_sector (const GrlVector2 *center,
                        gfloat            radius,
                        gfloat            start_angle,
                        gfloat            end_angle,
                        gint              segments,
                        const GrlColor   *color)
{
    g_return_if_fail (center != NULL);
    g_return_if_fail (color != NULL);

    DrawCircleSector (GRL_TO_RAYLIB_VECTOR2 (center),
                      radius,
                      start_angle,
                      end_angle,
                      segments,
                      GRL_TO_RAYLIB_COLOR (color));
}

/**
 * grl_draw_circle_sector_lines:
 * @center: Center position.
 * @radius: Circle radius.
 * @start_angle: Start angle in degrees.
 * @end_angle: End angle in degrees.
 * @segments: Number of segments.
 * @color: Outline color.
 *
 * Draws a circle sector outline.
 */
void
grl_draw_circle_sector_lines (const GrlVector2 *center,
                              gfloat            radius,
                              gfloat            start_angle,
                              gfloat            end_angle,
                              gint              segments,
                              const GrlColor   *color)
{
    g_return_if_fail (center != NULL);
    g_return_if_fail (color != NULL);

    DrawCircleSectorLines (GRL_TO_RAYLIB_VECTOR2 (center),
                           radius,
                           start_angle,
                           end_angle,
                           segments,
                           GRL_TO_RAYLIB_COLOR (color));
}

/**
 * grl_draw_circle_gradient:
 * @center_x: Center X position.
 * @center_y: Center Y position.
 * @radius: Circle radius.
 * @inner: Inner (center) color.
 * @outer: Outer (edge) color.
 *
 * Draws a gradient-filled circle.
 */
void
grl_draw_circle_gradient (gint            center_x,
                          gint            center_y,
                          gfloat          radius,
                          const GrlColor *inner,
                          const GrlColor *outer)
{
    g_return_if_fail (inner != NULL);
    g_return_if_fail (outer != NULL);

    /* raylib 6.0 changed DrawCircleGradient to take a Vector2 center */
    DrawCircleGradient ((Vector2){ .x = (gfloat)center_x, .y = (gfloat)center_y },
                        radius,
                        GRL_TO_RAYLIB_COLOR (inner),
                        GRL_TO_RAYLIB_COLOR (outer));
}

/*
 * Ellipse drawing
 */

/**
 * grl_draw_ellipse:
 * @center_x: Center X position.
 * @center_y: Center Y position.
 * @radius_h: Horizontal radius.
 * @radius_v: Vertical radius.
 * @color: Fill color.
 *
 * Draws a filled ellipse.
 */
void
grl_draw_ellipse (gint            center_x,
                  gint            center_y,
                  gfloat          radius_h,
                  gfloat          radius_v,
                  const GrlColor *color)
{
    g_return_if_fail (color != NULL);

    DrawEllipse (center_x, center_y, radius_h, radius_v, GRL_TO_RAYLIB_COLOR (color));
}

/**
 * grl_draw_ellipse_lines:
 * @center_x: Center X position.
 * @center_y: Center Y position.
 * @radius_h: Horizontal radius.
 * @radius_v: Vertical radius.
 * @color: Outline color.
 *
 * Draws an ellipse outline.
 */
void
grl_draw_ellipse_lines (gint            center_x,
                        gint            center_y,
                        gfloat          radius_h,
                        gfloat          radius_v,
                        const GrlColor *color)
{
    g_return_if_fail (color != NULL);

    DrawEllipseLines (center_x, center_y, radius_h, radius_v, GRL_TO_RAYLIB_COLOR (color));
}

/*
 * Ring drawing
 */

/**
 * grl_draw_ring:
 * @center: Center position.
 * @inner_radius: Inner radius.
 * @outer_radius: Outer radius.
 * @start_angle: Start angle in degrees.
 * @end_angle: End angle in degrees.
 * @segments: Number of segments.
 * @color: Fill color.
 *
 * Draws a filled ring (donut shape).
 */
void
grl_draw_ring (const GrlVector2 *center,
               gfloat            inner_radius,
               gfloat            outer_radius,
               gfloat            start_angle,
               gfloat            end_angle,
               gint              segments,
               const GrlColor   *color)
{
    g_return_if_fail (center != NULL);
    g_return_if_fail (color != NULL);

    DrawRing (GRL_TO_RAYLIB_VECTOR2 (center),
              inner_radius,
              outer_radius,
              start_angle,
              end_angle,
              segments,
              GRL_TO_RAYLIB_COLOR (color));
}

/**
 * grl_draw_ring_lines:
 * @center: Center position.
 * @inner_radius: Inner radius.
 * @outer_radius: Outer radius.
 * @start_angle: Start angle in degrees.
 * @end_angle: End angle in degrees.
 * @segments: Number of segments.
 * @color: Outline color.
 *
 * Draws a ring outline.
 */
void
grl_draw_ring_lines (const GrlVector2 *center,
                     gfloat            inner_radius,
                     gfloat            outer_radius,
                     gfloat            start_angle,
                     gfloat            end_angle,
                     gint              segments,
                     const GrlColor   *color)
{
    g_return_if_fail (center != NULL);
    g_return_if_fail (color != NULL);

    DrawRingLines (GRL_TO_RAYLIB_VECTOR2 (center),
                   inner_radius,
                   outer_radius,
                   start_angle,
                   end_angle,
                   segments,
                   GRL_TO_RAYLIB_COLOR (color));
}

/*
 * Rectangle drawing
 */

/**
 * grl_draw_rectangle:
 * @x: X position.
 * @y: Y position.
 * @width: Rectangle width.
 * @height: Rectangle height.
 * @color: Fill color.
 *
 * Draws a filled rectangle.
 */
void
grl_draw_rectangle (gint            x,
                    gint            y,
                    gint            width,
                    gint            height,
                    const GrlColor *color)
{
    g_return_if_fail (color != NULL);

    DrawRectangle (x, y, width, height, GRL_TO_RAYLIB_COLOR (color));
}

/**
 * grl_draw_rectangle_v:
 * @position: Top-left position.
 * @size: Rectangle size.
 * @color: Fill color.
 *
 * Draws a filled rectangle using vectors.
 */
void
grl_draw_rectangle_v (const GrlVector2 *position,
                      const GrlVector2 *size,
                      const GrlColor   *color)
{
    g_return_if_fail (position != NULL);
    g_return_if_fail (size != NULL);
    g_return_if_fail (color != NULL);

    DrawRectangleV (GRL_TO_RAYLIB_VECTOR2 (position),
                    GRL_TO_RAYLIB_VECTOR2 (size),
                    GRL_TO_RAYLIB_COLOR (color));
}

/**
 * grl_draw_rectangle_rec:
 * @rect: Rectangle to draw.
 * @color: Fill color.
 *
 * Draws a filled rectangle from a #GrlRectangle.
 */
void
grl_draw_rectangle_rec (const GrlRectangle *rect,
                        const GrlColor     *color)
{
    g_return_if_fail (rect != NULL);
    g_return_if_fail (color != NULL);

    DrawRectangleRec (GRL_TO_RAYLIB_RECTANGLE (rect), GRL_TO_RAYLIB_COLOR (color));
}

/**
 * grl_draw_rectangle_pro:
 * @rect: Rectangle to draw.
 * @origin: Origin point for rotation.
 * @rotation: Rotation angle in degrees.
 * @color: Fill color.
 *
 * Draws a filled rectangle with rotation.
 */
void
grl_draw_rectangle_pro (const GrlRectangle *rect,
                        const GrlVector2   *origin,
                        gfloat              rotation,
                        const GrlColor     *color)
{
    g_return_if_fail (rect != NULL);
    g_return_if_fail (origin != NULL);
    g_return_if_fail (color != NULL);

    DrawRectanglePro (GRL_TO_RAYLIB_RECTANGLE (rect),
                      GRL_TO_RAYLIB_VECTOR2 (origin),
                      rotation,
                      GRL_TO_RAYLIB_COLOR (color));
}

/**
 * grl_draw_rectangle_lines:
 * @x: X position.
 * @y: Y position.
 * @width: Rectangle width.
 * @height: Rectangle height.
 * @color: Outline color.
 *
 * Draws a rectangle outline.
 */
void
grl_draw_rectangle_lines (gint            x,
                          gint            y,
                          gint            width,
                          gint            height,
                          const GrlColor *color)
{
    g_return_if_fail (color != NULL);

    DrawRectangleLines (x, y, width, height, GRL_TO_RAYLIB_COLOR (color));
}

/**
 * grl_draw_rectangle_lines_ex:
 * @rect: Rectangle to draw.
 * @line_thickness: Thickness of the outline.
 * @color: Outline color.
 *
 * Draws a rectangle outline with specified thickness.
 */
void
grl_draw_rectangle_lines_ex (const GrlRectangle *rect,
                             gfloat              line_thickness,
                             const GrlColor     *color)
{
    g_return_if_fail (rect != NULL);
    g_return_if_fail (color != NULL);

    DrawRectangleLinesEx (GRL_TO_RAYLIB_RECTANGLE (rect),
                          line_thickness,
                          GRL_TO_RAYLIB_COLOR (color));
}

/**
 * grl_draw_rectangle_rounded:
 * @rect: Rectangle to draw.
 * @roundness: Roundness factor (0.0 to 1.0).
 * @segments: Number of segments per corner.
 * @color: Fill color.
 *
 * Draws a filled rectangle with rounded corners.
 */
void
grl_draw_rectangle_rounded (const GrlRectangle *rect,
                            gfloat              roundness,
                            gint                segments,
                            const GrlColor     *color)
{
    g_return_if_fail (rect != NULL);
    g_return_if_fail (color != NULL);

    DrawRectangleRounded (GRL_TO_RAYLIB_RECTANGLE (rect),
                          roundness,
                          segments,
                          GRL_TO_RAYLIB_COLOR (color));
}

/**
 * grl_draw_rectangle_rounded_lines:
 * @rect: Rectangle to draw.
 * @roundness: Roundness factor (0.0 to 1.0).
 * @segments: Number of segments per corner.
 * @color: Outline color.
 *
 * Draws a rectangle outline with rounded corners.
 */
void
grl_draw_rectangle_rounded_lines (const GrlRectangle *rect,
                                  gfloat              roundness,
                                  gint                segments,
                                  const GrlColor     *color)
{
    g_return_if_fail (rect != NULL);
    g_return_if_fail (color != NULL);

    DrawRectangleRoundedLines (GRL_TO_RAYLIB_RECTANGLE (rect),
                               roundness,
                               segments,
                               GRL_TO_RAYLIB_COLOR (color));
}

/**
 * grl_draw_rectangle_rounded_lines_ex:
 * @rect: Rectangle to draw.
 * @roundness: Roundness factor (0.0 to 1.0).
 * @segments: Number of segments per corner.
 * @line_thickness: Thickness of the outline.
 * @color: Outline color.
 *
 * Draws a rectangle outline with rounded corners and specified thickness.
 */
void
grl_draw_rectangle_rounded_lines_ex (const GrlRectangle *rect,
                                     gfloat              roundness,
                                     gint                segments,
                                     gfloat              line_thickness,
                                     const GrlColor     *color)
{
    g_return_if_fail (rect != NULL);
    g_return_if_fail (color != NULL);

    DrawRectangleRoundedLinesEx (GRL_TO_RAYLIB_RECTANGLE (rect),
                                 roundness,
                                 segments,
                                 line_thickness,
                                 GRL_TO_RAYLIB_COLOR (color));
}

/*
 * Rectangle gradient drawing
 */

/**
 * grl_draw_rectangle_gradient_v:
 * @x: X position.
 * @y: Y position.
 * @width: Rectangle width.
 * @height: Rectangle height.
 * @top: Top edge color.
 * @bottom: Bottom edge color.
 *
 * Draws a rectangle with a vertical gradient.
 */
void
grl_draw_rectangle_gradient_v (gint            x,
                               gint            y,
                               gint            width,
                               gint            height,
                               const GrlColor *top,
                               const GrlColor *bottom)
{
    g_return_if_fail (top != NULL);
    g_return_if_fail (bottom != NULL);

    DrawRectangleGradientV (x, y, width, height,
                            GRL_TO_RAYLIB_COLOR (top),
                            GRL_TO_RAYLIB_COLOR (bottom));
}

/**
 * grl_draw_rectangle_gradient_h:
 * @x: X position.
 * @y: Y position.
 * @width: Rectangle width.
 * @height: Rectangle height.
 * @left: Left edge color.
 * @right: Right edge color.
 *
 * Draws a rectangle with a horizontal gradient.
 */
void
grl_draw_rectangle_gradient_h (gint            x,
                               gint            y,
                               gint            width,
                               gint            height,
                               const GrlColor *left,
                               const GrlColor *right)
{
    g_return_if_fail (left != NULL);
    g_return_if_fail (right != NULL);

    DrawRectangleGradientH (x, y, width, height,
                            GRL_TO_RAYLIB_COLOR (left),
                            GRL_TO_RAYLIB_COLOR (right));
}

/**
 * grl_draw_rectangle_gradient_ex:
 * @rect: Rectangle to draw.
 * @top_left: Top-left corner color.
 * @bottom_left: Bottom-left corner color.
 * @top_right: Top-right corner color.
 * @bottom_right: Bottom-right corner color.
 *
 * Draws a rectangle with a four-corner gradient.
 */
void
grl_draw_rectangle_gradient_ex (const GrlRectangle *rect,
                                const GrlColor     *top_left,
                                const GrlColor     *bottom_left,
                                const GrlColor     *top_right,
                                const GrlColor     *bottom_right)
{
    g_return_if_fail (rect != NULL);
    g_return_if_fail (top_left != NULL);
    g_return_if_fail (bottom_left != NULL);
    g_return_if_fail (top_right != NULL);
    g_return_if_fail (bottom_right != NULL);

    DrawRectangleGradientEx (GRL_TO_RAYLIB_RECTANGLE (rect),
                             GRL_TO_RAYLIB_COLOR (top_left),
                             GRL_TO_RAYLIB_COLOR (bottom_left),
                             GRL_TO_RAYLIB_COLOR (top_right),
                             GRL_TO_RAYLIB_COLOR (bottom_right));
}

/*
 * Triangle drawing
 */

/**
 * grl_draw_triangle:
 * @v1: First vertex.
 * @v2: Second vertex.
 * @v3: Third vertex.
 * @color: Fill color.
 *
 * Draws a filled triangle. Vertices should be in counter-clockwise order.
 */
void
grl_draw_triangle (const GrlVector2 *v1,
                   const GrlVector2 *v2,
                   const GrlVector2 *v3,
                   const GrlColor   *color)
{
    g_return_if_fail (v1 != NULL);
    g_return_if_fail (v2 != NULL);
    g_return_if_fail (v3 != NULL);
    g_return_if_fail (color != NULL);

    DrawTriangle (GRL_TO_RAYLIB_VECTOR2 (v1),
                  GRL_TO_RAYLIB_VECTOR2 (v2),
                  GRL_TO_RAYLIB_VECTOR2 (v3),
                  GRL_TO_RAYLIB_COLOR (color));
}

/**
 * grl_draw_triangle_lines:
 * @v1: First vertex.
 * @v2: Second vertex.
 * @v3: Third vertex.
 * @color: Outline color.
 *
 * Draws a triangle outline. Vertices should be in counter-clockwise order.
 */
void
grl_draw_triangle_lines (const GrlVector2 *v1,
                         const GrlVector2 *v2,
                         const GrlVector2 *v3,
                         const GrlColor   *color)
{
    g_return_if_fail (v1 != NULL);
    g_return_if_fail (v2 != NULL);
    g_return_if_fail (v3 != NULL);
    g_return_if_fail (color != NULL);

    DrawTriangleLines (GRL_TO_RAYLIB_VECTOR2 (v1),
                       GRL_TO_RAYLIB_VECTOR2 (v2),
                       GRL_TO_RAYLIB_VECTOR2 (v3),
                       GRL_TO_RAYLIB_COLOR (color));
}

/**
 * grl_draw_triangle_fan:
 * @points: (array length=point_count): Array of points (first is center).
 * @point_count: Number of points.
 * @color: Fill color.
 *
 * Draws a triangle fan. The first point is the center vertex.
 */
void
grl_draw_triangle_fan (const GrlVector2 *points,
                       gint              point_count,
                       const GrlColor   *color)
{
    Vector2 *raylib_points;
    gint i;

    g_return_if_fail (points != NULL);
    g_return_if_fail (point_count >= 3);
    g_return_if_fail (color != NULL);

    raylib_points = g_alloca (sizeof (Vector2) * point_count);
    for (i = 0; i < point_count; i++)
    {
        raylib_points[i] = GRL_TO_RAYLIB_VECTOR2 (&points[i]);
    }

    DrawTriangleFan (raylib_points, point_count, GRL_TO_RAYLIB_COLOR (color));
}

/**
 * grl_draw_triangle_strip:
 * @points: (array length=point_count): Array of points.
 * @point_count: Number of points.
 * @color: Fill color.
 *
 * Draws a triangle strip.
 */
void
grl_draw_triangle_strip (const GrlVector2 *points,
                         gint              point_count,
                         const GrlColor   *color)
{
    Vector2 *raylib_points;
    gint i;

    g_return_if_fail (points != NULL);
    g_return_if_fail (point_count >= 3);
    g_return_if_fail (color != NULL);

    raylib_points = g_alloca (sizeof (Vector2) * point_count);
    for (i = 0; i < point_count; i++)
    {
        raylib_points[i] = GRL_TO_RAYLIB_VECTOR2 (&points[i]);
    }

    DrawTriangleStrip (raylib_points, point_count, GRL_TO_RAYLIB_COLOR (color));
}

/*
 * Polygon drawing
 */

/**
 * grl_draw_poly:
 * @center: Center position.
 * @sides: Number of sides.
 * @radius: Polygon radius.
 * @rotation: Rotation angle in degrees.
 * @color: Fill color.
 *
 * Draws a filled regular polygon.
 */
void
grl_draw_poly (const GrlVector2 *center,
               gint              sides,
               gfloat            radius,
               gfloat            rotation,
               const GrlColor   *color)
{
    g_return_if_fail (center != NULL);
    g_return_if_fail (sides >= 3);
    g_return_if_fail (color != NULL);

    DrawPoly (GRL_TO_RAYLIB_VECTOR2 (center),
              sides,
              radius,
              rotation,
              GRL_TO_RAYLIB_COLOR (color));
}

/**
 * grl_draw_poly_lines:
 * @center: Center position.
 * @sides: Number of sides.
 * @radius: Polygon radius.
 * @rotation: Rotation angle in degrees.
 * @color: Outline color.
 *
 * Draws a regular polygon outline.
 */
void
grl_draw_poly_lines (const GrlVector2 *center,
                     gint              sides,
                     gfloat            radius,
                     gfloat            rotation,
                     const GrlColor   *color)
{
    g_return_if_fail (center != NULL);
    g_return_if_fail (sides >= 3);
    g_return_if_fail (color != NULL);

    DrawPolyLines (GRL_TO_RAYLIB_VECTOR2 (center),
                   sides,
                   radius,
                   rotation,
                   GRL_TO_RAYLIB_COLOR (color));
}

/**
 * grl_draw_poly_lines_ex:
 * @center: Center position.
 * @sides: Number of sides.
 * @radius: Polygon radius.
 * @rotation: Rotation angle in degrees.
 * @line_thickness: Thickness of the outline.
 * @color: Outline color.
 *
 * Draws a regular polygon outline with specified thickness.
 */
void
grl_draw_poly_lines_ex (const GrlVector2 *center,
                        gint              sides,
                        gfloat            radius,
                        gfloat            rotation,
                        gfloat            line_thickness,
                        const GrlColor   *color)
{
    g_return_if_fail (center != NULL);
    g_return_if_fail (sides >= 3);
    g_return_if_fail (color != NULL);

    DrawPolyLinesEx (GRL_TO_RAYLIB_VECTOR2 (center),
                     sides,
                     radius,
                     rotation,
                     line_thickness,
                     GRL_TO_RAYLIB_COLOR (color));
}

/*
 * Spline drawing
 */

/**
 * grl_draw_spline_linear:
 * @points: (array length=point_count): Array of points (minimum 2).
 * @point_count: Number of points.
 * @thickness: Line thickness.
 * @color: Line color.
 *
 * Draws a linear spline.
 */
void
grl_draw_spline_linear (const GrlVector2 *points,
                        gint              point_count,
                        gfloat            thickness,
                        const GrlColor   *color)
{
    Vector2 *raylib_points;
    gint i;

    g_return_if_fail (points != NULL);
    g_return_if_fail (point_count >= 2);
    g_return_if_fail (color != NULL);

    raylib_points = g_alloca (sizeof (Vector2) * point_count);
    for (i = 0; i < point_count; i++)
    {
        raylib_points[i] = GRL_TO_RAYLIB_VECTOR2 (&points[i]);
    }

    DrawSplineLinear (raylib_points, point_count, thickness, GRL_TO_RAYLIB_COLOR (color));
}

/**
 * grl_draw_spline_basis:
 * @points: (array length=point_count): Array of points (minimum 4).
 * @point_count: Number of points.
 * @thickness: Line thickness.
 * @color: Line color.
 *
 * Draws a B-spline.
 */
void
grl_draw_spline_basis (const GrlVector2 *points,
                       gint              point_count,
                       gfloat            thickness,
                       const GrlColor   *color)
{
    Vector2 *raylib_points;
    gint i;

    g_return_if_fail (points != NULL);
    g_return_if_fail (point_count >= 4);
    g_return_if_fail (color != NULL);

    raylib_points = g_alloca (sizeof (Vector2) * point_count);
    for (i = 0; i < point_count; i++)
    {
        raylib_points[i] = GRL_TO_RAYLIB_VECTOR2 (&points[i]);
    }

    DrawSplineBasis (raylib_points, point_count, thickness, GRL_TO_RAYLIB_COLOR (color));
}

/**
 * grl_draw_spline_catmull_rom:
 * @points: (array length=point_count): Array of points (minimum 4).
 * @point_count: Number of points.
 * @thickness: Line thickness.
 * @color: Line color.
 *
 * Draws a Catmull-Rom spline.
 */
void
grl_draw_spline_catmull_rom (const GrlVector2 *points,
                             gint              point_count,
                             gfloat            thickness,
                             const GrlColor   *color)
{
    Vector2 *raylib_points;
    gint i;

    g_return_if_fail (points != NULL);
    g_return_if_fail (point_count >= 4);
    g_return_if_fail (color != NULL);

    raylib_points = g_alloca (sizeof (Vector2) * point_count);
    for (i = 0; i < point_count; i++)
    {
        raylib_points[i] = GRL_TO_RAYLIB_VECTOR2 (&points[i]);
    }

    DrawSplineCatmullRom (raylib_points, point_count, thickness, GRL_TO_RAYLIB_COLOR (color));
}

/**
 * grl_draw_spline_bezier_quadratic:
 * @points: (array length=point_count): Array of points (minimum 3).
 * @point_count: Number of points.
 * @thickness: Line thickness.
 * @color: Line color.
 *
 * Draws a quadratic Bezier spline.
 */
void
grl_draw_spline_bezier_quadratic (const GrlVector2 *points,
                                  gint              point_count,
                                  gfloat            thickness,
                                  const GrlColor   *color)
{
    Vector2 *raylib_points;
    gint i;

    g_return_if_fail (points != NULL);
    g_return_if_fail (point_count >= 3);
    g_return_if_fail (color != NULL);

    raylib_points = g_alloca (sizeof (Vector2) * point_count);
    for (i = 0; i < point_count; i++)
    {
        raylib_points[i] = GRL_TO_RAYLIB_VECTOR2 (&points[i]);
    }

    DrawSplineBezierQuadratic (raylib_points, point_count, thickness, GRL_TO_RAYLIB_COLOR (color));
}

/**
 * grl_draw_spline_bezier_cubic:
 * @points: (array length=point_count): Array of points (minimum 4).
 * @point_count: Number of points.
 * @thickness: Line thickness.
 * @color: Line color.
 *
 * Draws a cubic Bezier spline.
 */
void
grl_draw_spline_bezier_cubic (const GrlVector2 *points,
                              gint              point_count,
                              gfloat            thickness,
                              const GrlColor   *color)
{
    Vector2 *raylib_points;
    gint i;

    g_return_if_fail (points != NULL);
    g_return_if_fail (point_count >= 4);
    g_return_if_fail (color != NULL);

    raylib_points = g_alloca (sizeof (Vector2) * point_count);
    for (i = 0; i < point_count; i++)
    {
        raylib_points[i] = GRL_TO_RAYLIB_VECTOR2 (&points[i]);
    }

    DrawSplineBezierCubic (raylib_points, point_count, thickness, GRL_TO_RAYLIB_COLOR (color));
}

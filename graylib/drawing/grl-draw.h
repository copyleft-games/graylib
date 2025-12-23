/* grl-draw.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Drawing functions for Graylib.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "../grl-version.h"
#include "../grl-enums.h"
#include "../math/grl-vector2.h"
#include "../math/grl-vector3.h"
#include "../math/grl-color.h"
#include "../math/grl-rectangle.h"
#include "../graphics/grl-texture.h"
#include "../graphics/grl-font.h"

G_BEGIN_DECLS

/*
 * Background
 */

GRL_AVAILABLE_IN_ALL
void    grl_draw_clear_background       (const GrlColor     *color);

/*
 * Blend modes
 */

GRL_AVAILABLE_IN_ALL
void    grl_draw_begin_blend_mode       (GrlBlendMode        mode);

GRL_AVAILABLE_IN_ALL
void    grl_draw_end_blend_mode         (void);

/*
 * Scissor mode
 */

GRL_AVAILABLE_IN_ALL
void    grl_draw_begin_scissor_mode     (gint                x,
                                         gint                y,
                                         gint                width,
                                         gint                height);

GRL_AVAILABLE_IN_ALL
void    grl_draw_end_scissor_mode       (void);

/*
 * Pixel drawing
 */

GRL_AVAILABLE_IN_ALL
void    grl_draw_pixel                  (gint                x,
                                         gint                y,
                                         const GrlColor     *color);

GRL_AVAILABLE_IN_ALL
void    grl_draw_pixel_v                (const GrlVector2   *position,
                                         const GrlColor     *color);

/*
 * Line drawing
 */

GRL_AVAILABLE_IN_ALL
void    grl_draw_line                   (gint                start_x,
                                         gint                start_y,
                                         gint                end_x,
                                         gint                end_y,
                                         const GrlColor     *color);

GRL_AVAILABLE_IN_ALL
void    grl_draw_line_v                 (const GrlVector2   *start,
                                         const GrlVector2   *end,
                                         const GrlColor     *color);

GRL_AVAILABLE_IN_ALL
void    grl_draw_line_ex                (const GrlVector2   *start,
                                         const GrlVector2   *end,
                                         gfloat              thickness,
                                         const GrlColor     *color);

GRL_AVAILABLE_IN_ALL
void    grl_draw_line_strip             (const GrlVector2   *points,
                                         gint                point_count,
                                         const GrlColor     *color);

GRL_AVAILABLE_IN_ALL
void    grl_draw_line_bezier            (const GrlVector2   *start,
                                         const GrlVector2   *end,
                                         gfloat              thickness,
                                         const GrlColor     *color);

/*
 * Circle drawing
 */

GRL_AVAILABLE_IN_ALL
void    grl_draw_circle                 (gint                center_x,
                                         gint                center_y,
                                         gfloat              radius,
                                         const GrlColor     *color);

GRL_AVAILABLE_IN_ALL
void    grl_draw_circle_v               (const GrlVector2   *center,
                                         gfloat              radius,
                                         const GrlColor     *color);

GRL_AVAILABLE_IN_ALL
void    grl_draw_circle_lines           (gint                center_x,
                                         gint                center_y,
                                         gfloat              radius,
                                         const GrlColor     *color);

GRL_AVAILABLE_IN_ALL
void    grl_draw_circle_lines_v         (const GrlVector2   *center,
                                         gfloat              radius,
                                         const GrlColor     *color);

GRL_AVAILABLE_IN_ALL
void    grl_draw_circle_sector          (const GrlVector2   *center,
                                         gfloat              radius,
                                         gfloat              start_angle,
                                         gfloat              end_angle,
                                         gint                segments,
                                         const GrlColor     *color);

GRL_AVAILABLE_IN_ALL
void    grl_draw_circle_sector_lines    (const GrlVector2   *center,
                                         gfloat              radius,
                                         gfloat              start_angle,
                                         gfloat              end_angle,
                                         gint                segments,
                                         const GrlColor     *color);

GRL_AVAILABLE_IN_ALL
void    grl_draw_circle_gradient        (gint                center_x,
                                         gint                center_y,
                                         gfloat              radius,
                                         const GrlColor     *inner,
                                         const GrlColor     *outer);

/*
 * Ellipse drawing
 */

GRL_AVAILABLE_IN_ALL
void    grl_draw_ellipse                (gint                center_x,
                                         gint                center_y,
                                         gfloat              radius_h,
                                         gfloat              radius_v,
                                         const GrlColor     *color);

GRL_AVAILABLE_IN_ALL
void    grl_draw_ellipse_lines          (gint                center_x,
                                         gint                center_y,
                                         gfloat              radius_h,
                                         gfloat              radius_v,
                                         const GrlColor     *color);

/*
 * Ring drawing
 */

GRL_AVAILABLE_IN_ALL
void    grl_draw_ring                   (const GrlVector2   *center,
                                         gfloat              inner_radius,
                                         gfloat              outer_radius,
                                         gfloat              start_angle,
                                         gfloat              end_angle,
                                         gint                segments,
                                         const GrlColor     *color);

GRL_AVAILABLE_IN_ALL
void    grl_draw_ring_lines             (const GrlVector2   *center,
                                         gfloat              inner_radius,
                                         gfloat              outer_radius,
                                         gfloat              start_angle,
                                         gfloat              end_angle,
                                         gint                segments,
                                         const GrlColor     *color);

/*
 * Rectangle drawing
 */

GRL_AVAILABLE_IN_ALL
void    grl_draw_rectangle              (gint                x,
                                         gint                y,
                                         gint                width,
                                         gint                height,
                                         const GrlColor     *color);

GRL_AVAILABLE_IN_ALL
void    grl_draw_rectangle_v            (const GrlVector2   *position,
                                         const GrlVector2   *size,
                                         const GrlColor     *color);

GRL_AVAILABLE_IN_ALL
void    grl_draw_rectangle_rec          (const GrlRectangle *rect,
                                         const GrlColor     *color);

GRL_AVAILABLE_IN_ALL
void    grl_draw_rectangle_pro          (const GrlRectangle *rect,
                                         const GrlVector2   *origin,
                                         gfloat              rotation,
                                         const GrlColor     *color);

GRL_AVAILABLE_IN_ALL
void    grl_draw_rectangle_lines        (gint                x,
                                         gint                y,
                                         gint                width,
                                         gint                height,
                                         const GrlColor     *color);

GRL_AVAILABLE_IN_ALL
void    grl_draw_rectangle_lines_ex     (const GrlRectangle *rect,
                                         gfloat              line_thickness,
                                         const GrlColor     *color);

GRL_AVAILABLE_IN_ALL
void    grl_draw_rectangle_rounded      (const GrlRectangle *rect,
                                         gfloat              roundness,
                                         gint                segments,
                                         const GrlColor     *color);

GRL_AVAILABLE_IN_ALL
void    grl_draw_rectangle_rounded_lines (const GrlRectangle *rect,
                                          gfloat              roundness,
                                          gint                segments,
                                          const GrlColor     *color);

GRL_AVAILABLE_IN_ALL
void    grl_draw_rectangle_rounded_lines_ex (const GrlRectangle *rect,
                                             gfloat              roundness,
                                             gint                segments,
                                             gfloat              line_thickness,
                                             const GrlColor     *color);

/*
 * Rectangle gradient drawing
 */

GRL_AVAILABLE_IN_ALL
void    grl_draw_rectangle_gradient_v   (gint                x,
                                         gint                y,
                                         gint                width,
                                         gint                height,
                                         const GrlColor     *top,
                                         const GrlColor     *bottom);

GRL_AVAILABLE_IN_ALL
void    grl_draw_rectangle_gradient_h   (gint                x,
                                         gint                y,
                                         gint                width,
                                         gint                height,
                                         const GrlColor     *left,
                                         const GrlColor     *right);

GRL_AVAILABLE_IN_ALL
void    grl_draw_rectangle_gradient_ex  (const GrlRectangle *rect,
                                         const GrlColor     *top_left,
                                         const GrlColor     *bottom_left,
                                         const GrlColor     *top_right,
                                         const GrlColor     *bottom_right);

/*
 * Triangle drawing
 */

GRL_AVAILABLE_IN_ALL
void    grl_draw_triangle               (const GrlVector2   *v1,
                                         const GrlVector2   *v2,
                                         const GrlVector2   *v3,
                                         const GrlColor     *color);

GRL_AVAILABLE_IN_ALL
void    grl_draw_triangle_lines         (const GrlVector2   *v1,
                                         const GrlVector2   *v2,
                                         const GrlVector2   *v3,
                                         const GrlColor     *color);

GRL_AVAILABLE_IN_ALL
void    grl_draw_triangle_fan           (const GrlVector2   *points,
                                         gint                point_count,
                                         const GrlColor     *color);

GRL_AVAILABLE_IN_ALL
void    grl_draw_triangle_strip         (const GrlVector2   *points,
                                         gint                point_count,
                                         const GrlColor     *color);

/*
 * Polygon drawing
 */

GRL_AVAILABLE_IN_ALL
void    grl_draw_poly                   (const GrlVector2   *center,
                                         gint                sides,
                                         gfloat              radius,
                                         gfloat              rotation,
                                         const GrlColor     *color);

GRL_AVAILABLE_IN_ALL
void    grl_draw_poly_lines             (const GrlVector2   *center,
                                         gint                sides,
                                         gfloat              radius,
                                         gfloat              rotation,
                                         const GrlColor     *color);

GRL_AVAILABLE_IN_ALL
void    grl_draw_poly_lines_ex          (const GrlVector2   *center,
                                         gint                sides,
                                         gfloat              radius,
                                         gfloat              rotation,
                                         gfloat              line_thickness,
                                         const GrlColor     *color);

/*
 * Spline drawing
 */

GRL_AVAILABLE_IN_ALL
void    grl_draw_spline_linear          (const GrlVector2   *points,
                                         gint                point_count,
                                         gfloat              thickness,
                                         const GrlColor     *color);

GRL_AVAILABLE_IN_ALL
void    grl_draw_spline_basis           (const GrlVector2   *points,
                                         gint                point_count,
                                         gfloat              thickness,
                                         const GrlColor     *color);

GRL_AVAILABLE_IN_ALL
void    grl_draw_spline_catmull_rom     (const GrlVector2   *points,
                                         gint                point_count,
                                         gfloat              thickness,
                                         const GrlColor     *color);

GRL_AVAILABLE_IN_ALL
void    grl_draw_spline_bezier_quadratic (const GrlVector2  *points,
                                          gint               point_count,
                                          gfloat             thickness,
                                          const GrlColor    *color);

GRL_AVAILABLE_IN_ALL
void    grl_draw_spline_bezier_cubic    (const GrlVector2   *points,
                                         gint                point_count,
                                         gfloat              thickness,
                                         const GrlColor     *color);

/*
 * =============================================================================
 * Texture drawing
 * =============================================================================
 */

/**
 * grl_draw_texture:
 * @texture: Texture to draw
 * @x: X position
 * @y: Y position
 * @tint: Color tint (use white for no tint)
 *
 * Draws a texture at the specified position.
 */
GRL_AVAILABLE_IN_ALL
void    grl_draw_texture                (GrlTexture         *texture,
                                         gint                x,
                                         gint                y,
                                         const GrlColor     *tint);

/**
 * grl_draw_texture_v:
 * @texture: Texture to draw
 * @position: Position vector
 * @tint: Color tint
 *
 * Draws a texture at a position defined by a vector.
 */
GRL_AVAILABLE_IN_ALL
void    grl_draw_texture_v              (GrlTexture         *texture,
                                         const GrlVector2   *position,
                                         const GrlColor     *tint);

/**
 * grl_draw_texture_ex:
 * @texture: Texture to draw
 * @position: Position vector
 * @rotation: Rotation in degrees
 * @scale: Scale factor
 * @tint: Color tint
 *
 * Draws a texture with rotation and scaling.
 */
GRL_AVAILABLE_IN_ALL
void    grl_draw_texture_ex             (GrlTexture         *texture,
                                         const GrlVector2   *position,
                                         gfloat              rotation,
                                         gfloat              scale,
                                         const GrlColor     *tint);

/**
 * grl_draw_texture_rec:
 * @texture: Texture to draw
 * @source: Source rectangle (region of texture to draw)
 * @position: Position to draw at
 * @tint: Color tint
 *
 * Draws a portion of a texture defined by a source rectangle.
 */
GRL_AVAILABLE_IN_ALL
void    grl_draw_texture_rec            (GrlTexture         *texture,
                                         const GrlRectangle *source,
                                         const GrlVector2   *position,
                                         const GrlColor     *tint);

/**
 * grl_draw_texture_pro:
 * @texture: Texture to draw
 * @source: Source rectangle (region of texture)
 * @dest: Destination rectangle (where to draw)
 * @origin: Origin point for rotation
 * @rotation: Rotation in degrees
 * @tint: Color tint
 *
 * Draws a texture with full control over source, destination,
 * rotation origin, and rotation angle.
 */
GRL_AVAILABLE_IN_ALL
void    grl_draw_texture_pro            (GrlTexture         *texture,
                                         const GrlRectangle *source,
                                         const GrlRectangle *dest,
                                         const GrlVector2   *origin,
                                         gfloat              rotation,
                                         const GrlColor     *tint);

/*
 * =============================================================================
 * Text drawing
 * =============================================================================
 */

/**
 * grl_draw_fps:
 * @x: X position
 * @y: Y position
 *
 * Draws the current FPS value at the specified position.
 * Uses the default font.
 */
GRL_AVAILABLE_IN_ALL
void    grl_draw_fps                    (gint                x,
                                         gint                y);

/**
 * grl_draw_text:
 * @text: Text to draw
 * @x: X position
 * @y: Y position
 * @font_size: Font size in pixels
 * @color: Text color
 *
 * Draws text using the default font.
 */
GRL_AVAILABLE_IN_ALL
void    grl_draw_text                   (const gchar        *text,
                                         gint                x,
                                         gint                y,
                                         gint                font_size,
                                         const GrlColor     *color);

/**
 * grl_draw_text_ex:
 * @font: Font to use
 * @text: Text to draw
 * @position: Position vector
 * @font_size: Font size in pixels
 * @spacing: Character spacing
 * @tint: Text color
 *
 * Draws text using a custom font with extended parameters.
 */
GRL_AVAILABLE_IN_ALL
void    grl_draw_text_ex                (GrlFont            *font,
                                         const gchar        *text,
                                         const GrlVector2   *position,
                                         gfloat              font_size,
                                         gfloat              spacing,
                                         const GrlColor     *tint);

/**
 * grl_draw_text_pro:
 * @font: Font to use
 * @text: Text to draw
 * @position: Position vector
 * @origin: Origin point for rotation
 * @rotation: Rotation in degrees
 * @font_size: Font size in pixels
 * @spacing: Character spacing
 * @tint: Text color
 *
 * Draws text with rotation support.
 */
GRL_AVAILABLE_IN_ALL
void    grl_draw_text_pro               (GrlFont            *font,
                                         const gchar        *text,
                                         const GrlVector2   *position,
                                         const GrlVector2   *origin,
                                         gfloat              rotation,
                                         gfloat              font_size,
                                         gfloat              spacing,
                                         const GrlColor     *tint);

/**
 * grl_draw_text_codepoint:
 * @font: Font to use
 * @codepoint: Unicode codepoint to draw
 * @position: Position vector
 * @font_size: Font size in pixels
 * @tint: Text color
 *
 * Draws a single character (Unicode codepoint).
 */
GRL_AVAILABLE_IN_ALL
void    grl_draw_text_codepoint         (GrlFont            *font,
                                         gint                codepoint,
                                         const GrlVector2   *position,
                                         gfloat              font_size,
                                         const GrlColor     *tint);

/**
 * grl_measure_text:
 * @text: Text to measure
 * @font_size: Font size in pixels
 *
 * Measures text width using the default font.
 *
 * Returns: Text width in pixels
 */
GRL_AVAILABLE_IN_ALL
gint    grl_measure_text                (const gchar        *text,
                                         gint                font_size);

/*
 * =============================================================================
 * 3D Drawing
 * =============================================================================
 */

/**
 * grl_draw_line_3d:
 * @start: Start position
 * @end: End position
 * @color: Line color
 *
 * Draws a line in 3D world space.
 */
GRL_AVAILABLE_IN_ALL
void    grl_draw_line_3d                (const GrlVector3   *start,
                                         const GrlVector3   *end,
                                         const GrlColor     *color);

/**
 * grl_draw_point_3d:
 * @position: Point position
 * @color: Point color
 *
 * Draws a point in 3D space (actually a small line).
 */
GRL_AVAILABLE_IN_ALL
void    grl_draw_point_3d               (const GrlVector3   *position,
                                         const GrlColor     *color);

/**
 * grl_draw_circle_3d:
 * @center: Center position
 * @radius: Circle radius
 * @rotation_axis: Axis of rotation
 * @rotation_angle: Rotation angle in degrees
 * @color: Circle color
 *
 * Draws a circle in 3D world space.
 */
GRL_AVAILABLE_IN_ALL
void    grl_draw_circle_3d              (const GrlVector3   *center,
                                         gfloat              radius,
                                         const GrlVector3   *rotation_axis,
                                         gfloat              rotation_angle,
                                         const GrlColor     *color);

/**
 * grl_draw_triangle_3d:
 * @v1: First vertex
 * @v2: Second vertex
 * @v3: Third vertex
 * @color: Triangle color
 *
 * Draws a filled triangle in 3D space (vertices in counter-clockwise order).
 */
GRL_AVAILABLE_IN_ALL
void    grl_draw_triangle_3d            (const GrlVector3   *v1,
                                         const GrlVector3   *v2,
                                         const GrlVector3   *v3,
                                         const GrlColor     *color);

/**
 * grl_draw_cube:
 * @position: Center position
 * @width: Width (X axis)
 * @height: Height (Y axis)
 * @length: Length (Z axis)
 * @color: Cube color
 *
 * Draws a solid cube.
 */
GRL_AVAILABLE_IN_ALL
void    grl_draw_cube                   (const GrlVector3   *position,
                                         gfloat              width,
                                         gfloat              height,
                                         gfloat              length,
                                         const GrlColor     *color);

/**
 * grl_draw_cube_v:
 * @position: Center position
 * @size: Size vector (width, height, length)
 * @color: Cube color
 *
 * Draws a solid cube using vector for size.
 */
GRL_AVAILABLE_IN_ALL
void    grl_draw_cube_v                 (const GrlVector3   *position,
                                         const GrlVector3   *size,
                                         const GrlColor     *color);

/**
 * grl_draw_cube_wires:
 * @position: Center position
 * @width: Width (X axis)
 * @height: Height (Y axis)
 * @length: Length (Z axis)
 * @color: Wire color
 *
 * Draws cube wireframe.
 */
GRL_AVAILABLE_IN_ALL
void    grl_draw_cube_wires             (const GrlVector3   *position,
                                         gfloat              width,
                                         gfloat              height,
                                         gfloat              length,
                                         const GrlColor     *color);

/**
 * grl_draw_sphere:
 * @center: Center position
 * @radius: Sphere radius
 * @color: Sphere color
 *
 * Draws a solid sphere.
 */
GRL_AVAILABLE_IN_ALL
void    grl_draw_sphere                 (const GrlVector3   *center,
                                         gfloat              radius,
                                         const GrlColor     *color);

/**
 * grl_draw_sphere_ex:
 * @center: Center position
 * @radius: Sphere radius
 * @rings: Number of horizontal rings
 * @slices: Number of vertical slices
 * @color: Sphere color
 *
 * Draws a sphere with extended parameters.
 */
GRL_AVAILABLE_IN_ALL
void    grl_draw_sphere_ex              (const GrlVector3   *center,
                                         gfloat              radius,
                                         gint                rings,
                                         gint                slices,
                                         const GrlColor     *color);

/**
 * grl_draw_sphere_wires:
 * @center: Center position
 * @radius: Sphere radius
 * @rings: Number of horizontal rings
 * @slices: Number of vertical slices
 * @color: Wire color
 *
 * Draws sphere wireframe.
 */
GRL_AVAILABLE_IN_ALL
void    grl_draw_sphere_wires           (const GrlVector3   *center,
                                         gfloat              radius,
                                         gint                rings,
                                         gint                slices,
                                         const GrlColor     *color);

/**
 * grl_draw_cylinder:
 * @position: Base center position
 * @radius_top: Top radius
 * @radius_bottom: Bottom radius
 * @height: Cylinder height
 * @slices: Number of slices
 * @color: Cylinder color
 *
 * Draws a cylinder or cone.
 */
GRL_AVAILABLE_IN_ALL
void    grl_draw_cylinder               (const GrlVector3   *position,
                                         gfloat              radius_top,
                                         gfloat              radius_bottom,
                                         gfloat              height,
                                         gint                slices,
                                         const GrlColor     *color);

/**
 * grl_draw_cylinder_wires:
 * @position: Base center position
 * @radius_top: Top radius
 * @radius_bottom: Bottom radius
 * @height: Cylinder height
 * @slices: Number of slices
 * @color: Wire color
 *
 * Draws cylinder wireframe.
 */
GRL_AVAILABLE_IN_ALL
void    grl_draw_cylinder_wires         (const GrlVector3   *position,
                                         gfloat              radius_top,
                                         gfloat              radius_bottom,
                                         gfloat              height,
                                         gint                slices,
                                         const GrlColor     *color);

/**
 * grl_draw_plane:
 * @center: Center position
 * @size: Plane size (x = width, y = length)
 * @color: Plane color
 *
 * Draws a plane on the XZ plane.
 */
GRL_AVAILABLE_IN_ALL
void    grl_draw_plane                  (const GrlVector3   *center,
                                         const GrlVector2   *size,
                                         const GrlColor     *color);

/**
 * grl_draw_grid:
 * @slices: Number of grid divisions
 * @spacing: Spacing between grid lines
 *
 * Draws a grid centered at (0, 0, 0).
 */
GRL_AVAILABLE_IN_ALL
void    grl_draw_grid                   (gint                slices,
                                         gfloat              spacing);

G_END_DECLS

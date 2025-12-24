/* grl-draw-3d.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * 3D drawing functions implementation.
 */

#include "grl-draw.h"
#include <raylib.h>

/*
 * Helper macros to convert Graylib types to raylib types
 */

#define GRL_TO_RAYLIB_VECTOR3(v) \
    ((Vector3){ .x = (v)->x, .y = (v)->y, .z = (v)->z })

#define GRL_TO_RAYLIB_VECTOR2(v) \
    ((Vector2){ .x = (v)->x, .y = (v)->y })

#define GRL_TO_RAYLIB_COLOR(c) \
    ((Color){ .r = (c)->r, .g = (c)->g, .b = (c)->b, .a = (c)->a })

/**
 * grl_draw_line_3d:
 * @start: Start position
 * @end: End position
 * @color: Line color
 *
 * Draws a line in 3D world space.
 */
void
grl_draw_line_3d (const GrlVector3 *start,
                  const GrlVector3 *end,
                  const GrlColor   *color)
{
    g_return_if_fail (start != NULL);
    g_return_if_fail (end != NULL);
    g_return_if_fail (color != NULL);

    DrawLine3D (GRL_TO_RAYLIB_VECTOR3 (start),
                GRL_TO_RAYLIB_VECTOR3 (end),
                GRL_TO_RAYLIB_COLOR (color));
}

/**
 * grl_draw_point_3d:
 * @position: Point position
 * @color: Point color
 *
 * Draws a point in 3D space.
 */
void
grl_draw_point_3d (const GrlVector3 *position,
                   const GrlColor   *color)
{
    g_return_if_fail (position != NULL);
    g_return_if_fail (color != NULL);

    DrawPoint3D (GRL_TO_RAYLIB_VECTOR3 (position),
                 GRL_TO_RAYLIB_COLOR (color));
}

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
void
grl_draw_circle_3d (const GrlVector3 *center,
                    gfloat            radius,
                    const GrlVector3 *rotation_axis,
                    gfloat            rotation_angle,
                    const GrlColor   *color)
{
    g_return_if_fail (center != NULL);
    g_return_if_fail (rotation_axis != NULL);
    g_return_if_fail (color != NULL);

    DrawCircle3D (GRL_TO_RAYLIB_VECTOR3 (center),
                  radius,
                  GRL_TO_RAYLIB_VECTOR3 (rotation_axis),
                  rotation_angle,
                  GRL_TO_RAYLIB_COLOR (color));
}

/**
 * grl_draw_triangle_3d:
 * @v1: First vertex
 * @v2: Second vertex
 * @v3: Third vertex
 * @color: Triangle color
 *
 * Draws a filled triangle in 3D space.
 */
void
grl_draw_triangle_3d (const GrlVector3 *v1,
                      const GrlVector3 *v2,
                      const GrlVector3 *v3,
                      const GrlColor   *color)
{
    g_return_if_fail (v1 != NULL);
    g_return_if_fail (v2 != NULL);
    g_return_if_fail (v3 != NULL);
    g_return_if_fail (color != NULL);

    DrawTriangle3D (GRL_TO_RAYLIB_VECTOR3 (v1),
                    GRL_TO_RAYLIB_VECTOR3 (v2),
                    GRL_TO_RAYLIB_VECTOR3 (v3),
                    GRL_TO_RAYLIB_COLOR (color));
}

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
void
grl_draw_cube (const GrlVector3 *position,
               gfloat            width,
               gfloat            height,
               gfloat            length,
               const GrlColor   *color)
{
    g_return_if_fail (position != NULL);
    g_return_if_fail (color != NULL);

    DrawCube (GRL_TO_RAYLIB_VECTOR3 (position),
              width, height, length,
              GRL_TO_RAYLIB_COLOR (color));
}

/**
 * grl_draw_cube_v:
 * @position: Center position
 * @size: Size vector
 * @color: Cube color
 *
 * Draws a solid cube using vector for size.
 */
void
grl_draw_cube_v (const GrlVector3 *position,
                 const GrlVector3 *size,
                 const GrlColor   *color)
{
    g_return_if_fail (position != NULL);
    g_return_if_fail (size != NULL);
    g_return_if_fail (color != NULL);

    DrawCubeV (GRL_TO_RAYLIB_VECTOR3 (position),
               GRL_TO_RAYLIB_VECTOR3 (size),
               GRL_TO_RAYLIB_COLOR (color));
}

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
void
grl_draw_cube_wires (const GrlVector3 *position,
                     gfloat            width,
                     gfloat            height,
                     gfloat            length,
                     const GrlColor   *color)
{
    g_return_if_fail (position != NULL);
    g_return_if_fail (color != NULL);

    DrawCubeWires (GRL_TO_RAYLIB_VECTOR3 (position),
                   width, height, length,
                   GRL_TO_RAYLIB_COLOR (color));
}

/**
 * grl_draw_sphere:
 * @center: Center position
 * @radius: Sphere radius
 * @color: Sphere color
 *
 * Draws a solid sphere.
 */
void
grl_draw_sphere (const GrlVector3 *center,
                 gfloat            radius,
                 const GrlColor   *color)
{
    g_return_if_fail (center != NULL);
    g_return_if_fail (color != NULL);

    DrawSphere (GRL_TO_RAYLIB_VECTOR3 (center),
                radius,
                GRL_TO_RAYLIB_COLOR (color));
}

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
void
grl_draw_sphere_ex (const GrlVector3 *center,
                    gfloat            radius,
                    gint              rings,
                    gint              slices,
                    const GrlColor   *color)
{
    g_return_if_fail (center != NULL);
    g_return_if_fail (color != NULL);

    DrawSphereEx (GRL_TO_RAYLIB_VECTOR3 (center),
                  radius,
                  rings, slices,
                  GRL_TO_RAYLIB_COLOR (color));
}

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
void
grl_draw_sphere_wires (const GrlVector3 *center,
                       gfloat            radius,
                       gint              rings,
                       gint              slices,
                       const GrlColor   *color)
{
    g_return_if_fail (center != NULL);
    g_return_if_fail (color != NULL);

    DrawSphereWires (GRL_TO_RAYLIB_VECTOR3 (center),
                     radius,
                     rings, slices,
                     GRL_TO_RAYLIB_COLOR (color));
}

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
void
grl_draw_cylinder (const GrlVector3 *position,
                   gfloat            radius_top,
                   gfloat            radius_bottom,
                   gfloat            height,
                   gint              slices,
                   const GrlColor   *color)
{
    g_return_if_fail (position != NULL);
    g_return_if_fail (color != NULL);

    DrawCylinder (GRL_TO_RAYLIB_VECTOR3 (position),
                  radius_top, radius_bottom, height, slices,
                  GRL_TO_RAYLIB_COLOR (color));
}

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
void
grl_draw_cylinder_wires (const GrlVector3 *position,
                         gfloat            radius_top,
                         gfloat            radius_bottom,
                         gfloat            height,
                         gint              slices,
                         const GrlColor   *color)
{
    g_return_if_fail (position != NULL);
    g_return_if_fail (color != NULL);

    DrawCylinderWires (GRL_TO_RAYLIB_VECTOR3 (position),
                       radius_top, radius_bottom, height, slices,
                       GRL_TO_RAYLIB_COLOR (color));
}

/**
 * grl_draw_plane:
 * @center: Center position
 * @size: Plane size (x = width, y = length)
 * @color: Plane color
 *
 * Draws a plane on the XZ plane.
 */
void
grl_draw_plane (const GrlVector3 *center,
                const GrlVector2 *size,
                const GrlColor   *color)
{
    g_return_if_fail (center != NULL);
    g_return_if_fail (size != NULL);
    g_return_if_fail (color != NULL);

    DrawPlane (GRL_TO_RAYLIB_VECTOR3 (center),
               GRL_TO_RAYLIB_VECTOR2 (size),
               GRL_TO_RAYLIB_COLOR (color));
}

/**
 * grl_draw_grid:
 * @slices: Number of grid divisions
 * @spacing: Spacing between grid lines
 *
 * Draws a grid centered at (0, 0, 0).
 */
void
grl_draw_grid (gint   slices,
               gfloat spacing)
{
    DrawGrid (slices, spacing);
}

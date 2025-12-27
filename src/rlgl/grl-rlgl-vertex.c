/* grl-rlgl-vertex.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Immediate mode vertex operations for rlgl.
 */

#include "config.h"
#include "grl-rlgl.h"
#include <rlgl.h>

/**
 * grl_rlgl_begin:
 * @mode: Drawing mode (lines, triangles, or quads)
 *
 * Begins a new vertex batch for immediate mode drawing.
 */
void
grl_rlgl_begin (GrlRlglDrawMode mode)
{
    rlBegin ((int)mode);
}

/**
 * grl_rlgl_end:
 *
 * Ends the current vertex batch and flushes it to the GPU.
 */
void
grl_rlgl_end (void)
{
    rlEnd ();
}

/**
 * grl_rlgl_vertex2i:
 * @x: X position (integer)
 * @y: Y position (integer)
 *
 * Specifies a 2D vertex position using integers.
 */
void
grl_rlgl_vertex2i (gint x,
                   gint y)
{
    rlVertex2i (x, y);
}

/**
 * grl_rlgl_vertex2f:
 * @x: X position
 * @y: Y position
 *
 * Specifies a 2D vertex position using floats.
 */
void
grl_rlgl_vertex2f (gfloat x,
                   gfloat y)
{
    rlVertex2f (x, y);
}

/**
 * grl_rlgl_vertex3f:
 * @x: X position
 * @y: Y position
 * @z: Z position
 *
 * Specifies a 3D vertex position.
 */
void
grl_rlgl_vertex3f (gfloat x,
                   gfloat y,
                   gfloat z)
{
    rlVertex3f (x, y, z);
}

/**
 * grl_rlgl_tex_coord2f:
 * @x: U texture coordinate
 * @y: V texture coordinate
 *
 * Specifies texture coordinates for the current vertex.
 */
void
grl_rlgl_tex_coord2f (gfloat x,
                      gfloat y)
{
    rlTexCoord2f (x, y);
}

/**
 * grl_rlgl_normal3f:
 * @x: X component of normal
 * @y: Y component of normal
 * @z: Z component of normal
 *
 * Specifies a surface normal for the current vertex.
 */
void
grl_rlgl_normal3f (gfloat x,
                   gfloat y,
                   gfloat z)
{
    rlNormal3f (x, y, z);
}

/**
 * grl_rlgl_color4ub:
 * @r: Red component (0-255)
 * @g: Green component (0-255)
 * @b: Blue component (0-255)
 * @a: Alpha component (0-255)
 *
 * Specifies a color for the current vertex using unsigned bytes.
 */
void
grl_rlgl_color4ub (guint8 r,
                   guint8 g,
                   guint8 b,
                   guint8 a)
{
    rlColor4ub (r, g, b, a);
}

/**
 * grl_rlgl_color3f:
 * @r: Red component (0.0-1.0)
 * @g: Green component (0.0-1.0)
 * @b: Blue component (0.0-1.0)
 *
 * Specifies an RGB color for the current vertex using floats.
 */
void
grl_rlgl_color3f (gfloat r,
                  gfloat g,
                  gfloat b)
{
    rlColor3f (r, g, b);
}

/**
 * grl_rlgl_color4f:
 * @r: Red component (0.0-1.0)
 * @g: Green component (0.0-1.0)
 * @b: Blue component (0.0-1.0)
 * @a: Alpha component (0.0-1.0)
 *
 * Specifies an RGBA color for the current vertex using floats.
 */
void
grl_rlgl_color4f (gfloat r,
                  gfloat g,
                  gfloat b,
                  gfloat a)
{
    rlColor4f (r, g, b, a);
}

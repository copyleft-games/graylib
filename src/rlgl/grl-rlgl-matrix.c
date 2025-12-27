/* grl-rlgl-matrix.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Matrix stack operations for rlgl.
 */

#include "config.h"
#include "grl-rlgl.h"
#include <rlgl.h>

/**
 * grl_rlgl_matrix_mode:
 * @mode: Matrix mode to set
 *
 * Sets the current matrix mode (modelview, projection, or texture).
 */
void
grl_rlgl_matrix_mode (GrlRlglMatrixMode mode)
{
    rlMatrixMode ((int)mode);
}

/**
 * grl_rlgl_push_matrix:
 *
 * Pushes the current matrix onto the matrix stack.
 */
void
grl_rlgl_push_matrix (void)
{
    rlPushMatrix ();
}

/**
 * grl_rlgl_pop_matrix:
 *
 * Pops the top matrix from the matrix stack.
 */
void
grl_rlgl_pop_matrix (void)
{
    rlPopMatrix ();
}

/**
 * grl_rlgl_load_identity:
 *
 * Resets the current matrix to the identity matrix.
 */
void
grl_rlgl_load_identity (void)
{
    rlLoadIdentity ();
}

/**
 * grl_rlgl_translatef:
 * @x: X translation
 * @y: Y translation
 * @z: Z translation
 *
 * Multiplies the current matrix by a translation matrix.
 */
void
grl_rlgl_translatef (gfloat x,
                     gfloat y,
                     gfloat z)
{
    rlTranslatef (x, y, z);
}

/**
 * grl_rlgl_rotatef:
 * @angle: Rotation angle in degrees
 * @x: X component of rotation axis
 * @y: Y component of rotation axis
 * @z: Z component of rotation axis
 *
 * Multiplies the current matrix by a rotation matrix.
 */
void
grl_rlgl_rotatef (gfloat angle,
                  gfloat x,
                  gfloat y,
                  gfloat z)
{
    rlRotatef (angle, x, y, z);
}

/**
 * grl_rlgl_scalef:
 * @x: X scale factor
 * @y: Y scale factor
 * @z: Z scale factor
 *
 * Multiplies the current matrix by a scaling matrix.
 */
void
grl_rlgl_scalef (gfloat x,
                 gfloat y,
                 gfloat z)
{
    rlScalef (x, y, z);
}

/**
 * grl_rlgl_mult_matrixf:
 * @matf: (array fixed-size=16): 4x4 matrix in column-major order
 *
 * Multiplies the current matrix by the specified matrix.
 */
void
grl_rlgl_mult_matrixf (const gfloat *matf)
{
    g_return_if_fail (matf != NULL);

    rlMultMatrixf (matf);
}

/**
 * grl_rlgl_frustum:
 * @left: Left clipping plane
 * @right: Right clipping plane
 * @bottom: Bottom clipping plane
 * @top: Top clipping plane
 * @znear: Near clipping plane distance
 * @zfar: Far clipping plane distance
 *
 * Sets up a perspective projection frustum.
 */
void
grl_rlgl_frustum (gdouble left,
                  gdouble right,
                  gdouble bottom,
                  gdouble top,
                  gdouble znear,
                  gdouble zfar)
{
    rlFrustum (left, right, bottom, top, znear, zfar);
}

/**
 * grl_rlgl_ortho:
 * @left: Left clipping plane
 * @right: Right clipping plane
 * @bottom: Bottom clipping plane
 * @top: Top clipping plane
 * @znear: Near clipping plane distance
 * @zfar: Far clipping plane distance
 *
 * Sets up an orthographic projection.
 */
void
grl_rlgl_ortho (gdouble left,
                gdouble right,
                gdouble bottom,
                gdouble top,
                gdouble znear,
                gdouble zfar)
{
    rlOrtho (left, right, bottom, top, znear, zfar);
}

/**
 * grl_rlgl_viewport:
 * @x: Viewport X position
 * @y: Viewport Y position
 * @width: Viewport width
 * @height: Viewport height
 *
 * Sets the viewport area.
 */
void
grl_rlgl_viewport (gint x,
                   gint y,
                   gint width,
                   gint height)
{
    rlViewport (x, y, width, height);
}

/**
 * grl_rlgl_set_clip_planes:
 * @near_plane: Near clip plane distance
 * @far_plane: Far clip plane distance
 *
 * Sets the near and far clip plane distances.
 */
void
grl_rlgl_set_clip_planes (gdouble near_plane,
                          gdouble far_plane)
{
    rlSetClipPlanes (near_plane, far_plane);
}

/**
 * grl_rlgl_get_cull_distance_near:
 *
 * Gets the near cull distance.
 *
 * Returns: Near cull distance
 */
gdouble
grl_rlgl_get_cull_distance_near (void)
{
    return rlGetCullDistanceNear ();
}

/**
 * grl_rlgl_get_cull_distance_far:
 *
 * Gets the far cull distance.
 *
 * Returns: Far cull distance
 */
gdouble
grl_rlgl_get_cull_distance_far (void)
{
    return rlGetCullDistanceFar ();
}

/**
 * grl_rlgl_get_matrix_modelview:
 *
 * Gets the current modelview matrix.
 *
 * Returns: (transfer full): The current modelview matrix
 */
GrlMatrix *
grl_rlgl_get_matrix_modelview (void)
{
    Matrix m;
    GrlMatrix *result;

    m = rlGetMatrixModelview ();
    result = grl_matrix_new ();

    result->m0 = m.m0;
    result->m1 = m.m1;
    result->m2 = m.m2;
    result->m3 = m.m3;
    result->m4 = m.m4;
    result->m5 = m.m5;
    result->m6 = m.m6;
    result->m7 = m.m7;
    result->m8 = m.m8;
    result->m9 = m.m9;
    result->m10 = m.m10;
    result->m11 = m.m11;
    result->m12 = m.m12;
    result->m13 = m.m13;
    result->m14 = m.m14;
    result->m15 = m.m15;

    return result;
}

/**
 * grl_rlgl_get_matrix_projection:
 *
 * Gets the current projection matrix.
 *
 * Returns: (transfer full): The current projection matrix
 */
GrlMatrix *
grl_rlgl_get_matrix_projection (void)
{
    Matrix m;
    GrlMatrix *result;

    m = rlGetMatrixProjection ();
    result = grl_matrix_new ();

    result->m0 = m.m0;
    result->m1 = m.m1;
    result->m2 = m.m2;
    result->m3 = m.m3;
    result->m4 = m.m4;
    result->m5 = m.m5;
    result->m6 = m.m6;
    result->m7 = m.m7;
    result->m8 = m.m8;
    result->m9 = m.m9;
    result->m10 = m.m10;
    result->m11 = m.m11;
    result->m12 = m.m12;
    result->m13 = m.m13;
    result->m14 = m.m14;
    result->m15 = m.m15;

    return result;
}

/**
 * grl_rlgl_get_matrix_transform:
 *
 * Gets the current accumulated transform matrix.
 *
 * Returns: (transfer full): The current transform matrix
 */
GrlMatrix *
grl_rlgl_get_matrix_transform (void)
{
    Matrix m;
    GrlMatrix *result;

    m = rlGetMatrixTransform ();
    result = grl_matrix_new ();

    result->m0 = m.m0;
    result->m1 = m.m1;
    result->m2 = m.m2;
    result->m3 = m.m3;
    result->m4 = m.m4;
    result->m5 = m.m5;
    result->m6 = m.m6;
    result->m7 = m.m7;
    result->m8 = m.m8;
    result->m9 = m.m9;
    result->m10 = m.m10;
    result->m11 = m.m11;
    result->m12 = m.m12;
    result->m13 = m.m13;
    result->m14 = m.m14;
    result->m15 = m.m15;

    return result;
}

/**
 * grl_rlgl_set_matrix_projection:
 * @proj: Projection matrix to set
 *
 * Sets the current projection matrix.
 */
void
grl_rlgl_set_matrix_projection (const GrlMatrix *proj)
{
    Matrix m;

    g_return_if_fail (proj != NULL);

    m.m0 = proj->m0;
    m.m1 = proj->m1;
    m.m2 = proj->m2;
    m.m3 = proj->m3;
    m.m4 = proj->m4;
    m.m5 = proj->m5;
    m.m6 = proj->m6;
    m.m7 = proj->m7;
    m.m8 = proj->m8;
    m.m9 = proj->m9;
    m.m10 = proj->m10;
    m.m11 = proj->m11;
    m.m12 = proj->m12;
    m.m13 = proj->m13;
    m.m14 = proj->m14;
    m.m15 = proj->m15;

    rlSetMatrixProjection (m);
}

/**
 * grl_rlgl_set_matrix_modelview:
 * @view: Modelview matrix to set
 *
 * Sets the current modelview matrix.
 */
void
grl_rlgl_set_matrix_modelview (const GrlMatrix *view)
{
    Matrix m;

    g_return_if_fail (view != NULL);

    m.m0 = view->m0;
    m.m1 = view->m1;
    m.m2 = view->m2;
    m.m3 = view->m3;
    m.m4 = view->m4;
    m.m5 = view->m5;
    m.m6 = view->m6;
    m.m7 = view->m7;
    m.m8 = view->m8;
    m.m9 = view->m9;
    m.m10 = view->m10;
    m.m11 = view->m11;
    m.m12 = view->m12;
    m.m13 = view->m13;
    m.m14 = view->m14;
    m.m15 = view->m15;

    rlSetMatrixModelview (m);
}

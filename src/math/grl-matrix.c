/* grl-matrix.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "grl-matrix.h"
#include <raylib.h>
#include <math.h>

/* Declare raymath functions externally - these are provided by raylib */
/* We don't include raymath.h directly to avoid multiple definition issues */
extern Matrix MatrixIdentity(void);
extern Matrix MatrixTranslate(float x, float y, float z);
extern Matrix MatrixRotate(Vector3 axis, float angle);
extern Matrix MatrixRotateXYZ(Vector3 angle);
extern Matrix MatrixScale(float x, float y, float z);
extern Matrix MatrixFrustum(double left, double right, double bottom, double top, double nearPlane, double farPlane);
extern Matrix MatrixPerspective(double fovY, double aspect, double nearPlane, double farPlane);
extern Matrix MatrixOrtho(double left, double right, double bottom, double top, double nearPlane, double farPlane);
extern Matrix MatrixLookAt(Vector3 eye, Vector3 target, Vector3 up);
extern Matrix MatrixMultiply(Matrix left, Matrix right);
extern Matrix MatrixInvert(Matrix mat);
extern Matrix MatrixTranspose(Matrix mat);
extern float MatrixDeterminant(Matrix mat);
extern float MatrixTrace(Matrix mat);

/**
 * SECTION:grl-matrix
 * @Title: GrlMatrix
 * @Short_description: 4x4 transformation matrix
 *
 * #GrlMatrix is a 4x4 transformation matrix type used for 3D transformations
 * including translation, rotation, scaling, and projection.
 *
 * The matrix is stored in column-major order (OpenGL style) and is compatible
 * with raylib's Matrix type.
 */

G_DEFINE_BOXED_TYPE (GrlMatrix, grl_matrix, grl_matrix_copy, grl_matrix_free)

/*
 * Helper to convert raylib Matrix to GrlMatrix
 */
static GrlMatrix *
grl_matrix_from_raylib (Matrix m)
{
    GrlMatrix *self;

    self = g_slice_new (GrlMatrix);
    self->m0 = m.m0;   self->m4 = m.m4;   self->m8 = m.m8;   self->m12 = m.m12;
    self->m1 = m.m1;   self->m5 = m.m5;   self->m9 = m.m9;   self->m13 = m.m13;
    self->m2 = m.m2;   self->m6 = m.m6;   self->m10 = m.m10; self->m14 = m.m14;
    self->m3 = m.m3;   self->m7 = m.m7;   self->m11 = m.m11; self->m15 = m.m15;

    return self;
}

/*
 * Helper to convert GrlMatrix to raylib Matrix
 */
static Matrix
grl_matrix_to_raylib (const GrlMatrix *self)
{
    Matrix m;

    m.m0 = self->m0;   m.m4 = self->m4;   m.m8 = self->m8;   m.m12 = self->m12;
    m.m1 = self->m1;   m.m5 = self->m5;   m.m9 = self->m9;   m.m13 = self->m13;
    m.m2 = self->m2;   m.m6 = self->m6;   m.m10 = self->m10; m.m14 = self->m14;
    m.m3 = self->m3;   m.m7 = self->m7;   m.m11 = self->m11; m.m15 = self->m15;

    return m;
}

/**
 * grl_matrix_new:
 *
 * Creates a new uninitialized matrix.
 * For a useful starting point, use grl_matrix_new_identity().
 *
 * Returns: (transfer full): A newly allocated #GrlMatrix
 */
GrlMatrix *
grl_matrix_new (void)
{
    return g_slice_new0 (GrlMatrix);
}

/**
 * grl_matrix_copy:
 * @self: (nullable): A #GrlMatrix
 *
 * Creates a copy of the matrix.
 *
 * Returns: (transfer full) (nullable): A copy, or %NULL if @self is %NULL
 */
GrlMatrix *
grl_matrix_copy (const GrlMatrix *self)
{
    GrlMatrix *copy;

    if (self == NULL)
        return NULL;

    copy = g_slice_new (GrlMatrix);
    *copy = *self;

    return copy;
}

/**
 * grl_matrix_free:
 * @self: (nullable): A #GrlMatrix
 *
 * Frees a matrix allocated with grl_matrix_new() or grl_matrix_copy().
 */
void
grl_matrix_free (GrlMatrix *self)
{
    if (self != NULL)
        g_slice_free (GrlMatrix, self);
}

/**
 * grl_matrix_new_identity:
 *
 * Creates a new identity matrix.
 * The identity matrix has 1s on the diagonal and 0s elsewhere.
 *
 * Returns: (transfer full): A new identity matrix
 */
GrlMatrix *
grl_matrix_new_identity (void)
{
    return grl_matrix_from_raylib (MatrixIdentity ());
}

/**
 * grl_matrix_new_translate:
 * @x: X translation
 * @y: Y translation
 * @z: Z translation
 *
 * Creates a new translation matrix.
 *
 * Returns: (transfer full): A new translation matrix
 */
GrlMatrix *
grl_matrix_new_translate (gfloat x,
                          gfloat y,
                          gfloat z)
{
    return grl_matrix_from_raylib (MatrixTranslate (x, y, z));
}

/**
 * grl_matrix_new_rotate:
 * @axis: The axis to rotate around
 * @angle: The rotation angle in radians
 *
 * Creates a new rotation matrix around the given axis.
 *
 * Returns: (transfer full): A new rotation matrix
 */
GrlMatrix *
grl_matrix_new_rotate (const GrlVector3 *axis,
                       gfloat            angle)
{
    Vector3 rl_axis;

    g_return_val_if_fail (axis != NULL, NULL);

    rl_axis.x = axis->x;
    rl_axis.y = axis->y;
    rl_axis.z = axis->z;

    return grl_matrix_from_raylib (MatrixRotate (rl_axis, angle));
}

/**
 * grl_matrix_new_rotate_xyz:
 * @x: X rotation angle in radians
 * @y: Y rotation angle in radians
 * @z: Z rotation angle in radians
 *
 * Creates a new rotation matrix from Euler angles (XYZ order).
 *
 * Returns: (transfer full): A new rotation matrix
 */
GrlMatrix *
grl_matrix_new_rotate_xyz (gfloat x,
                           gfloat y,
                           gfloat z)
{
    return grl_matrix_from_raylib (MatrixRotateXYZ ((Vector3){ x, y, z }));
}

/**
 * grl_matrix_new_scale:
 * @x: X scale factor
 * @y: Y scale factor
 * @z: Z scale factor
 *
 * Creates a new scale matrix.
 *
 * Returns: (transfer full): A new scale matrix
 */
GrlMatrix *
grl_matrix_new_scale (gfloat x,
                      gfloat y,
                      gfloat z)
{
    return grl_matrix_from_raylib (MatrixScale (x, y, z));
}

/**
 * grl_matrix_new_frustum:
 * @left: Left clipping plane
 * @right: Right clipping plane
 * @bottom: Bottom clipping plane
 * @top: Top clipping plane
 * @near_plane: Near clipping plane
 * @far_plane: Far clipping plane
 *
 * Creates a new perspective frustum projection matrix.
 *
 * Returns: (transfer full): A new frustum matrix
 */
GrlMatrix *
grl_matrix_new_frustum (gdouble left,
                        gdouble right,
                        gdouble bottom,
                        gdouble top,
                        gdouble near_plane,
                        gdouble far_plane)
{
    return grl_matrix_from_raylib (MatrixFrustum (left, right, bottom, top,
                                                  near_plane, far_plane));
}

/**
 * grl_matrix_new_perspective:
 * @fovy: Field of view Y in radians
 * @aspect: Aspect ratio (width / height)
 * @near_plane: Near clipping plane
 * @far_plane: Far clipping plane
 *
 * Creates a new perspective projection matrix.
 *
 * Returns: (transfer full): A new perspective matrix
 */
GrlMatrix *
grl_matrix_new_perspective (gdouble fovy,
                            gdouble aspect,
                            gdouble near_plane,
                            gdouble far_plane)
{
    return grl_matrix_from_raylib (MatrixPerspective (fovy, aspect,
                                                      near_plane, far_plane));
}

/**
 * grl_matrix_new_ortho:
 * @left: Left clipping plane
 * @right: Right clipping plane
 * @bottom: Bottom clipping plane
 * @top: Top clipping plane
 * @near_plane: Near clipping plane
 * @far_plane: Far clipping plane
 *
 * Creates a new orthographic projection matrix.
 *
 * Returns: (transfer full): A new orthographic matrix
 */
GrlMatrix *
grl_matrix_new_ortho (gdouble left,
                      gdouble right,
                      gdouble bottom,
                      gdouble top,
                      gdouble near_plane,
                      gdouble far_plane)
{
    return grl_matrix_from_raylib (MatrixOrtho (left, right, bottom, top,
                                                near_plane, far_plane));
}

/**
 * grl_matrix_new_look_at:
 * @eye: Camera position
 * @target: Target position
 * @up: Up vector
 *
 * Creates a new look-at view matrix.
 *
 * Returns: (transfer full): A new look-at matrix
 */
GrlMatrix *
grl_matrix_new_look_at (const GrlVector3 *eye,
                        const GrlVector3 *target,
                        const GrlVector3 *up)
{
    Vector3 rl_eye, rl_target, rl_up;

    g_return_val_if_fail (eye != NULL, NULL);
    g_return_val_if_fail (target != NULL, NULL);
    g_return_val_if_fail (up != NULL, NULL);

    rl_eye.x = eye->x;
    rl_eye.y = eye->y;
    rl_eye.z = eye->z;

    rl_target.x = target->x;
    rl_target.y = target->y;
    rl_target.z = target->z;

    rl_up.x = up->x;
    rl_up.y = up->y;
    rl_up.z = up->z;

    return grl_matrix_from_raylib (MatrixLookAt (rl_eye, rl_target, rl_up));
}

/**
 * grl_matrix_multiply:
 * @left: Left matrix
 * @right: Right matrix
 *
 * Multiplies two matrices (left * right).
 *
 * Returns: (transfer full): A new matrix with the result
 */
GrlMatrix *
grl_matrix_multiply (const GrlMatrix *left,
                     const GrlMatrix *right)
{
    Matrix rl_left, rl_right;

    g_return_val_if_fail (left != NULL, NULL);
    g_return_val_if_fail (right != NULL, NULL);

    rl_left = grl_matrix_to_raylib (left);
    rl_right = grl_matrix_to_raylib (right);

    return grl_matrix_from_raylib (MatrixMultiply (rl_left, rl_right));
}

/**
 * grl_matrix_invert:
 * @self: A matrix
 *
 * Calculates the inverse of a matrix.
 * If the matrix is singular (determinant is 0), the result is undefined.
 *
 * Returns: (transfer full): A new inverted matrix
 */
GrlMatrix *
grl_matrix_invert (const GrlMatrix *self)
{
    g_return_val_if_fail (self != NULL, NULL);

    return grl_matrix_from_raylib (MatrixInvert (grl_matrix_to_raylib (self)));
}

/**
 * grl_matrix_transpose:
 * @self: A matrix
 *
 * Calculates the transpose of a matrix (rows become columns).
 *
 * Returns: (transfer full): A new transposed matrix
 */
GrlMatrix *
grl_matrix_transpose (const GrlMatrix *self)
{
    g_return_val_if_fail (self != NULL, NULL);

    return grl_matrix_from_raylib (MatrixTranspose (grl_matrix_to_raylib (self)));
}

/**
 * grl_matrix_determinant:
 * @self: A matrix
 *
 * Calculates the determinant of a matrix.
 *
 * Returns: The determinant value
 */
gfloat
grl_matrix_determinant (const GrlMatrix *self)
{
    g_return_val_if_fail (self != NULL, 0.0f);

    return MatrixDeterminant (grl_matrix_to_raylib (self));
}

/**
 * grl_matrix_trace:
 * @self: A matrix
 *
 * Calculates the trace (sum of diagonal elements) of a matrix.
 *
 * Returns: The trace value
 */
gfloat
grl_matrix_trace (const GrlMatrix *self)
{
    g_return_val_if_fail (self != NULL, 0.0f);

    return MatrixTrace (grl_matrix_to_raylib (self));
}

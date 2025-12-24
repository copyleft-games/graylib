/* grl-matrix.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * 4x4 transformation matrix type.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "../grl-version.h"
#include "grl-vector3.h"

G_BEGIN_DECLS

typedef struct _GrlMatrix GrlMatrix;

/**
 * GrlMatrix:
 * @m0: Element at row 0, column 0
 * @m4: Element at row 0, column 1
 * @m8: Element at row 0, column 2
 * @m12: Element at row 0, column 3
 * @m1: Element at row 1, column 0
 * @m5: Element at row 1, column 1
 * @m9: Element at row 1, column 2
 * @m13: Element at row 1, column 3
 * @m2: Element at row 2, column 0
 * @m6: Element at row 2, column 1
 * @m10: Element at row 2, column 2
 * @m14: Element at row 2, column 3
 * @m3: Element at row 3, column 0
 * @m7: Element at row 3, column 1
 * @m11: Element at row 3, column 2
 * @m15: Element at row 3, column 3
 *
 * A 4x4 transformation matrix stored in column-major order (OpenGL style).
 * The naming convention mN matches raylib's Matrix struct layout.
 */
struct _GrlMatrix
{
    gfloat m0, m4, m8, m12;     /* Row 0 */
    gfloat m1, m5, m9, m13;     /* Row 1 */
    gfloat m2, m6, m10, m14;    /* Row 2 */
    gfloat m3, m7, m11, m15;    /* Row 3 */
};

#define GRL_TYPE_MATRIX (grl_matrix_get_type())

GRL_AVAILABLE_IN_ALL
GType           grl_matrix_get_type         (void) G_GNUC_CONST;

/**
 * grl_matrix_new:
 *
 * Creates a new uninitialized matrix.
 *
 * Returns: (transfer full): A newly allocated #GrlMatrix
 */
GRL_AVAILABLE_IN_ALL
GrlMatrix *     grl_matrix_new              (void);

/**
 * grl_matrix_copy:
 * @self: (nullable): A #GrlMatrix
 *
 * Creates a copy of the matrix.
 *
 * Returns: (transfer full) (nullable): A copy, or %NULL
 */
GRL_AVAILABLE_IN_ALL
GrlMatrix *     grl_matrix_copy             (const GrlMatrix    *self);

/**
 * grl_matrix_free:
 * @self: (nullable): A #GrlMatrix
 *
 * Frees a matrix allocated with grl_matrix_new() or grl_matrix_copy().
 */
GRL_AVAILABLE_IN_ALL
void            grl_matrix_free             (GrlMatrix          *self);

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GrlMatrix, grl_matrix_free)

/**
 * grl_matrix_new_identity:
 *
 * Creates a new identity matrix.
 *
 * Returns: (transfer full): A new identity matrix
 */
GRL_AVAILABLE_IN_ALL
GrlMatrix *     grl_matrix_new_identity     (void);

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
GRL_AVAILABLE_IN_ALL
GrlMatrix *     grl_matrix_new_translate    (gfloat              x,
                                             gfloat              y,
                                             gfloat              z);

/**
 * grl_matrix_new_rotate:
 * @axis: The axis to rotate around
 * @angle: The rotation angle in radians
 *
 * Creates a new rotation matrix around the given axis.
 *
 * Returns: (transfer full): A new rotation matrix
 */
GRL_AVAILABLE_IN_ALL
GrlMatrix *     grl_matrix_new_rotate       (const GrlVector3   *axis,
                                             gfloat              angle);

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
GRL_AVAILABLE_IN_ALL
GrlMatrix *     grl_matrix_new_rotate_xyz   (gfloat              x,
                                             gfloat              y,
                                             gfloat              z);

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
GRL_AVAILABLE_IN_ALL
GrlMatrix *     grl_matrix_new_scale        (gfloat              x,
                                             gfloat              y,
                                             gfloat              z);

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
GRL_AVAILABLE_IN_ALL
GrlMatrix *     grl_matrix_new_frustum      (gdouble             left,
                                             gdouble             right,
                                             gdouble             bottom,
                                             gdouble             top,
                                             gdouble             near_plane,
                                             gdouble             far_plane);

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
GRL_AVAILABLE_IN_ALL
GrlMatrix *     grl_matrix_new_perspective  (gdouble             fovy,
                                             gdouble             aspect,
                                             gdouble             near_plane,
                                             gdouble             far_plane);

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
GRL_AVAILABLE_IN_ALL
GrlMatrix *     grl_matrix_new_ortho        (gdouble             left,
                                             gdouble             right,
                                             gdouble             bottom,
                                             gdouble             top,
                                             gdouble             near_plane,
                                             gdouble             far_plane);

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
GRL_AVAILABLE_IN_ALL
GrlMatrix *     grl_matrix_new_look_at      (const GrlVector3   *eye,
                                             const GrlVector3   *target,
                                             const GrlVector3   *up);

/**
 * grl_matrix_multiply:
 * @left: Left matrix
 * @right: Right matrix
 *
 * Multiplies two matrices (left * right).
 *
 * Returns: (transfer full): A new matrix with the result
 */
GRL_AVAILABLE_IN_ALL
GrlMatrix *     grl_matrix_multiply         (const GrlMatrix    *left,
                                             const GrlMatrix    *right);

/**
 * grl_matrix_invert:
 * @self: A matrix
 *
 * Calculates the inverse of a matrix.
 *
 * Returns: (transfer full): A new inverted matrix
 */
GRL_AVAILABLE_IN_ALL
GrlMatrix *     grl_matrix_invert           (const GrlMatrix    *self);

/**
 * grl_matrix_transpose:
 * @self: A matrix
 *
 * Calculates the transpose of a matrix.
 *
 * Returns: (transfer full): A new transposed matrix
 */
GRL_AVAILABLE_IN_ALL
GrlMatrix *     grl_matrix_transpose        (const GrlMatrix    *self);

/**
 * grl_matrix_determinant:
 * @self: A matrix
 *
 * Calculates the determinant of a matrix.
 *
 * Returns: The determinant value
 */
GRL_AVAILABLE_IN_ALL
gfloat          grl_matrix_determinant      (const GrlMatrix    *self);

/**
 * grl_matrix_trace:
 * @self: A matrix
 *
 * Calculates the trace (sum of diagonal elements) of a matrix.
 *
 * Returns: The trace value
 */
GRL_AVAILABLE_IN_ALL
gfloat          grl_matrix_trace            (const GrlMatrix    *self);

G_END_DECLS

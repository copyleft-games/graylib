/* grl-quaternion.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Quaternion type for 3D rotations and orientations.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "../grl-version.h"
#include "grl-vector3.h"

G_BEGIN_DECLS

typedef struct _GrlQuaternion GrlQuaternion;
typedef struct _GrlMatrix     GrlMatrix;

/**
 * GrlQuaternion:
 * @x: X component (imaginary i)
 * @y: Y component (imaginary j)
 * @z: Z component (imaginary k)
 * @w: W component (real/scalar)
 *
 * A quaternion for representing 3D rotations and orientations.
 * Quaternions avoid gimbal lock and provide smooth interpolation
 * for rotations.
 */
struct _GrlQuaternion
{
    gfloat x;
    gfloat y;
    gfloat z;
    gfloat w;
};

#define GRL_TYPE_QUATERNION (grl_quaternion_get_type())

GRL_AVAILABLE_IN_ALL
GType               grl_quaternion_get_type             (void) G_GNUC_CONST;

/*
 * Constructors
 */

/**
 * grl_quaternion_new:
 * @x: X component
 * @y: Y component
 * @z: Z component
 * @w: W component
 *
 * Creates a new quaternion with the given components.
 *
 * Returns: (transfer full): A newly allocated #GrlQuaternion
 */
GRL_AVAILABLE_IN_ALL
GrlQuaternion *     grl_quaternion_new                  (gfloat              x,
                                                         gfloat              y,
                                                         gfloat              z,
                                                         gfloat              w);

/**
 * grl_quaternion_copy:
 * @self: (nullable): A #GrlQuaternion
 *
 * Creates a copy of the quaternion.
 *
 * Returns: (transfer full) (nullable): A copy, or %NULL
 */
GRL_AVAILABLE_IN_ALL
GrlQuaternion *     grl_quaternion_copy                 (const GrlQuaternion *self);

/**
 * grl_quaternion_free:
 * @self: (nullable): A #GrlQuaternion
 *
 * Frees a quaternion allocated with grl_quaternion_new() or grl_quaternion_copy().
 */
GRL_AVAILABLE_IN_ALL
void                grl_quaternion_free                 (GrlQuaternion       *self);

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GrlQuaternion, grl_quaternion_free)

/**
 * grl_quaternion_new_identity:
 *
 * Creates a new identity quaternion (0, 0, 0, 1).
 * The identity quaternion represents no rotation.
 *
 * Returns: (transfer full): A new identity quaternion
 */
GRL_AVAILABLE_IN_ALL
GrlQuaternion *     grl_quaternion_new_identity         (void);

/**
 * grl_quaternion_new_from_axis_angle:
 * @axis: The rotation axis (should be normalized)
 * @angle: The rotation angle in radians
 *
 * Creates a quaternion from an axis-angle representation.
 *
 * Returns: (transfer full): A new quaternion representing the rotation
 */
GRL_AVAILABLE_IN_ALL
GrlQuaternion *     grl_quaternion_new_from_axis_angle  (const GrlVector3    *axis,
                                                         gfloat               angle);

/**
 * grl_quaternion_new_from_euler:
 * @pitch: Rotation around X axis in radians
 * @yaw: Rotation around Y axis in radians
 * @roll: Rotation around Z axis in radians
 *
 * Creates a quaternion from Euler angles.
 *
 * Returns: (transfer full): A new quaternion representing the rotation
 */
GRL_AVAILABLE_IN_ALL
GrlQuaternion *     grl_quaternion_new_from_euler       (gfloat               pitch,
                                                         gfloat               yaw,
                                                         gfloat               roll);

/**
 * grl_quaternion_new_from_matrix:
 * @matrix: A rotation matrix
 *
 * Creates a quaternion from a rotation matrix.
 *
 * Returns: (transfer full): A new quaternion representing the rotation
 */
GRL_AVAILABLE_IN_ALL
GrlQuaternion *     grl_quaternion_new_from_matrix      (const GrlMatrix     *matrix);

/**
 * grl_quaternion_new_from_vectors:
 * @from: Source direction vector
 * @to: Target direction vector
 *
 * Creates a quaternion that rotates @from to @to.
 *
 * Returns: (transfer full): A new quaternion representing the rotation
 */
GRL_AVAILABLE_IN_ALL
GrlQuaternion *     grl_quaternion_new_from_vectors     (const GrlVector3    *from,
                                                         const GrlVector3    *to);

/*
 * Arithmetic Operations
 */

/**
 * grl_quaternion_add:
 * @a: First quaternion
 * @b: Second quaternion
 *
 * Adds two quaternions component-wise.
 *
 * Returns: (transfer full): A new quaternion with the sum
 */
GRL_AVAILABLE_IN_ALL
GrlQuaternion *     grl_quaternion_add                  (const GrlQuaternion *a,
                                                         const GrlQuaternion *b);

/**
 * grl_quaternion_add_value:
 * @self: A quaternion
 * @value: Value to add to each component
 *
 * Adds a scalar value to each component.
 *
 * Returns: (transfer full): A new quaternion with the result
 */
GRL_AVAILABLE_IN_ALL
GrlQuaternion *     grl_quaternion_add_value            (const GrlQuaternion *self,
                                                         gfloat               value);

/**
 * grl_quaternion_subtract:
 * @a: First quaternion
 * @b: Second quaternion
 *
 * Subtracts @b from @a component-wise.
 *
 * Returns: (transfer full): A new quaternion with the difference
 */
GRL_AVAILABLE_IN_ALL
GrlQuaternion *     grl_quaternion_subtract             (const GrlQuaternion *a,
                                                         const GrlQuaternion *b);

/**
 * grl_quaternion_subtract_value:
 * @self: A quaternion
 * @value: Value to subtract from each component
 *
 * Subtracts a scalar value from each component.
 *
 * Returns: (transfer full): A new quaternion with the result
 */
GRL_AVAILABLE_IN_ALL
GrlQuaternion *     grl_quaternion_subtract_value       (const GrlQuaternion *self,
                                                         gfloat               value);

/**
 * grl_quaternion_multiply:
 * @a: First quaternion
 * @b: Second quaternion
 *
 * Multiplies two quaternions. Quaternion multiplication combines rotations.
 * Note: Quaternion multiplication is NOT commutative.
 *
 * Returns: (transfer full): A new quaternion with the product
 */
GRL_AVAILABLE_IN_ALL
GrlQuaternion *     grl_quaternion_multiply             (const GrlQuaternion *a,
                                                         const GrlQuaternion *b);

/**
 * grl_quaternion_scale:
 * @self: A quaternion
 * @scale: Scale factor
 *
 * Scales a quaternion by a scalar value.
 *
 * Returns: (transfer full): A new scaled quaternion
 */
GRL_AVAILABLE_IN_ALL
GrlQuaternion *     grl_quaternion_scale                (const GrlQuaternion *self,
                                                         gfloat               scale);

/**
 * grl_quaternion_divide:
 * @a: First quaternion
 * @b: Second quaternion
 *
 * Divides @a by @b component-wise.
 *
 * Returns: (transfer full): A new quaternion with the quotient
 */
GRL_AVAILABLE_IN_ALL
GrlQuaternion *     grl_quaternion_divide               (const GrlQuaternion *a,
                                                         const GrlQuaternion *b);

/**
 * grl_quaternion_negate:
 * @self: A quaternion
 *
 * Negates all components of the quaternion.
 *
 * Returns: (transfer full): A new negated quaternion
 */
GRL_AVAILABLE_IN_ALL
GrlQuaternion *     grl_quaternion_negate               (const GrlQuaternion *self);

/*
 * Quaternion Operations
 */

/**
 * grl_quaternion_length:
 * @self: A quaternion
 *
 * Calculates the length (magnitude) of the quaternion.
 *
 * Returns: The quaternion length
 */
GRL_AVAILABLE_IN_ALL
gfloat              grl_quaternion_length               (const GrlQuaternion *self);

/**
 * grl_quaternion_normalize:
 * @self: A quaternion
 *
 * Returns a normalized (unit length) quaternion.
 *
 * Returns: (transfer full): A new normalized quaternion
 */
GRL_AVAILABLE_IN_ALL
GrlQuaternion *     grl_quaternion_normalize            (const GrlQuaternion *self);

/**
 * grl_quaternion_invert:
 * @self: A quaternion
 *
 * Returns the inverse of a quaternion.
 *
 * Returns: (transfer full): A new inverted quaternion
 */
GRL_AVAILABLE_IN_ALL
GrlQuaternion *     grl_quaternion_invert               (const GrlQuaternion *self);

/*
 * Interpolation
 */

/**
 * grl_quaternion_lerp:
 * @a: Start quaternion
 * @b: End quaternion
 * @amount: Interpolation amount (0.0 to 1.0)
 *
 * Linearly interpolates between two quaternions.
 * Note: This does not produce unit quaternions. Use grl_quaternion_nlerp()
 * or grl_quaternion_slerp() for rotation interpolation.
 *
 * Returns: (transfer full): A new interpolated quaternion
 */
GRL_AVAILABLE_IN_ALL
GrlQuaternion *     grl_quaternion_lerp                 (const GrlQuaternion *a,
                                                         const GrlQuaternion *b,
                                                         gfloat               amount);

/**
 * grl_quaternion_nlerp:
 * @a: Start quaternion
 * @b: End quaternion
 * @amount: Interpolation amount (0.0 to 1.0)
 *
 * Normalized linear interpolation between two quaternions.
 * Faster than slerp but less accurate for large angles.
 *
 * Returns: (transfer full): A new interpolated quaternion
 */
GRL_AVAILABLE_IN_ALL
GrlQuaternion *     grl_quaternion_nlerp                (const GrlQuaternion *a,
                                                         const GrlQuaternion *b,
                                                         gfloat               amount);

/**
 * grl_quaternion_slerp:
 * @a: Start quaternion
 * @b: End quaternion
 * @amount: Interpolation amount (0.0 to 1.0)
 *
 * Spherical linear interpolation between two quaternions.
 * Provides constant angular velocity but is more expensive than nlerp.
 *
 * Returns: (transfer full): A new interpolated quaternion
 */
GRL_AVAILABLE_IN_ALL
GrlQuaternion *     grl_quaternion_slerp                (const GrlQuaternion *a,
                                                         const GrlQuaternion *b,
                                                         gfloat               amount);

/**
 * grl_quaternion_cubic_hermite_spline:
 * @q1: First control point
 * @out_tangent1: Outgoing tangent at @q1
 * @q2: Second control point
 * @in_tangent2: Incoming tangent at @q2
 * @t: Interpolation parameter (0.0 to 1.0)
 *
 * Performs cubic Hermite spline interpolation between quaternions.
 *
 * Returns: (transfer full): A new interpolated quaternion
 */
GRL_AVAILABLE_IN_ALL
GrlQuaternion *     grl_quaternion_cubic_hermite_spline (const GrlQuaternion *q1,
                                                         const GrlQuaternion *out_tangent1,
                                                         const GrlQuaternion *q2,
                                                         const GrlQuaternion *in_tangent2,
                                                         gfloat               t);

/*
 * Conversions
 */

/**
 * grl_quaternion_to_matrix:
 * @self: A quaternion
 *
 * Converts the quaternion to a rotation matrix.
 *
 * Returns: (transfer full): A new rotation matrix
 */
GRL_AVAILABLE_IN_ALL
GrlMatrix *         grl_quaternion_to_matrix            (const GrlQuaternion *self);

/**
 * grl_quaternion_to_euler:
 * @self: A quaternion
 * @pitch: (out): Rotation around X axis in radians
 * @yaw: (out): Rotation around Y axis in radians
 * @roll: (out): Rotation around Z axis in radians
 *
 * Converts the quaternion to Euler angles.
 */
GRL_AVAILABLE_IN_ALL
void                grl_quaternion_to_euler             (const GrlQuaternion *self,
                                                         gfloat              *pitch,
                                                         gfloat              *yaw,
                                                         gfloat              *roll);

/**
 * grl_quaternion_to_axis_angle:
 * @self: A quaternion
 * @axis: (out): The rotation axis
 * @angle: (out): The rotation angle in radians
 *
 * Converts the quaternion to axis-angle representation.
 */
GRL_AVAILABLE_IN_ALL
void                grl_quaternion_to_axis_angle        (const GrlQuaternion *self,
                                                         GrlVector3          *axis,
                                                         gfloat              *angle);

/*
 * Transformation
 */

/**
 * grl_quaternion_transform:
 * @self: A quaternion
 * @matrix: A transformation matrix
 *
 * Transforms a quaternion by a matrix.
 *
 * Returns: (transfer full): A new transformed quaternion
 */
GRL_AVAILABLE_IN_ALL
GrlQuaternion *     grl_quaternion_transform            (const GrlQuaternion *self,
                                                         const GrlMatrix     *matrix);

/*
 * Comparison
 */

/**
 * grl_quaternion_equal:
 * @a: First quaternion
 * @b: Second quaternion
 *
 * Checks if two quaternions are approximately equal.
 *
 * Returns: %TRUE if approximately equal
 */
GRL_AVAILABLE_IN_ALL
gboolean            grl_quaternion_equal                (const GrlQuaternion *a,
                                                         const GrlQuaternion *b);

G_END_DECLS

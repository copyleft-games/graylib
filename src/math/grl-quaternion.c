/* grl-quaternion.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "grl-quaternion.h"
#include "grl-matrix.h"
#include <math.h>

/**
 * SECTION:grl-quaternion
 * @Title: GrlQuaternion
 * @Short_description: Quaternion type for 3D rotations
 *
 * #GrlQuaternion represents a quaternion for 3D rotations and orientations.
 * Quaternions avoid gimbal lock and provide smooth interpolation for rotations.
 *
 * A quaternion has four components: x, y, z (imaginary/vector part) and w (real/scalar part).
 * The identity quaternion (0, 0, 0, 1) represents no rotation.
 */

G_DEFINE_BOXED_TYPE (GrlQuaternion, grl_quaternion,
                     grl_quaternion_copy, grl_quaternion_free)

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
GrlQuaternion *
grl_quaternion_new (gfloat x,
                    gfloat y,
                    gfloat z,
                    gfloat w)
{
    GrlQuaternion *self;

    self = g_new (GrlQuaternion, 1);
    self->x = x;
    self->y = y;
    self->z = z;
    self->w = w;

    return self;
}

/**
 * grl_quaternion_copy:
 * @self: (nullable): A #GrlQuaternion
 *
 * Creates a copy of the quaternion.
 *
 * Returns: (transfer full) (nullable): A copy, or %NULL
 */
GrlQuaternion *
grl_quaternion_copy (const GrlQuaternion *self)
{
    if (self == NULL)
        return NULL;

    return grl_quaternion_new (self->x, self->y, self->z, self->w);
}

/**
 * grl_quaternion_free:
 * @self: (nullable): A #GrlQuaternion
 *
 * Frees a quaternion.
 */
void
grl_quaternion_free (GrlQuaternion *self)
{
    g_free (self);
}

/**
 * grl_quaternion_new_identity:
 *
 * Creates a new identity quaternion (0, 0, 0, 1).
 *
 * Returns: (transfer full): A new identity quaternion
 */
GrlQuaternion *
grl_quaternion_new_identity (void)
{
    return grl_quaternion_new (0.0f, 0.0f, 0.0f, 1.0f);
}

/**
 * grl_quaternion_new_from_axis_angle:
 * @axis: The rotation axis (should be normalized)
 * @angle: The rotation angle in radians
 *
 * Creates a quaternion from an axis-angle representation.
 *
 * Returns: (transfer full): A new quaternion representing the rotation
 */
GrlQuaternion *
grl_quaternion_new_from_axis_angle (const GrlVector3 *axis,
                                    gfloat            angle)
{
    GrlQuaternion *result;
    gfloat axis_len;
    gfloat half_angle;
    gfloat sin_half;
    gfloat ax, ay, az;
    gfloat len;

    g_return_val_if_fail (axis != NULL, NULL);

    /* Normalize axis */
    axis_len = sqrtf (axis->x * axis->x + axis->y * axis->y + axis->z * axis->z);

    if (axis_len < 0.000001f)
        return grl_quaternion_new_identity ();

    ax = axis->x / axis_len;
    ay = axis->y / axis_len;
    az = axis->z / axis_len;

    half_angle = angle * 0.5f;
    sin_half = sinf (half_angle);

    result = grl_quaternion_new (
        ax * sin_half,
        ay * sin_half,
        az * sin_half,
        cosf (half_angle)
    );

    /* Normalize the result */
    len = sqrtf (result->x * result->x + result->y * result->y +
                 result->z * result->z + result->w * result->w);

    if (len > 0.000001f)
    {
        result->x /= len;
        result->y /= len;
        result->z /= len;
        result->w /= len;
    }

    return result;
}

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
GrlQuaternion *
grl_quaternion_new_from_euler (gfloat pitch,
                               gfloat yaw,
                               gfloat roll)
{
    gfloat x0, x1, y0, y1, z0, z1;

    x0 = cosf (pitch * 0.5f);
    x1 = sinf (pitch * 0.5f);
    y0 = cosf (yaw * 0.5f);
    y1 = sinf (yaw * 0.5f);
    z0 = cosf (roll * 0.5f);
    z1 = sinf (roll * 0.5f);

    return grl_quaternion_new (
        x1 * y0 * z0 - x0 * y1 * z1,
        x0 * y1 * z0 + x1 * y0 * z1,
        x0 * y0 * z1 - x1 * y1 * z0,
        x0 * y0 * z0 + x1 * y1 * z1
    );
}

/**
 * grl_quaternion_new_from_matrix:
 * @matrix: A rotation matrix
 *
 * Creates a quaternion from a rotation matrix.
 *
 * Returns: (transfer full): A new quaternion representing the rotation
 */
GrlQuaternion *
grl_quaternion_new_from_matrix (const GrlMatrix *matrix)
{
    GrlQuaternion *result;
    gfloat four_w_sqr_minus_1;
    gfloat four_x_sqr_minus_1;
    gfloat four_y_sqr_minus_1;
    gfloat four_z_sqr_minus_1;
    gint   biggest_index;
    gfloat four_biggest_sqr_minus_1;
    gfloat biggest_val;
    gfloat mult;

    g_return_val_if_fail (matrix != NULL, NULL);

    /* Algorithm from "Converting a Rotation Matrix to a Quaternion" by Mike Day */
    four_w_sqr_minus_1 = matrix->m0 + matrix->m5 + matrix->m10;
    four_x_sqr_minus_1 = matrix->m0 - matrix->m5 - matrix->m10;
    four_y_sqr_minus_1 = matrix->m5 - matrix->m0 - matrix->m10;
    four_z_sqr_minus_1 = matrix->m10 - matrix->m0 - matrix->m5;

    biggest_index = 0;
    four_biggest_sqr_minus_1 = four_w_sqr_minus_1;

    if (four_x_sqr_minus_1 > four_biggest_sqr_minus_1)
    {
        four_biggest_sqr_minus_1 = four_x_sqr_minus_1;
        biggest_index = 1;
    }

    if (four_y_sqr_minus_1 > four_biggest_sqr_minus_1)
    {
        four_biggest_sqr_minus_1 = four_y_sqr_minus_1;
        biggest_index = 2;
    }

    if (four_z_sqr_minus_1 > four_biggest_sqr_minus_1)
    {
        four_biggest_sqr_minus_1 = four_z_sqr_minus_1;
        biggest_index = 3;
    }

    biggest_val = sqrtf (four_biggest_sqr_minus_1 + 1.0f) * 0.5f;
    mult = 0.25f / biggest_val;

    result = g_new (GrlQuaternion, 1);

    switch (biggest_index)
    {
    case 0:
        result->w = biggest_val;
        result->x = (matrix->m6 - matrix->m9) * mult;
        result->y = (matrix->m8 - matrix->m2) * mult;
        result->z = (matrix->m1 - matrix->m4) * mult;
        break;

    case 1:
        result->x = biggest_val;
        result->w = (matrix->m6 - matrix->m9) * mult;
        result->y = (matrix->m1 + matrix->m4) * mult;
        result->z = (matrix->m8 + matrix->m2) * mult;
        break;

    case 2:
        result->y = biggest_val;
        result->w = (matrix->m8 - matrix->m2) * mult;
        result->x = (matrix->m1 + matrix->m4) * mult;
        result->z = (matrix->m6 + matrix->m9) * mult;
        break;

    case 3:
        result->z = biggest_val;
        result->w = (matrix->m1 - matrix->m4) * mult;
        result->x = (matrix->m8 + matrix->m2) * mult;
        result->y = (matrix->m6 + matrix->m9) * mult;
        break;

    default:
        result->x = 0.0f;
        result->y = 0.0f;
        result->z = 0.0f;
        result->w = 1.0f;
        break;
    }

    return result;
}

/**
 * grl_quaternion_new_from_vectors:
 * @from: Source direction vector
 * @to: Target direction vector
 *
 * Creates a quaternion that rotates @from to @to.
 *
 * Returns: (transfer full): A new quaternion representing the rotation
 */
GrlQuaternion *
grl_quaternion_new_from_vectors (const GrlVector3 *from,
                                 const GrlVector3 *to)
{
    GrlQuaternion *result;
    gfloat cos_2_theta;
    gfloat cross_x, cross_y, cross_z;
    gfloat len;

    g_return_val_if_fail (from != NULL, NULL);
    g_return_val_if_fail (to != NULL, NULL);

    cos_2_theta = from->x * to->x + from->y * to->y + from->z * to->z;

    cross_x = from->y * to->z - from->z * to->y;
    cross_y = from->z * to->x - from->x * to->z;
    cross_z = from->x * to->y - from->y * to->x;

    result = grl_quaternion_new (cross_x, cross_y, cross_z, 1.0f + cos_2_theta);

    /* Normalize */
    len = sqrtf (result->x * result->x + result->y * result->y +
                 result->z * result->z + result->w * result->w);

    if (len > 0.000001f)
    {
        result->x /= len;
        result->y /= len;
        result->z /= len;
        result->w /= len;
    }

    return result;
}

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
GrlQuaternion *
grl_quaternion_add (const GrlQuaternion *a,
                    const GrlQuaternion *b)
{
    g_return_val_if_fail (a != NULL, NULL);
    g_return_val_if_fail (b != NULL, NULL);

    return grl_quaternion_new (a->x + b->x, a->y + b->y, a->z + b->z, a->w + b->w);
}

/**
 * grl_quaternion_add_value:
 * @self: A quaternion
 * @value: Value to add to each component
 *
 * Adds a scalar value to each component.
 *
 * Returns: (transfer full): A new quaternion with the result
 */
GrlQuaternion *
grl_quaternion_add_value (const GrlQuaternion *self,
                          gfloat               value)
{
    g_return_val_if_fail (self != NULL, NULL);

    return grl_quaternion_new (self->x + value, self->y + value,
                               self->z + value, self->w + value);
}

/**
 * grl_quaternion_subtract:
 * @a: First quaternion
 * @b: Second quaternion
 *
 * Subtracts @b from @a component-wise.
 *
 * Returns: (transfer full): A new quaternion with the difference
 */
GrlQuaternion *
grl_quaternion_subtract (const GrlQuaternion *a,
                         const GrlQuaternion *b)
{
    g_return_val_if_fail (a != NULL, NULL);
    g_return_val_if_fail (b != NULL, NULL);

    return grl_quaternion_new (a->x - b->x, a->y - b->y, a->z - b->z, a->w - b->w);
}

/**
 * grl_quaternion_subtract_value:
 * @self: A quaternion
 * @value: Value to subtract from each component
 *
 * Subtracts a scalar value from each component.
 *
 * Returns: (transfer full): A new quaternion with the result
 */
GrlQuaternion *
grl_quaternion_subtract_value (const GrlQuaternion *self,
                               gfloat               value)
{
    g_return_val_if_fail (self != NULL, NULL);

    return grl_quaternion_new (self->x - value, self->y - value,
                               self->z - value, self->w - value);
}

/**
 * grl_quaternion_multiply:
 * @a: First quaternion
 * @b: Second quaternion
 *
 * Multiplies two quaternions. Quaternion multiplication combines rotations.
 *
 * Returns: (transfer full): A new quaternion with the product
 */
GrlQuaternion *
grl_quaternion_multiply (const GrlQuaternion *a,
                         const GrlQuaternion *b)
{
    g_return_val_if_fail (a != NULL, NULL);
    g_return_val_if_fail (b != NULL, NULL);

    return grl_quaternion_new (
        a->x * b->w + a->w * b->x + a->y * b->z - a->z * b->y,
        a->y * b->w + a->w * b->y + a->z * b->x - a->x * b->z,
        a->z * b->w + a->w * b->z + a->x * b->y - a->y * b->x,
        a->w * b->w - a->x * b->x - a->y * b->y - a->z * b->z
    );
}

/**
 * grl_quaternion_scale:
 * @self: A quaternion
 * @scale: Scale factor
 *
 * Scales a quaternion by a scalar value.
 *
 * Returns: (transfer full): A new scaled quaternion
 */
GrlQuaternion *
grl_quaternion_scale (const GrlQuaternion *self,
                      gfloat               scale)
{
    g_return_val_if_fail (self != NULL, NULL);

    return grl_quaternion_new (self->x * scale, self->y * scale,
                               self->z * scale, self->w * scale);
}

/**
 * grl_quaternion_divide:
 * @a: First quaternion
 * @b: Second quaternion
 *
 * Divides @a by @b component-wise.
 *
 * Returns: (transfer full): A new quaternion with the quotient
 */
GrlQuaternion *
grl_quaternion_divide (const GrlQuaternion *a,
                       const GrlQuaternion *b)
{
    g_return_val_if_fail (a != NULL, NULL);
    g_return_val_if_fail (b != NULL, NULL);

    return grl_quaternion_new (a->x / b->x, a->y / b->y, a->z / b->z, a->w / b->w);
}

/**
 * grl_quaternion_negate:
 * @self: A quaternion
 *
 * Negates all components of the quaternion.
 *
 * Returns: (transfer full): A new negated quaternion
 */
GrlQuaternion *
grl_quaternion_negate (const GrlQuaternion *self)
{
    g_return_val_if_fail (self != NULL, NULL);

    return grl_quaternion_new (-self->x, -self->y, -self->z, -self->w);
}

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
gfloat
grl_quaternion_length (const GrlQuaternion *self)
{
    g_return_val_if_fail (self != NULL, 0.0f);

    return sqrtf (self->x * self->x + self->y * self->y +
                  self->z * self->z + self->w * self->w);
}

/**
 * grl_quaternion_normalize:
 * @self: A quaternion
 *
 * Returns a normalized (unit length) quaternion.
 *
 * Returns: (transfer full): A new normalized quaternion
 */
GrlQuaternion *
grl_quaternion_normalize (const GrlQuaternion *self)
{
    GrlQuaternion *result;
    gfloat len;

    g_return_val_if_fail (self != NULL, NULL);

    len = sqrtf (self->x * self->x + self->y * self->y +
                 self->z * self->z + self->w * self->w);

    if (len < 0.000001f)
        return grl_quaternion_new_identity ();

    result = grl_quaternion_new (self->x / len, self->y / len,
                                 self->z / len, self->w / len);

    return result;
}

/**
 * grl_quaternion_invert:
 * @self: A quaternion
 *
 * Returns the inverse of a quaternion.
 *
 * Returns: (transfer full): A new inverted quaternion
 */
GrlQuaternion *
grl_quaternion_invert (const GrlQuaternion *self)
{
    GrlQuaternion *result;
    gfloat len_sqr;

    g_return_val_if_fail (self != NULL, NULL);

    len_sqr = self->x * self->x + self->y * self->y +
              self->z * self->z + self->w * self->w;

    if (len_sqr < 0.000001f)
        return grl_quaternion_new_identity ();

    result = grl_quaternion_new (
        -self->x / len_sqr,
        -self->y / len_sqr,
        -self->z / len_sqr,
        self->w / len_sqr
    );

    return result;
}

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
 *
 * Returns: (transfer full): A new interpolated quaternion
 */
GrlQuaternion *
grl_quaternion_lerp (const GrlQuaternion *a,
                     const GrlQuaternion *b,
                     gfloat               amount)
{
    g_return_val_if_fail (a != NULL, NULL);
    g_return_val_if_fail (b != NULL, NULL);

    return grl_quaternion_new (
        a->x + amount * (b->x - a->x),
        a->y + amount * (b->y - a->y),
        a->z + amount * (b->z - a->z),
        a->w + amount * (b->w - a->w)
    );
}

/**
 * grl_quaternion_nlerp:
 * @a: Start quaternion
 * @b: End quaternion
 * @amount: Interpolation amount (0.0 to 1.0)
 *
 * Normalized linear interpolation between two quaternions.
 *
 * Returns: (transfer full): A new interpolated quaternion
 */
GrlQuaternion *
grl_quaternion_nlerp (const GrlQuaternion *a,
                      const GrlQuaternion *b,
                      gfloat               amount)
{
    g_autoptr(GrlQuaternion) lerped = NULL;

    g_return_val_if_fail (a != NULL, NULL);
    g_return_val_if_fail (b != NULL, NULL);

    lerped = grl_quaternion_lerp (a, b, amount);

    return grl_quaternion_normalize (lerped);
}

/**
 * grl_quaternion_slerp:
 * @a: Start quaternion
 * @b: End quaternion
 * @amount: Interpolation amount (0.0 to 1.0)
 *
 * Spherical linear interpolation between two quaternions.
 *
 * Returns: (transfer full): A new interpolated quaternion
 */
GrlQuaternion *
grl_quaternion_slerp (const GrlQuaternion *a,
                      const GrlQuaternion *b,
                      gfloat               amount)
{
    GrlQuaternion *result;
    GrlQuaternion b_copy;
    gfloat cos_half_theta;
    gfloat half_theta;
    gfloat sin_half_theta;
    gfloat ratio_a;
    gfloat ratio_b;

    g_return_val_if_fail (a != NULL, NULL);
    g_return_val_if_fail (b != NULL, NULL);

    b_copy.x = b->x;
    b_copy.y = b->y;
    b_copy.z = b->z;
    b_copy.w = b->w;

    cos_half_theta = a->x * b->x + a->y * b->y + a->z * b->z + a->w * b->w;

    /* Take shortest path */
    if (cos_half_theta < 0.0f)
    {
        b_copy.x = -b_copy.x;
        b_copy.y = -b_copy.y;
        b_copy.z = -b_copy.z;
        b_copy.w = -b_copy.w;
        cos_half_theta = -cos_half_theta;
    }

    /* If quaternions are very close, use linear interpolation */
    if (cos_half_theta > 0.95f)
        return grl_quaternion_nlerp (a, &b_copy, amount);

    half_theta = acosf (cos_half_theta);
    sin_half_theta = sqrtf (1.0f - cos_half_theta * cos_half_theta);

    /* Avoid division by zero */
    if (fabsf (sin_half_theta) < 0.001f)
    {
        return grl_quaternion_new (
            (a->x * 0.5f + b_copy.x * 0.5f),
            (a->y * 0.5f + b_copy.y * 0.5f),
            (a->z * 0.5f + b_copy.z * 0.5f),
            (a->w * 0.5f + b_copy.w * 0.5f)
        );
    }

    ratio_a = sinf ((1.0f - amount) * half_theta) / sin_half_theta;
    ratio_b = sinf (amount * half_theta) / sin_half_theta;

    result = grl_quaternion_new (
        a->x * ratio_a + b_copy.x * ratio_b,
        a->y * ratio_a + b_copy.y * ratio_b,
        a->z * ratio_a + b_copy.z * ratio_b,
        a->w * ratio_a + b_copy.w * ratio_b
    );

    return result;
}

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
GrlQuaternion *
grl_quaternion_cubic_hermite_spline (const GrlQuaternion *q1,
                                     const GrlQuaternion *out_tangent1,
                                     const GrlQuaternion *q2,
                                     const GrlQuaternion *in_tangent2,
                                     gfloat               t)
{
    g_autoptr(GrlQuaternion) p0 = NULL;
    g_autoptr(GrlQuaternion) m0 = NULL;
    g_autoptr(GrlQuaternion) p1 = NULL;
    g_autoptr(GrlQuaternion) m1 = NULL;
    g_autoptr(GrlQuaternion) temp1 = NULL;
    g_autoptr(GrlQuaternion) temp2 = NULL;
    g_autoptr(GrlQuaternion) temp3 = NULL;
    gfloat t2;
    gfloat t3;
    gfloat h00, h10, h01, h11;

    g_return_val_if_fail (q1 != NULL, NULL);
    g_return_val_if_fail (out_tangent1 != NULL, NULL);
    g_return_val_if_fail (q2 != NULL, NULL);
    g_return_val_if_fail (in_tangent2 != NULL, NULL);

    t2 = t * t;
    t3 = t2 * t;

    /* Hermite basis functions */
    h00 = 2.0f * t3 - 3.0f * t2 + 1.0f;
    h10 = t3 - 2.0f * t2 + t;
    h01 = -2.0f * t3 + 3.0f * t2;
    h11 = t3 - t2;

    p0 = grl_quaternion_scale (q1, h00);
    m0 = grl_quaternion_scale (out_tangent1, h10);
    p1 = grl_quaternion_scale (q2, h01);
    m1 = grl_quaternion_scale (in_tangent2, h11);

    temp1 = grl_quaternion_add (p0, m0);
    temp2 = grl_quaternion_add (temp1, p1);
    temp3 = grl_quaternion_add (temp2, m1);

    return grl_quaternion_normalize (temp3);
}

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
GrlMatrix *
grl_quaternion_to_matrix (const GrlQuaternion *self)
{
    GrlMatrix *result;
    gfloat a2, b2, c2, d2;
    gfloat ab, ac, ad, bc, bd, cd;

    g_return_val_if_fail (self != NULL, NULL);

    result = grl_matrix_new ();

    a2 = self->w * self->w;
    b2 = self->x * self->x;
    c2 = self->y * self->y;
    d2 = self->z * self->z;

    ab = self->w * self->x;
    ac = self->w * self->y;
    ad = self->w * self->z;
    bc = self->x * self->y;
    bd = self->x * self->z;
    cd = self->y * self->z;

    result->m0 = a2 + b2 - c2 - d2;
    result->m1 = 2.0f * (bc + ad);
    result->m2 = 2.0f * (bd - ac);
    result->m3 = 0.0f;

    result->m4 = 2.0f * (bc - ad);
    result->m5 = a2 - b2 + c2 - d2;
    result->m6 = 2.0f * (cd + ab);
    result->m7 = 0.0f;

    result->m8 = 2.0f * (bd + ac);
    result->m9 = 2.0f * (cd - ab);
    result->m10 = a2 - b2 - c2 + d2;
    result->m11 = 0.0f;

    result->m12 = 0.0f;
    result->m13 = 0.0f;
    result->m14 = 0.0f;
    result->m15 = 1.0f;

    return result;
}

/**
 * grl_quaternion_to_euler:
 * @self: A quaternion
 * @pitch: (out): Rotation around X axis in radians
 * @yaw: (out): Rotation around Y axis in radians
 * @roll: (out): Rotation around Z axis in radians
 *
 * Converts the quaternion to Euler angles.
 */
void
grl_quaternion_to_euler (const GrlQuaternion *self,
                         gfloat              *pitch,
                         gfloat              *yaw,
                         gfloat              *roll)
{
    gfloat x0, x1, y0, z0, z1;

    g_return_if_fail (self != NULL);
    g_return_if_fail (pitch != NULL);
    g_return_if_fail (yaw != NULL);
    g_return_if_fail (roll != NULL);

    /* Roll (x-axis rotation) */
    x0 = 2.0f * (self->w * self->x + self->y * self->z);
    x1 = 1.0f - 2.0f * (self->x * self->x + self->y * self->y);
    *pitch = atan2f (x0, x1);

    /* Pitch (y-axis rotation) */
    y0 = 2.0f * (self->w * self->y - self->z * self->x);
    y0 = y0 > 1.0f ? 1.0f : y0;
    y0 = y0 < -1.0f ? -1.0f : y0;
    *yaw = asinf (y0);

    /* Yaw (z-axis rotation) */
    z0 = 2.0f * (self->w * self->z + self->x * self->y);
    z1 = 1.0f - 2.0f * (self->y * self->y + self->z * self->z);
    *roll = atan2f (z0, z1);
}

/**
 * grl_quaternion_to_axis_angle:
 * @self: A quaternion
 * @axis: (out): The rotation axis
 * @angle: (out): The rotation angle in radians
 *
 * Converts the quaternion to axis-angle representation.
 */
void
grl_quaternion_to_axis_angle (const GrlQuaternion *self,
                              GrlVector3          *axis,
                              gfloat              *angle)
{
    GrlQuaternion q;
    gfloat res_angle;
    gfloat den;

    g_return_if_fail (self != NULL);
    g_return_if_fail (axis != NULL);
    g_return_if_fail (angle != NULL);

    q = *self;

    /* Normalize if needed */
    if (fabsf (q.w) > 1.0f)
    {
        gfloat len = sqrtf (q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
        if (len > 0.000001f)
        {
            q.x /= len;
            q.y /= len;
            q.z /= len;
            q.w /= len;
        }
    }

    res_angle = 2.0f * acosf (q.w);
    den = sqrtf (1.0f - q.w * q.w);

    if (den > 0.0001f)
    {
        axis->x = q.x / den;
        axis->y = q.y / den;
        axis->z = q.z / den;
    }
    else
    {
        /* Arbitrary axis for zero rotation */
        axis->x = 1.0f;
        axis->y = 0.0f;
        axis->z = 0.0f;
    }

    *angle = res_angle;
}

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
GrlQuaternion *
grl_quaternion_transform (const GrlQuaternion *self,
                          const GrlMatrix     *matrix)
{
    GrlQuaternion *result;

    g_return_val_if_fail (self != NULL, NULL);
    g_return_val_if_fail (matrix != NULL, NULL);

    result = grl_quaternion_new (
        matrix->m0 * self->x + matrix->m4 * self->y + matrix->m8 * self->z + matrix->m12 * self->w,
        matrix->m1 * self->x + matrix->m5 * self->y + matrix->m9 * self->z + matrix->m13 * self->w,
        matrix->m2 * self->x + matrix->m6 * self->y + matrix->m10 * self->z + matrix->m14 * self->w,
        matrix->m3 * self->x + matrix->m7 * self->y + matrix->m11 * self->z + matrix->m15 * self->w
    );

    return result;
}

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
gboolean
grl_quaternion_equal (const GrlQuaternion *a,
                      const GrlQuaternion *b)
{
    gfloat epsilon;
    gboolean same;
    gboolean opposite;

    if (a == b)
        return TRUE;

    if (a == NULL || b == NULL)
        return FALSE;

    epsilon = 0.000001f;

    /* Quaternions q and -q represent the same rotation */
    same = (fabsf (a->x - b->x) <= epsilon &&
            fabsf (a->y - b->y) <= epsilon &&
            fabsf (a->z - b->z) <= epsilon &&
            fabsf (a->w - b->w) <= epsilon);

    opposite = (fabsf (a->x + b->x) <= epsilon &&
                fabsf (a->y + b->y) <= epsilon &&
                fabsf (a->z + b->z) <= epsilon &&
                fabsf (a->w + b->w) <= epsilon);

    return same || opposite;
}

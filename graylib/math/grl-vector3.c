/* grl-vector3.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "grl-vector3.h"
#include <math.h>

/**
 * SECTION:grl-vector3
 * @Title: GrlVector3
 * @Short_description: 3D vector type
 *
 * #GrlVector3 represents a 3D vector with x, y, and z components.
 * It is used for positions, directions, velocities, and 3D math operations.
 */

G_DEFINE_BOXED_TYPE (GrlVector3, grl_vector3,
                     grl_vector3_copy, grl_vector3_free)

/**
 * grl_vector3_new:
 * @x: X component
 * @y: Y component
 * @z: Z component
 *
 * Creates a new 3D vector.
 *
 * Returns: (transfer full): A newly allocated #GrlVector3
 */
GrlVector3 *
grl_vector3_new (gfloat x,
                 gfloat y,
                 gfloat z)
{
    GrlVector3 *self;

    self = g_new (GrlVector3, 1);
    self->x = x;
    self->y = y;
    self->z = z;

    return self;
}

/**
 * grl_vector3_copy:
 * @self: (nullable): A #GrlVector3
 *
 * Creates a copy of the vector.
 *
 * Returns: (transfer full) (nullable): A copy, or %NULL
 */
GrlVector3 *
grl_vector3_copy (const GrlVector3 *self)
{
    if (self == NULL)
        return NULL;

    return grl_vector3_new (self->x, self->y, self->z);
}

/**
 * grl_vector3_free:
 * @self: (nullable): A #GrlVector3
 *
 * Frees a vector.
 */
void
grl_vector3_free (GrlVector3 *self)
{
    g_free (self);
}

/**
 * grl_vector3_new_zero:
 *
 * Creates a new zero vector.
 *
 * Returns: (transfer full): A new zero vector
 */
GrlVector3 *
grl_vector3_new_zero (void)
{
    return grl_vector3_new (0.0f, 0.0f, 0.0f);
}

/**
 * grl_vector3_new_one:
 *
 * Creates a new (1, 1, 1) vector.
 *
 * Returns: (transfer full): A new one vector
 */
GrlVector3 *
grl_vector3_new_one (void)
{
    return grl_vector3_new (1.0f, 1.0f, 1.0f);
}

/**
 * grl_vector3_new_up:
 *
 * Creates a new up vector (0, 1, 0).
 *
 * Returns: (transfer full): A new up vector
 */
GrlVector3 *
grl_vector3_new_up (void)
{
    return grl_vector3_new (0.0f, 1.0f, 0.0f);
}

/**
 * grl_vector3_new_forward:
 *
 * Creates a new forward vector (0, 0, -1).
 *
 * Returns: (transfer full): A new forward vector
 */
GrlVector3 *
grl_vector3_new_forward (void)
{
    return grl_vector3_new (0.0f, 0.0f, -1.0f);
}

/**
 * grl_vector3_new_right:
 *
 * Creates a new right vector (1, 0, 0).
 *
 * Returns: (transfer full): A new right vector
 */
GrlVector3 *
grl_vector3_new_right (void)
{
    return grl_vector3_new (1.0f, 0.0f, 0.0f);
}

/**
 * grl_vector3_add:
 * @a: First vector
 * @b: Second vector
 *
 * Adds two vectors.
 *
 * Returns: (transfer full): A new vector with the sum
 */
GrlVector3 *
grl_vector3_add (const GrlVector3 *a,
                 const GrlVector3 *b)
{
    g_return_val_if_fail (a != NULL, NULL);
    g_return_val_if_fail (b != NULL, NULL);

    return grl_vector3_new (a->x + b->x, a->y + b->y, a->z + b->z);
}

/**
 * grl_vector3_subtract:
 * @a: First vector
 * @b: Second vector
 *
 * Subtracts @b from @a.
 *
 * Returns: (transfer full): A new vector with the difference
 */
GrlVector3 *
grl_vector3_subtract (const GrlVector3 *a,
                      const GrlVector3 *b)
{
    g_return_val_if_fail (a != NULL, NULL);
    g_return_val_if_fail (b != NULL, NULL);

    return grl_vector3_new (a->x - b->x, a->y - b->y, a->z - b->z);
}

/**
 * grl_vector3_scale:
 * @self: A vector
 * @scale: Scale factor
 *
 * Scales a vector.
 *
 * Returns: (transfer full): A new scaled vector
 */
GrlVector3 *
grl_vector3_scale (const GrlVector3 *self,
                   gfloat            scale)
{
    g_return_val_if_fail (self != NULL, NULL);

    return grl_vector3_new (self->x * scale, self->y * scale, self->z * scale);
}

/**
 * grl_vector3_length:
 * @self: A vector
 *
 * Calculates the length of the vector.
 *
 * Returns: The vector length
 */
gfloat
grl_vector3_length (const GrlVector3 *self)
{
    g_return_val_if_fail (self != NULL, 0.0f);

    return sqrtf (self->x * self->x + self->y * self->y + self->z * self->z);
}

/**
 * grl_vector3_length_sqr:
 * @self: A vector
 *
 * Calculates the squared length of the vector.
 *
 * Returns: The squared vector length
 */
gfloat
grl_vector3_length_sqr (const GrlVector3 *self)
{
    g_return_val_if_fail (self != NULL, 0.0f);

    return self->x * self->x + self->y * self->y + self->z * self->z;
}

/**
 * grl_vector3_normalize:
 * @self: A vector
 *
 * Returns a normalized version of the vector.
 *
 * Returns: (transfer full): A new normalized vector
 */
GrlVector3 *
grl_vector3_normalize (const GrlVector3 *self)
{
    gfloat len;

    g_return_val_if_fail (self != NULL, NULL);

    len = grl_vector3_length (self);

    if (len == 0.0f)
        return grl_vector3_new_zero ();

    return grl_vector3_new (self->x / len, self->y / len, self->z / len);
}

/**
 * grl_vector3_dot:
 * @a: First vector
 * @b: Second vector
 *
 * Calculates the dot product.
 *
 * Returns: The dot product
 */
gfloat
grl_vector3_dot (const GrlVector3 *a,
                 const GrlVector3 *b)
{
    g_return_val_if_fail (a != NULL, 0.0f);
    g_return_val_if_fail (b != NULL, 0.0f);

    return a->x * b->x + a->y * b->y + a->z * b->z;
}

/**
 * grl_vector3_cross:
 * @a: First vector
 * @b: Second vector
 *
 * Calculates the cross product.
 *
 * Returns: (transfer full): A new vector with the cross product
 */
GrlVector3 *
grl_vector3_cross (const GrlVector3 *a,
                   const GrlVector3 *b)
{
    g_return_val_if_fail (a != NULL, NULL);
    g_return_val_if_fail (b != NULL, NULL);

    return grl_vector3_new (
        a->y * b->z - a->z * b->y,
        a->z * b->x - a->x * b->z,
        a->x * b->y - a->y * b->x
    );
}

/**
 * grl_vector3_distance:
 * @a: First vector
 * @b: Second vector
 *
 * Calculates the distance between two points.
 *
 * Returns: The distance
 */
gfloat
grl_vector3_distance (const GrlVector3 *a,
                      const GrlVector3 *b)
{
    gfloat dx, dy, dz;

    g_return_val_if_fail (a != NULL, 0.0f);
    g_return_val_if_fail (b != NULL, 0.0f);

    dx = b->x - a->x;
    dy = b->y - a->y;
    dz = b->z - a->z;

    return sqrtf (dx * dx + dy * dy + dz * dz);
}

/**
 * grl_vector3_lerp:
 * @a: Start vector
 * @b: End vector
 * @amount: Interpolation amount (0.0 to 1.0)
 *
 * Linearly interpolates between two vectors.
 *
 * Returns: (transfer full): A new interpolated vector
 */
GrlVector3 *
grl_vector3_lerp (const GrlVector3 *a,
                  const GrlVector3 *b,
                  gfloat            amount)
{
    g_return_val_if_fail (a != NULL, NULL);
    g_return_val_if_fail (b != NULL, NULL);

    return grl_vector3_new (
        a->x + amount * (b->x - a->x),
        a->y + amount * (b->y - a->y),
        a->z + amount * (b->z - a->z)
    );
}

/**
 * grl_vector3_negate:
 * @self: A vector
 *
 * Returns the negation of the vector.
 *
 * Returns: (transfer full): A new negated vector
 */
GrlVector3 *
grl_vector3_negate (const GrlVector3 *self)
{
    g_return_val_if_fail (self != NULL, NULL);

    return grl_vector3_new (-self->x, -self->y, -self->z);
}

/**
 * grl_vector3_equal:
 * @a: First vector
 * @b: Second vector
 *
 * Checks if two vectors are exactly equal.
 *
 * Returns: %TRUE if equal
 */
gboolean
grl_vector3_equal (const GrlVector3 *a,
                   const GrlVector3 *b)
{
    if (a == b)
        return TRUE;

    if (a == NULL || b == NULL)
        return FALSE;

    return (a->x == b->x && a->y == b->y && a->z == b->z);
}

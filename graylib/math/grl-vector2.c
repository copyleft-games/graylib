/* grl-vector2.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Implementation of 2D vector type.
 */

#include "grl-vector2.h"
#include <math.h>

/**
 * SECTION:grl-vector2
 * @Title: GrlVector2
 * @Short_description: A 2D vector type
 *
 * #GrlVector2 represents a 2D vector with x and y components.
 * It is used throughout Graylib for positions, velocities,
 * directions, and other 2-component values.
 *
 * This is a GBoxed type that supports automatic memory management
 * with g_autoptr().
 *
 * # Example
 *
 * |[<!-- language="C" -->
 * g_autoptr(GrlVector2) pos = grl_vector2_new (100.0f, 200.0f);
 * g_autoptr(GrlVector2) vel = grl_vector2_new (5.0f, 0.0f);
 * g_autoptr(GrlVector2) new_pos = grl_vector2_add (pos, vel);
 *
 * gfloat speed = grl_vector2_length (vel);
 * g_print ("Speed: %f\n", speed);
 * ]|
 */

G_DEFINE_BOXED_TYPE (GrlVector2, grl_vector2,
                     grl_vector2_copy, grl_vector2_free)

/**
 * grl_vector2_new:
 * @x: X component
 * @y: Y component
 *
 * Creates a new vector with the given components.
 *
 * Returns: (transfer full): A newly allocated #GrlVector2
 */
GrlVector2 *
grl_vector2_new (gfloat x,
                 gfloat y)
{
    GrlVector2 *self;

    self = g_new (GrlVector2, 1);
    self->x = x;
    self->y = y;

    return self;
}

/**
 * grl_vector2_new_zero:
 *
 * Creates a new zero vector (0, 0).
 *
 * Returns: (transfer full): A newly allocated #GrlVector2
 */
GrlVector2 *
grl_vector2_new_zero (void)
{
    return grl_vector2_new (0.0f, 0.0f);
}

/**
 * grl_vector2_new_one:
 *
 * Creates a new unit vector (1, 1).
 *
 * Returns: (transfer full): A newly allocated #GrlVector2
 */
GrlVector2 *
grl_vector2_new_one (void)
{
    return grl_vector2_new (1.0f, 1.0f);
}

/**
 * grl_vector2_copy:
 * @self: (nullable): A #GrlVector2
 *
 * Creates a copy of the vector.
 *
 * Returns: (transfer full) (nullable): A copy of @self, or %NULL
 */
GrlVector2 *
grl_vector2_copy (const GrlVector2 *self)
{
    if (self == NULL)
        return NULL;

    return grl_vector2_new (self->x, self->y);
}

/**
 * grl_vector2_free:
 * @self: (nullable): A #GrlVector2
 *
 * Frees a vector allocated with grl_vector2_new() or grl_vector2_copy().
 */
void
grl_vector2_free (GrlVector2 *self)
{
    g_free (self);
}

/*
 * Accessors
 */

/**
 * grl_vector2_get_x:
 * @self: A #GrlVector2
 *
 * Gets the X component of the vector.
 *
 * Returns: The X component
 */
gfloat
grl_vector2_get_x (const GrlVector2 *self)
{
    g_return_val_if_fail (self != NULL, 0.0f);
    return self->x;
}

/**
 * grl_vector2_get_y:
 * @self: A #GrlVector2
 *
 * Gets the Y component of the vector.
 *
 * Returns: The Y component
 */
gfloat
grl_vector2_get_y (const GrlVector2 *self)
{
    g_return_val_if_fail (self != NULL, 0.0f);
    return self->y;
}

/**
 * grl_vector2_set_x:
 * @self: A #GrlVector2
 * @x: The new X component
 *
 * Sets the X component of the vector.
 */
void
grl_vector2_set_x (GrlVector2 *self,
                   gfloat      x)
{
    g_return_if_fail (self != NULL);
    self->x = x;
}

/**
 * grl_vector2_set_y:
 * @self: A #GrlVector2
 * @y: The new Y component
 *
 * Sets the Y component of the vector.
 */
void
grl_vector2_set_y (GrlVector2 *self,
                   gfloat      y)
{
    g_return_if_fail (self != NULL);
    self->y = y;
}

/**
 * grl_vector2_set:
 * @self: A #GrlVector2
 * @x: The new X component
 * @y: The new Y component
 *
 * Sets both components of the vector.
 */
void
grl_vector2_set (GrlVector2 *self,
                 gfloat      x,
                 gfloat      y)
{
    g_return_if_fail (self != NULL);
    self->x = x;
    self->y = y;
}

/*
 * Operations
 */

/**
 * grl_vector2_add:
 * @a: First vector
 * @b: Second vector
 *
 * Adds two vectors component-wise.
 *
 * Returns: (transfer full): A new vector containing the sum
 */
GrlVector2 *
grl_vector2_add (const GrlVector2 *a,
                 const GrlVector2 *b)
{
    g_return_val_if_fail (a != NULL, NULL);
    g_return_val_if_fail (b != NULL, NULL);

    return grl_vector2_new (a->x + b->x, a->y + b->y);
}

/**
 * grl_vector2_subtract:
 * @a: First vector
 * @b: Second vector
 *
 * Subtracts @b from @a component-wise.
 *
 * Returns: (transfer full): A new vector containing the difference
 */
GrlVector2 *
grl_vector2_subtract (const GrlVector2 *a,
                      const GrlVector2 *b)
{
    g_return_val_if_fail (a != NULL, NULL);
    g_return_val_if_fail (b != NULL, NULL);

    return grl_vector2_new (a->x - b->x, a->y - b->y);
}

/**
 * grl_vector2_scale:
 * @self: A vector
 * @scalar: Scale factor
 *
 * Multiplies the vector by a scalar.
 *
 * Returns: (transfer full): A new scaled vector
 */
GrlVector2 *
grl_vector2_scale (const GrlVector2 *self,
                   gfloat            scalar)
{
    g_return_val_if_fail (self != NULL, NULL);

    return grl_vector2_new (self->x * scalar, self->y * scalar);
}

/**
 * grl_vector2_multiply:
 * @a: First vector
 * @b: Second vector
 *
 * Multiplies two vectors component-wise (Hadamard product).
 *
 * Returns: (transfer full): A new vector containing the product
 */
GrlVector2 *
grl_vector2_multiply (const GrlVector2 *a,
                      const GrlVector2 *b)
{
    g_return_val_if_fail (a != NULL, NULL);
    g_return_val_if_fail (b != NULL, NULL);

    return grl_vector2_new (a->x * b->x, a->y * b->y);
}

/**
 * grl_vector2_divide:
 * @a: First vector
 * @b: Second vector
 *
 * Divides @a by @b component-wise.
 *
 * Returns: (transfer full): A new vector containing the quotient
 */
GrlVector2 *
grl_vector2_divide (const GrlVector2 *a,
                    const GrlVector2 *b)
{
    g_return_val_if_fail (a != NULL, NULL);
    g_return_val_if_fail (b != NULL, NULL);

    return grl_vector2_new (a->x / b->x, a->y / b->y);
}

/**
 * grl_vector2_negate:
 * @self: A vector
 *
 * Negates the vector.
 *
 * Returns: (transfer full): A new negated vector
 */
GrlVector2 *
grl_vector2_negate (const GrlVector2 *self)
{
    g_return_val_if_fail (self != NULL, NULL);

    return grl_vector2_new (-self->x, -self->y);
}

/**
 * grl_vector2_normalize:
 * @self: A vector
 *
 * Normalizes the vector to unit length.
 *
 * Returns: (transfer full): A new unit vector in the same direction
 */
GrlVector2 *
grl_vector2_normalize (const GrlVector2 *self)
{
    gfloat length;
    gfloat inv_length;

    g_return_val_if_fail (self != NULL, NULL);

    length = grl_vector2_length (self);
    if (length == 0.0f)
        return grl_vector2_new_zero ();

    inv_length = 1.0f / length;
    return grl_vector2_new (self->x * inv_length, self->y * inv_length);
}

/**
 * grl_vector2_lerp:
 * @a: Start vector
 * @b: End vector
 * @t: Interpolation factor (0.0 to 1.0)
 *
 * Linearly interpolates between two vectors.
 *
 * Returns: (transfer full): A new interpolated vector
 */
GrlVector2 *
grl_vector2_lerp (const GrlVector2 *a,
                  const GrlVector2 *b,
                  gfloat            t)
{
    g_return_val_if_fail (a != NULL, NULL);
    g_return_val_if_fail (b != NULL, NULL);

    return grl_vector2_new (
        a->x + t * (b->x - a->x),
        a->y + t * (b->y - a->y)
    );
}

/**
 * grl_vector2_rotate:
 * @self: A vector
 * @angle: Rotation angle in radians
 *
 * Rotates the vector by the given angle.
 *
 * Returns: (transfer full): A new rotated vector
 */
GrlVector2 *
grl_vector2_rotate (const GrlVector2 *self,
                    gfloat            angle)
{
    gfloat cos_angle;
    gfloat sin_angle;

    g_return_val_if_fail (self != NULL, NULL);

    cos_angle = cosf (angle);
    sin_angle = sinf (angle);

    return grl_vector2_new (
        self->x * cos_angle - self->y * sin_angle,
        self->x * sin_angle + self->y * cos_angle
    );
}

/**
 * grl_vector2_clamp:
 * @self: A vector
 * @min: Minimum bounds
 * @max: Maximum bounds
 *
 * Clamps the vector components to the given range.
 *
 * Returns: (transfer full): A new clamped vector
 */
GrlVector2 *
grl_vector2_clamp (const GrlVector2 *self,
                   const GrlVector2 *min,
                   const GrlVector2 *max)
{
    gfloat x;
    gfloat y;

    g_return_val_if_fail (self != NULL, NULL);
    g_return_val_if_fail (min != NULL, NULL);
    g_return_val_if_fail (max != NULL, NULL);

    x = CLAMP (self->x, min->x, max->x);
    y = CLAMP (self->y, min->y, max->y);

    return grl_vector2_new (x, y);
}

/**
 * grl_vector2_reflect:
 * @self: Incident vector
 * @normal: Surface normal (should be normalized)
 *
 * Reflects the vector off a surface with the given normal.
 *
 * Returns: (transfer full): A new reflected vector
 */
GrlVector2 *
grl_vector2_reflect (const GrlVector2 *self,
                     const GrlVector2 *normal)
{
    gfloat dot;

    g_return_val_if_fail (self != NULL, NULL);
    g_return_val_if_fail (normal != NULL, NULL);

    dot = grl_vector2_dot (self, normal);

    return grl_vector2_new (
        self->x - 2.0f * dot * normal->x,
        self->y - 2.0f * dot * normal->y
    );
}

/*
 * Scalar operations
 */

/**
 * grl_vector2_length:
 * @self: A vector
 *
 * Calculates the length (magnitude) of the vector.
 *
 * Returns: The length of the vector
 */
gfloat
grl_vector2_length (const GrlVector2 *self)
{
    g_return_val_if_fail (self != NULL, 0.0f);

    return sqrtf (self->x * self->x + self->y * self->y);
}

/**
 * grl_vector2_length_sqr:
 * @self: A vector
 *
 * Calculates the squared length of the vector.
 * This is faster than grl_vector2_length() when you only need
 * to compare lengths.
 *
 * Returns: The squared length of the vector
 */
gfloat
grl_vector2_length_sqr (const GrlVector2 *self)
{
    g_return_val_if_fail (self != NULL, 0.0f);

    return self->x * self->x + self->y * self->y;
}

/**
 * grl_vector2_dot:
 * @a: First vector
 * @b: Second vector
 *
 * Calculates the dot product of two vectors.
 *
 * Returns: The dot product
 */
gfloat
grl_vector2_dot (const GrlVector2 *a,
                 const GrlVector2 *b)
{
    g_return_val_if_fail (a != NULL, 0.0f);
    g_return_val_if_fail (b != NULL, 0.0f);

    return a->x * b->x + a->y * b->y;
}

/**
 * grl_vector2_distance:
 * @a: First vector
 * @b: Second vector
 *
 * Calculates the distance between two points.
 *
 * Returns: The distance
 */
gfloat
grl_vector2_distance (const GrlVector2 *a,
                      const GrlVector2 *b)
{
    gfloat dx;
    gfloat dy;

    g_return_val_if_fail (a != NULL, 0.0f);
    g_return_val_if_fail (b != NULL, 0.0f);

    dx = b->x - a->x;
    dy = b->y - a->y;

    return sqrtf (dx * dx + dy * dy);
}

/**
 * grl_vector2_distance_sqr:
 * @a: First vector
 * @b: Second vector
 *
 * Calculates the squared distance between two points.
 * This is faster than grl_vector2_distance() when you only need
 * to compare distances.
 *
 * Returns: The squared distance
 */
gfloat
grl_vector2_distance_sqr (const GrlVector2 *a,
                          const GrlVector2 *b)
{
    gfloat dx;
    gfloat dy;

    g_return_val_if_fail (a != NULL, 0.0f);
    g_return_val_if_fail (b != NULL, 0.0f);

    dx = b->x - a->x;
    dy = b->y - a->y;

    return dx * dx + dy * dy;
}

/**
 * grl_vector2_angle:
 * @a: First vector
 * @b: Second vector
 *
 * Calculates the angle between two vectors in radians.
 *
 * Returns: The angle in radians
 */
gfloat
grl_vector2_angle (const GrlVector2 *a,
                   const GrlVector2 *b)
{
    g_return_val_if_fail (a != NULL, 0.0f);
    g_return_val_if_fail (b != NULL, 0.0f);

    return atan2f (b->y - a->y, b->x - a->x);
}

/*
 * Comparison
 */

/**
 * grl_vector2_equal:
 * @a: First vector
 * @b: Second vector
 *
 * Checks if two vectors are exactly equal.
 *
 * Returns: %TRUE if the vectors are equal
 */
gboolean
grl_vector2_equal (const GrlVector2 *a,
                   const GrlVector2 *b)
{
    if (a == b)
        return TRUE;
    if (a == NULL || b == NULL)
        return FALSE;

    return a->x == b->x && a->y == b->y;
}

/**
 * grl_vector2_equal_epsilon:
 * @a: First vector
 * @b: Second vector
 * @epsilon: Maximum difference allowed
 *
 * Checks if two vectors are approximately equal within a tolerance.
 *
 * Returns: %TRUE if the vectors are approximately equal
 */
gboolean
grl_vector2_equal_epsilon (const GrlVector2 *a,
                           const GrlVector2 *b,
                           gfloat            epsilon)
{
    if (a == b)
        return TRUE;
    if (a == NULL || b == NULL)
        return FALSE;

    return fabsf (a->x - b->x) <= epsilon &&
           fabsf (a->y - b->y) <= epsilon;
}

/*
 * Utility
 */

/**
 * grl_vector2_to_string:
 * @self: A vector
 *
 * Creates a string representation of the vector.
 *
 * Returns: (transfer full): A newly allocated string
 */
gchar *
grl_vector2_to_string (const GrlVector2 *self)
{
    g_return_val_if_fail (self != NULL, NULL);

    return g_strdup_printf ("GrlVector2(%f, %f)", self->x, self->y);
}

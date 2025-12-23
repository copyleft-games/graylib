/* grl-rectangle.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Implementation of 2D axis-aligned rectangle type.
 */

#include "grl-rectangle.h"
#include <math.h>

/**
 * SECTION:grl-rectangle
 * @Title: GrlRectangle
 * @Short_description: A 2D axis-aligned rectangle
 *
 * #GrlRectangle represents a 2D axis-aligned rectangle defined by
 * its top-left position and size. It is used throughout Graylib
 * for bounding boxes, sprite regions, UI layouts, and collision detection.
 *
 * # Example
 *
 * |[<!-- language="C" -->
 * g_autoptr(GrlRectangle) bounds = grl_rectangle_new (0, 0, 100, 50);
 * g_autoptr(GrlVector2) center = grl_rectangle_get_center (bounds);
 *
 * if (grl_rectangle_contains_point (bounds, mouse_x, mouse_y))
 *     g_print ("Mouse is inside!\n");
 * ]|
 */

G_DEFINE_BOXED_TYPE (GrlRectangle, grl_rectangle,
                     grl_rectangle_copy, grl_rectangle_free)

/*
 * Constructors
 */

/**
 * grl_rectangle_new:
 * @x: X position of top-left corner
 * @y: Y position of top-left corner
 * @width: Width of the rectangle
 * @height: Height of the rectangle
 *
 * Creates a new rectangle with the given position and size.
 *
 * Returns: (transfer full): A newly allocated #GrlRectangle
 */
GrlRectangle *
grl_rectangle_new (gfloat x,
                   gfloat y,
                   gfloat width,
                   gfloat height)
{
    GrlRectangle *self;

    self = g_new (GrlRectangle, 1);
    self->x = x;
    self->y = y;
    self->width = width;
    self->height = height;

    return self;
}

/**
 * grl_rectangle_new_empty:
 *
 * Creates a new empty rectangle at the origin.
 *
 * Returns: (transfer full): A newly allocated #GrlRectangle
 */
GrlRectangle *
grl_rectangle_new_empty (void)
{
    return grl_rectangle_new (0.0f, 0.0f, 0.0f, 0.0f);
}

/**
 * grl_rectangle_new_from_vectors:
 * @position: Position of top-left corner
 * @size: Size (width and height)
 *
 * Creates a new rectangle from position and size vectors.
 *
 * Returns: (transfer full): A newly allocated #GrlRectangle
 */
GrlRectangle *
grl_rectangle_new_from_vectors (const GrlVector2 *position,
                                const GrlVector2 *size)
{
    g_return_val_if_fail (position != NULL, NULL);
    g_return_val_if_fail (size != NULL, NULL);

    return grl_rectangle_new (position->x, position->y, size->x, size->y);
}

/**
 * grl_rectangle_new_from_corners:
 * @top_left: Top-left corner
 * @bottom_right: Bottom-right corner
 *
 * Creates a new rectangle from two corner points.
 *
 * Returns: (transfer full): A newly allocated #GrlRectangle
 */
GrlRectangle *
grl_rectangle_new_from_corners (const GrlVector2 *top_left,
                                const GrlVector2 *bottom_right)
{
    g_return_val_if_fail (top_left != NULL, NULL);
    g_return_val_if_fail (bottom_right != NULL, NULL);

    return grl_rectangle_new (
        top_left->x,
        top_left->y,
        bottom_right->x - top_left->x,
        bottom_right->y - top_left->y
    );
}

/**
 * grl_rectangle_copy:
 * @self: (nullable): A #GrlRectangle
 *
 * Creates a copy of the rectangle.
 *
 * Returns: (transfer full) (nullable): A copy of @self, or %NULL
 */
GrlRectangle *
grl_rectangle_copy (const GrlRectangle *self)
{
    if (self == NULL)
        return NULL;

    return grl_rectangle_new (self->x, self->y, self->width, self->height);
}

/**
 * grl_rectangle_free:
 * @self: (nullable): A #GrlRectangle
 *
 * Frees a rectangle allocated with grl_rectangle_new() or grl_rectangle_copy().
 */
void
grl_rectangle_free (GrlRectangle *self)
{
    g_free (self);
}

/*
 * Accessors
 */

/**
 * grl_rectangle_get_x:
 * @self: A #GrlRectangle
 *
 * Gets the X position.
 *
 * Returns: The X position
 */
gfloat
grl_rectangle_get_x (const GrlRectangle *self)
{
    g_return_val_if_fail (self != NULL, 0.0f);
    return self->x;
}

/**
 * grl_rectangle_get_y:
 * @self: A #GrlRectangle
 *
 * Gets the Y position.
 *
 * Returns: The Y position
 */
gfloat
grl_rectangle_get_y (const GrlRectangle *self)
{
    g_return_val_if_fail (self != NULL, 0.0f);
    return self->y;
}

/**
 * grl_rectangle_get_width:
 * @self: A #GrlRectangle
 *
 * Gets the width.
 *
 * Returns: The width
 */
gfloat
grl_rectangle_get_width (const GrlRectangle *self)
{
    g_return_val_if_fail (self != NULL, 0.0f);
    return self->width;
}

/**
 * grl_rectangle_get_height:
 * @self: A #GrlRectangle
 *
 * Gets the height.
 *
 * Returns: The height
 */
gfloat
grl_rectangle_get_height (const GrlRectangle *self)
{
    g_return_val_if_fail (self != NULL, 0.0f);
    return self->height;
}

/**
 * grl_rectangle_set:
 * @self: A #GrlRectangle
 * @x: New X position
 * @y: New Y position
 * @width: New width
 * @height: New height
 *
 * Sets all components of the rectangle.
 */
void
grl_rectangle_set (GrlRectangle *self,
                   gfloat        x,
                   gfloat        y,
                   gfloat        width,
                   gfloat        height)
{
    g_return_if_fail (self != NULL);

    self->x = x;
    self->y = y;
    self->width = width;
    self->height = height;
}

/*
 * Derived properties
 */

/**
 * grl_rectangle_get_left:
 * @self: A #GrlRectangle
 *
 * Gets the left edge X coordinate (same as x).
 *
 * Returns: The left edge
 */
gfloat
grl_rectangle_get_left (const GrlRectangle *self)
{
    g_return_val_if_fail (self != NULL, 0.0f);
    return self->x;
}

/**
 * grl_rectangle_get_right:
 * @self: A #GrlRectangle
 *
 * Gets the right edge X coordinate.
 *
 * Returns: The right edge
 */
gfloat
grl_rectangle_get_right (const GrlRectangle *self)
{
    g_return_val_if_fail (self != NULL, 0.0f);
    return self->x + self->width;
}

/**
 * grl_rectangle_get_top:
 * @self: A #GrlRectangle
 *
 * Gets the top edge Y coordinate (same as y).
 *
 * Returns: The top edge
 */
gfloat
grl_rectangle_get_top (const GrlRectangle *self)
{
    g_return_val_if_fail (self != NULL, 0.0f);
    return self->y;
}

/**
 * grl_rectangle_get_bottom:
 * @self: A #GrlRectangle
 *
 * Gets the bottom edge Y coordinate.
 *
 * Returns: The bottom edge
 */
gfloat
grl_rectangle_get_bottom (const GrlRectangle *self)
{
    g_return_val_if_fail (self != NULL, 0.0f);
    return self->y + self->height;
}

/**
 * grl_rectangle_get_position:
 * @self: A #GrlRectangle
 *
 * Gets the top-left position as a vector.
 *
 * Returns: (transfer full): A new #GrlVector2
 */
GrlVector2 *
grl_rectangle_get_position (const GrlRectangle *self)
{
    g_return_val_if_fail (self != NULL, NULL);
    return grl_vector2_new (self->x, self->y);
}

/**
 * grl_rectangle_get_size:
 * @self: A #GrlRectangle
 *
 * Gets the size as a vector.
 *
 * Returns: (transfer full): A new #GrlVector2
 */
GrlVector2 *
grl_rectangle_get_size (const GrlRectangle *self)
{
    g_return_val_if_fail (self != NULL, NULL);
    return grl_vector2_new (self->width, self->height);
}

/**
 * grl_rectangle_get_center:
 * @self: A #GrlRectangle
 *
 * Gets the center point of the rectangle.
 *
 * Returns: (transfer full): A new #GrlVector2
 */
GrlVector2 *
grl_rectangle_get_center (const GrlRectangle *self)
{
    g_return_val_if_fail (self != NULL, NULL);
    return grl_vector2_new (
        self->x + self->width / 2.0f,
        self->y + self->height / 2.0f
    );
}

/**
 * grl_rectangle_get_area:
 * @self: A #GrlRectangle
 *
 * Calculates the area of the rectangle.
 *
 * Returns: The area
 */
gfloat
grl_rectangle_get_area (const GrlRectangle *self)
{
    g_return_val_if_fail (self != NULL, 0.0f);
    return self->width * self->height;
}

/**
 * grl_rectangle_get_perimeter:
 * @self: A #GrlRectangle
 *
 * Calculates the perimeter of the rectangle.
 *
 * Returns: The perimeter
 */
gfloat
grl_rectangle_get_perimeter (const GrlRectangle *self)
{
    g_return_val_if_fail (self != NULL, 0.0f);
    return 2.0f * (self->width + self->height);
}

/*
 * Operations
 */

/**
 * grl_rectangle_expand:
 * @self: A #GrlRectangle
 * @amount: Amount to expand on all sides
 *
 * Creates an expanded rectangle. The position is adjusted to keep
 * the center in the same place.
 *
 * Returns: (transfer full): A new expanded rectangle
 */
GrlRectangle *
grl_rectangle_expand (const GrlRectangle *self,
                      gfloat              amount)
{
    g_return_val_if_fail (self != NULL, NULL);

    return grl_rectangle_new (
        self->x - amount,
        self->y - amount,
        self->width + amount * 2.0f,
        self->height + amount * 2.0f
    );
}

/**
 * grl_rectangle_expand_xy:
 * @self: A #GrlRectangle
 * @h_amount: Horizontal expansion amount
 * @v_amount: Vertical expansion amount
 *
 * Creates an expanded rectangle with different horizontal and vertical amounts.
 *
 * Returns: (transfer full): A new expanded rectangle
 */
GrlRectangle *
grl_rectangle_expand_xy (const GrlRectangle *self,
                         gfloat              h_amount,
                         gfloat              v_amount)
{
    g_return_val_if_fail (self != NULL, NULL);

    return grl_rectangle_new (
        self->x - h_amount,
        self->y - v_amount,
        self->width + h_amount * 2.0f,
        self->height + v_amount * 2.0f
    );
}

/**
 * grl_rectangle_translate:
 * @self: A #GrlRectangle
 * @dx: X offset
 * @dy: Y offset
 *
 * Creates a translated rectangle.
 *
 * Returns: (transfer full): A new translated rectangle
 */
GrlRectangle *
grl_rectangle_translate (const GrlRectangle *self,
                         gfloat              dx,
                         gfloat              dy)
{
    g_return_val_if_fail (self != NULL, NULL);

    return grl_rectangle_new (
        self->x + dx,
        self->y + dy,
        self->width,
        self->height
    );
}

/**
 * grl_rectangle_translate_v:
 * @self: A #GrlRectangle
 * @offset: Translation offset
 *
 * Creates a translated rectangle using a vector offset.
 *
 * Returns: (transfer full): A new translated rectangle
 */
GrlRectangle *
grl_rectangle_translate_v (const GrlRectangle *self,
                           const GrlVector2   *offset)
{
    g_return_val_if_fail (self != NULL, NULL);
    g_return_val_if_fail (offset != NULL, NULL);

    return grl_rectangle_translate (self, offset->x, offset->y);
}

/**
 * grl_rectangle_scale:
 * @self: A #GrlRectangle
 * @scale_x: Horizontal scale factor
 * @scale_y: Vertical scale factor
 *
 * Creates a scaled rectangle. The position is also scaled.
 *
 * Returns: (transfer full): A new scaled rectangle
 */
GrlRectangle *
grl_rectangle_scale (const GrlRectangle *self,
                     gfloat              scale_x,
                     gfloat              scale_y)
{
    g_return_val_if_fail (self != NULL, NULL);

    return grl_rectangle_new (
        self->x * scale_x,
        self->y * scale_y,
        self->width * scale_x,
        self->height * scale_y
    );
}

/*
 * Collision / Containment
 */

/**
 * grl_rectangle_contains_point:
 * @self: A #GrlRectangle
 * @x: X coordinate to test
 * @y: Y coordinate to test
 *
 * Checks if a point is inside the rectangle.
 *
 * Returns: %TRUE if the point is inside
 */
gboolean
grl_rectangle_contains_point (const GrlRectangle *self,
                              gfloat              x,
                              gfloat              y)
{
    g_return_val_if_fail (self != NULL, FALSE);

    return x >= self->x &&
           x < self->x + self->width &&
           y >= self->y &&
           y < self->y + self->height;
}

/**
 * grl_rectangle_contains_point_v:
 * @self: A #GrlRectangle
 * @point: Point to test
 *
 * Checks if a point vector is inside the rectangle.
 *
 * Returns: %TRUE if the point is inside
 */
gboolean
grl_rectangle_contains_point_v (const GrlRectangle *self,
                                const GrlVector2   *point)
{
    g_return_val_if_fail (self != NULL, FALSE);
    g_return_val_if_fail (point != NULL, FALSE);

    return grl_rectangle_contains_point (self, point->x, point->y);
}

/**
 * grl_rectangle_contains_rect:
 * @self: A #GrlRectangle
 * @other: Rectangle to test
 *
 * Checks if another rectangle is completely inside this rectangle.
 *
 * Returns: %TRUE if @other is fully contained
 */
gboolean
grl_rectangle_contains_rect (const GrlRectangle *self,
                             const GrlRectangle *other)
{
    g_return_val_if_fail (self != NULL, FALSE);
    g_return_val_if_fail (other != NULL, FALSE);

    return other->x >= self->x &&
           other->x + other->width <= self->x + self->width &&
           other->y >= self->y &&
           other->y + other->height <= self->y + self->height;
}

/**
 * grl_rectangle_intersects:
 * @a: First rectangle
 * @b: Second rectangle
 *
 * Checks if two rectangles overlap.
 *
 * Returns: %TRUE if the rectangles intersect
 */
gboolean
grl_rectangle_intersects (const GrlRectangle *a,
                          const GrlRectangle *b)
{
    g_return_val_if_fail (a != NULL, FALSE);
    g_return_val_if_fail (b != NULL, FALSE);

    return !(a->x + a->width <= b->x ||
             b->x + b->width <= a->x ||
             a->y + a->height <= b->y ||
             b->y + b->height <= a->y);
}

/**
 * grl_rectangle_intersection:
 * @a: First rectangle
 * @b: Second rectangle
 *
 * Calculates the intersection of two rectangles.
 *
 * Returns: (transfer full) (nullable): A new rectangle representing the
 *          intersection, or %NULL if no intersection
 */
GrlRectangle *
grl_rectangle_intersection (const GrlRectangle *a,
                            const GrlRectangle *b)
{
    gfloat x1;
    gfloat y1;
    gfloat x2;
    gfloat y2;

    g_return_val_if_fail (a != NULL, NULL);
    g_return_val_if_fail (b != NULL, NULL);

    x1 = MAX (a->x, b->x);
    y1 = MAX (a->y, b->y);
    x2 = MIN (a->x + a->width, b->x + b->width);
    y2 = MIN (a->y + a->height, b->y + b->height);

    if (x2 <= x1 || y2 <= y1)
        return NULL;

    return grl_rectangle_new (x1, y1, x2 - x1, y2 - y1);
}

/**
 * grl_rectangle_union:
 * @a: First rectangle
 * @b: Second rectangle
 *
 * Calculates the bounding rectangle that contains both rectangles.
 *
 * Returns: (transfer full): A new rectangle containing both inputs
 */
GrlRectangle *
grl_rectangle_union (const GrlRectangle *a,
                     const GrlRectangle *b)
{
    gfloat x1;
    gfloat y1;
    gfloat x2;
    gfloat y2;

    g_return_val_if_fail (a != NULL, NULL);
    g_return_val_if_fail (b != NULL, NULL);

    x1 = MIN (a->x, b->x);
    y1 = MIN (a->y, b->y);
    x2 = MAX (a->x + a->width, b->x + b->width);
    y2 = MAX (a->y + a->height, b->y + b->height);

    return grl_rectangle_new (x1, y1, x2 - x1, y2 - y1);
}

/*
 * Comparison
 */

/**
 * grl_rectangle_equal:
 * @a: First rectangle
 * @b: Second rectangle
 *
 * Checks if two rectangles are equal.
 *
 * Returns: %TRUE if the rectangles are equal
 */
gboolean
grl_rectangle_equal (const GrlRectangle *a,
                     const GrlRectangle *b)
{
    if (a == b)
        return TRUE;
    if (a == NULL || b == NULL)
        return FALSE;

    return a->x == b->x &&
           a->y == b->y &&
           a->width == b->width &&
           a->height == b->height;
}

/**
 * grl_rectangle_is_empty:
 * @self: A #GrlRectangle
 *
 * Checks if the rectangle has zero or negative area.
 *
 * Returns: %TRUE if the rectangle is empty
 */
gboolean
grl_rectangle_is_empty (const GrlRectangle *self)
{
    g_return_val_if_fail (self != NULL, TRUE);

    return self->width <= 0.0f || self->height <= 0.0f;
}

/*
 * Utility
 */

/**
 * grl_rectangle_to_string:
 * @self: A #GrlRectangle
 *
 * Creates a string representation of the rectangle.
 *
 * Returns: (transfer full): A newly allocated string
 */
gchar *
grl_rectangle_to_string (const GrlRectangle *self)
{
    g_return_val_if_fail (self != NULL, NULL);

    return g_strdup_printf ("GrlRectangle(%f, %f, %f, %f)",
                            self->x, self->y, self->width, self->height);
}

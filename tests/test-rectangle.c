/* test-rectangle.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Unit tests for GrlRectangle.
 */

#include <glib.h>
#include <math.h>
#include "graylib/math/grl-rectangle.h"
#include "graylib/math/grl-vector2.h"

#define EPSILON 0.0001f

static gboolean
float_equal (gfloat a, gfloat b)
{
    return fabsf (a - b) < EPSILON;
}

/*
 * Test constructors
 */

static void
test_rectangle_new (void)
{
    g_autoptr(GrlRectangle) r = grl_rectangle_new (10.0f, 20.0f, 100.0f, 50.0f);

    g_assert_nonnull (r);
    g_assert_true (float_equal (r->x, 10.0f));
    g_assert_true (float_equal (r->y, 20.0f));
    g_assert_true (float_equal (r->width, 100.0f));
    g_assert_true (float_equal (r->height, 50.0f));
}

static void
test_rectangle_new_empty (void)
{
    g_autoptr(GrlRectangle) r = grl_rectangle_new_empty ();

    g_assert_nonnull (r);
    g_assert_true (float_equal (r->x, 0.0f));
    g_assert_true (float_equal (r->y, 0.0f));
    g_assert_true (float_equal (r->width, 0.0f));
    g_assert_true (float_equal (r->height, 0.0f));
}

static void
test_rectangle_new_from_vectors (void)
{
    g_autoptr(GrlVector2) pos = grl_vector2_new (5.0f, 10.0f);
    g_autoptr(GrlVector2) size = grl_vector2_new (200.0f, 100.0f);
    g_autoptr(GrlRectangle) r = grl_rectangle_new_from_vectors (pos, size);

    g_assert_nonnull (r);
    g_assert_true (float_equal (r->x, 5.0f));
    g_assert_true (float_equal (r->y, 10.0f));
    g_assert_true (float_equal (r->width, 200.0f));
    g_assert_true (float_equal (r->height, 100.0f));
}

static void
test_rectangle_new_from_corners (void)
{
    g_autoptr(GrlVector2) tl = grl_vector2_new (10.0f, 20.0f);
    g_autoptr(GrlVector2) br = grl_vector2_new (110.0f, 70.0f);
    g_autoptr(GrlRectangle) r = grl_rectangle_new_from_corners (tl, br);

    g_assert_nonnull (r);
    g_assert_true (float_equal (r->x, 10.0f));
    g_assert_true (float_equal (r->y, 20.0f));
    g_assert_true (float_equal (r->width, 100.0f));
    g_assert_true (float_equal (r->height, 50.0f));
}

static void
test_rectangle_init (void)
{
    GrlRectangle r = grl_rectangle_init (1.0f, 2.0f, 3.0f, 4.0f);

    g_assert_true (float_equal (r.x, 1.0f));
    g_assert_true (float_equal (r.y, 2.0f));
    g_assert_true (float_equal (r.width, 3.0f));
    g_assert_true (float_equal (r.height, 4.0f));
}

static void
test_rectangle_copy (void)
{
    g_autoptr(GrlRectangle) r1 = grl_rectangle_new (5.0f, 6.0f, 7.0f, 8.0f);
    g_autoptr(GrlRectangle) r2 = grl_rectangle_copy (r1);

    g_assert_nonnull (r2);
    g_assert_true (r1 != r2);
    g_assert_true (float_equal (r2->x, 5.0f));
    g_assert_true (float_equal (r2->y, 6.0f));
    g_assert_true (float_equal (r2->width, 7.0f));
    g_assert_true (float_equal (r2->height, 8.0f));
}

/*
 * Test accessors
 */

static void
test_rectangle_accessors (void)
{
    g_autoptr(GrlRectangle) r = grl_rectangle_new (10.0f, 20.0f, 30.0f, 40.0f);

    g_assert_true (float_equal (grl_rectangle_get_x (r), 10.0f));
    g_assert_true (float_equal (grl_rectangle_get_y (r), 20.0f));
    g_assert_true (float_equal (grl_rectangle_get_width (r), 30.0f));
    g_assert_true (float_equal (grl_rectangle_get_height (r), 40.0f));

    grl_rectangle_set (r, 50.0f, 60.0f, 70.0f, 80.0f);
    g_assert_true (float_equal (r->x, 50.0f));
    g_assert_true (float_equal (r->y, 60.0f));
    g_assert_true (float_equal (r->width, 70.0f));
    g_assert_true (float_equal (r->height, 80.0f));
}

/*
 * Test derived properties
 */

static void
test_rectangle_edges (void)
{
    g_autoptr(GrlRectangle) r = grl_rectangle_new (10.0f, 20.0f, 100.0f, 50.0f);

    g_assert_true (float_equal (grl_rectangle_get_left (r), 10.0f));
    g_assert_true (float_equal (grl_rectangle_get_right (r), 110.0f));
    g_assert_true (float_equal (grl_rectangle_get_top (r), 20.0f));
    g_assert_true (float_equal (grl_rectangle_get_bottom (r), 70.0f));
}

static void
test_rectangle_position (void)
{
    g_autoptr(GrlRectangle) r = grl_rectangle_new (15.0f, 25.0f, 50.0f, 30.0f);
    g_autoptr(GrlVector2) pos = grl_rectangle_get_position (r);

    g_assert_nonnull (pos);
    g_assert_true (float_equal (pos->x, 15.0f));
    g_assert_true (float_equal (pos->y, 25.0f));
}

static void
test_rectangle_size (void)
{
    g_autoptr(GrlRectangle) r = grl_rectangle_new (15.0f, 25.0f, 50.0f, 30.0f);
    g_autoptr(GrlVector2) size = grl_rectangle_get_size (r);

    g_assert_nonnull (size);
    g_assert_true (float_equal (size->x, 50.0f));
    g_assert_true (float_equal (size->y, 30.0f));
}

static void
test_rectangle_center (void)
{
    g_autoptr(GrlRectangle) r = grl_rectangle_new (0.0f, 0.0f, 100.0f, 50.0f);
    g_autoptr(GrlVector2) center = grl_rectangle_get_center (r);

    g_assert_nonnull (center);
    g_assert_true (float_equal (center->x, 50.0f));
    g_assert_true (float_equal (center->y, 25.0f));
}

static void
test_rectangle_area (void)
{
    g_autoptr(GrlRectangle) r = grl_rectangle_new (0.0f, 0.0f, 10.0f, 5.0f);
    gfloat area = grl_rectangle_get_area (r);

    g_assert_true (float_equal (area, 50.0f));
}

static void
test_rectangle_perimeter (void)
{
    g_autoptr(GrlRectangle) r = grl_rectangle_new (0.0f, 0.0f, 10.0f, 5.0f);
    gfloat perimeter = grl_rectangle_get_perimeter (r);

    g_assert_true (float_equal (perimeter, 30.0f));
}

/*
 * Test operations
 */

static void
test_rectangle_expand (void)
{
    g_autoptr(GrlRectangle) r = grl_rectangle_new (10.0f, 20.0f, 100.0f, 50.0f);
    g_autoptr(GrlRectangle) expanded = grl_rectangle_expand (r, 5.0f);

    g_assert_nonnull (expanded);
    g_assert_true (float_equal (expanded->x, 5.0f));
    g_assert_true (float_equal (expanded->y, 15.0f));
    g_assert_true (float_equal (expanded->width, 110.0f));
    g_assert_true (float_equal (expanded->height, 60.0f));
}

static void
test_rectangle_translate (void)
{
    g_autoptr(GrlRectangle) r = grl_rectangle_new (10.0f, 20.0f, 100.0f, 50.0f);
    g_autoptr(GrlRectangle) moved = grl_rectangle_translate (r, 5.0f, 10.0f);

    g_assert_nonnull (moved);
    g_assert_true (float_equal (moved->x, 15.0f));
    g_assert_true (float_equal (moved->y, 30.0f));
    g_assert_true (float_equal (moved->width, 100.0f));
    g_assert_true (float_equal (moved->height, 50.0f));
}

static void
test_rectangle_scale (void)
{
    g_autoptr(GrlRectangle) r = grl_rectangle_new (10.0f, 20.0f, 100.0f, 50.0f);
    g_autoptr(GrlRectangle) scaled = grl_rectangle_scale (r, 2.0f, 3.0f);

    g_assert_nonnull (scaled);
    g_assert_true (float_equal (scaled->x, 20.0f));
    g_assert_true (float_equal (scaled->y, 60.0f));
    g_assert_true (float_equal (scaled->width, 200.0f));
    g_assert_true (float_equal (scaled->height, 150.0f));
}

/*
 * Test collision/containment
 */

static void
test_rectangle_contains_point (void)
{
    g_autoptr(GrlRectangle) r = grl_rectangle_new (0.0f, 0.0f, 100.0f, 50.0f);

    g_assert_true (grl_rectangle_contains_point (r, 50.0f, 25.0f));
    g_assert_true (grl_rectangle_contains_point (r, 0.0f, 0.0f));
    g_assert_true (grl_rectangle_contains_point (r, 99.0f, 49.0f));
    g_assert_false (grl_rectangle_contains_point (r, 100.0f, 50.0f));
    g_assert_false (grl_rectangle_contains_point (r, -1.0f, 25.0f));
    g_assert_false (grl_rectangle_contains_point (r, 50.0f, 51.0f));
}

static void
test_rectangle_contains_rect (void)
{
    g_autoptr(GrlRectangle) outer = grl_rectangle_new (0.0f, 0.0f, 100.0f, 100.0f);
    g_autoptr(GrlRectangle) inner = grl_rectangle_new (10.0f, 10.0f, 30.0f, 30.0f);
    g_autoptr(GrlRectangle) partial = grl_rectangle_new (80.0f, 80.0f, 30.0f, 30.0f);

    g_assert_true (grl_rectangle_contains_rect (outer, inner));
    g_assert_false (grl_rectangle_contains_rect (outer, partial));
    g_assert_false (grl_rectangle_contains_rect (inner, outer));
}

static void
test_rectangle_intersects (void)
{
    g_autoptr(GrlRectangle) r1 = grl_rectangle_new (0.0f, 0.0f, 50.0f, 50.0f);
    g_autoptr(GrlRectangle) r2 = grl_rectangle_new (25.0f, 25.0f, 50.0f, 50.0f);
    g_autoptr(GrlRectangle) r3 = grl_rectangle_new (100.0f, 100.0f, 50.0f, 50.0f);

    g_assert_true (grl_rectangle_intersects (r1, r2));
    g_assert_true (grl_rectangle_intersects (r2, r1));
    g_assert_false (grl_rectangle_intersects (r1, r3));
}

static void
test_rectangle_intersection (void)
{
    g_autoptr(GrlRectangle) r1 = grl_rectangle_new (0.0f, 0.0f, 50.0f, 50.0f);
    g_autoptr(GrlRectangle) r2 = grl_rectangle_new (25.0f, 25.0f, 50.0f, 50.0f);
    g_autoptr(GrlRectangle) inter = grl_rectangle_intersection (r1, r2);

    g_assert_nonnull (inter);
    g_assert_true (float_equal (inter->x, 25.0f));
    g_assert_true (float_equal (inter->y, 25.0f));
    g_assert_true (float_equal (inter->width, 25.0f));
    g_assert_true (float_equal (inter->height, 25.0f));
}

static void
test_rectangle_union (void)
{
    g_autoptr(GrlRectangle) r1 = grl_rectangle_new (0.0f, 0.0f, 50.0f, 50.0f);
    g_autoptr(GrlRectangle) r2 = grl_rectangle_new (25.0f, 25.0f, 50.0f, 50.0f);
    g_autoptr(GrlRectangle) u = grl_rectangle_union (r1, r2);

    g_assert_nonnull (u);
    g_assert_true (float_equal (u->x, 0.0f));
    g_assert_true (float_equal (u->y, 0.0f));
    g_assert_true (float_equal (u->width, 75.0f));
    g_assert_true (float_equal (u->height, 75.0f));
}

/*
 * Test comparison
 */

static void
test_rectangle_equal (void)
{
    g_autoptr(GrlRectangle) r1 = grl_rectangle_new (10.0f, 20.0f, 30.0f, 40.0f);
    g_autoptr(GrlRectangle) r2 = grl_rectangle_new (10.0f, 20.0f, 30.0f, 40.0f);
    g_autoptr(GrlRectangle) r3 = grl_rectangle_new (11.0f, 20.0f, 30.0f, 40.0f);

    g_assert_true (grl_rectangle_equal (r1, r2));
    g_assert_false (grl_rectangle_equal (r1, r3));
}

static void
test_rectangle_is_empty (void)
{
    g_autoptr(GrlRectangle) empty = grl_rectangle_new_empty ();
    g_autoptr(GrlRectangle) not_empty = grl_rectangle_new (0.0f, 0.0f, 1.0f, 1.0f);

    g_assert_true (grl_rectangle_is_empty (empty));
    g_assert_false (grl_rectangle_is_empty (not_empty));
}

/*
 * Test utility
 */

static void
test_rectangle_to_string (void)
{
    g_autoptr(GrlRectangle) r = grl_rectangle_new (10.0f, 20.0f, 30.0f, 40.0f);
    g_autofree gchar *str = grl_rectangle_to_string (r);

    g_assert_nonnull (str);
    g_assert_true (g_str_has_prefix (str, "GrlRectangle("));
}

/*
 * Test registration
 */

int
main (int   argc,
      char *argv[])
{
    g_test_init (&argc, &argv, NULL);

    /* Constructors */
    g_test_add_func ("/rectangle/new", test_rectangle_new);
    g_test_add_func ("/rectangle/new-empty", test_rectangle_new_empty);
    g_test_add_func ("/rectangle/new-from-vectors", test_rectangle_new_from_vectors);
    g_test_add_func ("/rectangle/new-from-corners", test_rectangle_new_from_corners);
    g_test_add_func ("/rectangle/init", test_rectangle_init);
    g_test_add_func ("/rectangle/copy", test_rectangle_copy);

    /* Accessors */
    g_test_add_func ("/rectangle/accessors", test_rectangle_accessors);

    /* Derived properties */
    g_test_add_func ("/rectangle/edges", test_rectangle_edges);
    g_test_add_func ("/rectangle/position", test_rectangle_position);
    g_test_add_func ("/rectangle/size", test_rectangle_size);
    g_test_add_func ("/rectangle/center", test_rectangle_center);
    g_test_add_func ("/rectangle/area", test_rectangle_area);
    g_test_add_func ("/rectangle/perimeter", test_rectangle_perimeter);

    /* Operations */
    g_test_add_func ("/rectangle/expand", test_rectangle_expand);
    g_test_add_func ("/rectangle/translate", test_rectangle_translate);
    g_test_add_func ("/rectangle/scale", test_rectangle_scale);

    /* Collision/Containment */
    g_test_add_func ("/rectangle/contains-point", test_rectangle_contains_point);
    g_test_add_func ("/rectangle/contains-rect", test_rectangle_contains_rect);
    g_test_add_func ("/rectangle/intersects", test_rectangle_intersects);
    g_test_add_func ("/rectangle/intersection", test_rectangle_intersection);
    g_test_add_func ("/rectangle/union", test_rectangle_union);

    /* Comparison */
    g_test_add_func ("/rectangle/equal", test_rectangle_equal);
    g_test_add_func ("/rectangle/is-empty", test_rectangle_is_empty);

    /* Utility */
    g_test_add_func ("/rectangle/to-string", test_rectangle_to_string);

    return g_test_run ();
}

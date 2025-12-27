/* test-camera2d.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Unit tests for GrlCamera2D.
 *
 * Note: Tests that require an active window/GPU context are skipped.
 * This tests property getters/setters only.
 */

#include <glib.h>
#include <math.h>
#include "src/graphics/grl-camera2d.h"
#include "src/math/grl-vector2.h"

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
test_camera2d_new (void)
{
    g_autoptr(GrlCamera2D) camera = grl_camera2d_new ();

    g_assert_nonnull (camera);
}

static void
test_camera2d_default_offset (void)
{
    g_autoptr(GrlCamera2D) camera = grl_camera2d_new ();
    g_autoptr(GrlVector2) offset = grl_camera2d_get_offset (camera);

    g_assert_nonnull (offset);
    g_assert_true (float_equal (offset->x, 0.0f));
    g_assert_true (float_equal (offset->y, 0.0f));
}

static void
test_camera2d_default_target (void)
{
    g_autoptr(GrlCamera2D) camera = grl_camera2d_new ();
    g_autoptr(GrlVector2) target = grl_camera2d_get_target (camera);

    g_assert_nonnull (target);
    g_assert_true (float_equal (target->x, 0.0f));
    g_assert_true (float_equal (target->y, 0.0f));
}

static void
test_camera2d_default_rotation (void)
{
    g_autoptr(GrlCamera2D) camera = grl_camera2d_new ();
    gfloat rotation = grl_camera2d_get_rotation (camera);

    g_assert_true (float_equal (rotation, 0.0f));
}

static void
test_camera2d_default_zoom (void)
{
    g_autoptr(GrlCamera2D) camera = grl_camera2d_new ();
    gfloat zoom = grl_camera2d_get_zoom (camera);

    g_assert_true (float_equal (zoom, 1.0f));
}

/*
 * Test setters/getters
 */

static void
test_camera2d_set_offset (void)
{
    g_autoptr(GrlCamera2D) camera = grl_camera2d_new ();
    g_autoptr(GrlVector2) new_offset = grl_vector2_new (100.0f, 200.0f);

    grl_camera2d_set_offset (camera, new_offset);

    g_autoptr(GrlVector2) offset = grl_camera2d_get_offset (camera);
    g_assert_true (float_equal (offset->x, 100.0f));
    g_assert_true (float_equal (offset->y, 200.0f));
}

static void
test_camera2d_set_offset_xy (void)
{
    g_autoptr(GrlCamera2D) camera = grl_camera2d_new ();

    grl_camera2d_set_offset_xy (camera, 150.0f, 250.0f);

    g_autoptr(GrlVector2) offset = grl_camera2d_get_offset (camera);
    g_assert_true (float_equal (offset->x, 150.0f));
    g_assert_true (float_equal (offset->y, 250.0f));
}

static void
test_camera2d_set_target (void)
{
    g_autoptr(GrlCamera2D) camera = grl_camera2d_new ();
    g_autoptr(GrlVector2) new_target = grl_vector2_new (500.0f, 300.0f);

    grl_camera2d_set_target (camera, new_target);

    g_autoptr(GrlVector2) target = grl_camera2d_get_target (camera);
    g_assert_true (float_equal (target->x, 500.0f));
    g_assert_true (float_equal (target->y, 300.0f));
}

static void
test_camera2d_set_target_xy (void)
{
    g_autoptr(GrlCamera2D) camera = grl_camera2d_new ();

    grl_camera2d_set_target_xy (camera, 600.0f, 400.0f);

    g_autoptr(GrlVector2) target = grl_camera2d_get_target (camera);
    g_assert_true (float_equal (target->x, 600.0f));
    g_assert_true (float_equal (target->y, 400.0f));
}

static void
test_camera2d_set_rotation (void)
{
    g_autoptr(GrlCamera2D) camera = grl_camera2d_new ();

    grl_camera2d_set_rotation (camera, 45.0f);

    gfloat rotation = grl_camera2d_get_rotation (camera);
    g_assert_true (float_equal (rotation, 45.0f));
}

static void
test_camera2d_set_zoom (void)
{
    g_autoptr(GrlCamera2D) camera = grl_camera2d_new ();

    grl_camera2d_set_zoom (camera, 2.0f);

    gfloat zoom = grl_camera2d_get_zoom (camera);
    g_assert_true (float_equal (zoom, 2.0f));
}

static void
test_camera2d_set_zoom_half (void)
{
    g_autoptr(GrlCamera2D) camera = grl_camera2d_new ();

    grl_camera2d_set_zoom (camera, 0.5f);

    gfloat zoom = grl_camera2d_get_zoom (camera);
    g_assert_true (float_equal (zoom, 0.5f));
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
    g_test_add_func ("/camera2d/new", test_camera2d_new);
    g_test_add_func ("/camera2d/default-offset", test_camera2d_default_offset);
    g_test_add_func ("/camera2d/default-target", test_camera2d_default_target);
    g_test_add_func ("/camera2d/default-rotation", test_camera2d_default_rotation);
    g_test_add_func ("/camera2d/default-zoom", test_camera2d_default_zoom);

    /* Setters/Getters */
    g_test_add_func ("/camera2d/set-offset", test_camera2d_set_offset);
    g_test_add_func ("/camera2d/set-offset-xy", test_camera2d_set_offset_xy);
    g_test_add_func ("/camera2d/set-target", test_camera2d_set_target);
    g_test_add_func ("/camera2d/set-target-xy", test_camera2d_set_target_xy);
    g_test_add_func ("/camera2d/set-rotation", test_camera2d_set_rotation);
    g_test_add_func ("/camera2d/set-zoom", test_camera2d_set_zoom);
    g_test_add_func ("/camera2d/set-zoom-half", test_camera2d_set_zoom_half);

    return g_test_run ();
}

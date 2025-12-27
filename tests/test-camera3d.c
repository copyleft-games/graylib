/* test-camera3d.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Unit tests for GrlCamera3D.
 *
 * Note: Tests that require an active window/GPU context are skipped.
 * This tests property getters/setters only.
 */

#include <glib.h>
#include <math.h>
#include "src/graphics/grl-camera3d.h"
#include "src/math/grl-vector3.h"
#include "src/grl-enums.h"

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
test_camera3d_new (void)
{
    g_autoptr(GrlCamera3D) camera = grl_camera3d_new ();

    g_assert_nonnull (camera);
}

static void
test_camera3d_default_position (void)
{
    g_autoptr(GrlCamera3D) camera = grl_camera3d_new ();
    g_autoptr(GrlVector3) position = grl_camera3d_get_position (camera);

    g_assert_nonnull (position);
    /* Default position is (0, 10, 10) */
    g_assert_true (float_equal (position->x, 0.0f));
    g_assert_true (float_equal (position->y, 10.0f));
    g_assert_true (float_equal (position->z, 10.0f));
}

static void
test_camera3d_default_target (void)
{
    g_autoptr(GrlCamera3D) camera = grl_camera3d_new ();
    g_autoptr(GrlVector3) target = grl_camera3d_get_target (camera);

    g_assert_nonnull (target);
    /* Default target is (0, 0, 0) */
    g_assert_true (float_equal (target->x, 0.0f));
    g_assert_true (float_equal (target->y, 0.0f));
    g_assert_true (float_equal (target->z, 0.0f));
}

static void
test_camera3d_default_up (void)
{
    g_autoptr(GrlCamera3D) camera = grl_camera3d_new ();
    g_autoptr(GrlVector3) up = grl_camera3d_get_up (camera);

    g_assert_nonnull (up);
    /* Default up is (0, 1, 0) */
    g_assert_true (float_equal (up->x, 0.0f));
    g_assert_true (float_equal (up->y, 1.0f));
    g_assert_true (float_equal (up->z, 0.0f));
}

static void
test_camera3d_default_fovy (void)
{
    g_autoptr(GrlCamera3D) camera = grl_camera3d_new ();
    gfloat fovy = grl_camera3d_get_fovy (camera);

    /* Default fovy is 45 degrees */
    g_assert_true (float_equal (fovy, 45.0f));
}

static void
test_camera3d_default_projection (void)
{
    g_autoptr(GrlCamera3D) camera = grl_camera3d_new ();
    GrlCameraProjection projection = grl_camera3d_get_projection (camera);

    /* Default is perspective */
    g_assert_true (projection == GRL_CAMERA_PERSPECTIVE);
}

/*
 * Test setters/getters
 */

static void
test_camera3d_set_position (void)
{
    g_autoptr(GrlCamera3D) camera = grl_camera3d_new ();
    g_autoptr(GrlVector3) new_pos = grl_vector3_new (5.0f, 15.0f, 20.0f);

    grl_camera3d_set_position (camera, new_pos);

    g_autoptr(GrlVector3) position = grl_camera3d_get_position (camera);
    g_assert_true (float_equal (position->x, 5.0f));
    g_assert_true (float_equal (position->y, 15.0f));
    g_assert_true (float_equal (position->z, 20.0f));
}

static void
test_camera3d_set_position_xyz (void)
{
    g_autoptr(GrlCamera3D) camera = grl_camera3d_new ();

    grl_camera3d_set_position_xyz (camera, 10.0f, 25.0f, 30.0f);

    g_autoptr(GrlVector3) position = grl_camera3d_get_position (camera);
    g_assert_true (float_equal (position->x, 10.0f));
    g_assert_true (float_equal (position->y, 25.0f));
    g_assert_true (float_equal (position->z, 30.0f));
}

static void
test_camera3d_set_target (void)
{
    g_autoptr(GrlCamera3D) camera = grl_camera3d_new ();
    g_autoptr(GrlVector3) new_target = grl_vector3_new (5.0f, 5.0f, 5.0f);

    grl_camera3d_set_target (camera, new_target);

    g_autoptr(GrlVector3) target = grl_camera3d_get_target (camera);
    g_assert_true (float_equal (target->x, 5.0f));
    g_assert_true (float_equal (target->y, 5.0f));
    g_assert_true (float_equal (target->z, 5.0f));
}

static void
test_camera3d_set_target_xyz (void)
{
    g_autoptr(GrlCamera3D) camera = grl_camera3d_new ();

    grl_camera3d_set_target_xyz (camera, 1.0f, 2.0f, 3.0f);

    g_autoptr(GrlVector3) target = grl_camera3d_get_target (camera);
    g_assert_true (float_equal (target->x, 1.0f));
    g_assert_true (float_equal (target->y, 2.0f));
    g_assert_true (float_equal (target->z, 3.0f));
}

static void
test_camera3d_set_up (void)
{
    g_autoptr(GrlCamera3D) camera = grl_camera3d_new ();
    g_autoptr(GrlVector3) new_up = grl_vector3_new (0.0f, 0.0f, 1.0f);

    grl_camera3d_set_up (camera, new_up);

    g_autoptr(GrlVector3) up = grl_camera3d_get_up (camera);
    g_assert_true (float_equal (up->x, 0.0f));
    g_assert_true (float_equal (up->y, 0.0f));
    g_assert_true (float_equal (up->z, 1.0f));
}

static void
test_camera3d_set_fovy (void)
{
    g_autoptr(GrlCamera3D) camera = grl_camera3d_new ();

    grl_camera3d_set_fovy (camera, 60.0f);

    gfloat fovy = grl_camera3d_get_fovy (camera);
    g_assert_true (float_equal (fovy, 60.0f));
}

static void
test_camera3d_set_projection_orthographic (void)
{
    g_autoptr(GrlCamera3D) camera = grl_camera3d_new ();

    grl_camera3d_set_projection (camera, GRL_CAMERA_ORTHOGRAPHIC);

    GrlCameraProjection projection = grl_camera3d_get_projection (camera);
    g_assert_true (projection == GRL_CAMERA_ORTHOGRAPHIC);
}

static void
test_camera3d_set_projection_perspective (void)
{
    g_autoptr(GrlCamera3D) camera = grl_camera3d_new ();

    /* First set to orthographic, then back to perspective */
    grl_camera3d_set_projection (camera, GRL_CAMERA_ORTHOGRAPHIC);
    grl_camera3d_set_projection (camera, GRL_CAMERA_PERSPECTIVE);

    GrlCameraProjection projection = grl_camera3d_get_projection (camera);
    g_assert_true (projection == GRL_CAMERA_PERSPECTIVE);
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
    g_test_add_func ("/camera3d/new", test_camera3d_new);
    g_test_add_func ("/camera3d/default-position", test_camera3d_default_position);
    g_test_add_func ("/camera3d/default-target", test_camera3d_default_target);
    g_test_add_func ("/camera3d/default-up", test_camera3d_default_up);
    g_test_add_func ("/camera3d/default-fovy", test_camera3d_default_fovy);
    g_test_add_func ("/camera3d/default-projection", test_camera3d_default_projection);

    /* Setters/Getters */
    g_test_add_func ("/camera3d/set-position", test_camera3d_set_position);
    g_test_add_func ("/camera3d/set-position-xyz", test_camera3d_set_position_xyz);
    g_test_add_func ("/camera3d/set-target", test_camera3d_set_target);
    g_test_add_func ("/camera3d/set-target-xyz", test_camera3d_set_target_xyz);
    g_test_add_func ("/camera3d/set-up", test_camera3d_set_up);
    g_test_add_func ("/camera3d/set-fovy", test_camera3d_set_fovy);
    g_test_add_func ("/camera3d/set-projection-orthographic", test_camera3d_set_projection_orthographic);
    g_test_add_func ("/camera3d/set-projection-perspective", test_camera3d_set_projection_perspective);

    return g_test_run ();
}

/* test-interpolation.c
 *
 * Copyright 2026 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Unit tests for the render interpolation clock and GrlEntity render poses.
 */

#include <glib.h>
#include <math.h>
#include "src/scene/grl-interpolation.h"
#include "src/scene/grl-entity.h"

#define EPSILON 0.0001f

/* Every test leaves the process-wide clock off */
static void
clock_off (void)
{
    grl_interpolation_end_step ();
    grl_interpolation_set_enabled (FALSE);
}

/* One simulated fixed step in which @entity moves by (@dx, 0) */
static void
step_move (GrlEntity *entity,
           gfloat     dx)
{
    grl_interpolation_begin_step ();
    grl_entity_move_xy (entity, dx, 0.0f, 1.0f);
    grl_interpolation_end_step ();
}

static void
test_interpolation_lerp_angle (void)
{
    /* the shorter arc through 0, endpoints exact */
    g_assert_cmpfloat_with_epsilon (grl_interpolation_lerp_angle (350.0f, 10.0f, 0.5f, 360.0f),
                                    360.0f, EPSILON);
    g_assert_cmpfloat_with_epsilon (grl_interpolation_lerp_angle (10.0f, 350.0f, 0.5f, 360.0f),
                                    0.0f, EPSILON);
    g_assert_cmpfloat_with_epsilon (grl_interpolation_lerp_angle (350.0f, 10.0f, 0.0f, 360.0f),
                                    350.0f, EPSILON);
    g_assert_cmpfloat_with_epsilon (grl_interpolation_lerp_angle (350.0f, 10.0f, 1.0f, 360.0f),
                                    10.0f, EPSILON);
    g_assert_cmpfloat_with_epsilon (grl_interpolation_lerp_angle (0.0f, 90.0f, 0.5f, 360.0f),
                                    45.0f, EPSILON);
    g_assert_cmpfloat_with_epsilon (grl_interpolation_lerp_angle (0.0f, (gfloat) (G_PI / 2.0), 0.5f,
                                                                  (gfloat) (2.0 * G_PI)),
                                    (gfloat) (G_PI / 4.0), EPSILON);
}

static void
test_interpolation_disabled (void)
{
    g_autoptr(GrlEntity) entity = NULL;
    gfloat x;

    clock_off ();
    entity = grl_entity_new_at (0.0f, 0.0f);

    step_move (entity, 10.0f);
    grl_interpolation_set_alpha (0.25f);

    /* off: the render pose is the true pose, the alpha reads 1 */
    grl_entity_get_render_position_xy (entity, &x, NULL);
    g_assert_cmpfloat_with_epsilon (x, 10.0f, EPSILON);
    g_assert_cmpfloat_with_epsilon (grl_interpolation_get_alpha (), 1.0f, EPSILON);
}

static void
test_interpolation_entity_blend (void)
{
    g_autoptr(GrlEntity) entity = NULL;
    gfloat x;
    gfloat y;
    gfloat rotation;
    gfloat scale;

    grl_interpolation_set_enabled (TRUE);
    entity = grl_entity_new_at (100.0f, 50.0f);

    /* a new entity is drawn where it is */
    grl_interpolation_set_alpha (0.5f);
    grl_entity_get_render_position_xy (entity, &x, &y);
    g_assert_cmpfloat_with_epsilon (x, 100.0f, EPSILON);

    /* moves in a step: drawn between the step's start and end */
    grl_interpolation_begin_step ();
    grl_entity_set_position_xy (entity, 110.0f, 70.0f);
    grl_entity_set_rotation (entity, 350.0f);
    grl_entity_set_scale (entity, 3.0f);
    grl_interpolation_end_step ();

    grl_interpolation_set_alpha (0.25f);
    grl_entity_get_render_pose (entity, &x, &y, &rotation, &scale);
    g_assert_cmpfloat_with_epsilon (x, 102.5f, EPSILON);
    g_assert_cmpfloat_with_epsilon (y, 55.0f, EPSILON);
    g_assert_cmpfloat_with_epsilon (rotation, -2.5f, EPSILON); /* 0 -> 350 the short way */
    g_assert_cmpfloat_with_epsilon (scale, 1.5f, EPSILON);

    /* true state is untouched */
    g_assert_cmpfloat_with_epsilon (grl_entity_get_x (entity), 110.0f, EPSILON);

    /* several writes in one step blend from the step's start */
    grl_interpolation_begin_step ();
    grl_entity_set_x (entity, 111.0f);
    grl_entity_set_x (entity, 114.0f);
    grl_interpolation_end_step ();
    grl_interpolation_set_alpha (0.5f);
    g_assert_cmpfloat_with_epsilon (grl_entity_get_render_rotation (entity), 350.0f, EPSILON);
    grl_entity_get_render_position_xy (entity, &x, NULL);
    g_assert_cmpfloat_with_epsilon (x, 112.0f, EPSILON);

    /* a step without motion: drawn at the true pose */
    grl_interpolation_begin_step ();
    grl_interpolation_end_step ();
    grl_entity_get_render_position_xy (entity, &x, NULL);
    g_assert_cmpfloat_with_epsilon (x, 114.0f, EPSILON);

    clock_off ();
}

static void
test_interpolation_out_of_step_write (void)
{
    g_autoptr(GrlEntity) entity = NULL;
    gfloat x;

    grl_interpolation_set_enabled (TRUE);
    entity = grl_entity_new_at (0.0f, 0.0f);

    step_move (entity, 10.0f);

    /* a per-frame nudge outside a step shows in full, the blend stays */
    grl_entity_set_x (entity, 15.0f);
    grl_interpolation_set_alpha (0.5f);
    grl_entity_get_render_position_xy (entity, &x, NULL);
    g_assert_cmpfloat_with_epsilon (x, 10.0f, EPSILON);

    clock_off ();
}

static void
test_interpolation_teleport (void)
{
    g_autoptr(GrlEntity) entity = NULL;
    gfloat x;

    grl_interpolation_set_enabled (TRUE);
    entity = grl_entity_new_at (0.0f, 0.0f);
    step_move (entity, 10.0f);

    /* teleport, then keep moving in the same step: no streak */
    grl_interpolation_begin_step ();
    grl_entity_teleport (entity, 500.0f, 0.0f);
    grl_entity_move_xy (entity, 10.0f, 0.0f, 1.0f);
    grl_interpolation_end_step ();

    grl_interpolation_set_alpha (0.0f);
    grl_entity_get_render_position_xy (entity, &x, NULL);
    g_assert_cmpfloat_with_epsilon (x, 510.0f, EPSILON);

    /* the next step blends normally again */
    step_move (entity, 10.0f);
    grl_interpolation_set_alpha (0.5f);
    grl_entity_get_render_position_xy (entity, &x, NULL);
    g_assert_cmpfloat_with_epsilon (x, 515.0f, EPSILON);

    clock_off ();
}

static void
test_interpolation_created_in_step (void)
{
    g_autoptr(GrlEntity) entity = NULL;
    gfloat x;

    grl_interpolation_set_enabled (TRUE);

    /* spawned and positioned in one step: drawn at the spawn point */
    grl_interpolation_begin_step ();
    entity = grl_entity_new ();
    grl_entity_set_position_xy (entity, 300.0f, 40.0f);
    grl_interpolation_end_step ();

    grl_interpolation_set_alpha (0.0f);
    grl_entity_get_render_position_xy (entity, &x, NULL);
    g_assert_cmpfloat_with_epsilon (x, 300.0f, EPSILON);

    clock_off ();
}

static void
test_interpolation_opt_out (void)
{
    g_autoptr(GrlEntity) entity = NULL;
    gfloat x;

    grl_interpolation_set_enabled (TRUE);
    entity = g_object_new (GRL_TYPE_ENTITY, "interpolate", FALSE, NULL);
    g_assert_false (grl_entity_get_interpolate (entity));

    step_move (entity, 10.0f);
    grl_interpolation_set_alpha (0.5f);
    grl_entity_get_render_position_xy (entity, &x, NULL);
    g_assert_cmpfloat_with_epsilon (x, 10.0f, EPSILON);

    grl_entity_set_interpolate (entity, TRUE);
    step_move (entity, 10.0f);
    grl_entity_get_render_position_xy (entity, &x, NULL);
    g_assert_cmpfloat_with_epsilon (x, 15.0f, EPSILON);

    clock_off ();
}

int
main (int   argc,
      char *argv[])
{
    g_test_init (&argc, &argv, NULL);

    g_test_add_func ("/interpolation/lerp-angle", test_interpolation_lerp_angle);
    g_test_add_func ("/interpolation/disabled", test_interpolation_disabled);
    g_test_add_func ("/interpolation/entity-blend", test_interpolation_entity_blend);
    g_test_add_func ("/interpolation/out-of-step-write", test_interpolation_out_of_step_write);
    g_test_add_func ("/interpolation/teleport", test_interpolation_teleport);
    g_test_add_func ("/interpolation/created-in-step", test_interpolation_created_in_step);
    g_test_add_func ("/interpolation/opt-out", test_interpolation_opt_out);

    return g_test_run ();
}

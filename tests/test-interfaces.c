/* test-interfaces.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Unit tests for GrlDrawable, GrlUpdatable, and GrlCollidable interfaces.
 *
 * These tests verify that the interface types are properly registered
 * and that GrlEntity (which implements all three) works with them.
 */

#include <glib.h>
#include <math.h>
#include "src/scene/grl-entity.h"
#include "src/scene/grl-drawable.h"
#include "src/scene/grl-updatable.h"
#include "src/scene/grl-collidable.h"
#include "src/math/grl-rectangle.h"

#define EPSILON 0.0001f

static gboolean
float_equal (gfloat a, gfloat b)
{
    return fabsf (a - b) < EPSILON;
}

/*
 * Test GrlDrawable interface
 */

static void
test_drawable_type (void)
{
    GType type = grl_drawable_get_type ();

    g_assert_true (G_TYPE_IS_INTERFACE (type));
    g_assert_cmpstr (g_type_name (type), ==, "GrlDrawable");
}

static void
test_entity_is_drawable (void)
{
    g_autoptr(GrlEntity) entity = grl_entity_new ();

    g_assert_true (GRL_IS_DRAWABLE (entity));
}

static void
test_drawable_get_visible (void)
{
    g_autoptr(GrlEntity) entity = grl_entity_new ();

    g_assert_true (grl_drawable_get_visible (GRL_DRAWABLE (entity)));
}

static void
test_drawable_set_visible (void)
{
    g_autoptr(GrlEntity) entity = grl_entity_new ();

    grl_drawable_set_visible (GRL_DRAWABLE (entity), FALSE);

    g_assert_false (grl_drawable_get_visible (GRL_DRAWABLE (entity)));
}

static void
test_drawable_get_z_index (void)
{
    g_autoptr(GrlEntity) entity = grl_entity_new ();

    grl_entity_set_z_index (entity, 42);

    g_assert_cmpint (grl_drawable_get_z_index (GRL_DRAWABLE (entity)), ==, 42);
}

static void
test_drawable_set_z_index (void)
{
    g_autoptr(GrlEntity) entity = grl_entity_new ();

    grl_drawable_set_z_index (GRL_DRAWABLE (entity), 100);

    g_assert_cmpint (grl_drawable_get_z_index (GRL_DRAWABLE (entity)), ==, 100);
}

/*
 * Test GrlUpdatable interface
 */

static void
test_updatable_type (void)
{
    GType type = grl_updatable_get_type ();

    g_assert_true (G_TYPE_IS_INTERFACE (type));
    g_assert_cmpstr (g_type_name (type), ==, "GrlUpdatable");
}

static void
test_entity_is_updatable (void)
{
    g_autoptr(GrlEntity) entity = grl_entity_new ();

    g_assert_true (GRL_IS_UPDATABLE (entity));
}

static void
test_updatable_get_active (void)
{
    g_autoptr(GrlEntity) entity = grl_entity_new ();

    g_assert_true (grl_updatable_get_active (GRL_UPDATABLE (entity)));
}

static void
test_updatable_set_active (void)
{
    g_autoptr(GrlEntity) entity = grl_entity_new ();

    grl_updatable_set_active (GRL_UPDATABLE (entity), FALSE);

    g_assert_false (grl_updatable_get_active (GRL_UPDATABLE (entity)));
}

/*
 * Test GrlCollidable interface
 */

static void
test_collidable_type (void)
{
    GType type = grl_collidable_get_type ();

    g_assert_true (G_TYPE_IS_INTERFACE (type));
    g_assert_cmpstr (g_type_name (type), ==, "GrlCollidable");
}

static void
test_entity_is_collidable (void)
{
    g_autoptr(GrlEntity) entity = grl_entity_new ();

    g_assert_true (GRL_IS_COLLIDABLE (entity));
}

static void
test_collidable_get_bounds (void)
{
    g_autoptr(GrlEntity) entity = grl_entity_new_at (100.0f, 200.0f);

    grl_entity_set_width (entity, 50.0f);
    grl_entity_set_height (entity, 30.0f);

    g_autoptr(GrlRectangle) bounds = grl_collidable_get_bounds (GRL_COLLIDABLE (entity));

    g_assert_nonnull (bounds);
    g_assert_true (float_equal (bounds->x, 100.0f));
    g_assert_true (float_equal (bounds->y, 200.0f));
    g_assert_true (float_equal (bounds->width, 50.0f));
    g_assert_true (float_equal (bounds->height, 30.0f));
}

static void
test_collidable_check_collision_true (void)
{
    g_autoptr(GrlEntity) e1 = grl_entity_new_at (0.0f, 0.0f);
    g_autoptr(GrlEntity) e2 = grl_entity_new_at (25.0f, 25.0f);

    grl_entity_set_width (e1, 50.0f);
    grl_entity_set_height (e1, 50.0f);
    grl_entity_set_width (e2, 50.0f);
    grl_entity_set_height (e2, 50.0f);

    g_assert_true (grl_collidable_check_collision (GRL_COLLIDABLE (e1),
                                                   GRL_COLLIDABLE (e2)));
}

static void
test_collidable_check_collision_false (void)
{
    g_autoptr(GrlEntity) e1 = grl_entity_new_at (0.0f, 0.0f);
    g_autoptr(GrlEntity) e2 = grl_entity_new_at (100.0f, 100.0f);

    grl_entity_set_width (e1, 50.0f);
    grl_entity_set_height (e1, 50.0f);
    grl_entity_set_width (e2, 50.0f);
    grl_entity_set_height (e2, 50.0f);

    g_assert_false (grl_collidable_check_collision (GRL_COLLIDABLE (e1),
                                                    GRL_COLLIDABLE (e2)));
}

/*
 * Test interface inheritance/casting
 */

static void
test_entity_cast_to_interfaces (void)
{
    g_autoptr(GrlEntity) entity = grl_entity_new ();

    /* Should be able to cast to all interfaces */
    GrlDrawable *drawable = GRL_DRAWABLE (entity);
    GrlUpdatable *updatable = GRL_UPDATABLE (entity);
    GrlCollidable *collidable = GRL_COLLIDABLE (entity);

    g_assert_nonnull (drawable);
    g_assert_nonnull (updatable);
    g_assert_nonnull (collidable);

    /* Casts should all point to the same underlying object */
    g_assert_true (G_OBJECT (drawable) == G_OBJECT (entity));
    g_assert_true (G_OBJECT (updatable) == G_OBJECT (entity));
    g_assert_true (G_OBJECT (collidable) == G_OBJECT (entity));
}

static void
test_drawable_updatable_consistency (void)
{
    g_autoptr(GrlEntity) entity = grl_entity_new ();

    /* Setting via drawable should be visible via entity */
    grl_drawable_set_visible (GRL_DRAWABLE (entity), FALSE);
    g_assert_false (grl_entity_get_visible (entity));

    /* Setting via updatable should be visible via entity */
    grl_updatable_set_active (GRL_UPDATABLE (entity), FALSE);
    g_assert_false (grl_entity_get_active (entity));
}

/*
 * Test registration
 */

int
main (int   argc,
      char *argv[])
{
    g_test_init (&argc, &argv, NULL);

    /* GrlDrawable interface */
    g_test_add_func ("/interfaces/drawable-type", test_drawable_type);
    g_test_add_func ("/interfaces/entity-is-drawable", test_entity_is_drawable);
    g_test_add_func ("/interfaces/drawable-get-visible", test_drawable_get_visible);
    g_test_add_func ("/interfaces/drawable-set-visible", test_drawable_set_visible);
    g_test_add_func ("/interfaces/drawable-get-z-index", test_drawable_get_z_index);
    g_test_add_func ("/interfaces/drawable-set-z-index", test_drawable_set_z_index);

    /* GrlUpdatable interface */
    g_test_add_func ("/interfaces/updatable-type", test_updatable_type);
    g_test_add_func ("/interfaces/entity-is-updatable", test_entity_is_updatable);
    g_test_add_func ("/interfaces/updatable-get-active", test_updatable_get_active);
    g_test_add_func ("/interfaces/updatable-set-active", test_updatable_set_active);

    /* GrlCollidable interface */
    g_test_add_func ("/interfaces/collidable-type", test_collidable_type);
    g_test_add_func ("/interfaces/entity-is-collidable", test_entity_is_collidable);
    g_test_add_func ("/interfaces/collidable-get-bounds", test_collidable_get_bounds);
    g_test_add_func ("/interfaces/collidable-check-collision-true", test_collidable_check_collision_true);
    g_test_add_func ("/interfaces/collidable-check-collision-false", test_collidable_check_collision_false);

    /* Cross-interface tests */
    g_test_add_func ("/interfaces/entity-cast-to-interfaces", test_entity_cast_to_interfaces);
    g_test_add_func ("/interfaces/drawable-updatable-consistency", test_drawable_updatable_consistency);

    return g_test_run ();
}

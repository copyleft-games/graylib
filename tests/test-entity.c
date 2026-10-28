/* test-entity.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Unit tests for GrlEntity.
 *
 * Note: Tests that require an active window/GPU context are skipped.
 * This tests property getters/setters and non-drawing methods only.
 */

#include <glib.h>
#include <math.h>
#include "src/scene/grl-entity.h"
#include "src/scene/grl-updatable.h"
#include "src/scene/grl-drawable.h"
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
test_entity_new (void)
{
    g_autoptr(GrlEntity) entity = grl_entity_new ();

    g_assert_nonnull (entity);
}

static void
test_entity_new_default_position (void)
{
    g_autoptr(GrlEntity) entity = grl_entity_new ();
    g_autoptr(GrlVector2) pos = grl_entity_get_position (entity);

    g_assert_nonnull (pos);
    g_assert_true (float_equal (pos->x, 0.0f));
    g_assert_true (float_equal (pos->y, 0.0f));
}

static void
test_entity_new_at (void)
{
    g_autoptr(GrlEntity) entity = grl_entity_new_at (100.0f, 200.0f);
    g_autoptr(GrlVector2) pos = grl_entity_get_position (entity);

    g_assert_nonnull (entity);
    g_assert_true (float_equal (pos->x, 100.0f));
    g_assert_true (float_equal (pos->y, 200.0f));
}

/*
 * Test position
 */

static void
test_entity_set_position (void)
{
    g_autoptr(GrlEntity) entity = grl_entity_new ();
    g_autoptr(GrlVector2) new_pos = grl_vector2_new (50.0f, 75.0f);

    grl_entity_set_position (entity, new_pos);

    g_autoptr(GrlVector2) pos = grl_entity_get_position (entity);
    g_assert_true (float_equal (pos->x, 50.0f));
    g_assert_true (float_equal (pos->y, 75.0f));
}

static void
test_entity_set_position_xy (void)
{
    g_autoptr(GrlEntity) entity = grl_entity_new ();

    grl_entity_set_position_xy (entity, 150.0f, 250.0f);

    g_assert_true (float_equal (grl_entity_get_x (entity), 150.0f));
    g_assert_true (float_equal (grl_entity_get_y (entity), 250.0f));
}

static void
test_entity_get_set_x (void)
{
    g_autoptr(GrlEntity) entity = grl_entity_new ();

    grl_entity_set_x (entity, 42.0f);

    g_assert_true (float_equal (grl_entity_get_x (entity), 42.0f));
}

static void
test_entity_get_set_y (void)
{
    g_autoptr(GrlEntity) entity = grl_entity_new ();

    grl_entity_set_y (entity, 84.0f);

    g_assert_true (float_equal (grl_entity_get_y (entity), 84.0f));
}

/*
 * Test size
 */

static void
test_entity_get_set_width (void)
{
    g_autoptr(GrlEntity) entity = grl_entity_new ();

    grl_entity_set_width (entity, 64.0f);

    g_assert_true (float_equal (grl_entity_get_width (entity), 64.0f));
}

static void
test_entity_get_set_height (void)
{
    g_autoptr(GrlEntity) entity = grl_entity_new ();

    grl_entity_set_height (entity, 128.0f);

    g_assert_true (float_equal (grl_entity_get_height (entity), 128.0f));
}

/*
 * Test rotation and scale
 */

static void
test_entity_get_set_rotation (void)
{
    g_autoptr(GrlEntity) entity = grl_entity_new ();

    grl_entity_set_rotation (entity, 45.0f);

    g_assert_true (float_equal (grl_entity_get_rotation (entity), 45.0f));
}

static void
test_entity_get_set_scale (void)
{
    g_autoptr(GrlEntity) entity = grl_entity_new ();

    grl_entity_set_scale (entity, 2.0f);

    g_assert_true (float_equal (grl_entity_get_scale (entity), 2.0f));
}

static void
test_entity_default_scale (void)
{
    g_autoptr(GrlEntity) entity = grl_entity_new ();

    /* Default scale should be 1.0 */
    g_assert_true (float_equal (grl_entity_get_scale (entity), 1.0f));
}

/*
 * Test origin
 */

static void
test_entity_get_set_origin (void)
{
    g_autoptr(GrlEntity) entity = grl_entity_new ();
    g_autoptr(GrlVector2) new_origin = grl_vector2_new (16.0f, 16.0f);

    grl_entity_set_origin (entity, new_origin);

    g_autoptr(GrlVector2) origin = grl_entity_get_origin (entity);
    g_assert_true (float_equal (origin->x, 16.0f));
    g_assert_true (float_equal (origin->y, 16.0f));
}

static void
test_entity_center_origin (void)
{
    g_autoptr(GrlEntity) entity = grl_entity_new ();

    grl_entity_set_width (entity, 100.0f);
    grl_entity_set_height (entity, 50.0f);
    grl_entity_center_origin (entity);

    g_autoptr(GrlVector2) origin = grl_entity_get_origin (entity);
    g_assert_true (float_equal (origin->x, 50.0f));
    g_assert_true (float_equal (origin->y, 25.0f));
}

/*
 * Test movement
 */

static void
test_entity_move_xy (void)
{
    g_autoptr(GrlEntity) entity = grl_entity_new_at (100.0f, 100.0f);

    /* Move at 10 units/sec for 1 second */
    grl_entity_move_xy (entity, 10.0f, 5.0f, 1.0f);

    g_assert_true (float_equal (grl_entity_get_x (entity), 110.0f));
    g_assert_true (float_equal (grl_entity_get_y (entity), 105.0f));
}

static void
test_entity_move_xy_half_delta (void)
{
    g_autoptr(GrlEntity) entity = grl_entity_new_at (0.0f, 0.0f);

    /* Move at 100 units/sec for 0.5 seconds */
    grl_entity_move_xy (entity, 100.0f, 100.0f, 0.5f);

    g_assert_true (float_equal (grl_entity_get_x (entity), 50.0f));
    g_assert_true (float_equal (grl_entity_get_y (entity), 50.0f));
}

static void
test_entity_translate (void)
{
    g_autoptr(GrlEntity) entity = grl_entity_new_at (100.0f, 100.0f);
    g_autoptr(GrlVector2) offset = grl_vector2_new (25.0f, -25.0f);

    grl_entity_translate (entity, offset);

    g_assert_true (float_equal (grl_entity_get_x (entity), 125.0f));
    g_assert_true (float_equal (grl_entity_get_y (entity), 75.0f));
}

/*
 * Test state
 */

static void
test_entity_default_visible (void)
{
    g_autoptr(GrlEntity) entity = grl_entity_new ();

    /* Default should be visible */
    g_assert_true (grl_entity_get_visible (entity));
}

static void
test_entity_set_visible_false (void)
{
    g_autoptr(GrlEntity) entity = grl_entity_new ();

    grl_entity_set_visible (entity, FALSE);

    g_assert_false (grl_entity_get_visible (entity));
}

static void
test_entity_default_active (void)
{
    g_autoptr(GrlEntity) entity = grl_entity_new ();

    /* Default should be active */
    g_assert_true (grl_entity_get_active (entity));
}

static void
test_entity_set_active_false (void)
{
    g_autoptr(GrlEntity) entity = grl_entity_new ();

    grl_entity_set_active (entity, FALSE);

    g_assert_false (grl_entity_get_active (entity));
}

static void
test_entity_get_set_z_index (void)
{
    g_autoptr(GrlEntity) entity = grl_entity_new ();

    grl_entity_set_z_index (entity, 10);

    g_assert_cmpint (grl_entity_get_z_index (entity), ==, 10);
}

static void
test_entity_z_index_negative (void)
{
    g_autoptr(GrlEntity) entity = grl_entity_new ();

    grl_entity_set_z_index (entity, -5);

    g_assert_cmpint (grl_entity_get_z_index (entity), ==, -5);
}

/*
 * Test tag system
 */

static void
test_entity_default_tag_null (void)
{
    g_autoptr(GrlEntity) entity = grl_entity_new ();

    g_assert_null (grl_entity_get_tag (entity));
}

static void
test_entity_set_tag (void)
{
    g_autoptr(GrlEntity) entity = grl_entity_new ();

    grl_entity_set_tag (entity, "player");

    g_assert_cmpstr (grl_entity_get_tag (entity), ==, "player");
}

static void
test_entity_has_tag_true (void)
{
    g_autoptr(GrlEntity) entity = grl_entity_new ();

    grl_entity_set_tag (entity, "enemy");

    g_assert_true (grl_entity_has_tag (entity, "enemy"));
}

static void
test_entity_has_tag_false (void)
{
    g_autoptr(GrlEntity) entity = grl_entity_new ();

    grl_entity_set_tag (entity, "enemy");

    g_assert_false (grl_entity_has_tag (entity, "player"));
}

static void
test_entity_has_tag_null (void)
{
    g_autoptr(GrlEntity) entity = grl_entity_new ();

    /* Entity with no tag */
    g_assert_false (grl_entity_has_tag (entity, "anything"));
}

static void
test_entity_clear_tag (void)
{
    g_autoptr(GrlEntity) entity = grl_entity_new ();

    grl_entity_set_tag (entity, "player");
    grl_entity_set_tag (entity, NULL);

    g_assert_null (grl_entity_get_tag (entity));
}

/*
 * Cached instance checks and interface dispatch
 *
 * The entity accessors and the GrlUpdatable/GrlDrawable entry points
 * cache the last instance type that passed their check, and update()/
 * draw() look the vtable up once for both of their calls. A subclass
 * (which misses GLib's exact-type fast path) must be accepted, anything
 * else still rejected after a cached pass, and a subclass that
 * re-implements an interface must still be dispatched through its own
 * vtable.
 */

#define TEST_TYPE_SUB_ENTITY (test_sub_entity_get_type ())
G_DECLARE_FINAL_TYPE (TestSubEntity, test_sub_entity, TEST, SUB_ENTITY, GrlEntity)

struct _TestSubEntity
{
    GrlEntity parent_instance;
    guint     updates;
    guint     draws;
};

G_DEFINE_TYPE (TestSubEntity, test_sub_entity, GRL_TYPE_ENTITY)

static void
test_sub_entity_update (GrlEntity *entity,
                        gfloat     delta)
{
    ((TestSubEntity *) entity)->updates++;
}

static void
test_sub_entity_draw (GrlEntity *entity)
{
    ((TestSubEntity *) entity)->draws++;
}

static void
test_sub_entity_class_init (TestSubEntityClass *klass)
{
    GRL_ENTITY_CLASS (klass)->update = test_sub_entity_update;
    GRL_ENTITY_CLASS (klass)->draw = test_sub_entity_draw;
}

static void
test_sub_entity_init (TestSubEntity *self)
{
}

/* Re-implements both interfaces: never active, never visible */
#define TEST_TYPE_HIDDEN_ENTITY (test_hidden_entity_get_type ())
G_DECLARE_FINAL_TYPE (TestHiddenEntity, test_hidden_entity, TEST, HIDDEN_ENTITY, GrlEntity)

struct _TestHiddenEntity
{
    GrlEntity parent_instance;
    guint     calls;
};

static gboolean
test_hidden_entity_never (GrlUpdatable *updatable)
{
    return FALSE;
}

static gboolean
test_hidden_entity_never_visible (GrlDrawable *drawable)
{
    return FALSE;
}

static void
test_hidden_entity_update (GrlUpdatable *updatable,
                           gfloat        delta)
{
    ((TestHiddenEntity *) updatable)->calls++;
}

static void
test_hidden_entity_draw (GrlDrawable *drawable)
{
    ((TestHiddenEntity *) drawable)->calls++;
}

static void
test_hidden_entity_updatable_init (GrlUpdatableInterface *iface)
{
    iface->update = test_hidden_entity_update;
    iface->get_active = test_hidden_entity_never;
}

static void
test_hidden_entity_drawable_init (GrlDrawableInterface *iface)
{
    iface->draw = test_hidden_entity_draw;
    iface->get_visible = test_hidden_entity_never_visible;
}

G_DEFINE_TYPE_WITH_CODE (TestHiddenEntity, test_hidden_entity, GRL_TYPE_ENTITY,
    G_IMPLEMENT_INTERFACE (GRL_TYPE_UPDATABLE, test_hidden_entity_updatable_init)
    G_IMPLEMENT_INTERFACE (GRL_TYPE_DRAWABLE, test_hidden_entity_drawable_init))

static void
test_hidden_entity_class_init (TestHiddenEntityClass *klass)
{
}

static void
test_hidden_entity_init (TestHiddenEntity *self)
{
}

static void
test_entity_cached_type_checks (void)
{
    g_autoptr(TestSubEntity) sub = g_object_new (TEST_TYPE_SUB_ENTITY, NULL);
    g_autoptr(GObject) plain = g_object_new (G_TYPE_OBJECT, NULL);

    /* Subclass instances pass, repeatedly (the second hits the cache) */
    grl_entity_set_width (GRL_ENTITY (sub), 3.0f);
    g_assert_true (float_equal (grl_entity_get_width (GRL_ENTITY (sub)), 3.0f));
    g_assert_true (float_equal (grl_entity_get_width (GRL_ENTITY (sub)), 3.0f));
    grl_updatable_update (GRL_UPDATABLE (sub), 0.1f);
    grl_updatable_update (GRL_UPDATABLE (sub), 0.1f);
    g_assert_cmpuint (sub->updates, ==, 2);
    grl_drawable_draw (GRL_DRAWABLE (sub));
    g_assert_cmpuint (sub->draws, ==, 1);

    /* Inactive / invisible through the entity's own interface state */
    grl_entity_set_active (GRL_ENTITY (sub), FALSE);
    grl_updatable_update (GRL_UPDATABLE (sub), 0.1f);
    g_assert_cmpuint (sub->updates, ==, 2);
    grl_entity_set_visible (GRL_ENTITY (sub), FALSE);
    grl_drawable_draw (GRL_DRAWABLE (sub));
    g_assert_cmpuint (sub->draws, ==, 1);

    /* Non-conforming instances are still refused after a cached pass */
    g_test_expect_message (NULL, G_LOG_LEVEL_CRITICAL, "*GRL_IS_ENTITY*");
    g_assert_true (float_equal (grl_entity_get_width ((GrlEntity *) plain), 0.0f));
    g_test_assert_expected_messages ();
    g_test_expect_message (NULL, G_LOG_LEVEL_CRITICAL, "*GRL_IS_ENTITY*");
    g_assert_false (grl_entity_get_visible (NULL));
    g_test_assert_expected_messages ();
    g_test_expect_message (NULL, G_LOG_LEVEL_CRITICAL, "*GRL_IS_UPDATABLE*");
    grl_updatable_update ((GrlUpdatable *) plain, 0.1f);
    g_test_assert_expected_messages ();
    g_test_expect_message (NULL, G_LOG_LEVEL_CRITICAL, "*GRL_IS_DRAWABLE*");
    grl_drawable_draw ((GrlDrawable *) plain);
    g_test_assert_expected_messages ();
}

static void
test_entity_interface_override (void)
{
    g_autoptr(TestHiddenEntity) hidden = g_object_new (TEST_TYPE_HIDDEN_ENTITY, NULL);

    /* The entity's own flags say active and visible; the re-implemented
     * interfaces say neither, and they are what update()/draw() ask. */
    g_assert_true (grl_entity_get_active (GRL_ENTITY (hidden)));
    g_assert_true (grl_entity_get_visible (GRL_ENTITY (hidden)));
    g_assert_false (grl_updatable_get_active (GRL_UPDATABLE (hidden)));
    g_assert_false (grl_drawable_get_visible (GRL_DRAWABLE (hidden)));
    grl_updatable_update (GRL_UPDATABLE (hidden), 0.1f);
    grl_drawable_draw (GRL_DRAWABLE (hidden));
    g_assert_cmpuint (hidden->calls, ==, 0);
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
    g_test_add_func ("/entity/new", test_entity_new);
    g_test_add_func ("/entity/new-default-position", test_entity_new_default_position);
    g_test_add_func ("/entity/new-at", test_entity_new_at);

    /* Position */
    g_test_add_func ("/entity/set-position", test_entity_set_position);
    g_test_add_func ("/entity/set-position-xy", test_entity_set_position_xy);
    g_test_add_func ("/entity/get-set-x", test_entity_get_set_x);
    g_test_add_func ("/entity/get-set-y", test_entity_get_set_y);

    /* Size */
    g_test_add_func ("/entity/get-set-width", test_entity_get_set_width);
    g_test_add_func ("/entity/get-set-height", test_entity_get_set_height);

    /* Rotation and Scale */
    g_test_add_func ("/entity/get-set-rotation", test_entity_get_set_rotation);
    g_test_add_func ("/entity/get-set-scale", test_entity_get_set_scale);
    g_test_add_func ("/entity/default-scale", test_entity_default_scale);

    /* Origin */
    g_test_add_func ("/entity/get-set-origin", test_entity_get_set_origin);
    g_test_add_func ("/entity/center-origin", test_entity_center_origin);

    /* Movement */
    g_test_add_func ("/entity/move-xy", test_entity_move_xy);
    g_test_add_func ("/entity/move-xy-half-delta", test_entity_move_xy_half_delta);
    g_test_add_func ("/entity/translate", test_entity_translate);

    /* State */
    g_test_add_func ("/entity/default-visible", test_entity_default_visible);
    g_test_add_func ("/entity/set-visible-false", test_entity_set_visible_false);
    g_test_add_func ("/entity/default-active", test_entity_default_active);
    g_test_add_func ("/entity/set-active-false", test_entity_set_active_false);
    g_test_add_func ("/entity/get-set-z-index", test_entity_get_set_z_index);
    g_test_add_func ("/entity/z-index-negative", test_entity_z_index_negative);

    /* Tags */
    g_test_add_func ("/entity/default-tag-null", test_entity_default_tag_null);
    g_test_add_func ("/entity/set-tag", test_entity_set_tag);
    g_test_add_func ("/entity/has-tag-true", test_entity_has_tag_true);
    g_test_add_func ("/entity/has-tag-false", test_entity_has_tag_false);
    g_test_add_func ("/entity/has-tag-null", test_entity_has_tag_null);
    g_test_add_func ("/entity/clear-tag", test_entity_clear_tag);

    /* Cached checks and interface dispatch */
    g_test_add_func ("/entity/cached-type-checks", test_entity_cached_type_checks);
    g_test_add_func ("/entity/interface-override", test_entity_interface_override);

    return g_test_run ();
}

/* test-scene.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Unit tests for GrlScene.
 *
 * Note: Tests that require an active window/GPU context are skipped.
 * This tests property getters/setters and entity management only.
 */

#include <glib.h>
#include "src/scene/grl-scene.h"
#include "src/scene/grl-entity.h"

/*
 * Test constructors
 */

static void
test_scene_new (void)
{
    g_autoptr(GrlScene) scene = grl_scene_new ();

    g_assert_nonnull (scene);
}

static void
test_scene_new_with_name (void)
{
    g_autoptr(GrlScene) scene = grl_scene_new_with_name ("TestScene");

    g_assert_nonnull (scene);
    g_assert_cmpstr (grl_scene_get_name (scene), ==, "TestScene");
}

/*
 * Test properties
 */

static void
test_scene_get_set_name (void)
{
    g_autoptr(GrlScene) scene = grl_scene_new ();

    grl_scene_set_name (scene, "MyScene");

    g_assert_cmpstr (grl_scene_get_name (scene), ==, "MyScene");
}

static void
test_scene_default_visible (void)
{
    g_autoptr(GrlScene) scene = grl_scene_new ();

    g_assert_true (grl_scene_get_visible (scene));
}

static void
test_scene_set_visible (void)
{
    g_autoptr(GrlScene) scene = grl_scene_new ();

    grl_scene_set_visible (scene, FALSE);

    g_assert_false (grl_scene_get_visible (scene));
}

static void
test_scene_update_when_paused_default (void)
{
    g_autoptr(GrlScene) scene = grl_scene_new ();

    g_assert_false (grl_scene_get_update_when_paused (scene));
}

static void
test_scene_set_update_when_paused (void)
{
    g_autoptr(GrlScene) scene = grl_scene_new ();

    grl_scene_set_update_when_paused (scene, TRUE);

    g_assert_true (grl_scene_get_update_when_paused (scene));
}

static void
test_scene_draw_when_paused_default (void)
{
    g_autoptr(GrlScene) scene = grl_scene_new ();

    /* Default is TRUE - scenes draw even when paused */
    g_assert_true (grl_scene_get_draw_when_paused (scene));
}

static void
test_scene_set_draw_when_paused (void)
{
    g_autoptr(GrlScene) scene = grl_scene_new ();

    /* Default is TRUE, so test setting to FALSE */
    grl_scene_set_draw_when_paused (scene, FALSE);

    g_assert_false (grl_scene_get_draw_when_paused (scene));
}

/*
 * Test entity management
 */

static void
test_scene_add_entity (void)
{
    g_autoptr(GrlScene) scene = grl_scene_new ();
    g_autoptr(GrlEntity) entity = grl_entity_new ();

    grl_scene_add_entity (scene, entity);

    g_assert_cmpuint (grl_scene_get_entity_count (scene), ==, 1);
}

static void
test_scene_add_multiple_entities (void)
{
    g_autoptr(GrlScene) scene = grl_scene_new ();
    g_autoptr(GrlEntity) e1 = grl_entity_new ();
    g_autoptr(GrlEntity) e2 = grl_entity_new ();
    g_autoptr(GrlEntity) e3 = grl_entity_new ();

    grl_scene_add_entity (scene, e1);
    grl_scene_add_entity (scene, e2);
    grl_scene_add_entity (scene, e3);

    g_assert_cmpuint (grl_scene_get_entity_count (scene), ==, 3);
}

static void
test_scene_remove_entity (void)
{
    g_autoptr(GrlScene) scene = grl_scene_new ();
    g_autoptr(GrlEntity) entity = grl_entity_new ();

    grl_scene_add_entity (scene, entity);
    g_assert_cmpuint (grl_scene_get_entity_count (scene), ==, 1);

    grl_scene_remove_entity (scene, entity);
    g_assert_cmpuint (grl_scene_get_entity_count (scene), ==, 0);
}

static void
test_scene_clear_entities (void)
{
    g_autoptr(GrlScene) scene = grl_scene_new ();
    g_autoptr(GrlEntity) e1 = grl_entity_new ();
    g_autoptr(GrlEntity) e2 = grl_entity_new ();

    grl_scene_add_entity (scene, e1);
    grl_scene_add_entity (scene, e2);
    g_assert_cmpuint (grl_scene_get_entity_count (scene), ==, 2);

    grl_scene_clear_entities (scene);
    g_assert_cmpuint (grl_scene_get_entity_count (scene), ==, 0);
}

static void
test_scene_get_entities (void)
{
    g_autoptr(GrlScene) scene = grl_scene_new ();
    g_autoptr(GrlEntity) e1 = grl_entity_new ();
    g_autoptr(GrlEntity) e2 = grl_entity_new ();

    grl_scene_add_entity (scene, e1);
    grl_scene_add_entity (scene, e2);

    GList *entities = grl_scene_get_entities (scene);
    g_assert_cmpuint (g_list_length (entities), ==, 2);
}

static void
test_scene_empty_entity_count (void)
{
    g_autoptr(GrlScene) scene = grl_scene_new ();

    g_assert_cmpuint (grl_scene_get_entity_count (scene), ==, 0);
}

/*
 * Test entity lookup by tag
 */

static void
test_scene_find_entity_by_tag (void)
{
    g_autoptr(GrlScene) scene = grl_scene_new ();
    g_autoptr(GrlEntity) player = grl_entity_new ();
    g_autoptr(GrlEntity) enemy = grl_entity_new ();

    grl_entity_set_tag (player, "player");
    grl_entity_set_tag (enemy, "enemy");

    grl_scene_add_entity (scene, player);
    grl_scene_add_entity (scene, enemy);

    GrlEntity *found = grl_scene_find_entity_by_tag (scene, "player");
    g_assert_nonnull (found);
    g_assert_true (found == player);
}

static void
test_scene_find_entity_by_tag_not_found (void)
{
    g_autoptr(GrlScene) scene = grl_scene_new ();
    g_autoptr(GrlEntity) entity = grl_entity_new ();

    grl_entity_set_tag (entity, "enemy");
    grl_scene_add_entity (scene, entity);

    GrlEntity *found = grl_scene_find_entity_by_tag (scene, "player");
    g_assert_null (found);
}

static void
test_scene_find_entities_by_tag (void)
{
    g_autoptr(GrlScene) scene = grl_scene_new ();
    g_autoptr(GrlEntity) e1 = grl_entity_new ();
    g_autoptr(GrlEntity) e2 = grl_entity_new ();
    g_autoptr(GrlEntity) e3 = grl_entity_new ();

    grl_entity_set_tag (e1, "enemy");
    grl_entity_set_tag (e2, "enemy");
    grl_entity_set_tag (e3, "player");

    grl_scene_add_entity (scene, e1);
    grl_scene_add_entity (scene, e2);
    grl_scene_add_entity (scene, e3);

    GList *enemies = grl_scene_find_entities_by_tag (scene, "enemy");
    g_assert_cmpuint (g_list_length (enemies), ==, 2);
    g_list_free (enemies);
}

static void
test_scene_find_entities_by_tag_empty (void)
{
    g_autoptr(GrlScene) scene = grl_scene_new ();
    g_autoptr(GrlEntity) entity = grl_entity_new ();

    grl_entity_set_tag (entity, "player");
    grl_scene_add_entity (scene, entity);

    GList *enemies = grl_scene_find_entities_by_tag (scene, "enemy");
    g_assert_null (enemies);
}

/*
 * Test foreach
 */

static gboolean
count_entities_callback (GrlEntity *entity,
                         gpointer   user_data)
{
    gint *count = (gint *)user_data;
    (*count)++;
    return TRUE;
}

static void
test_scene_foreach_entity (void)
{
    g_autoptr(GrlScene) scene = grl_scene_new ();
    g_autoptr(GrlEntity) e1 = grl_entity_new ();
    g_autoptr(GrlEntity) e2 = grl_entity_new ();
    g_autoptr(GrlEntity) e3 = grl_entity_new ();
    gint count = 0;

    grl_scene_add_entity (scene, e1);
    grl_scene_add_entity (scene, e2);
    grl_scene_add_entity (scene, e3);

    grl_scene_foreach_entity (scene, count_entities_callback, &count);

    g_assert_cmpint (count, ==, 3);
}

static gboolean
stop_early_callback (GrlEntity *entity,
                     gpointer   user_data)
{
    gint *count = (gint *)user_data;
    (*count)++;
    return (*count < 2);  /* Stop after 2 */
}

static void
test_scene_foreach_entity_stop_early (void)
{
    g_autoptr(GrlScene) scene = grl_scene_new ();
    g_autoptr(GrlEntity) e1 = grl_entity_new ();
    g_autoptr(GrlEntity) e2 = grl_entity_new ();
    g_autoptr(GrlEntity) e3 = grl_entity_new ();
    gint count = 0;

    grl_scene_add_entity (scene, e1);
    grl_scene_add_entity (scene, e2);
    grl_scene_add_entity (scene, e3);

    grl_scene_foreach_entity (scene, stop_early_callback, &count);

    g_assert_cmpint (count, ==, 2);
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
    g_test_add_func ("/scene/new", test_scene_new);
    g_test_add_func ("/scene/new-with-name", test_scene_new_with_name);

    /* Properties */
    g_test_add_func ("/scene/get-set-name", test_scene_get_set_name);
    g_test_add_func ("/scene/default-visible", test_scene_default_visible);
    g_test_add_func ("/scene/set-visible", test_scene_set_visible);
    g_test_add_func ("/scene/update-when-paused-default", test_scene_update_when_paused_default);
    g_test_add_func ("/scene/set-update-when-paused", test_scene_set_update_when_paused);
    g_test_add_func ("/scene/draw-when-paused-default", test_scene_draw_when_paused_default);
    g_test_add_func ("/scene/set-draw-when-paused", test_scene_set_draw_when_paused);

    /* Entity management */
    g_test_add_func ("/scene/add-entity", test_scene_add_entity);
    g_test_add_func ("/scene/add-multiple-entities", test_scene_add_multiple_entities);
    g_test_add_func ("/scene/remove-entity", test_scene_remove_entity);
    g_test_add_func ("/scene/clear-entities", test_scene_clear_entities);
    g_test_add_func ("/scene/get-entities", test_scene_get_entities);
    g_test_add_func ("/scene/empty-entity-count", test_scene_empty_entity_count);

    /* Entity lookup */
    g_test_add_func ("/scene/find-entity-by-tag", test_scene_find_entity_by_tag);
    g_test_add_func ("/scene/find-entity-by-tag-not-found", test_scene_find_entity_by_tag_not_found);
    g_test_add_func ("/scene/find-entities-by-tag", test_scene_find_entities_by_tag);
    g_test_add_func ("/scene/find-entities-by-tag-empty", test_scene_find_entities_by_tag_empty);

    /* Foreach */
    g_test_add_func ("/scene/foreach-entity", test_scene_foreach_entity);
    g_test_add_func ("/scene/foreach-entity-stop-early", test_scene_foreach_entity_stop_early);

    return g_test_run ();
}

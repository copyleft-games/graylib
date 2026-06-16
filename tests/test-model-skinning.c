/* test-model-skinning.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Unit tests for the raylib-6.0 skeletal-animation model accessors:
 * grl_model_get_bone_count(), grl_model_get_bone(),
 * grl_model_get_bind_pose_transform(), grl_mesh_get_bone_count(), and the
 * error path of grl_model_animation_load().
 *
 * Creating a GrlMesh/GrlModel uploads to VRAM, so it needs a live GL context
 * (and therefore a window and a display). These tests are display-gated and
 * skip cleanly when no display, no GL context, or no model can be created.
 * The guard-path assertions on a skeleton-less cube model verify that the new
 * accessors return 0 / NULL exactly where the API documents they should.
 */

#include <glib.h>
#include "src/core/grl-window.h"
#include "src/graphics/grl-mesh.h"
#include "src/graphics/grl-model.h"
#include "src/graphics/grl-model-animation.h"
#include "src/graphics/grl-bone-info.h"
#include "src/graphics/grl-transform.h"
#include "src/grl-enums.h"

/* Skip if no display available (CI/headless). */
#define SKIP_IF_NO_DISPLAY() \
    do { \
        if (g_getenv ("DISPLAY") == NULL && g_getenv ("WAYLAND_DISPLAY") == NULL) \
        { \
            g_test_skip ("No display available (headless environment)"); \
            return; \
        } \
    } while (0)

/* Skip if a required resource could not be created. */
#define SKIP_IF_NULL(ptr) \
    do { \
        if ((ptr) == NULL) \
        { \
            g_test_skip ("Resource not available in this environment"); \
            return; \
        } \
    } while (0)

/*
 * Skeleton-less model: a cube mesh promoted to a model has no skeleton, so all
 * the new skeletal accessors must report "no bones" and the bone/pose getters
 * must return NULL for every index, including out-of-range and negative ones.
 */

static void
test_model_skinning_cube_has_no_bones (void)
{
    g_autoptr(GrlWindow) window = NULL;
    g_autoptr(GrlMesh)   mesh = NULL;
    g_autoptr(GrlModel)  model = NULL;

    SKIP_IF_NO_DISPLAY ();

    /* A GL context is required to generate and upload the mesh. If the window
     * isn't GL-ready (e.g. no usable driver), skip rather than fail. */
    window = grl_window_new (64, 48, "test-model-skinning");
    SKIP_IF_NULL (window);
    if (!grl_window_is_ready (window))
    {
        g_test_skip ("Window/GL context not ready in this environment");
        return;
    }

    mesh = grl_mesh_new_cube (1.0f, 1.0f, 1.0f);
    SKIP_IF_NULL (mesh);
    if (!grl_mesh_is_valid (mesh))
    {
        g_test_skip ("Cube mesh could not be created");
        return;
    }

    /* A primitive mesh carries no skinning data. */
    g_assert_cmpint (grl_mesh_get_bone_count (mesh), ==, 0);

    model = grl_model_new_from_mesh (mesh);
    SKIP_IF_NULL (model);
    if (!grl_model_is_valid (model))
    {
        g_test_skip ("Model could not be created from mesh");
        return;
    }

    /* No skeleton => zero bones. */
    g_assert_cmpint (grl_model_get_bone_count (model), ==, 0);

    /* Every bone/pose query must return NULL: index 0 (no skeleton at all),
     * a negative index, and a large out-of-range index. */
    g_assert_null (grl_model_get_bone (model, 0));
    g_assert_null (grl_model_get_bone (model, -1));
    g_assert_null (grl_model_get_bone (model, 1));
    g_assert_null (grl_model_get_bone (model, 99999));

    g_assert_null (grl_model_get_bind_pose_transform (model, 0));
    g_assert_null (grl_model_get_bind_pose_transform (model, -1));
    g_assert_null (grl_model_get_bind_pose_transform (model, 99999));
}

/*
 * Error path: loading animations from a nonexistent file returns NULL with the
 * error set and the out count zeroed.
 */

static void
test_model_skinning_animation_load_missing (void)
{
    g_autoptr(GError)    error = NULL;
    GrlModelAnimation  **anims;
    gint                 count = -1;

    SKIP_IF_NO_DISPLAY ();

    anims = grl_model_animation_load ("/nonexistent-graylib-test.iqm",
                                      &count, &error);

    g_assert_null (anims);
    g_assert_cmpint (count, ==, 0);
    g_assert_nonnull (error);
}

/*
 * Test registration
 */

int
main (int   argc,
      char *argv[])
{
    g_test_init (&argc, &argv, NULL);

    g_test_add_func ("/model-skinning/cube-has-no-bones",
                     test_model_skinning_cube_has_no_bones);
    g_test_add_func ("/model-skinning/animation-load-missing",
                     test_model_skinning_animation_load_missing);

    return g_test_run ();
}

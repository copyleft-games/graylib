/* test-model-skinning.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Unit tests for the raylib-6.0 skeletal-animation model accessors:
 * grl_model_get_bone_count(), grl_model_get_bone(),
 * grl_model_get_bind_pose_transform(), grl_mesh_get_bone_count(), the
 * error path of grl_model_animation_load(), and the GPU skinning helpers
 * (grl_model_animation_update_bones(), grl_model_animation_blend_bones(),
 * grl_model_prepare_gpu_skinning()) on a one-bone glTF written by the test.
 *
 * Creating a GrlMesh/GrlModel uploads to VRAM, so it needs a live GL context
 * (and therefore a window and a display). These tests are display-gated and
 * skip cleanly when no display, no GL context, or no model can be created.
 * The guard-path assertions on a skeleton-less cube model verify that the new
 * accessors return 0 / NULL exactly where the API documents they should.
 */

#include <glib.h>
#include <glib/gstdio.h>
#include <string.h>
#include <raylib.h>
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
 * GPU skinning helpers. The fixture is one triangle skinned to one joint
 * whose single animation lifts the joint by 1 on Y over 0.5 s (31 raylib
 * keyframes), so keyframe 30 has the joint fully up.
 */

static void
put_u32 (GByteArray *out,
         guint32     value)
{
    guint8 bytes[4];

    bytes[0] = (guint8)(value & 0xff);
    bytes[1] = (guint8)((value >> 8) & 0xff);
    bytes[2] = (guint8)((value >> 16) & 0xff);
    bytes[3] = (guint8)((value >> 24) & 0xff);
    g_byte_array_append (out, bytes, 4);
}

static void
write_lift_glb (const gchar *path)
{
    static const gchar json[] =
        "{\"asset\":{\"version\":\"2.0\"},\"scene\":0,\"scenes\":[{\"nodes\":[0]}],"
        "\"nodes\":[{\"name\":\"Armature\",\"children\":[1,2]},{\"name\":\"Bone\"},"
        "{\"name\":\"Body\",\"mesh\":0,\"skin\":0}],"
        "\"meshes\":[{\"primitives\":[{\"attributes\":{\"POSITION\":0,\"JOINTS_0\":4,"
        "\"WEIGHTS_0\":5},\"indices\":1}]}],"
        "\"skins\":[{\"joints\":[1]}],"
        "\"animations\":[{\"name\":\"Lift\",\"channels\":[{\"sampler\":0,"
        "\"target\":{\"node\":1,\"path\":\"translation\"}}],"
        "\"samplers\":[{\"input\":2,\"output\":3}]}],"
        "\"buffers\":[{\"byteLength\":136}],"
        "\"bufferViews\":[{\"buffer\":0,\"byteOffset\":0,\"byteLength\":36},"
        "{\"buffer\":0,\"byteOffset\":36,\"byteLength\":6},"
        "{\"buffer\":0,\"byteOffset\":44,\"byteLength\":8},"
        "{\"buffer\":0,\"byteOffset\":52,\"byteLength\":24},"
        "{\"buffer\":0,\"byteOffset\":76,\"byteLength\":12},"
        "{\"buffer\":0,\"byteOffset\":88,\"byteLength\":48}],"
        "\"accessors\":[{\"bufferView\":0,\"componentType\":5126,\"count\":3,\"type\":\"VEC3\","
        "\"min\":[0,0,0],\"max\":[1,1,0]},"
        "{\"bufferView\":1,\"componentType\":5123,\"count\":3,\"type\":\"SCALAR\"},"
        "{\"bufferView\":2,\"componentType\":5126,\"count\":2,\"type\":\"SCALAR\",\"min\":[0],\"max\":[0.5]},"
        "{\"bufferView\":3,\"componentType\":5126,\"count\":2,\"type\":\"VEC3\"},"
        "{\"bufferView\":4,\"componentType\":5121,\"count\":3,\"type\":\"VEC4\"},"
        "{\"bufferView\":5,\"componentType\":5126,\"count\":3,\"type\":\"VEC4\"}]}";
    static const gfloat  positions[9] = { 0, 0, 0, 1, 0, 0, 0, 1, 0 };
    static const guint16 indices[4] = { 0, 1, 2, 0 };
    static const gfloat  times[2] = { 0.0f, 0.5f };
    static const gfloat  moves[6] = { 0, 0, 0, 0, 1, 0 };
    static const gfloat  weights[12] = { 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 };
    g_autoptr(GError)    error = NULL;
    GByteArray          *out = g_byte_array_new ();
    guint8               bin[136];
    gsize                json_len = strlen (json);
    gsize                json_padded = (json_len + 3) & ~(gsize)3;
    gsize                i;

    memset (bin, 0, sizeof bin);
    memcpy (bin, positions, 36);
    memcpy (bin + 36, indices, 8);
    memcpy (bin + 44, times, 8);
    memcpy (bin + 52, moves, 24);
    memcpy (bin + 88, weights, 48);   /* joints (76..87) stay 0 */

    put_u32 (out, 0x46546C67u);       /* "glTF" */
    put_u32 (out, 2);
    put_u32 (out, (guint32)(12 + 8 + json_padded + 8 + sizeof bin));
    put_u32 (out, (guint32)json_padded);
    put_u32 (out, 0x4E4F534Au);       /* "JSON" */
    g_byte_array_append (out, (const guint8 *)json, (guint)json_len);
    for (i = json_len; i < json_padded; i++)
        g_byte_array_append (out, (const guint8 *)" ", 1);
    put_u32 (out, (guint32)sizeof bin);
    put_u32 (out, 0x004E4942u);       /* "BIN\0" */
    g_byte_array_append (out, bin, sizeof bin);

    g_assert_true (g_file_set_contents (path, (const gchar *)out->data, out->len, &error));
    g_assert_no_error (error);
    g_byte_array_unref (out);
}

static void
test_model_skinning_gpu_helpers (void)
{
    g_autoptr(GrlWindow) window = NULL;
    g_autoptr(GrlModel)  model = NULL;
    g_autoptr(GError)    error = NULL;
    g_autofree gchar    *path = NULL;
    GrlModelAnimation  **anims;
    Model               *raw;
    Mesh                *mesh;
    gint                 count = 0;
    gint                 i;

    SKIP_IF_NO_DISPLAY ();

    window = grl_window_new (64, 48, "test-model-skinning");
    SKIP_IF_NULL (window);
    if (!grl_window_is_ready (window))
    {
        g_test_skip ("Window/GL context not ready in this environment");
        return;
    }

    path = g_build_filename (g_get_tmp_dir (), "graylib-test-lift.glb", NULL);
    write_lift_glb (path);
    model = grl_model_new_from_file (path, &error);
    g_assert_no_error (error);
    anims = grl_model_animation_load (path, &count, &error);
    g_assert_no_error (error);
    g_assert_cmpint (count, ==, 1);
    g_remove (path);

    raw = grl_model_get_handle (model);
    mesh = &raw->meshes[0];
    g_assert_nonnull (raw->boneMatrices);
    g_assert_nonnull (mesh->boneIndices);
    g_assert_nonnull (mesh->animVertices);
    g_assert_false (grl_model_has_gpu_skinning (model));

    /* Bones only: the joint is up, the vertices stay in bind pose. */
    grl_model_animation_update_bones (anims[0], model, 30.0f);
    g_assert_cmpfloat_with_epsilon (raw->boneMatrices[0].m13, 1.0, 1e-4);
    g_assert_cmpfloat_with_epsilon (mesh->animVertices[7], 1.0, 1e-6);

    grl_model_animation_blend_bones (model, anims[0], 0.0f, anims[0], 30.0f, 0.5f);
    g_assert_cmpfloat_with_epsilon (raw->boneMatrices[0].m13, 0.5, 1e-3);
    g_assert_cmpfloat_with_epsilon (mesh->animVertices[7], 1.0, 1e-6);

    /* CPU skinning moves the vertices (vertex 2 is at y 1 in bind pose). */
    grl_model_animation_update (anims[0], model, 30);
    g_assert_cmpfloat_with_epsilon (mesh->animVertices[7], 2.0, 1e-4);

    /* Preparing for GPU skinning (the desktop build is OpenGL 3.3)
     * restores the bind pose. */
    g_assert_true (grl_model_prepare_gpu_skinning (model));
    g_assert_true (grl_model_has_gpu_skinning (model));
    g_assert_cmpfloat_with_epsilon (mesh->animVertices[7], 1.0, 1e-6);
    /* again: nothing to restore, still ready */
    g_assert_true (grl_model_prepare_gpu_skinning (model));

    for (i = 0; i < count; i++)
        g_object_unref (anims[i]);
    g_free (anims);
}

static void
test_model_skinning_gpu_no_skeleton (void)
{
    g_autoptr(GrlWindow) window = NULL;
    g_autoptr(GrlMesh)   mesh = NULL;
    g_autoptr(GrlModel)  model = NULL;

    SKIP_IF_NO_DISPLAY ();

    window = grl_window_new (64, 48, "test-model-skinning");
    SKIP_IF_NULL (window);
    if (!grl_window_is_ready (window))
    {
        g_test_skip ("Window/GL context not ready in this environment");
        return;
    }

    mesh = grl_mesh_new_cube (1.0f, 1.0f, 1.0f);
    SKIP_IF_NULL (mesh);
    model = grl_model_new_from_mesh (mesh);
    SKIP_IF_NULL (model);

    /* Nothing to skin: never prepared. */
    g_assert_false (grl_model_prepare_gpu_skinning (model));
    g_assert_false (grl_model_has_gpu_skinning (model));
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
    g_test_add_func ("/model-skinning/gpu-helpers",
                     test_model_skinning_gpu_helpers);
    g_test_add_func ("/model-skinning/gpu-no-skeleton",
                     test_model_skinning_gpu_no_skeleton);

    return g_test_run ();
}

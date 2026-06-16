/* test-sound-pan.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Unit tests for GrlSound stereo pan get/set and clamping.
 *
 * A loaded GrlSound normally needs an initialized audio device, but pan is
 * tracked on the GObject itself and clamped on set independently of whether
 * sample data is loaded. An unloaded GrlSound (g_object_new) finalizes
 * cleanly without touching any audio device, so these tests are fully
 * headless. If a sound object cannot be created at all, the tests skip.
 */

#include <glib.h>
#include "src/audio/grl-sound.h"

#define EPSILON 0.0001f

#define SKIP_IF_NULL(ptr) \
    do { \
        if ((ptr) == NULL) \
        { \
            g_test_skip ("Sound object not available in this environment"); \
            return; \
        } \
    } while (0)

/*
 * Test GType registration
 */

static void
test_sound_type (void)
{
    GType type = grl_sound_get_type ();

    g_assert_true (type != G_TYPE_INVALID);
    g_assert_cmpstr (g_type_name (type), ==, "GrlSound");
    g_assert_true (G_TYPE_IS_OBJECT (type));
}

/*
 * Pan defaults to centre (0.0).
 */

static void
test_sound_pan_default (void)
{
    g_autoptr(GrlSound) sound = g_object_new (GRL_TYPE_SOUND, NULL);

    SKIP_IF_NULL (sound);

    g_assert_cmpfloat_with_epsilon (grl_sound_get_pan (sound), 0.0f, EPSILON);
}

/*
 * Pan round-trips an in-range value.
 */

static void
test_sound_pan_roundtrip (void)
{
    g_autoptr(GrlSound) sound = g_object_new (GRL_TYPE_SOUND, NULL);

    SKIP_IF_NULL (sound);

    grl_sound_set_pan (sound, 0.5f);
    g_assert_cmpfloat_with_epsilon (grl_sound_get_pan (sound), 0.5f, EPSILON);
}

/*
 * Pan is clamped to the lower bound of -1.0.
 */

static void
test_sound_pan_clamp_low (void)
{
    g_autoptr(GrlSound) sound = g_object_new (GRL_TYPE_SOUND, NULL);

    SKIP_IF_NULL (sound);

    grl_sound_set_pan (sound, -2.0f);
    g_assert_cmpfloat_with_epsilon (grl_sound_get_pan (sound), -1.0f, EPSILON);
}

/*
 * Pan is clamped to the upper bound of 1.0.
 */

static void
test_sound_pan_clamp_high (void)
{
    g_autoptr(GrlSound) sound = g_object_new (GRL_TYPE_SOUND, NULL);

    SKIP_IF_NULL (sound);

    grl_sound_set_pan (sound, 2.0f);
    g_assert_cmpfloat_with_epsilon (grl_sound_get_pan (sound), 1.0f, EPSILON);
}

/*
 * The exact boundary values -1.0 and 1.0 are valid and pass through unclamped.
 */

static void
test_sound_pan_boundary_low (void)
{
    g_autoptr(GrlSound) sound = g_object_new (GRL_TYPE_SOUND, NULL);

    SKIP_IF_NULL (sound);

    grl_sound_set_pan (sound, -1.0f);
    g_assert_cmpfloat_with_epsilon (grl_sound_get_pan (sound), -1.0f, EPSILON);
}

static void
test_sound_pan_boundary_high (void)
{
    g_autoptr(GrlSound) sound = g_object_new (GRL_TYPE_SOUND, NULL);

    SKIP_IF_NULL (sound);

    grl_sound_set_pan (sound, 1.0f);
    g_assert_cmpfloat_with_epsilon (grl_sound_get_pan (sound), 1.0f, EPSILON);
}

/*
 * The "pan" GObject property get/set path agrees with the typed accessors.
 */

static void
test_sound_pan_property_get (void)
{
    g_autoptr(GrlSound) sound = g_object_new (GRL_TYPE_SOUND, NULL);
    gfloat              prop_pan = -42.0f;

    SKIP_IF_NULL (sound);

    grl_sound_set_pan (sound, -0.25f);
    g_object_get (sound, "pan", &prop_pan, NULL);
    g_assert_cmpfloat_with_epsilon (prop_pan, -0.25f, EPSILON);
    g_assert_cmpfloat_with_epsilon (prop_pan, grl_sound_get_pan (sound), EPSILON);
}

static void
test_sound_pan_property_set (void)
{
    g_autoptr(GrlSound) sound = g_object_new (GRL_TYPE_SOUND, NULL);

    SKIP_IF_NULL (sound);

    g_object_set (sound, "pan", 0.75f, NULL);
    g_assert_cmpfloat_with_epsilon (grl_sound_get_pan (sound), 0.75f, EPSILON);
}

/*
 * Setting the property out of range still clamps (the set path routes through
 * grl_sound_set_pan, which clamps to [-1.0, 1.0]). The param spec range is
 * [-1.0, 1.0], so feed an in-spec value and clamp via the accessor for the
 * out-of-range coverage; for the property path verify the in-spec extreme.
 */

static void
test_sound_pan_property_clamps (void)
{
    g_autoptr(GrlSound) sound = g_object_new (GRL_TYPE_SOUND, NULL);

    SKIP_IF_NULL (sound);

    /* Within the param-spec range; clamping is a no-op here but the path is
     * exercised end-to-end. */
    g_object_set (sound, "pan", -1.0f, NULL);
    g_assert_cmpfloat_with_epsilon (grl_sound_get_pan (sound), -1.0f, EPSILON);
}

/*
 * Test registration
 */

int
main (int   argc,
      char *argv[])
{
    g_test_init (&argc, &argv, NULL);

    g_test_add_func ("/sound-pan/type", test_sound_type);
    g_test_add_func ("/sound-pan/default", test_sound_pan_default);
    g_test_add_func ("/sound-pan/roundtrip", test_sound_pan_roundtrip);
    g_test_add_func ("/sound-pan/clamp-low", test_sound_pan_clamp_low);
    g_test_add_func ("/sound-pan/clamp-high", test_sound_pan_clamp_high);
    g_test_add_func ("/sound-pan/boundary-low", test_sound_pan_boundary_low);
    g_test_add_func ("/sound-pan/boundary-high", test_sound_pan_boundary_high);
    g_test_add_func ("/sound-pan/property-get", test_sound_pan_property_get);
    g_test_add_func ("/sound-pan/property-set", test_sound_pan_property_set);
    g_test_add_func ("/sound-pan/property-clamps", test_sound_pan_property_clamps);

    return g_test_run ();
}

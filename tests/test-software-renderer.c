/* test-software-renderer.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Unit tests for GrlSoftwareRenderer.
 *
 * The rlsw rasterizer is pure CPU: no GL context, no window and no display
 * are required, so these tests run in fully headless environments. If, for
 * some reason, the renderer cannot be created in the running environment,
 * the tests skip gracefully.
 */

#include <glib.h>
#include "src/rlsw/grl-software-renderer.h"
#include "src/graphics/grl-image.h"
#include "src/math/grl-color.h"

#define SKIP_IF_NULL(ptr) \
    do { \
        if ((ptr) == NULL) \
        { \
            g_test_skip ("Software renderer not available in this environment"); \
            return; \
        } \
    } while (0)

/*
 * Test GType registration
 */

static void
test_software_renderer_type (void)
{
    GType type = grl_software_renderer_get_type ();

    g_assert_true (type != G_TYPE_INVALID);
    g_assert_cmpstr (g_type_name (type), ==, "GrlSoftwareRenderer");
    g_assert_true (G_TYPE_IS_OBJECT (type));
}

/*
 * Test constructor and dimension accessors
 */

static void
test_software_renderer_new (void)
{
    g_autoptr(GrlSoftwareRenderer) sw = grl_software_renderer_new (64, 48);

    SKIP_IF_NULL (sw);

    g_assert_true (GRL_IS_SOFTWARE_RENDERER (sw));
    g_assert_cmpint (grl_software_renderer_get_width (sw), ==, 64);
    g_assert_cmpint (grl_software_renderer_get_height (sw), ==, 48);
}

/*
 * Functional pixel check: clear to a known colour, read back into a GrlImage,
 * and verify the image dimensions and a sampled pixel.
 */

static void
test_software_renderer_clear_pixel (void)
{
    g_autoptr(GrlSoftwareRenderer) sw = grl_software_renderer_new (64, 48);
    g_autoptr(GrlImage)            image = NULL;
    g_autoptr(GrlColor)            pixel = NULL;

    SKIP_IF_NULL (sw);

    /*
     * Clear to an ASYMMETRIC colour (r=255, g~=128, b=0) so the check also
     * verifies correct RGBA channel order: the wrapper compiles rlsw with
     * SW_FRAMEBUFFER_OUTPUT_BGRA disabled, so read_to_image() yields true RGBA
     * into the R8G8B8A8 GrlImage (red must read back as red, blue as blue).
     */
    grl_software_renderer_clear_color (sw, 1.0f, 0.5f, 0.0f, 1.0f);
    grl_software_renderer_clear (sw, GRL_SW_BUFFER_COLOR | GRL_SW_BUFFER_DEPTH);

    image = grl_software_renderer_read_to_image (sw, 0, 0, 64, 48);
    g_assert_nonnull (image);
    g_assert_cmpint (grl_image_get_width (image), ==, 64);
    g_assert_cmpint (grl_image_get_height (image), ==, 48);

    /* Every pixel of a flat clear is identical, so sampling the centre is
     * representative regardless of rlsw's bottom-left origin. */
    pixel = grl_image_get_pixel (image, 32, 24);
    g_assert_nonnull (pixel);
    g_assert_cmpuint (pixel->r, ==, 255);   /* red channel, not swapped */
    g_assert_cmpuint (pixel->b, ==, 0);     /* blue channel, not swapped */
    g_assert_cmpuint (pixel->a, ==, 255);
    /* 0.5f * 255 == 127.5; rlsw rounds to 127. Allow +/-2 for rounding. */
    g_assert_cmpuint (pixel->g, >=, 125);
    g_assert_cmpuint (pixel->g, <=, 129);
}

/*
 * Resize updates the reported dimensions.
 */

static void
test_software_renderer_resize (void)
{
    g_autoptr(GrlSoftwareRenderer) sw = grl_software_renderer_new (64, 48);
    gboolean                       ok;

    SKIP_IF_NULL (sw);

    ok = grl_software_renderer_resize (sw, 100, 80);
    g_assert_true (ok);
    g_assert_cmpint (grl_software_renderer_get_width (sw), ==, 100);
    g_assert_cmpint (grl_software_renderer_get_height (sw), ==, 80);
}

/*
 * An immediate-mode draw must not crash, and readback must still work after.
 */

static void
test_software_renderer_immediate_draw (void)
{
    g_autoptr(GrlSoftwareRenderer) sw = grl_software_renderer_new (64, 48);
    g_autoptr(GrlImage)            image = NULL;

    SKIP_IF_NULL (sw);

    grl_software_renderer_clear_color (sw, 0.0f, 0.0f, 0.0f, 1.0f);
    grl_software_renderer_clear (sw, GRL_SW_BUFFER_COLOR | GRL_SW_BUFFER_DEPTH);

    /* A single filled triangle in normalized device coordinates. */
    grl_software_renderer_begin (sw, GRL_SW_DRAW_MODE_TRIANGLES);
    grl_software_renderer_color3ub (sw, 255, 0, 0);
    grl_software_renderer_vertex2f (sw, -0.5f, -0.5f);
    grl_software_renderer_color3ub (sw, 0, 255, 0);
    grl_software_renderer_vertex2f (sw,  0.5f, -0.5f);
    grl_software_renderer_color3ub (sw, 0, 0, 255);
    grl_software_renderer_vertex2f (sw,  0.0f,  0.5f);
    grl_software_renderer_end (sw);

    image = grl_software_renderer_read_to_image (sw, 0, 0, 64, 48);
    g_assert_nonnull (image);
    g_assert_cmpint (grl_image_get_width (image), ==, 64);
    g_assert_cmpint (grl_image_get_height (image), ==, 48);
}

/*
 * Singleton constraint: while one renderer is live, a second
 * grl_software_renderer_new() must return NULL and emit a g_warning.
 */

static void
test_software_renderer_singleton (void)
{
    GrlSoftwareRenderer *first = grl_software_renderer_new (32, 32);
    GrlSoftwareRenderer *second;

    SKIP_IF_NULL (first);

    /* A second instance cannot be created while the first is alive: the
     * constructor emits a warning and returns NULL. Capture the expected
     * warning so GTest does not treat it as a fatal failure. */
    g_test_expect_message (G_LOG_DOMAIN, G_LOG_LEVEL_WARNING, "*");
    second = grl_software_renderer_new (16, 16);
    g_test_assert_expected_messages ();
    g_assert_null (second);

    /* Release the first only after asserting the constraint. */
    g_object_unref (first);
}

/*
 * Sequential lifecycle: creating, unreffing (which calls swClose() and clears
 * the global singleton flag in finalize), then creating again must succeed.
 * This proves finalize releases the global rlsw context.
 */

static void
test_software_renderer_sequential (void)
{
    GrlSoftwareRenderer *a = grl_software_renderer_new (32, 32);
    GrlSoftwareRenderer *b;

    SKIP_IF_NULL (a);

    /* Drop the first instance; finalize must call swClose() and clear the
     * singleton flag. */
    g_object_unref (a);

    /* A fresh instance must now be creatable. */
    b = grl_software_renderer_new (48, 24);
    g_assert_nonnull (b);
    g_assert_cmpint (grl_software_renderer_get_width (b), ==, 48);
    g_assert_cmpint (grl_software_renderer_get_height (b), ==, 24);

    g_object_unref (b);
}

/*
 * Pixel readback into a GrlImage after a black clear (dimension check).
 */

static void
test_software_renderer_read_to_image (void)
{
    g_autoptr(GrlSoftwareRenderer) sw = grl_software_renderer_new (64, 48);
    g_autoptr(GrlImage) image = NULL;

    SKIP_IF_NULL (sw);

    grl_software_renderer_clear_color (sw, 0.0f, 0.0f, 0.0f, 1.0f);
    grl_software_renderer_clear (sw, GRL_SW_BUFFER_COLOR | GRL_SW_BUFFER_DEPTH);

    image = grl_software_renderer_read_to_image (sw, 0, 0, 64, 48);

    g_assert_nonnull (image);
    g_assert_cmpint (grl_image_get_width (image), ==, 64);
    g_assert_cmpint (grl_image_get_height (image), ==, 48);
}

/*
 * Test registration
 */

int
main (int   argc,
      char *argv[])
{
    g_test_init (&argc, &argv, NULL);

    g_test_add_func ("/software-renderer/type", test_software_renderer_type);
    g_test_add_func ("/software-renderer/new", test_software_renderer_new);
    g_test_add_func ("/software-renderer/clear-pixel", test_software_renderer_clear_pixel);
    g_test_add_func ("/software-renderer/resize", test_software_renderer_resize);
    g_test_add_func ("/software-renderer/immediate-draw", test_software_renderer_immediate_draw);
    g_test_add_func ("/software-renderer/singleton", test_software_renderer_singleton);
    g_test_add_func ("/software-renderer/sequential", test_software_renderer_sequential);
    g_test_add_func ("/software-renderer/read-to-image", test_software_renderer_read_to_image);

    return g_test_run ();
}

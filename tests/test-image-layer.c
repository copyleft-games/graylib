/* test-image-layer.c
 *
 * Copyright 2026 Ben Doty
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * TDD suite for GrlLayer + grl_image_composite_layer.
 */

#include <glib.h>
#include "src/graphics/grl-image.h"
#include "src/math/grl-color.h"

#define W 64
#define H 64

static GrlImage *
make_solid (guint8 r, guint8 g, guint8 b, guint8 a)
{
    g_autoptr (GrlColor) c = grl_color_new (r, g, b, a);
    return grl_image_new_color (W, H, c);
}

/* GrlLayer creation and get_image returns a valid image. */
static void
test_layer_new (void)
{
    GrlLayer *layer = grl_layer_new (W, H);
    g_assert_nonnull (layer);
    GrlImage *img = grl_layer_get_image (layer);
    g_assert_nonnull (img);
    grl_layer_unref (layer);
}

/* Composite fully opaque white layer over black dst → dst becomes white. */
static void
test_composite_opaque_overwrites (void)
{
    g_autoptr (GrlImage) dst = make_solid (0, 0, 0, 255);
    GrlLayer *layer = grl_layer_new (W, H);
    GrlImage *limg  = grl_layer_get_image (layer);

    /* Paint the layer white. */
    g_autoptr (GrlColor) white = grl_color_new (255, 255, 255, 255);
    grl_image_clear_background (limg, white);

    grl_image_composite_layer (dst, layer, 0, 0, GRL_LAYER_BLEND_NORMAL, 1.0f);

    g_autoptr (GrlColor) px = grl_image_get_pixel (dst, W/2, H/2);
    g_assert_cmpuint (px->r, >=, 240);
    grl_layer_unref (layer);
}

/* Composite with opacity=0 → dst unchanged. */
static void
test_composite_zero_opacity_unchanged (void)
{
    g_autoptr (GrlImage) dst = make_solid (100, 100, 100, 255);
    GrlLayer *layer = grl_layer_new (W, H);
    GrlImage *limg  = grl_layer_get_image (layer);

    g_autoptr (GrlColor) white = grl_color_new (255, 255, 255, 255);
    grl_image_clear_background (limg, white);

    grl_image_composite_layer (dst, layer, 0, 0, GRL_LAYER_BLEND_NORMAL, 0.0f);

    g_autoptr (GrlColor) px = grl_image_get_pixel (dst, W/2, H/2);
    g_assert_cmpuint (px->r, ==, 100);
    grl_layer_unref (layer);
}

/* MULTIPLY blend: white*gray = gray (neutral). */
static void
test_composite_multiply_neutral (void)
{
    g_autoptr (GrlImage) dst = make_solid (128, 128, 128, 255);
    GrlLayer *layer = grl_layer_new (W, H);
    GrlImage *limg  = grl_layer_get_image (layer);

    g_autoptr (GrlColor) white = grl_color_new (255, 255, 255, 255);
    grl_image_clear_background (limg, white);

    grl_image_composite_layer (dst, layer, 0, 0, GRL_LAYER_BLEND_MULTIPLY, 1.0f);

    g_autoptr (GrlColor) px = grl_image_get_pixel (dst, W/2, H/2);
    /* white multiply = identity, so result should stay near 128 */
    g_assert_cmpuint (px->r, >=, 120);
    g_assert_cmpuint (px->r, <=, 135);
    grl_layer_unref (layer);
}

/* ADD blend: black + white = white (clamped). */
static void
test_composite_add_clamps (void)
{
    g_autoptr (GrlImage) dst = make_solid (200, 200, 200, 255);
    GrlLayer *layer = grl_layer_new (W, H);
    GrlImage *limg  = grl_layer_get_image (layer);

    g_autoptr (GrlColor) white = grl_color_new (255, 255, 255, 255);
    grl_image_clear_background (limg, white);

    grl_image_composite_layer (dst, layer, 0, 0, GRL_LAYER_BLEND_ADD, 1.0f);

    g_autoptr (GrlColor) px = grl_image_get_pixel (dst, W/2, H/2);
    g_assert_cmpuint (px->r, ==, 255);
    grl_layer_unref (layer);
}

/* grl_layer_ref increments ref; unref twice should not crash. */
static void
test_layer_refcount (void)
{
    GrlLayer *layer = grl_layer_new (W, H);
    GrlLayer *ref2  = grl_layer_ref (layer);
    g_assert_nonnull (ref2);
    grl_layer_unref (ref2);
    grl_layer_unref (layer);
    /* no crash = pass */
}

int
main (int argc, char **argv)
{
    g_test_init (&argc, &argv, NULL);
    g_test_add_func ("/layer/new",                  test_layer_new);
    g_test_add_func ("/layer/composite-opaque",     test_composite_opaque_overwrites);
    g_test_add_func ("/layer/composite-zero-opacity", test_composite_zero_opacity_unchanged);
    g_test_add_func ("/layer/composite-multiply",   test_composite_multiply_neutral);
    g_test_add_func ("/layer/composite-add",        test_composite_add_clamps);
    g_test_add_func ("/layer/refcount",             test_layer_refcount);
    return g_test_run ();
}

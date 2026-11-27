/* test-image-layer.c
 *
 * Copyright 2026 Ben Doty
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * TDD suite for GrlLayer + grl_image_composite_layer.
 */

#include <glib.h>
#include <math.h>
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

/*
 * Reference for the RGBA8 fast path of grl_image_composite_layer() (graylib
 * 0.13.2): the per-pixel loop it replaced, copied verbatim apart from reading
 * and writing through grl_image_get_pixel()/grl_image_draw_pixel() in
 * REPLACE.  The fast path must match it byte for byte.
 */
static gfloat
ref_blend_chan (gfloat dst, gfloat src, GrlLayerBlendMode mode)
{
    switch (mode) {
    case GRL_LAYER_BLEND_MULTIPLY:
        return dst * src;
    case GRL_LAYER_BLEND_SCREEN:
        return 1.0f - (1.0f - dst) * (1.0f - src);
    case GRL_LAYER_BLEND_OVERLAY:
        return (dst < 0.5f) ? (2.0f * dst * src)
                            : (1.0f - 2.0f * (1.0f - dst) * (1.0f - src));
    case GRL_LAYER_BLEND_SOFT_LIGHT:
        if (src <= 0.5f)
            return dst - (1.0f - 2.0f * src) * dst * (1.0f - dst);
        else {
            gfloat d = (dst <= 0.25f)
                     ? ((16.0f * dst - 12.0f) * dst + 4.0f) * dst
                     : sqrtf (dst);
            return dst + (2.0f * src - 1.0f) * (d - dst);
        }
    case GRL_LAYER_BLEND_ADD:
        {
            gfloat v = dst + src;
            return v > 1.0f ? 1.0f : v;
        }
    case GRL_LAYER_BLEND_COLOR_DODGE:
        if (src >= 1.0f) return 1.0f;
        return dst / (1.0f - src) > 1.0f ? 1.0f : dst / (1.0f - src);
    case GRL_LAYER_BLEND_COLOR_BURN:
        if (src <= 0.0f) return 0.0f;
        {
            gfloat v = 1.0f - (1.0f - dst) / src;
            return v < 0.0f ? 0.0f : v;
        }
    case GRL_LAYER_BLEND_NORMAL:
    default:
        return src;
    }
}

static gint
ref_clamp (gint v, gint lo, gint hi)
{
    return v < lo ? lo : (v > hi ? hi : v);
}

static void
ref_composite_layer (GrlImage *dst, GrlImage *src, gint x, gint y,
                     GrlLayerBlendMode mode, gfloat opacity)
{
    gint lw = grl_image_get_width (src);
    gint lh = grl_image_get_height (src);
    gint dw = grl_image_get_width (dst);
    gint dh = grl_image_get_height (dst);
    gint lx, ly;

    if (opacity <= 0.0f)
        return;
    if (opacity > 1.0f) opacity = 1.0f;

    for (ly = 0; ly < lh; ly++) {
        for (lx = 0; lx < lw; lx++) {
            g_autoptr(GrlColor) sp = NULL;
            g_autoptr(GrlColor) dp = NULL;
            g_autoptr(GrlColor) out = NULL;
            gfloat sa, da, oa, sr, sg, sb, dr, dg, db, br_, bg, bb;
            gint dx = x + lx;
            gint dy = y + ly;

            if (dx < 0 || dy < 0 || dx >= dw || dy >= dh)
                continue;
            sp = grl_image_get_pixel (src, lx, ly);
            if (sp->a == 0)
                continue;
            dp = grl_image_get_pixel (dst, dx, dy);

            sa = (sp->a / 255.0f) * opacity;
            da = dp->a / 255.0f;
            sr = sp->r / 255.0f; sg = sp->g / 255.0f; sb = sp->b / 255.0f;
            dr = dp->r / 255.0f; dg = dp->g / 255.0f; db = dp->b / 255.0f;
            br_ = ref_blend_chan (dr, sr, mode);
            bg  = ref_blend_chan (dg, sg, mode);
            bb  = ref_blend_chan (db, sb, mode);
            oa = sa + da * (1.0f - sa);
            if (oa <= 0.0f)
                out = grl_color_new (0, 0, 0, 0);
            else
                out = grl_color_new (
                    (guint8) ref_clamp ((gint) ((sa * br_ + (1.0f - sa) * da * dr) / oa * 255.0f + 0.5f), 0, 255),
                    (guint8) ref_clamp ((gint) ((sa * bg  + (1.0f - sa) * da * dg) / oa * 255.0f + 0.5f), 0, 255),
                    (guint8) ref_clamp ((gint) ((sa * bb  + (1.0f - sa) * da * db) / oa * 255.0f + 0.5f), 0, 255),
                    (guint8) ref_clamp ((gint) (oa * 255.0f + 0.5f), 0, 255));
            grl_image_draw_pixel (dst, dx, dy, out);
        }
    }
}

/* Fills @img with seeded random RGBA, with some fully transparent pixels. */
static void
fill_random (GrlImage *img, guint32 seed)
{
    GRand *rng = g_rand_new_with_seed (seed);
    gsize n;
    guint8 *p = grl_image_get_pixels (img, &n);
    gsize i;

    for (i = 0; i < n; i++)
        p[i] = (guint8) g_rand_int_range (rng, 0, 256);
    for (i = 3; i < n; i += 4 * 7)
        p[i] = 0;
    g_rand_free (rng);
}

/* Every blend mode, several opacities and offsets (inside, overhanging
 * every edge, fully outside): the fast path matches the reference. */
static void
test_composite_fast_path_matches_reference (void)
{
    const gfloat opacities[] = { 0.0f, 0.25f, 0.5f, 0.87f, 1.0f, 1.5f };
    const gint offsets[][2] = { { 0, 0 }, { 5, 9 }, { -7, -3 }, { 30, -10 },
                                { -20, 40 }, { 200, 0 } };
    guint mode;
    guint o;
    guint k;

    for (mode = GRL_LAYER_BLEND_NORMAL; mode <= GRL_LAYER_BLEND_COLOR_BURN; mode++)
        for (o = 0; o < G_N_ELEMENTS (opacities); o++)
            for (k = 0; k < G_N_ELEMENTS (offsets); k++)
            {
                g_autoptr(GrlColor) clear = grl_color_new (0, 0, 0, 0);
                g_autoptr(GrlImage) fast = grl_image_new_color (W, H, clear);
                g_autoptr(GrlImage) ref = grl_image_new_color (W, H, clear);
                GrlLayer *layer = grl_layer_new (40, 33);
                GrlImage *limg = grl_layer_get_image (layer);
                guint32 seed = mode * 1000 + o * 10 + k;

                fill_random (fast, seed);
                fill_random (ref, seed);
                fill_random (limg, seed + 7777);

                grl_image_composite_layer (fast, layer, offsets[k][0], offsets[k][1],
                                           (GrlLayerBlendMode) mode, opacities[o]);
                ref_composite_layer (ref, limg, offsets[k][0], offsets[k][1],
                                     (GrlLayerBlendMode) mode, opacities[o]);
                g_assert_cmpmem (grl_image_get_pixels (fast, NULL), W * H * 4,
                                 grl_image_get_pixels (ref, NULL), W * H * 4);
                grl_layer_unref (layer);
            }
}

/* GrlLayer creation and get_image returns a valid image. */
static void
test_layer_new (void)
{
    GrlLayer *layer = grl_layer_new (W, H);
	GrlImage *img;
    g_assert_nonnull (layer);
	img = grl_layer_get_image (layer);
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
	g_autoptr (GrlColor) px = NULL;

    /* Paint the layer white. */
    g_autoptr (GrlColor) white = grl_color_new (255, 255, 255, 255);
    grl_image_clear_background (limg, white);

    grl_image_composite_layer (dst, layer, 0, 0, GRL_LAYER_BLEND_NORMAL, 1.0f);

	px = grl_image_get_pixel (dst, W/2, H/2);
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
	g_autoptr (GrlColor) px = NULL;

    g_autoptr (GrlColor) white = grl_color_new (255, 255, 255, 255);
    grl_image_clear_background (limg, white);

    grl_image_composite_layer (dst, layer, 0, 0, GRL_LAYER_BLEND_NORMAL, 0.0f);

	px = grl_image_get_pixel (dst, W/2, H/2);
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
	g_autoptr (GrlColor) px = NULL;

    g_autoptr (GrlColor) white = grl_color_new (255, 255, 255, 255);
    grl_image_clear_background (limg, white);

    grl_image_composite_layer (dst, layer, 0, 0, GRL_LAYER_BLEND_MULTIPLY, 1.0f);

	px = grl_image_get_pixel (dst, W/2, H/2);
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
	g_autoptr (GrlColor) px = NULL;

    g_autoptr (GrlColor) white = grl_color_new (255, 255, 255, 255);
    grl_image_clear_background (limg, white);

    grl_image_composite_layer (dst, layer, 0, 0, GRL_LAYER_BLEND_ADD, 1.0f);

	px = grl_image_get_pixel (dst, W/2, H/2);
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
    g_test_add_func ("/layer/composite-fast-path",  test_composite_fast_path_matches_reference);
    return g_test_run ();
}

/* test-path.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Tests for GrlPath and the winding-aware scanline fill.
 */

#include <graylib.h>
#include <math.h>

/* --------------------------------------------------------------------------
 * Helper: read an RGBA pixel from the image
 * -------------------------------------------------------------------------- */

static void
get_pixel_rgba (GrlImage *img, gint x, gint y,
                guint8 *r, guint8 *g, guint8 *b, guint8 *a)
{
    GrlColor *c = grl_image_get_pixel (img, x, y);
    *r = c->r; *g = c->g; *b = c->b; *a = c->a;
    grl_color_free (c);
}

/* Assert that a pixel is approximately the given RGBA values. */
static void
assert_pixel (GrlImage *img, gint x, gint y,
              guint8 er, guint8 eg, guint8 eb, guint8 ea, gint tol,
              const gchar *label)
{
    guint8 ar, ag, ab, aa;
    get_pixel_rgba (img, x, y, &ar, &ag, &ab, &aa);
    if (abs ((gint)ar - (gint)er) > tol ||
        abs ((gint)ag - (gint)eg) > tol ||
        abs ((gint)ab - (gint)eb) > tol ||
        abs ((gint)aa - (gint)ea) > tol)
    {
        g_test_message ("%s: pixel (%d,%d) expected (%u,%u,%u,%u) got (%u,%u,%u,%u)",
                        label, x, y, er, eg, eb, ea, ar, ag, ab, aa);
        g_test_fail ();
    }
}

/* ===========================================================================
 * GrlPath construction tests
 * ===========================================================================
 */

static void
test_path_new_empty (void)
{
    g_autoptr(GrlPath) path = grl_path_new ();

    g_assert_nonnull (path);
    g_assert_true (grl_path_is_empty (path));
    g_assert_cmpuint (grl_path_get_subpath_count (path), ==, 0);
}

static void
test_path_subpath_count (void)
{
    g_autoptr(GrlPath) path = grl_path_new ();

    grl_path_move_to (path, 0, 0);
    grl_path_line_to (path, 10, 0);
    g_assert_cmpuint (grl_path_get_subpath_count (path), ==, 1);

    grl_path_move_to (path, 20, 20);
    grl_path_line_to (path, 30, 20);
    g_assert_cmpuint (grl_path_get_subpath_count (path), ==, 2);
}

static void
test_path_bounds_known (void)
{
    g_autoptr(GrlPath) path = grl_path_new ();
    GrlRectangle bounds;
    gboolean ok;

    /* Empty path has no bounds */
    ok = grl_path_get_bounds (path, &bounds);
    g_assert_false (ok);

    grl_path_move_to (path, 10.0f, 20.0f);
    grl_path_line_to (path, 50.0f, 20.0f);
    grl_path_line_to (path, 50.0f, 80.0f);
    grl_path_line_to (path, 10.0f, 80.0f);
    grl_path_close (path);

    ok = grl_path_get_bounds (path, &bounds);
    g_assert_true (ok);
    g_assert_cmpfloat_with_epsilon (bounds.x, 10.0f, 0.001f);
    g_assert_cmpfloat_with_epsilon (bounds.y, 20.0f, 0.001f);
    g_assert_cmpfloat_with_epsilon (bounds.width, 40.0f, 0.001f);
    g_assert_cmpfloat_with_epsilon (bounds.height, 60.0f, 0.001f);
}

static void
test_path_copy_independence (void)
{
    g_autoptr(GrlPath) original = grl_path_new ();
    g_autoptr(GrlPath) copy     = NULL;
    GrlRectangle rb_orig, rb_copy;

    grl_path_move_to (original, 0, 0);
    grl_path_line_to (original, 100, 0);
    grl_path_line_to (original, 100, 100);
    grl_path_line_to (original, 0, 100);
    grl_path_close (original);

    copy = grl_path_copy (original);
    g_assert_nonnull (copy);

    /* Mutate the copy — translate it by (200, 200) */
    grl_path_translate (copy, 200.0f, 200.0f);

    grl_path_get_bounds (original, &rb_orig);
    grl_path_get_bounds (copy, &rb_copy);

    /* Original must be unchanged */
    g_assert_cmpfloat_with_epsilon (rb_orig.x, 0.0f, 0.001f);
    g_assert_cmpfloat_with_epsilon (rb_orig.y, 0.0f, 0.001f);

    /* Copy must be moved */
    g_assert_cmpfloat_with_epsilon (rb_copy.x, 200.0f, 0.001f);
    g_assert_cmpfloat_with_epsilon (rb_copy.y, 200.0f, 0.001f);
}

static void
test_path_cubic_flatten (void)
{
    g_autoptr(GrlPath) path = grl_path_new ();
    GrlVector2 *pts;
    guint      *lengths;
    guint       n_sub, total;

    grl_path_move_to  (path, 0.0f, 0.0f);
    grl_path_cubic_to (path, 50.0f, 0.0f, 100.0f, 50.0f, 100.0f, 100.0f);

    pts = grl_path_get_flattened (path, 0.5f, &lengths, &n_sub, &total);
    g_assert_cmpuint (n_sub, ==, 1);
    /* A cubic should flatten to more than 2 points */
    g_assert_cmpuint (total, >, 2);

    g_free (pts);
    g_free (lengths);
}

static void
test_path_quad_flatten (void)
{
    g_autoptr(GrlPath) path = grl_path_new ();
    GrlVector2 *pts;
    guint      *lengths;
    guint       n_sub, total;

    grl_path_move_to (path, 0.0f, 0.0f);
    grl_path_quad_to (path, 50.0f, 0.0f, 100.0f, 100.0f);

    pts = grl_path_get_flattened (path, 0.5f, &lengths, &n_sub, &total);
    g_assert_cmpuint (n_sub, ==, 1);
    g_assert_cmpuint (total, >, 2);

    g_free (pts);
    g_free (lengths);
}

static void
test_path_add_rect (void)
{
    g_autoptr(GrlPath) path = grl_path_new ();
    GrlRectangle rect;
    GrlRectangle bounds;

    rect.x = 5.0f; rect.y = 10.0f;
    rect.width = 20.0f; rect.height = 30.0f;

    grl_path_add_rect (path, &rect);

    g_assert_false (grl_path_is_empty (path));
    g_assert_cmpuint (grl_path_get_subpath_count (path), ==, 1);
    grl_path_get_bounds (path, &bounds);
    g_assert_cmpfloat_with_epsilon (bounds.x, 5.0f, 0.001f);
    g_assert_cmpfloat_with_epsilon (bounds.y, 10.0f, 0.001f);
    g_assert_cmpfloat_with_epsilon (bounds.width, 20.0f, 0.001f);
    g_assert_cmpfloat_with_epsilon (bounds.height, 30.0f, 0.001f);
}

static void
test_path_add_circle (void)
{
    g_autoptr(GrlPath) path = grl_path_new ();
    GrlRectangle bounds;

    grl_path_add_circle (path, 50.0f, 50.0f, 20.0f);

    g_assert_false (grl_path_is_empty (path));
    grl_path_get_bounds (path, &bounds);
    /* Bounds of a circle(50,50,20) should be ~(30,30,40,40) */
    g_assert_cmpfloat_with_epsilon (bounds.x,      30.0f, 1.0f);
    g_assert_cmpfloat_with_epsilon (bounds.y,      30.0f, 1.0f);
    g_assert_cmpfloat_with_epsilon (bounds.width,  40.0f, 1.0f);
    g_assert_cmpfloat_with_epsilon (bounds.height, 40.0f, 1.0f);
}

static void
test_path_add_ellipse (void)
{
    g_autoptr(GrlPath) path = grl_path_new ();
    GrlRectangle bounds;

    grl_path_add_ellipse (path, 50.0f, 50.0f, 30.0f, 10.0f);

    g_assert_false (grl_path_is_empty (path));
    grl_path_get_bounds (path, &bounds);
    g_assert_cmpfloat_with_epsilon (bounds.x,      20.0f, 1.5f);
    g_assert_cmpfloat_with_epsilon (bounds.y,      40.0f, 1.5f);
    g_assert_cmpfloat_with_epsilon (bounds.width,  60.0f, 1.5f);
    g_assert_cmpfloat_with_epsilon (bounds.height, 20.0f, 1.5f);
}

/* ===========================================================================
 * Transform tests
 * ===========================================================================
 */

static void
test_path_translate (void)
{
    g_autoptr(GrlPath) path = grl_path_new ();
    GrlRectangle b;

    grl_path_move_to (path, 0.0f, 0.0f);
    grl_path_line_to (path, 10.0f, 0.0f);
    grl_path_line_to (path, 10.0f, 10.0f);
    grl_path_close (path);

    grl_path_translate (path, 20.0f, 30.0f);
    grl_path_get_bounds (path, &b);

    g_assert_cmpfloat_with_epsilon (b.x, 20.0f, 0.001f);
    g_assert_cmpfloat_with_epsilon (b.y, 30.0f, 0.001f);
}

static void
test_path_scale (void)
{
    g_autoptr(GrlPath) path = grl_path_new ();
    GrlRectangle b;

    grl_path_move_to (path, 1.0f, 1.0f);
    grl_path_line_to (path, 10.0f, 1.0f);
    grl_path_line_to (path, 10.0f, 10.0f);
    grl_path_close (path);

    grl_path_scale (path, 2.0f, 3.0f);
    grl_path_get_bounds (path, &b);

    g_assert_cmpfloat_with_epsilon (b.x, 2.0f, 0.001f);
    g_assert_cmpfloat_with_epsilon (b.y, 3.0f, 0.001f);
    g_assert_cmpfloat_with_epsilon (b.width, 18.0f, 0.001f);
    g_assert_cmpfloat_with_epsilon (b.height, 27.0f, 0.001f);
}

static void
test_path_rotate_90 (void)
{
    g_autoptr(GrlPath) path = grl_path_new ();
    GrlRectangle b;

    /* A horizontal rectangle, rotated 90deg CW should become vertical. */
    grl_path_move_to (path,  0.0f, -5.0f);
    grl_path_line_to (path, 40.0f, -5.0f);
    grl_path_line_to (path, 40.0f,  5.0f);
    grl_path_line_to (path,  0.0f,  5.0f);
    grl_path_close (path);

    grl_path_rotate (path, 90.0f);
    grl_path_get_bounds (path, &b);

    /* After CW 90 rotation, x span ~[−5, 5], y span ~[0, 40] */
    g_assert_cmpfloat_with_epsilon (b.width,  10.0f, 1.0f);
    g_assert_cmpfloat_with_epsilon (b.height, 40.0f, 1.0f);
}

/* ===========================================================================
 * Fill tests
 * ===========================================================================
 */

static void
test_fill_triangle (void)
{
    /* Fill a triangle: top-left quarter of a 64x64 image */
    GrlColor *black;
    GrlColor *white;
    g_autoptr(GrlImage) img  = NULL;
    g_autoptr(GrlPath)  path = grl_path_new ();

    black = grl_color_new (0, 0, 0, 255);
    white = grl_color_new (255, 255, 255, 255);

    img = grl_image_new_color (64, 64, black);

    grl_path_move_to (path,  2.0f,  2.0f);
    grl_path_line_to (path, 30.0f,  2.0f);
    grl_path_line_to (path,  2.0f, 30.0f);
    grl_path_close (path);

    grl_image_fill_path (img, path, GRL_FILL_RULE_NONZERO, white);

    /* Centre of the triangle should be white */
    assert_pixel (img, 8, 8, 255, 255, 255, 255, 10, "triangle fill");
    /* Outside corner should remain black */
    assert_pixel (img, 60, 60, 0, 0, 0, 255, 10, "triangle exterior");

    grl_color_free (black);
    grl_color_free (white);
}

static void
test_fill_donut_nonzero (void)
{
    /* Donut: CCW inner square inside CW outer square.
     * Under NONZERO, the hole should be left unfilled. */
    GrlColor *black;
    GrlColor *white;
    g_autoptr(GrlImage) img  = NULL;
    g_autoptr(GrlPath)  path = grl_path_new ();

    black = grl_color_new (0, 0, 0, 255);
    white = grl_color_new (255, 255, 255, 255);

    img = grl_image_new_color (64, 64, black);

    /* Outer square CW: TL → TR → BR → BL */
    grl_path_move_to (path,  4.0f,  4.0f);
    grl_path_line_to (path, 60.0f,  4.0f);
    grl_path_line_to (path, 60.0f, 60.0f);
    grl_path_line_to (path,  4.0f, 60.0f);
    grl_path_close (path);

    /* Inner square CCW: TL → BL → BR → TR */
    grl_path_move_to (path, 16.0f, 16.0f);
    grl_path_line_to (path, 16.0f, 48.0f);
    grl_path_line_to (path, 48.0f, 48.0f);
    grl_path_line_to (path, 48.0f, 16.0f);
    grl_path_close (path);

    grl_image_fill_path (img, path, GRL_FILL_RULE_NONZERO, white);

    /* Ring region (between outer and inner squares) should be white */
    assert_pixel (img, 8, 32, 255, 255, 255, 255, 10, "donut ring");
    /* Interior of hole should be BLACK (unfilled) */
    assert_pixel (img, 32, 32, 0, 0, 0, 255, 30, "donut hole");
    /* Exterior should be black */
    assert_pixel (img, 1, 1, 0, 0, 0, 255, 10, "donut exterior");

    grl_color_free (black);
    grl_color_free (white);
}

static void
test_fill_nonzero_vs_evenodd (void)
{
    /* A figure-8: two triangles sharing a point, self-intersecting.
     * Upper triangle: CW; lower triangle: CW.
     * Under EVEN_ODD the centre strip (overlap) is empty.
     * Under NONZERO the centre strip is filled. */
    GrlColor *black;
    GrlColor *white;
    g_autoptr(GrlImage) img_eo = NULL;
    g_autoptr(GrlImage) img_nz = NULL;
    g_autoptr(GrlPath)  path   = grl_path_new ();
    guint8 r_eo, g_eo, b_eo, a_eo;
    guint8 r_nz, g_nz, b_nz, a_nz;

    black = grl_color_new (0, 0, 0, 255);
    white = grl_color_new (255, 255, 255, 255);

    img_eo = grl_image_new_color (64, 64, black);
    img_nz = grl_image_new_color (64, 64, black);

    /* Star-like self-intersecting polygon (pentagram vertices clockwise) */
    grl_path_move_to (path, 32.0f,  4.0f);
    grl_path_line_to (path, 24.0f, 60.0f);
    grl_path_line_to (path, 60.0f, 20.0f);
    grl_path_line_to (path,  4.0f, 20.0f);
    grl_path_line_to (path, 40.0f, 60.0f);
    grl_path_close (path);

    grl_image_fill_path (img_eo, path, GRL_FILL_RULE_EVEN_ODD,  white);
    grl_image_fill_path (img_nz, path, GRL_FILL_RULE_NONZERO,   white);

    /* Sample a pixel inside the star's inner pentagon (x=32, y=28).
     * EVEN_ODD leaves it black (crossed 2 times → even → outside).
     * NONZERO leaves it white (winding ≠ 0 → inside). */
    get_pixel_rgba (img_eo, 32, 28, &r_eo, &g_eo, &b_eo, &a_eo);
    get_pixel_rgba (img_nz, 32, 28, &r_nz, &g_nz, &b_nz, &a_nz);

    /* Under EVEN_ODD the centre should be dark (unfilled) */
    g_assert_cmpuint (r_eo, <, 200);
    /* Under NONZERO the centre should be light (filled) */
    g_assert_cmpuint (r_nz, >, 100);
    /* They must differ */
    g_assert_true (r_eo != r_nz);

    grl_color_free (black);
    grl_color_free (white);
}

static void
test_fill_aa_interior (void)
{
    /* With AA on, the interior of a solid square should still be fully filled */
    GrlColor *black;
    GrlColor *white;
    g_autoptr(GrlImage) img  = NULL;
    g_autoptr(GrlPath)  path = grl_path_new ();

    black = grl_color_new (0, 0, 0, 255);
    white = grl_color_new (255, 255, 255, 255);

    img = grl_image_new_color (64, 64, black);
    grl_image_set_antialias (img, TRUE);

    grl_path_move_to (path,  2.0f,  2.0f);
    grl_path_line_to (path, 62.0f,  2.0f);
    grl_path_line_to (path, 62.0f, 62.0f);
    grl_path_line_to (path,  2.0f, 62.0f);
    grl_path_close (path);

    grl_image_fill_path (img, path, GRL_FILL_RULE_NONZERO, white);

    /* Interior pixel should be white */
    assert_pixel (img, 32, 32, 255, 255, 255, 255, 10, "AA interior");

    grl_color_free (black);
    grl_color_free (white);
}

static void
test_fill_clip_respected (void)
{
    /* Set a clip rect and verify pixels outside the clip are untouched */
    GrlColor    *black;
    GrlColor    *white;
    GrlRectangle clip;
    g_autoptr(GrlImage) img  = NULL;
    g_autoptr(GrlPath)  path = grl_path_new ();

    black = grl_color_new (0, 0, 0, 255);
    white = grl_color_new (255, 255, 255, 255);

    img = grl_image_new_color (64, 64, black);
    /* Clip to the top-left 32x32 region */
    clip.x = 0.0f; clip.y = 0.0f; clip.width = 32.0f; clip.height = 32.0f;
    grl_image_set_clip_rect (img, &clip);

    /* Full-image square fill */
    grl_path_move_to (path,  0.0f,  0.0f);
    grl_path_line_to (path, 63.0f,  0.0f);
    grl_path_line_to (path, 63.0f, 63.0f);
    grl_path_line_to (path,  0.0f, 63.0f);
    grl_path_close (path);

    grl_image_fill_path (img, path, GRL_FILL_RULE_NONZERO, white);

    /* Inside clip — should be white */
    assert_pixel (img, 10, 10, 255, 255, 255, 255, 10, "clip inside");
    /* Outside clip — should remain black */
    assert_pixel (img, 50, 50, 0, 0, 0, 255, 10, "clip outside");

    grl_color_free (black);
    grl_color_free (white);
}

static void
test_fill_empty_path_noop (void)
{
    /* Filling an empty path should not crash and must leave image unchanged */
    GrlColor *black;
    GrlColor *white;
    g_autoptr(GrlImage) img  = NULL;
    g_autoptr(GrlPath)  path = grl_path_new ();

    black = grl_color_new (0, 0, 0, 255);
    white = grl_color_new (255, 255, 255, 255);

    img = grl_image_new_color (32, 32, black);
    grl_image_fill_path (img, path, GRL_FILL_RULE_NONZERO, white);

    /* All pixels should still be black */
    assert_pixel (img, 16, 16, 0, 0, 0, 255, 5, "empty path noop");

    grl_color_free (black);
    grl_color_free (white);
}

/* ===========================================================================
 * Transform then fill pixel checks
 * ===========================================================================
 */

static void
test_fill_translate_then_fill (void)
{
    /* Fill a small square, then translate the path and fill again — only the
     * translated region should be painted. */
    GrlColor *black;
    GrlColor *white;
    g_autoptr(GrlImage) img  = NULL;
    g_autoptr(GrlPath)  path = grl_path_new ();

    black = grl_color_new (0, 0, 0, 255);
    white = grl_color_new (255, 255, 255, 255);

    img = grl_image_new_color (128, 64, black);

    /* Square at (4,4)-(20,20) */
    grl_path_move_to (path,  4.0f,  4.0f);
    grl_path_line_to (path, 20.0f,  4.0f);
    grl_path_line_to (path, 20.0f, 20.0f);
    grl_path_line_to (path,  4.0f, 20.0f);
    grl_path_close (path);

    grl_image_fill_path (img, path, GRL_FILL_RULE_NONZERO, white);

    /* Before translate: (12,12) should be white, (80,12) should be black */
    assert_pixel (img, 12, 12, 255, 255, 255, 255, 10, "pre-translate inside");
    assert_pixel (img, 80, 12, 0, 0, 0, 255, 10, "pre-translate outside");

    /* Translate path by (64, 0) and fill again */
    grl_path_translate (path, 64.0f, 0.0f);
    grl_image_fill_path (img, path, GRL_FILL_RULE_NONZERO, white);

    /* Now (80,12) should be white */
    assert_pixel (img, 80, 12, 255, 255, 255, 255, 10, "post-translate inside");

    grl_color_free (black);
    grl_color_free (white);
}

/* ===========================================================================
 * Boolean operation tests (pixel re-rasterization checks)
 * ===========================================================================
 */

static void
rasterize_path (GrlPath *path, gint w, gint h, GrlColor *fill_color,
                guint8 *out_r, guint8 *out_g, guint8 *out_b, guint8 *out_a,
                gint sample_x, gint sample_y)
{
    GrlColor *black = grl_color_new (0, 0, 0, 255);
    GrlImage *img = grl_image_new_color (w, h, black);
    grl_color_free (black);
    grl_image_fill_path (img, path, GRL_FILL_RULE_NONZERO, fill_color);
    get_pixel_rgba (img, sample_x, sample_y, out_r, out_g, out_b, out_a);
    g_object_unref (img);
}

static void
test_bool_union_overlapping (void)
{
    /* Two overlapping squares — union should fill both regions. */
    g_autoptr(GrlPath) a = grl_path_new ();
    g_autoptr(GrlPath) b = grl_path_new ();
    g_autoptr(GrlPath) u = NULL;
    GrlColor *white;
    guint8 r, g_c, bv, av;

    white = grl_color_new (255, 255, 255, 255);

    /* Square A: (4,4)-(40,40) */
    grl_path_move_to (a,  4.0f,  4.0f);
    grl_path_line_to (a, 40.0f,  4.0f);
    grl_path_line_to (a, 40.0f, 40.0f);
    grl_path_line_to (a,  4.0f, 40.0f);
    grl_path_close (a);

    /* Square B: (24,4)-(60,40) — overlaps A by 16px */
    grl_path_move_to (b, 24.0f,  4.0f);
    grl_path_line_to (b, 60.0f,  4.0f);
    grl_path_line_to (b, 60.0f, 40.0f);
    grl_path_line_to (b, 24.0f, 40.0f);
    grl_path_close (b);

    u = grl_path_union (a, b);
    g_assert_nonnull (u);

    /* Sample inside A only (not B) — should be covered by union */
    rasterize_path (u, 64, 64, white, &r, &g_c, &bv, &av, 12, 22);
    g_assert_cmpuint (r, >, 100);  /* filled */

    /* Sample inside B only (not A) — should be covered by union */
    rasterize_path (u, 64, 64, white, &r, &g_c, &bv, &av, 52, 22);
    g_assert_cmpuint (r, >, 100);  /* filled */

    /* Sample outside both — should be empty */
    rasterize_path (u, 64, 64, white, &r, &g_c, &bv, &av, 1, 1);
    g_assert_cmpuint (r, <, 50);   /* empty */

    grl_color_free (white);
}

static void
test_bool_intersect_overlapping (void)
{
    g_autoptr(GrlPath) a = grl_path_new ();
    g_autoptr(GrlPath) b = grl_path_new ();
    g_autoptr(GrlPath) inter = NULL;
    GrlColor *white;
    guint8 r, g_c, bv, av;

    white = grl_color_new (255, 255, 255, 255);

    /* Square A: (4,4)-(44,44) */
    grl_path_move_to (a,  4.0f,  4.0f);
    grl_path_line_to (a, 44.0f,  4.0f);
    grl_path_line_to (a, 44.0f, 44.0f);
    grl_path_line_to (a,  4.0f, 44.0f);
    grl_path_close (a);

    /* Square B: (24,4)-(60,44) */
    grl_path_move_to (b, 24.0f,  4.0f);
    grl_path_line_to (b, 60.0f,  4.0f);
    grl_path_line_to (b, 60.0f, 44.0f);
    grl_path_line_to (b, 24.0f, 44.0f);
    grl_path_close (b);

    inter = grl_path_intersect (a, b);
    g_assert_nonnull (inter);

    /* Sample inside overlap region — should be filled */
    rasterize_path (inter, 64, 64, white, &r, &g_c, &bv, &av, 32, 24);
    g_assert_cmpuint (r, >, 100);

    /* Sample inside A only (not in overlap) — should be empty */
    rasterize_path (inter, 64, 64, white, &r, &g_c, &bv, &av, 10, 24);
    g_assert_cmpuint (r, <, 50);

    grl_color_free (white);
}

static void
test_bool_subtract (void)
{
    g_autoptr(GrlPath) a = grl_path_new ();
    g_autoptr(GrlPath) b = grl_path_new ();
    g_autoptr(GrlPath) diff = NULL;
    GrlColor *white;
    guint8 r, g_c, bv, av;

    white = grl_color_new (255, 255, 255, 255);

    /* Square A: (2,2)-(62,62) */
    grl_path_move_to (a,  2.0f,  2.0f);
    grl_path_line_to (a, 62.0f,  2.0f);
    grl_path_line_to (a, 62.0f, 62.0f);
    grl_path_line_to (a,  2.0f, 62.0f);
    grl_path_close (a);

    /* Square B: (30,2)-(62,62) — right half of A */
    grl_path_move_to (b, 30.0f,  2.0f);
    grl_path_line_to (b, 62.0f,  2.0f);
    grl_path_line_to (b, 62.0f, 62.0f);
    grl_path_line_to (b, 30.0f, 62.0f);
    grl_path_close (b);

    diff = grl_path_subtract (a, b);
    g_assert_nonnull (diff);

    /* Left half of A (not in B) — should be filled */
    rasterize_path (diff, 64, 64, white, &r, &g_c, &bv, &av, 15, 32);
    g_assert_cmpuint (r, >, 100);

    /* Right half (in B) — should be empty */
    rasterize_path (diff, 64, 64, white, &r, &g_c, &bv, &av, 50, 32);
    g_assert_cmpuint (r, <, 50);

    grl_color_free (white);
}

static void
test_bool_xor (void)
{
    g_autoptr(GrlPath) a = grl_path_new ();
    g_autoptr(GrlPath) b = grl_path_new ();
    g_autoptr(GrlPath) xr = NULL;
    GrlColor *white;
    guint8 r, g_c, bv, av;

    white = grl_color_new (255, 255, 255, 255);

    /* Square A: (2,2)-(42,42) */
    grl_path_move_to (a,  2.0f,  2.0f);
    grl_path_line_to (a, 42.0f,  2.0f);
    grl_path_line_to (a, 42.0f, 42.0f);
    grl_path_line_to (a,  2.0f, 42.0f);
    grl_path_close (a);

    /* Square B: (22,22)-(62,62) — overlaps A in (22..42, 22..42) */
    grl_path_move_to (b, 22.0f, 22.0f);
    grl_path_line_to (b, 62.0f, 22.0f);
    grl_path_line_to (b, 62.0f, 62.0f);
    grl_path_line_to (b, 22.0f, 62.0f);
    grl_path_close (b);

    xr = grl_path_xor (a, b);
    g_assert_nonnull (xr);

    /* A-only region — should be filled */
    rasterize_path (xr, 64, 64, white, &r, &g_c, &bv, &av, 10, 10);
    g_assert_cmpuint (r, >, 100);

    /* B-only region — should be filled */
    rasterize_path (xr, 64, 64, white, &r, &g_c, &bv, &av, 52, 52);
    g_assert_cmpuint (r, >, 100);

    /* Overlap (both A and B) — should be empty (symmetric difference) */
    rasterize_path (xr, 64, 64, white, &r, &g_c, &bv, &av, 32, 32);
    g_assert_cmpuint (r, <, 50);

    grl_color_free (white);
}

static void
test_bool_disjoint_union (void)
{
    g_autoptr(GrlPath) a = grl_path_new ();
    g_autoptr(GrlPath) b = grl_path_new ();
    g_autoptr(GrlPath) u = NULL;
    GrlColor *white;
    guint8 r, g_c, bv, av;

    white = grl_color_new (255, 255, 255, 255);

    /* A: (2,2)-(20,20), B: (44,44)-(62,62) — no overlap */
    grl_path_move_to (a,  2.0f,  2.0f);
    grl_path_line_to (a, 20.0f,  2.0f);
    grl_path_line_to (a, 20.0f, 20.0f);
    grl_path_line_to (a,  2.0f, 20.0f);
    grl_path_close (a);

    grl_path_move_to (b, 44.0f, 44.0f);
    grl_path_line_to (b, 62.0f, 44.0f);
    grl_path_line_to (b, 62.0f, 62.0f);
    grl_path_line_to (b, 44.0f, 62.0f);
    grl_path_close (b);

    u = grl_path_union (a, b);

    /* Both regions filled */
    rasterize_path (u, 64, 64, white, &r, &g_c, &bv, &av, 11, 11);
    g_assert_cmpuint (r, >, 100);

    rasterize_path (u, 64, 64, white, &r, &g_c, &bv, &av, 53, 53);
    g_assert_cmpuint (r, >, 100);

    grl_color_free (white);
}

static void
test_bool_disjoint_intersect (void)
{
    g_autoptr(GrlPath) a = grl_path_new ();
    g_autoptr(GrlPath) b = grl_path_new ();
    g_autoptr(GrlPath) inter = NULL;
    GrlColor *white;
    guint8 r, g_c, bv, av;

    white = grl_color_new (255, 255, 255, 255);

    /* A and B disjoint — intersection should be empty */
    grl_path_move_to (a,  2.0f,  2.0f);
    grl_path_line_to (a, 20.0f,  2.0f);
    grl_path_line_to (a, 20.0f, 20.0f);
    grl_path_line_to (a,  2.0f, 20.0f);
    grl_path_close (a);

    grl_path_move_to (b, 44.0f, 44.0f);
    grl_path_line_to (b, 62.0f, 44.0f);
    grl_path_line_to (b, 62.0f, 62.0f);
    grl_path_line_to (b, 44.0f, 62.0f);
    grl_path_close (b);

    inter = grl_path_intersect (a, b);

    /* Neither region should be filled */
    rasterize_path (inter, 64, 64, white, &r, &g_c, &bv, &av, 11, 11);
    g_assert_cmpuint (r, <, 50);

    rasterize_path (inter, 64, 64, white, &r, &g_c, &bv, &av, 53, 53);
    g_assert_cmpuint (r, <, 50);

    grl_color_free (white);
}

static void
test_bool_empty_union (void)
{
    /* union(empty, B) == B */
    g_autoptr(GrlPath) a = grl_path_new ();  /* empty */
    g_autoptr(GrlPath) b = grl_path_new ();
    g_autoptr(GrlPath) u = NULL;
    GrlColor *white;
    guint8 r, g_c, bv, av;

    white = grl_color_new (255, 255, 255, 255);

    grl_path_move_to (b, 10.0f, 10.0f);
    grl_path_line_to (b, 50.0f, 10.0f);
    grl_path_line_to (b, 50.0f, 50.0f);
    grl_path_line_to (b, 10.0f, 50.0f);
    grl_path_close (b);

    u = grl_path_union (a, b);
    g_assert_nonnull (u);

    /* B's interior should be filled in the union */
    rasterize_path (u, 64, 64, white, &r, &g_c, &bv, &av, 30, 30);
    g_assert_cmpuint (r, >, 100);

    grl_color_free (white);
}

/* ===========================================================================
 * Degenerate / edge case tests
 * ===========================================================================
 */

static void
test_path_degenerate_zero_area (void)
{
    /* A path with coincident points should not crash. */
    g_autoptr(GrlPath) path = grl_path_new ();
    GrlColor *black;
    GrlColor *white;
    g_autoptr(GrlImage) img = NULL;

    black = grl_color_new (0, 0, 0, 255);
    white = grl_color_new (255, 255, 255, 255);

    grl_path_move_to (path, 32.0f, 32.0f);
    grl_path_line_to (path, 32.0f, 32.0f);
    grl_path_line_to (path, 32.0f, 32.0f);
    grl_path_close (path);

    img = grl_image_new_color (64, 64, black);
    grl_image_fill_path (img, path, GRL_FILL_RULE_NONZERO, white);
    /* No crash — that's the test */

    grl_color_free (black);
    grl_color_free (white);
}

static void
test_path_precondition_null (void)
{
    /* Calling fill with NULL path should log a critical and not crash. */
    GrlColor *black;
    GrlColor *white;
    g_autoptr(GrlImage) img = NULL;

    black = grl_color_new (0, 0, 0, 255);
    white = grl_color_new (255, 255, 255, 255);

    img = grl_image_new_color (64, 64, black);

    g_test_expect_message (NULL, G_LOG_LEVEL_CRITICAL, "*assertion*");
    grl_image_fill_path (img, NULL, GRL_FILL_RULE_NONZERO, white);
    g_test_assert_expected_messages ();

    grl_color_free (black);
    grl_color_free (white);
}

/* ===========================================================================
 * main
 * ===========================================================================
 */

int
main (int   argc,
      char *argv[])
{
    g_test_init (&argc, &argv, NULL);

    /* GrlPath construction */
    g_test_add_func ("/path/new-empty",          test_path_new_empty);
    g_test_add_func ("/path/subpath-count",       test_path_subpath_count);
    g_test_add_func ("/path/bounds-known",        test_path_bounds_known);
    g_test_add_func ("/path/copy-independence",   test_path_copy_independence);
    g_test_add_func ("/path/cubic-flatten",       test_path_cubic_flatten);
    g_test_add_func ("/path/quad-flatten",        test_path_quad_flatten);
    g_test_add_func ("/path/add-rect",            test_path_add_rect);
    g_test_add_func ("/path/add-circle",          test_path_add_circle);
    g_test_add_func ("/path/add-ellipse",         test_path_add_ellipse);

    /* Transforms */
    g_test_add_func ("/path/translate",           test_path_translate);
    g_test_add_func ("/path/scale",               test_path_scale);
    g_test_add_func ("/path/rotate-90",           test_path_rotate_90);

    /* Fill */
    g_test_add_func ("/fill/triangle",            test_fill_triangle);
    g_test_add_func ("/fill/donut-nonzero",       test_fill_donut_nonzero);
    g_test_add_func ("/fill/nonzero-vs-evenodd",  test_fill_nonzero_vs_evenodd);
    g_test_add_func ("/fill/aa-interior",         test_fill_aa_interior);
    g_test_add_func ("/fill/clip-respected",      test_fill_clip_respected);
    g_test_add_func ("/fill/empty-noop",          test_fill_empty_path_noop);
    g_test_add_func ("/fill/translate-then-fill", test_fill_translate_then_fill);

    /* Boolean ops */
    g_test_add_func ("/bool/union-overlapping",   test_bool_union_overlapping);
    g_test_add_func ("/bool/intersect-overlapping", test_bool_intersect_overlapping);
    g_test_add_func ("/bool/subtract",            test_bool_subtract);
    g_test_add_func ("/bool/xor",                 test_bool_xor);
    g_test_add_func ("/bool/disjoint-union",      test_bool_disjoint_union);
    g_test_add_func ("/bool/disjoint-intersect",  test_bool_disjoint_intersect);
    g_test_add_func ("/bool/empty-union",         test_bool_empty_union);

    /* Degenerate */
    g_test_add_func ("/degenerate/zero-area",     test_path_degenerate_zero_area);
    g_test_add_func ("/degenerate/null-path",     test_path_precondition_null);

    return g_test_run ();
}

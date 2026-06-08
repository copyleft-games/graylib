/* test-svg.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Tests for GrlVectorShape / SVG import-export round-trip.
 */

#include <graylib.h>
#include <math.h>
#include <string.h>

/* -------------------------------------------------------------------------
 * Helper macros
 * ------------------------------------------------------------------------- */

/* Free a shape array produced by grl_svg_load_from_*() */
#define FREE_SHAPES(arr, n) \
    do { \
        guint _i; \
        for (_i = 0; _i < (n); _i++) \
            grl_vector_shape_free ((arr)[_i]); \
        g_free (arr); \
    } while (0)

/* Bounds comparison tolerance (pixels) */
#define BOUNDS_EPS 0.5f

static void
assert_bounds_near (GrlPath     *path,
                    gfloat       ex, gfloat ey,
                    gfloat       ew, gfloat eh,
                    gfloat       eps,
                    const gchar *label)
{
    GrlRectangle b;
    gboolean     ok;

    ok = grl_path_get_bounds (path, &b);
    g_assert_true (ok);

    if (fabsf (b.x      - ex) > eps ||
        fabsf (b.y      - ey) > eps ||
        fabsf (b.width  - ew) > eps ||
        fabsf (b.height - eh) > eps)
    {
        g_test_message ("%s: expected bounds (%.2f,%.2f,%.2f,%.2f) "
                        "got (%.2f,%.2f,%.2f,%.2f)",
                        label,
                        (gdouble)ex, (gdouble)ey, (gdouble)ew, (gdouble)eh,
                        (gdouble)b.x, (gdouble)b.y,
                        (gdouble)b.width, (gdouble)b.height);
        g_test_fail ();
    }
}

/* =========================================================================
 * Error / edge-case tests
 * ========================================================================= */

static void
test_malformed_xml (void)
{
    GrlVectorShape **shapes;
    GError          *error  = NULL;
    guint            n      = 42; /* should be overwritten */

    /* No '<' at all — build_shapes detects this as a parse error */
    const gchar bad_no_lt[] = "this is not xml at all, no angle brackets";

    shapes = grl_svg_load_from_memory (bad_no_lt, strlen (bad_no_lt), 96, &n, &error);

    /* Must return NULL and set an error */
    g_assert_null (shapes);
    g_assert_nonnull (error);
    g_assert_cmpint (error->domain, ==, (gint)GRL_SVG_ERROR);
    g_assert_cmpuint (n, ==, 0);

    g_error_free (error);
}

static void
test_truncated_xml (void)
{
    /* Truncated mid-tag — must not crash, may return NULL or 0 shapes */
    GrlVectorShape **shapes;
    GError          *error = NULL;
    guint            n     = 0;
    const gchar      bad[] = "<svg><rect x=\"0\"";

    shapes = grl_svg_load_from_memory (bad, strlen (bad), 96, &n, &error);

    /* Either NULL+error or 0 shapes — no crash is the key assertion */
    if (shapes != NULL)
    {
        g_assert_cmpuint (n, ==, 0);
        g_free (shapes);
    }
    g_clear_error (&error);
}

static void
test_empty_svg (void)
{
    GrlVectorShape **shapes;
    GError          *error = NULL;
    guint            n     = 99;
    const gchar      src[] = "<svg xmlns=\"http://www.w3.org/2000/svg\"/>";

    shapes = grl_svg_load_from_memory (src, strlen (src), 96, &n, &error);

    g_assert_nonnull (shapes);
    g_assert_cmpuint (n, ==, 0);
    g_assert_null (error);

    g_free (shapes);
}

/* =========================================================================
 * Basic shape import tests
 * ========================================================================= */

static void
test_import_rect (void)
{
    GrlVectorShape **shapes;
    GError          *error = NULL;
    guint            n     = 0;
    const gchar      src[] =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"100\" height=\"100\">"
        "<rect x=\"10\" y=\"20\" width=\"80\" height=\"60\" fill=\"#ff0000\"/>"
        "</svg>";

    shapes = grl_svg_load_from_memory (src, strlen (src), 96, &n, &error);

    g_assert_nonnull (shapes);
    g_assert_cmpuint (n, ==, 1);
    g_assert_null (error);

    g_assert_false (grl_path_is_empty (shapes[0]->path));
    /* Bounds should be at (10,20) with size 80x60 */
    assert_bounds_near (shapes[0]->path, 10.f, 20.f, 80.f, 60.f,
                        BOUNDS_EPS, "rect bounds");

    FREE_SHAPES (shapes, n);
}

static void
test_import_circle (void)
{
    GrlVectorShape **shapes;
    GError          *error = NULL;
    guint            n     = 0;
    const gchar      src[] =
        "<svg xmlns=\"http://www.w3.org/2000/svg\">"
        "<circle cx=\"50\" cy=\"50\" r=\"30\"/>"
        "</svg>";

    shapes = grl_svg_load_from_memory (src, strlen (src), 96, &n, &error);

    g_assert_nonnull (shapes);
    g_assert_cmpuint (n, ==, 1);
    g_assert_null (error);
    g_assert_false (grl_path_is_empty (shapes[0]->path));

    {
        GrlRectangle b;
        grl_path_get_bounds (shapes[0]->path, &b);
        /* Centre should be near (50,50), radius 30 */
        g_assert_cmpfloat_with_epsilon ((gdouble)(b.x + b.width  / 2), 50.0, 1.0);
        g_assert_cmpfloat_with_epsilon ((gdouble)(b.y + b.height / 2), 50.0, 1.0);
        g_assert_cmpfloat_with_epsilon ((gdouble)(b.width  / 2), 30.0, 1.5);
        g_assert_cmpfloat_with_epsilon ((gdouble)(b.height / 2), 30.0, 1.5);
    }

    FREE_SHAPES (shapes, n);
}

static void
test_import_ellipse (void)
{
    GrlVectorShape **shapes;
    GError          *error = NULL;
    guint            n     = 0;
    const gchar      src[] =
        "<svg xmlns=\"http://www.w3.org/2000/svg\">"
        "<ellipse cx=\"60\" cy=\"40\" rx=\"20\" ry=\"10\"/>"
        "</svg>";

    shapes = grl_svg_load_from_memory (src, strlen (src), 96, &n, &error);

    g_assert_nonnull (shapes);
    g_assert_cmpuint (n, ==, 1);
    g_assert_null (error);
    g_assert_false (grl_path_is_empty (shapes[0]->path));

    {
        GrlRectangle b;
        grl_path_get_bounds (shapes[0]->path, &b);
        g_assert_cmpfloat_with_epsilon ((gdouble)(b.width  / 2), 20.0, 1.5);
        g_assert_cmpfloat_with_epsilon ((gdouble)(b.height / 2), 10.0, 1.5);
    }

    FREE_SHAPES (shapes, n);
}

static void
test_import_line (void)
{
    GrlVectorShape **shapes;
    GError          *error = NULL;
    guint            n     = 0;
    const gchar      src[] =
        "<svg xmlns=\"http://www.w3.org/2000/svg\">"
        "<line x1=\"0\" y1=\"0\" x2=\"100\" y2=\"50\"/>"
        "</svg>";

    shapes = grl_svg_load_from_memory (src, strlen (src), 96, &n, &error);

    g_assert_nonnull (shapes);
    g_assert_cmpuint (n, ==, 1);
    g_assert_null (error);
    g_assert_false (grl_path_is_empty (shapes[0]->path));

    FREE_SHAPES (shapes, n);
}

static void
test_import_polyline (void)
{
    GrlVectorShape **shapes;
    GError          *error = NULL;
    guint            n     = 0;
    const gchar      src[] =
        "<svg xmlns=\"http://www.w3.org/2000/svg\">"
        "<polyline points=\"0,0 10,0 10,10 0,10\"/>"
        "</svg>";

    shapes = grl_svg_load_from_memory (src, strlen (src), 96, &n, &error);

    g_assert_nonnull (shapes);
    g_assert_cmpuint (n, ==, 1);
    g_assert_null (error);
    g_assert_false (grl_path_is_empty (shapes[0]->path));

    FREE_SHAPES (shapes, n);
}

static void
test_import_polygon (void)
{
    GrlVectorShape **shapes;
    GError          *error = NULL;
    guint            n     = 0;
    const gchar      src[] =
        "<svg xmlns=\"http://www.w3.org/2000/svg\">"
        "<polygon points=\"5,0 10,10 0,10\"/>"
        "</svg>";

    shapes = grl_svg_load_from_memory (src, strlen (src), 96, &n, &error);

    g_assert_nonnull (shapes);
    g_assert_cmpuint (n, ==, 1);
    g_assert_null (error);
    g_assert_false (grl_path_is_empty (shapes[0]->path));

    FREE_SHAPES (shapes, n);
}

/* =========================================================================
 * Path-d parsing tests
 * ========================================================================= */

static void
test_path_d_absolute (void)
{
    GrlVectorShape **shapes;
    GError          *error = NULL;
    guint            n     = 0;
    const gchar      src[] =
        "<svg xmlns=\"http://www.w3.org/2000/svg\">"
        "<path d=\"M 0 0 L 10 0 L 10 10 Z\"/>"
        "</svg>";

    shapes = grl_svg_load_from_memory (src, strlen (src), 96, &n, &error);

    g_assert_nonnull (shapes);
    g_assert_cmpuint (n, ==, 1);
    g_assert_null (error);

    assert_bounds_near (shapes[0]->path, 0.f, 0.f, 10.f, 10.f,
                        BOUNDS_EPS, "path M L L Z");

    FREE_SHAPES (shapes, n);
}

static void
test_path_d_relative (void)
{
    GrlVectorShape **shapes;
    GError          *error = NULL;
    guint            n     = 0;
    /* m l l z — same triangle as absolute version */
    const gchar      src[] =
        "<svg xmlns=\"http://www.w3.org/2000/svg\">"
        "<path d=\"m 0 0 l 10 0 l 0 10 z\"/>"
        "</svg>";

    shapes = grl_svg_load_from_memory (src, strlen (src), 96, &n, &error);

    g_assert_nonnull (shapes);
    g_assert_cmpuint (n, ==, 1);
    g_assert_null (error);

    assert_bounds_near (shapes[0]->path, 0.f, 0.f, 10.f, 10.f,
                        BOUNDS_EPS, "relative path");

    FREE_SHAPES (shapes, n);
}

static void
test_path_d_h_v (void)
{
    GrlVectorShape **shapes;
    GError          *error = NULL;
    guint            n     = 0;
    const gchar      src[] =
        "<svg xmlns=\"http://www.w3.org/2000/svg\">"
        "<path d=\"M 5 5 H 15 V 15 H 5 Z\"/>"
        "</svg>";

    shapes = grl_svg_load_from_memory (src, strlen (src), 96, &n, &error);

    g_assert_nonnull (shapes);
    g_assert_cmpuint (n, ==, 1);
    g_assert_null (error);

    assert_bounds_near (shapes[0]->path, 5.f, 5.f, 10.f, 10.f,
                        BOUNDS_EPS, "H/V path");

    FREE_SHAPES (shapes, n);
}

/* =========================================================================
 * Paint attribute tests
 * ========================================================================= */

static void
test_fill_none (void)
{
    GrlVectorShape **shapes;
    GError          *error = NULL;
    guint            n     = 0;
    const gchar      src[] =
        "<svg xmlns=\"http://www.w3.org/2000/svg\">"
        "<rect x=\"0\" y=\"0\" width=\"10\" height=\"10\" fill=\"none\"/>"
        "</svg>";

    shapes = grl_svg_load_from_memory (src, strlen (src), 96, &n, &error);

    g_assert_nonnull (shapes);
    g_assert_cmpuint (n, ==, 1);
    g_assert_false (shapes[0]->has_fill);

    FREE_SHAPES (shapes, n);
}

static void
test_stroke_none (void)
{
    GrlVectorShape **shapes;
    GError          *error = NULL;
    guint            n     = 0;
    const gchar      src[] =
        "<svg xmlns=\"http://www.w3.org/2000/svg\">"
        "<rect x=\"0\" y=\"0\" width=\"10\" height=\"10\""
        " fill=\"blue\" stroke=\"none\"/>"
        "</svg>";

    shapes = grl_svg_load_from_memory (src, strlen (src), 96, &n, &error);

    g_assert_nonnull (shapes);
    g_assert_cmpuint (n, ==, 1);
    g_assert_false (shapes[0]->has_stroke);

    FREE_SHAPES (shapes, n);
}

static void
test_fill_red (void)
{
    GrlVectorShape **shapes;
    GError          *error = NULL;
    guint            n     = 0;
    const gchar      src[] =
        "<svg xmlns=\"http://www.w3.org/2000/svg\">"
        "<rect x=\"0\" y=\"0\" width=\"10\" height=\"10\" fill=\"#ff0000\"/>"
        "</svg>";

    shapes = grl_svg_load_from_memory (src, strlen (src), 96, &n, &error);

    g_assert_nonnull (shapes);
    g_assert_cmpuint (n, ==, 1);
    g_assert_true (shapes[0]->has_fill);
    g_assert_cmpuint (shapes[0]->fill.r, ==, 255);
    g_assert_cmpuint (shapes[0]->fill.g, ==, 0);
    g_assert_cmpuint (shapes[0]->fill.b, ==, 0);

    FREE_SHAPES (shapes, n);
}

/* =========================================================================
 * fill-rule preservation
 * ========================================================================= */

static void
test_fill_rule_evenodd_roundtrip (void)
{
    GrlVectorShape  *src_shape;
    gchar           *svg_str;
    GrlVectorShape **loaded;
    GError          *error = NULL;
    guint            n     = 0;
    GrlPath         *p;
    GrlVectorShape  *shapes_arr[1];

    /* Build a shape programmatically */
    src_shape = grl_vector_shape_new ();
    src_shape->fill_rule  = GRL_FILL_RULE_EVEN_ODD;
    src_shape->has_fill   = TRUE;
    src_shape->fill.r = 0; src_shape->fill.g = 128; src_shape->fill.b = 0; src_shape->fill.a = 255;
    src_shape->has_stroke = FALSE;

    p = src_shape->path;
    grl_path_move_to (p, 0.f, 0.f);
    grl_path_line_to (p, 20.f, 0.f);
    grl_path_line_to (p, 20.f, 20.f);
    grl_path_line_to (p, 0.f, 20.f);
    grl_path_close (p);

    shapes_arr[0] = src_shape;
    svg_str = grl_svg_to_string ((GrlVectorShape * const *)shapes_arr, 1, 100, 100);

    g_assert_nonnull (svg_str);
    g_assert_nonnull (strstr (svg_str, "evenodd"));

    loaded = grl_svg_load_from_memory (svg_str, strlen (svg_str), 96, &n, &error);
    g_free (svg_str);

    g_assert_nonnull (loaded);
    g_assert_cmpuint (n, ==, 1);
    g_assert_null (error);
    g_assert_cmpint ((gint)loaded[0]->fill_rule, ==, (gint)GRL_FILL_RULE_EVEN_ODD);

    FREE_SHAPES (loaded, n);
    grl_vector_shape_free (src_shape);
}

/* =========================================================================
 * Transform tests
 * ========================================================================= */

static void
test_transform_translate_rect (void)
{
    GrlVectorShape **shapes;
    GError          *error = NULL;
    guint            n     = 0;
    /* A rect at (0,0) with a translate(10,20) transform — bounds should shift */
    const gchar      src[] =
        "<svg xmlns=\"http://www.w3.org/2000/svg\">"
        "<rect x=\"0\" y=\"0\" width=\"30\" height=\"20\""
        " transform=\"translate(10,20)\"/>"
        "</svg>";

    shapes = grl_svg_load_from_memory (src, strlen (src), 96, &n, &error);

    g_assert_nonnull (shapes);
    g_assert_cmpuint (n, ==, 1);
    g_assert_null (error);

    /* Baked transform: x→10, y→20 */
    assert_bounds_near (shapes[0]->path, 10.f, 20.f, 30.f, 20.f,
                        BOUNDS_EPS, "translate(10,20) rect");

    FREE_SHAPES (shapes, n);
}

static void
test_group_transform (void)
{
    GrlVectorShape **shapes;
    GError          *error = NULL;
    guint            n     = 0;
    const gchar      src[] =
        "<svg xmlns=\"http://www.w3.org/2000/svg\">"
        "<g transform=\"translate(5,5)\">"
        "  <rect x=\"0\" y=\"0\" width=\"10\" height=\"10\"/>"
        "</g>"
        "</svg>";

    shapes = grl_svg_load_from_memory (src, strlen (src), 96, &n, &error);

    g_assert_nonnull (shapes);
    g_assert_cmpuint (n, ==, 1);
    g_assert_null (error);

    assert_bounds_near (shapes[0]->path, 5.f, 5.f, 10.f, 10.f,
                        BOUNDS_EPS, "group translate");

    FREE_SHAPES (shapes, n);
}

/* =========================================================================
 * Unknown elements: must not be fatal
 * ========================================================================= */

static void
test_unknown_elements_skipped (void)
{
    GrlVectorShape **shapes;
    GError          *error = NULL;
    guint            n     = 0;
    const gchar      src[] =
        "<svg xmlns=\"http://www.w3.org/2000/svg\">"
        "<defs><linearGradient id=\"g\"/></defs>"
        "<text x=\"10\" y=\"10\">hello</text>"
        "<rect x=\"0\" y=\"0\" width=\"50\" height=\"50\" fill=\"blue\"/>"
        "</svg>";

    shapes = grl_svg_load_from_memory (src, strlen (src), 96, &n, &error);

    /* Should parse successfully and return just the rect */
    g_assert_nonnull (shapes);
    g_assert_null (error);
    /* May return 1 shape (rect); text and defs are discarded */
    g_assert_cmpuint (n, >=, 1);

    FREE_SHAPES (shapes, n);
}

/* =========================================================================
 * Round-trip test (THE headline test)
 * ========================================================================= */

static void
test_roundtrip (void)
{
    /* Build two shapes programmatically */
    GrlVectorShape  *shape1 = NULL;
    GrlVectorShape  *shape2 = NULL;
    GrlVectorShape  *src_shapes[2];
    gchar           *svg_str  = NULL;
    GrlVectorShape **loaded   = NULL;
    GError          *error    = NULL;
    guint            n        = 0;
    GrlRectangle     b1, b2, l1, l2;

    /* Shape 1: a 50×40 rect at (10,20), red fill, nonzero rule */
    shape1 = grl_vector_shape_new ();
    shape1->has_fill  = TRUE;
    shape1->fill.r = 200; shape1->fill.g = 50; shape1->fill.b = 50; shape1->fill.a = 255;
    shape1->has_stroke   = TRUE;
    shape1->stroke.r = 0; shape1->stroke.g = 0; shape1->stroke.b = 0; shape1->stroke.a = 255;
    shape1->stroke_width = 2.f;
    shape1->fill_rule    = GRL_FILL_RULE_NONZERO;
    {
        GrlRectangle rect;
        rect.x = 10.f; rect.y = 20.f; rect.width = 50.f; rect.height = 40.f;
        grl_path_add_rect (shape1->path, &rect);
    }

    /* Shape 2: a circle, blue fill, evenodd rule */
    shape2 = grl_vector_shape_new ();
    shape2->has_fill  = TRUE;
    shape2->fill.r = 30; shape2->fill.g = 80; shape2->fill.b = 220; shape2->fill.a = 200;
    shape2->has_stroke   = FALSE;
    shape2->stroke_width = 1.f;
    shape2->fill_rule    = GRL_FILL_RULE_EVEN_ODD;
    grl_path_add_circle (shape2->path, 80.f, 80.f, 25.f);

    src_shapes[0] = shape1;
    src_shapes[1] = shape2;

    /* Serialise → deserialise */
    svg_str = grl_svg_to_string ((GrlVectorShape * const *)src_shapes, 2, 200, 200);
    g_assert_nonnull (svg_str);

    loaded = grl_svg_load_from_memory (svg_str, strlen (svg_str), 96, &n, &error);
    g_free (svg_str);

    g_assert_nonnull (loaded);
    g_assert_cmpuint (n, ==, 2);
    g_assert_null (error);

    /* Geometry: bounds must match within BOUNDS_EPS */
    grl_path_get_bounds (shape1->path, &b1);
    grl_path_get_bounds (shape2->path, &b2);
    grl_path_get_bounds (loaded[0]->path, &l1);
    grl_path_get_bounds (loaded[1]->path, &l2);

    g_assert_cmpfloat_with_epsilon ((gdouble)l1.x,      (gdouble)b1.x,      BOUNDS_EPS);
    g_assert_cmpfloat_with_epsilon ((gdouble)l1.y,      (gdouble)b1.y,      BOUNDS_EPS);
    g_assert_cmpfloat_with_epsilon ((gdouble)l1.width,  (gdouble)b1.width,  BOUNDS_EPS);
    g_assert_cmpfloat_with_epsilon ((gdouble)l1.height, (gdouble)b1.height, BOUNDS_EPS);

    g_assert_cmpfloat_with_epsilon ((gdouble)l2.x,      (gdouble)b2.x,      BOUNDS_EPS);
    g_assert_cmpfloat_with_epsilon ((gdouble)l2.y,      (gdouble)b2.y,      BOUNDS_EPS);
    g_assert_cmpfloat_with_epsilon ((gdouble)l2.width,  (gdouble)b2.width,  BOUNDS_EPS);
    g_assert_cmpfloat_with_epsilon ((gdouble)l2.height, (gdouble)b2.height, BOUNDS_EPS);

    /* Paint: shape1 */
    g_assert_true  (loaded[0]->has_fill);
    g_assert_cmpuint (loaded[0]->fill.r, ==, 200);
    g_assert_cmpuint (loaded[0]->fill.g, ==, 50);
    g_assert_cmpuint (loaded[0]->fill.b, ==, 50);
    g_assert_true  (loaded[0]->has_stroke);
    g_assert_cmpuint (loaded[0]->stroke.r, ==, 0);
    g_assert_cmpfloat_with_epsilon ((gdouble)loaded[0]->stroke_width, 2.0, 0.01);
    g_assert_cmpint ((gint)loaded[0]->fill_rule, ==, (gint)GRL_FILL_RULE_NONZERO);

    /* Paint: shape2 */
    g_assert_true (loaded[1]->has_fill);
    g_assert_cmpuint (loaded[1]->fill.r, ==, 30);
    g_assert_cmpuint (loaded[1]->fill.g, ==, 80);
    g_assert_cmpuint (loaded[1]->fill.b, ==, 220);
    g_assert_false (loaded[1]->has_stroke);
    g_assert_cmpint ((gint)loaded[1]->fill_rule, ==, (gint)GRL_FILL_RULE_EVEN_ODD);

    /* Alpha: shape2 fill had a=200 — allow ±1 due to float round-trip */
    g_assert_cmpuint (loaded[1]->fill.a, >=, 199);
    g_assert_cmpuint (loaded[1]->fill.a, <=, 201);

    FREE_SHAPES (loaded, n);
    grl_vector_shape_free (shape1);
    grl_vector_shape_free (shape2);
}

/* =========================================================================
 * Rendering test
 * ========================================================================= */

static void
test_draw_svg_shapes (void)
{
    GrlImage        *img     = NULL;
    GrlVectorShape **shapes  = NULL;
    GError          *error   = NULL;
    guint            n       = 0;
    GrlColor        *pixel   = NULL;

    /* 100×100 RGBA image, initialised white */
    {
        GrlColor white;
        white.r = 255; white.g = 255; white.b = 255; white.a = 255;
        img = grl_image_new_color (100, 100, &white);
    }
    if (img == NULL)
    {
        g_test_skip ("Could not create GrlImage (no display / headless)");
        return;
    }

    /* Load a simple red rect SVG */
    {
        const gchar src[] =
            "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"100\" height=\"100\">"
            "<rect x=\"10\" y=\"10\" width=\"80\" height=\"80\" fill=\"#ff0000\"/>"
            "</svg>";

        shapes = grl_svg_load_from_memory (src, strlen (src), 96, &n, &error);
    }

    g_assert_nonnull (shapes);
    g_assert_cmpuint (n, ==, 1);
    g_assert_null (error);

    /* Draw — must not crash */
    grl_image_draw_svg_shapes (img, (GrlVectorShape * const *)shapes, n);

    /* Check that a pixel inside the rect (50,50) is reddish */
    pixel = grl_image_get_pixel (img, 50, 50);
    if (pixel != NULL)
    {
        g_assert_cmpuint (pixel->r, >, 200);
        g_assert_cmpuint (pixel->g, <, 100);
        g_assert_cmpuint (pixel->b, <, 100);
        grl_color_free (pixel);
    }

    FREE_SHAPES (shapes, n);
    g_object_unref (img);
}

/* =========================================================================
 * main
 * ========================================================================= */

int
main (int   argc,
      char *argv[])
{
    g_test_init (&argc, &argv, NULL);

    /* Error / edge cases */
    g_test_add_func ("/svg/malformed-xml",            test_malformed_xml);
    g_test_add_func ("/svg/truncated-xml",            test_truncated_xml);
    g_test_add_func ("/svg/empty-svg",                test_empty_svg);

    /* Basic shape import */
    g_test_add_func ("/svg/import/rect",              test_import_rect);
    g_test_add_func ("/svg/import/circle",            test_import_circle);
    g_test_add_func ("/svg/import/ellipse",           test_import_ellipse);
    g_test_add_func ("/svg/import/line",              test_import_line);
    g_test_add_func ("/svg/import/polyline",          test_import_polyline);
    g_test_add_func ("/svg/import/polygon",           test_import_polygon);

    /* path `d` parsing */
    g_test_add_func ("/svg/path-d/absolute",          test_path_d_absolute);
    g_test_add_func ("/svg/path-d/relative",          test_path_d_relative);
    g_test_add_func ("/svg/path-d/h-v",               test_path_d_h_v);

    /* Paint attributes */
    g_test_add_func ("/svg/paint/fill-none",          test_fill_none);
    g_test_add_func ("/svg/paint/stroke-none",        test_stroke_none);
    g_test_add_func ("/svg/paint/fill-red",           test_fill_red);

    /* fill-rule */
    g_test_add_func ("/svg/fill-rule/evenodd-roundtrip", test_fill_rule_evenodd_roundtrip);

    /* Transforms */
    g_test_add_func ("/svg/transform/translate-rect", test_transform_translate_rect);
    g_test_add_func ("/svg/transform/group",          test_group_transform);

    /* Unknown elements */
    g_test_add_func ("/svg/unknown-elements",         test_unknown_elements_skipped);

    /* Round-trip */
    g_test_add_func ("/svg/roundtrip",                test_roundtrip);

    /* Rendering */
    g_test_add_func ("/svg/draw-svg-shapes",          test_draw_svg_shapes);

    return g_test_run ();
}

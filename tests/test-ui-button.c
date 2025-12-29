/* test-ui-button.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Unit tests for GrlUiButton.
 *
 * Note: Tests that require GPU context (drawing) are skipped.
 * GrlUiButton is a final type that derives from GrlUiControl.
 */

#include <glib.h>
#include <math.h>
#include "src/ui/grl-ui-button.h"
#include "src/math/grl-rectangle.h"

#define EPSILON 0.0001f

static gboolean
float_equal (gfloat a, gfloat b)
{
    return fabsf (a - b) < EPSILON;
}

/*
 * Test GType registration
 */

static void
test_ui_button_type (void)
{
    GType type = grl_ui_button_get_type ();

    g_assert_true (type != G_TYPE_INVALID);
    g_assert_cmpstr (g_type_name (type), ==, "GrlUiButton");
}

static void
test_ui_button_type_is_object (void)
{
    GType type = grl_ui_button_get_type ();

    g_assert_true (g_type_is_a (type, G_TYPE_OBJECT));
}

static void
test_ui_button_type_is_control (void)
{
    GType type = grl_ui_button_get_type ();

    g_assert_true (g_type_is_a (type, GRL_TYPE_UI_CONTROL));
}

static void
test_ui_button_type_not_abstract (void)
{
    GType type = grl_ui_button_get_type ();

    /* GrlUiButton can be instantiated (not abstract) */
    g_assert_false (G_TYPE_IS_ABSTRACT (type));
}

/*
 * Test constructors
 */

static void
test_ui_button_new (void)
{
    g_autoptr(GrlUiButton) button = grl_ui_button_new ("Test Button");

    g_assert_nonnull (button);
    g_assert_true (GRL_IS_UI_BUTTON (button));
}

static void
test_ui_button_new_null_text (void)
{
    g_autoptr(GrlUiButton) button = grl_ui_button_new (NULL);

    g_assert_nonnull (button);
    g_assert_true (GRL_IS_UI_BUTTON (button));
}

static void
test_ui_button_new_with_bounds (void)
{
    g_autoptr(GrlUiButton) button = grl_ui_button_new_with_bounds (10, 20, 100, 30, "Test");
    g_autoptr(GrlRectangle) bounds = NULL;

    g_assert_nonnull (button);
    g_assert_true (GRL_IS_UI_BUTTON (button));

    bounds = grl_ui_control_get_bounds (GRL_UI_CONTROL (button));
    g_assert_nonnull (bounds);
    g_assert_true (float_equal (bounds->x, 10.0f));
    g_assert_true (float_equal (bounds->y, 20.0f));
    g_assert_true (float_equal (bounds->width, 100.0f));
    g_assert_true (float_equal (bounds->height, 30.0f));
}

/*
 * Test text property
 */

static void
test_ui_button_get_text (void)
{
    g_autoptr(GrlUiButton) button = grl_ui_button_new ("Hello World");
    const gchar *text;

    text = grl_ui_button_get_text (button);
    g_assert_cmpstr (text, ==, "Hello World");
}

static void
test_ui_button_set_text (void)
{
    g_autoptr(GrlUiButton) button = grl_ui_button_new ("Initial");
    const gchar *text;

    grl_ui_button_set_text (button, "Changed");
    text = grl_ui_button_get_text (button);
    g_assert_cmpstr (text, ==, "Changed");
}

static void
test_ui_button_set_text_null (void)
{
    g_autoptr(GrlUiButton) button = grl_ui_button_new ("Initial");
    const gchar *text;

    grl_ui_button_set_text (button, NULL);
    text = grl_ui_button_get_text (button);
    g_assert_null (text);
}

/*
 * Test icon property
 */

static void
test_ui_button_icon_default (void)
{
    g_autoptr(GrlUiButton) button = grl_ui_button_new ("Test");
    gint icon;

    /* Default should be -1 (no icon) */
    icon = grl_ui_button_get_icon (button);
    g_assert_cmpint (icon, ==, -1);
}

static void
test_ui_button_set_icon (void)
{
    g_autoptr(GrlUiButton) button = grl_ui_button_new ("Test");

    grl_ui_button_set_icon (button, 5);
    g_assert_cmpint (grl_ui_button_get_icon (button), ==, 5);
}

static void
test_ui_button_set_icon_none (void)
{
    g_autoptr(GrlUiButton) button = grl_ui_button_new ("Test");

    grl_ui_button_set_icon (button, 5);
    g_assert_cmpint (grl_ui_button_get_icon (button), ==, 5);

    grl_ui_button_set_icon (button, -1);
    g_assert_cmpint (grl_ui_button_get_icon (button), ==, -1);
}

/*
 * Test inheritance from GrlUiControl
 */

static void
test_ui_button_inherits_enabled (void)
{
    g_autoptr(GrlUiButton) button = grl_ui_button_new ("Test");

    /* Should be enabled by default */
    g_assert_true (grl_ui_control_get_enabled (GRL_UI_CONTROL (button)));

    grl_ui_control_set_enabled (GRL_UI_CONTROL (button), FALSE);
    g_assert_false (grl_ui_control_get_enabled (GRL_UI_CONTROL (button)));
}

static void
test_ui_button_inherits_visible (void)
{
    g_autoptr(GrlUiButton) button = grl_ui_button_new ("Test");

    /* Should be visible by default */
    g_assert_true (grl_ui_control_get_visible (GRL_UI_CONTROL (button)));

    grl_ui_control_set_visible (GRL_UI_CONTROL (button), FALSE);
    g_assert_false (grl_ui_control_get_visible (GRL_UI_CONTROL (button)));
}

static void
test_ui_button_inherits_tooltip (void)
{
    g_autoptr(GrlUiButton) button = grl_ui_button_new ("Test");

    g_assert_null (grl_ui_control_get_tooltip (GRL_UI_CONTROL (button)));

    grl_ui_control_set_tooltip (GRL_UI_CONTROL (button), "Click to submit");
    g_assert_cmpstr (grl_ui_control_get_tooltip (GRL_UI_CONTROL (button)), ==, "Click to submit");
}

static void
test_ui_button_inherits_bounds (void)
{
    g_autoptr(GrlUiButton) button = grl_ui_button_new_with_bounds (10, 20, 100, 30, "Test");
    g_autoptr(GrlRectangle) new_bounds = grl_rectangle_new (50, 60, 200, 40);
    g_autoptr(GrlRectangle) result = NULL;

    grl_ui_control_set_bounds (GRL_UI_CONTROL (button), new_bounds);
    result = grl_ui_control_get_bounds (GRL_UI_CONTROL (button));

    g_assert_true (float_equal (result->x, 50.0f));
    g_assert_true (float_equal (result->y, 60.0f));
    g_assert_true (float_equal (result->width, 200.0f));
    g_assert_true (float_equal (result->height, 40.0f));
}

/*
 * Test registration
 */

int
main (int   argc,
      char *argv[])
{
    g_test_init (&argc, &argv, NULL);

    /* Type registration */
    g_test_add_func ("/ui-button/type", test_ui_button_type);
    g_test_add_func ("/ui-button/type-is-object", test_ui_button_type_is_object);
    g_test_add_func ("/ui-button/type-is-control", test_ui_button_type_is_control);
    g_test_add_func ("/ui-button/type-not-abstract", test_ui_button_type_not_abstract);

    /* Constructors */
    g_test_add_func ("/ui-button/new", test_ui_button_new);
    g_test_add_func ("/ui-button/new-null-text", test_ui_button_new_null_text);
    g_test_add_func ("/ui-button/new-with-bounds", test_ui_button_new_with_bounds);

    /* Text property */
    g_test_add_func ("/ui-button/get-text", test_ui_button_get_text);
    g_test_add_func ("/ui-button/set-text", test_ui_button_set_text);
    g_test_add_func ("/ui-button/set-text-null", test_ui_button_set_text_null);

    /* Icon property */
    g_test_add_func ("/ui-button/icon-default", test_ui_button_icon_default);
    g_test_add_func ("/ui-button/set-icon", test_ui_button_set_icon);
    g_test_add_func ("/ui-button/set-icon-none", test_ui_button_set_icon_none);

    /* Inheritance */
    g_test_add_func ("/ui-button/inherits-enabled", test_ui_button_inherits_enabled);
    g_test_add_func ("/ui-button/inherits-visible", test_ui_button_inherits_visible);
    g_test_add_func ("/ui-button/inherits-tooltip", test_ui_button_inherits_tooltip);
    g_test_add_func ("/ui-button/inherits-bounds", test_ui_button_inherits_bounds);

    return g_test_run ();
}

/* test-ui-control.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Unit tests for GrlUiControl base class.
 *
 * Note: GrlUiControl is an abstract base class (derivable type) and
 * cannot be instantiated directly. Tests that require control instances
 * use GrlUiButton as a concrete subclass. Tests that require GPU context
 * (drawing) are skipped.
 */

#include <glib.h>
#include <math.h>
#include "src/ui/grl-ui-control.h"
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
test_ui_control_type (void)
{
    GType type = grl_ui_control_get_type ();

    g_assert_true (type != G_TYPE_INVALID);
    g_assert_cmpstr (g_type_name (type), ==, "GrlUiControl");
}

static void
test_ui_control_type_is_object (void)
{
    GType type = grl_ui_control_get_type ();

    g_assert_true (g_type_is_a (type, G_TYPE_OBJECT));
}

static void
test_ui_control_type_is_derivable (void)
{
    GType type = grl_ui_control_get_type ();

    /* GrlUiControl is derivable, not final */
    g_assert_false (G_TYPE_IS_FINAL (type));
}

/*
 * Test using GrlUiButton as a concrete subclass
 */

static void
test_ui_control_button_is_control (void)
{
    g_autoptr(GrlUiButton) button = grl_ui_button_new_with_bounds (10, 20, 100, 30, "Test");

    g_assert_true (GRL_IS_UI_CONTROL (button));
}

static void
test_ui_control_get_bounds (void)
{
    g_autoptr(GrlUiButton) button = grl_ui_button_new_with_bounds (10, 20, 100, 30, "Test");
    g_autoptr(GrlRectangle) result = NULL;

    result = grl_ui_control_get_bounds (GRL_UI_CONTROL (button));

    g_assert_nonnull (result);
    g_assert_true (float_equal (result->x, 10.0f));
    g_assert_true (float_equal (result->y, 20.0f));
    g_assert_true (float_equal (result->width, 100.0f));
    g_assert_true (float_equal (result->height, 30.0f));
}

static void
test_ui_control_set_bounds (void)
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

static void
test_ui_control_get_x (void)
{
    g_autoptr(GrlUiButton) button = grl_ui_button_new_with_bounds (15, 25, 100, 30, "Test");

    g_assert_true (float_equal (grl_ui_control_get_x (GRL_UI_CONTROL (button)), 15.0f));
}

static void
test_ui_control_set_x (void)
{
    g_autoptr(GrlUiButton) button = grl_ui_button_new_with_bounds (10, 20, 100, 30, "Test");

    grl_ui_control_set_x (GRL_UI_CONTROL (button), 50.0f);
    g_assert_true (float_equal (grl_ui_control_get_x (GRL_UI_CONTROL (button)), 50.0f));
}

static void
test_ui_control_get_y (void)
{
    g_autoptr(GrlUiButton) button = grl_ui_button_new_with_bounds (15, 25, 100, 30, "Test");

    g_assert_true (float_equal (grl_ui_control_get_y (GRL_UI_CONTROL (button)), 25.0f));
}

static void
test_ui_control_set_y (void)
{
    g_autoptr(GrlUiButton) button = grl_ui_button_new_with_bounds (10, 20, 100, 30, "Test");

    grl_ui_control_set_y (GRL_UI_CONTROL (button), 75.0f);
    g_assert_true (float_equal (grl_ui_control_get_y (GRL_UI_CONTROL (button)), 75.0f));
}

static void
test_ui_control_get_width (void)
{
    g_autoptr(GrlUiButton) button = grl_ui_button_new_with_bounds (10, 20, 150, 30, "Test");

    g_assert_true (float_equal (grl_ui_control_get_width (GRL_UI_CONTROL (button)), 150.0f));
}

static void
test_ui_control_set_width (void)
{
    g_autoptr(GrlUiButton) button = grl_ui_button_new_with_bounds (10, 20, 100, 30, "Test");

    grl_ui_control_set_width (GRL_UI_CONTROL (button), 200.0f);
    g_assert_true (float_equal (grl_ui_control_get_width (GRL_UI_CONTROL (button)), 200.0f));
}

static void
test_ui_control_get_height (void)
{
    g_autoptr(GrlUiButton) button = grl_ui_button_new_with_bounds (10, 20, 100, 45, "Test");

    g_assert_true (float_equal (grl_ui_control_get_height (GRL_UI_CONTROL (button)), 45.0f));
}

static void
test_ui_control_set_height (void)
{
    g_autoptr(GrlUiButton) button = grl_ui_button_new_with_bounds (10, 20, 100, 30, "Test");

    grl_ui_control_set_height (GRL_UI_CONTROL (button), 60.0f);
    g_assert_true (float_equal (grl_ui_control_get_height (GRL_UI_CONTROL (button)), 60.0f));
}

static void
test_ui_control_enabled_default (void)
{
    g_autoptr(GrlUiButton) button = grl_ui_button_new_with_bounds (10, 20, 100, 30, "Test");

    /* Controls are enabled by default */
    g_assert_true (grl_ui_control_get_enabled (GRL_UI_CONTROL (button)));
}

static void
test_ui_control_set_enabled (void)
{
    g_autoptr(GrlUiButton) button = grl_ui_button_new_with_bounds (10, 20, 100, 30, "Test");

    grl_ui_control_set_enabled (GRL_UI_CONTROL (button), FALSE);
    g_assert_false (grl_ui_control_get_enabled (GRL_UI_CONTROL (button)));

    grl_ui_control_set_enabled (GRL_UI_CONTROL (button), TRUE);
    g_assert_true (grl_ui_control_get_enabled (GRL_UI_CONTROL (button)));
}

static void
test_ui_control_visible_default (void)
{
    g_autoptr(GrlUiButton) button = grl_ui_button_new_with_bounds (10, 20, 100, 30, "Test");

    /* Controls are visible by default */
    g_assert_true (grl_ui_control_get_visible (GRL_UI_CONTROL (button)));
}

static void
test_ui_control_set_visible (void)
{
    g_autoptr(GrlUiButton) button = grl_ui_button_new_with_bounds (10, 20, 100, 30, "Test");

    grl_ui_control_set_visible (GRL_UI_CONTROL (button), FALSE);
    g_assert_false (grl_ui_control_get_visible (GRL_UI_CONTROL (button)));

    grl_ui_control_set_visible (GRL_UI_CONTROL (button), TRUE);
    g_assert_true (grl_ui_control_get_visible (GRL_UI_CONTROL (button)));
}

static void
test_ui_control_tooltip (void)
{
    g_autoptr(GrlUiButton) button = grl_ui_button_new_with_bounds (10, 20, 100, 30, "Test");
    const gchar *tooltip;

    /* Default is NULL */
    g_assert_null (grl_ui_control_get_tooltip (GRL_UI_CONTROL (button)));

    grl_ui_control_set_tooltip (GRL_UI_CONTROL (button), "Click me!");
    tooltip = grl_ui_control_get_tooltip (GRL_UI_CONTROL (button));
    g_assert_cmpstr (tooltip, ==, "Click me!");

    /* Set back to NULL */
    grl_ui_control_set_tooltip (GRL_UI_CONTROL (button), NULL);
    g_assert_null (grl_ui_control_get_tooltip (GRL_UI_CONTROL (button)));
}

static void
test_ui_control_contains_point_inside (void)
{
    g_autoptr(GrlUiButton) button = grl_ui_button_new_with_bounds (10, 20, 100, 50, "Test");

    g_assert_true (grl_ui_control_contains_point (GRL_UI_CONTROL (button), 50.0f, 40.0f));
}

static void
test_ui_control_contains_point_outside (void)
{
    g_autoptr(GrlUiButton) button = grl_ui_button_new_with_bounds (10, 20, 100, 50, "Test");

    g_assert_false (grl_ui_control_contains_point (GRL_UI_CONTROL (button), 5.0f, 40.0f));
    g_assert_false (grl_ui_control_contains_point (GRL_UI_CONTROL (button), 50.0f, 15.0f));
    g_assert_false (grl_ui_control_contains_point (GRL_UI_CONTROL (button), 150.0f, 40.0f));
    g_assert_false (grl_ui_control_contains_point (GRL_UI_CONTROL (button), 50.0f, 80.0f));
}

static void
test_ui_control_contains_point_edge (void)
{
    g_autoptr(GrlUiButton) button = grl_ui_button_new_with_bounds (10, 20, 100, 50, "Test");

    /* Top-left corner should be inside */
    g_assert_true (grl_ui_control_contains_point (GRL_UI_CONTROL (button), 10.0f, 20.0f));
    /* Bottom-right edge is exclusive (x+width, y+height is outside) */
    g_assert_false (grl_ui_control_contains_point (GRL_UI_CONTROL (button), 110.0f, 70.0f));
    /* Just inside the bottom-right corner should be inside */
    g_assert_true (grl_ui_control_contains_point (GRL_UI_CONTROL (button), 109.0f, 69.0f));
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
    g_test_add_func ("/ui-control/type", test_ui_control_type);
    g_test_add_func ("/ui-control/type-is-object", test_ui_control_type_is_object);
    g_test_add_func ("/ui-control/type-is-derivable", test_ui_control_type_is_derivable);

    /* Subclass relationship */
    g_test_add_func ("/ui-control/button-is-control", test_ui_control_button_is_control);

    /* Bounds */
    g_test_add_func ("/ui-control/get-bounds", test_ui_control_get_bounds);
    g_test_add_func ("/ui-control/set-bounds", test_ui_control_set_bounds);
    g_test_add_func ("/ui-control/get-x", test_ui_control_get_x);
    g_test_add_func ("/ui-control/set-x", test_ui_control_set_x);
    g_test_add_func ("/ui-control/get-y", test_ui_control_get_y);
    g_test_add_func ("/ui-control/set-y", test_ui_control_set_y);
    g_test_add_func ("/ui-control/get-width", test_ui_control_get_width);
    g_test_add_func ("/ui-control/set-width", test_ui_control_set_width);
    g_test_add_func ("/ui-control/get-height", test_ui_control_get_height);
    g_test_add_func ("/ui-control/set-height", test_ui_control_set_height);

    /* Enabled/Visible */
    g_test_add_func ("/ui-control/enabled-default", test_ui_control_enabled_default);
    g_test_add_func ("/ui-control/set-enabled", test_ui_control_set_enabled);
    g_test_add_func ("/ui-control/visible-default", test_ui_control_visible_default);
    g_test_add_func ("/ui-control/set-visible", test_ui_control_set_visible);

    /* Tooltip */
    g_test_add_func ("/ui-control/tooltip", test_ui_control_tooltip);

    /* Point containment */
    g_test_add_func ("/ui-control/contains-point-inside", test_ui_control_contains_point_inside);
    g_test_add_func ("/ui-control/contains-point-outside", test_ui_control_contains_point_outside);
    g_test_add_func ("/ui-control/contains-point-edge", test_ui_control_contains_point_edge);

    return g_test_run ();
}

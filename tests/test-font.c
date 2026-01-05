/* test-font.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Unit tests for GrlFont.
 *
 * Note: Tests that require an active window/GPU context are skipped
 * when raylib is not initialized. Only type registration tests can run
 * without a graphics context.
 */

#include <glib.h>
#include "src/graphics/grl-font.h"
#include "src/grl-enums.h"

/*
 * Type registration tests (no GPU required)
 */

static void
test_font_type (void)
{
    GType type = grl_font_get_type ();

    g_assert_true (type != G_TYPE_INVALID);
    g_assert_cmpstr (g_type_name (type), ==, "GrlFont");
}

/*
 * Test registration
 */

int
main (int   argc,
      char *argv[])
{
    g_test_init (&argc, &argv, NULL);

    /* Type registration (no GPU required) */
    g_test_add_func ("/font/type", test_font_type);

    /*
     * Note: Tests for grl_font_new_default(), grl_font_set_filter(), etc.
     * require an initialized raylib context and are better suited for
     * integration tests or example programs.
     */

    return g_test_run ();
}

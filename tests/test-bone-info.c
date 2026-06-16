/* test-bone-info.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Unit tests for GrlBoneInfo.
 */

#include <glib.h>
#include <string.h>
#include "src/graphics/grl-bone-info.h"

/*
 * Test GType registration: GrlBoneInfo is a registered GBoxed value type.
 */

static void
test_bone_info_type (void)
{
    GType type = grl_bone_info_get_type ();

    g_assert_true (type != G_TYPE_INVALID);
    g_assert_cmpstr (g_type_name (type), ==, "GrlBoneInfo");

    /* It must be a boxed type, not a GObject/fundamental. */
    g_assert_true (G_TYPE_IS_BOXED (type));
    g_assert_false (G_TYPE_IS_OBJECT (type));
}

/*
 * Test constructors
 */

static void
test_bone_info_new (void)
{
    g_autoptr(GrlBoneInfo) bone = grl_bone_info_new ("spine", 0);

    g_assert_nonnull (bone);
    g_assert_cmpstr (grl_bone_info_get_name (bone), ==, "spine");
    g_assert_cmpint (grl_bone_info_get_parent (bone), ==, 0);
}

static void
test_bone_info_new_root (void)
{
    /* A root bone has a parent index of -1 */
    g_autoptr(GrlBoneInfo) bone = grl_bone_info_new ("root", -1);

    g_assert_nonnull (bone);
    g_assert_cmpstr (grl_bone_info_get_name (bone), ==, "root");
    g_assert_cmpint (grl_bone_info_get_parent (bone), ==, -1);
}

static void
test_bone_info_new_parent_zero (void)
{
    /* Parent index 0 is a valid (non-root) parent. */
    g_autoptr(GrlBoneInfo) bone = grl_bone_info_new ("child", 0);

    g_assert_nonnull (bone);
    g_assert_cmpint (grl_bone_info_get_parent (bone), ==, 0);
}

static void
test_bone_info_new_parent_large (void)
{
    /* A large positive parent index is stored verbatim (no validation). */
    g_autoptr(GrlBoneInfo) bone = grl_bone_info_new ("deep", 1000000);

    g_assert_nonnull (bone);
    g_assert_cmpint (grl_bone_info_get_parent (bone), ==, 1000000);
}

static void
test_bone_info_new_null_name (void)
{
    /* A NULL name should produce an empty name. */
    g_autoptr(GrlBoneInfo) bone = grl_bone_info_new (NULL, 2);

    g_assert_nonnull (bone);
    g_assert_cmpstr (grl_bone_info_get_name (bone), ==, "");
    g_assert_cmpint (grl_bone_info_get_parent (bone), ==, 2);
}

static void
test_bone_info_new_empty_name (void)
{
    /* An explicit empty name round-trips as "". */
    g_autoptr(GrlBoneInfo) bone = grl_bone_info_new ("", 4);

    g_assert_nonnull (bone);
    g_assert_cmpstr (grl_bone_info_get_name (bone), ==, "");
    g_assert_cmpuint (strlen (grl_bone_info_get_name (bone)), ==, 0);
    g_assert_cmpint (grl_bone_info_get_parent (bone), ==, 4);
}

static void
test_bone_info_new_name_exactly_31 (void)
{
    /* The name buffer is 32 bytes. A name of exactly 31 characters fits whole
     * (plus the NUL terminator) and must not be truncated. */
    const gchar *name31 = "0123456789012345678901234567890"; /* 31 chars */
    g_autoptr(GrlBoneInfo) bone = NULL;
    const gchar          *stored;

    g_assert_cmpuint (strlen (name31), ==, 31);

    bone = grl_bone_info_new (name31, 7);
    g_assert_nonnull (bone);

    stored = grl_bone_info_get_name (bone);
    g_assert_nonnull (stored);
    g_assert_cmpuint (strlen (stored), ==, 31);
    g_assert_cmpstr (stored, ==, name31);
    g_assert_cmpint (grl_bone_info_get_parent (bone), ==, 7);
}

static void
test_bone_info_new_truncates (void)
{
    /* The name buffer is 32 bytes, so names of 32+ chars are truncated to 31
     * and NUL-terminated. */
    const gchar *long_name = "this_is_a_very_long_bone_name_exceeding_thirty_one";
    g_autoptr(GrlBoneInfo) bone = grl_bone_info_new (long_name, 1);
    const gchar          *stored;

    g_assert_cmpuint (strlen (long_name), >, 31);

    g_assert_nonnull (bone);

    stored = grl_bone_info_get_name (bone);
    g_assert_nonnull (stored);

    /* At most 31 characters are stored (plus the NUL terminator). */
    g_assert_cmpuint (strlen (stored), ==, 31);

    /* The stored name is exactly the 31-character prefix of the request. */
    g_assert_true (strncmp (stored, long_name, 31) == 0);
}

static void
test_bone_info_copy (void)
{
    g_autoptr(GrlBoneInfo) bone1 = grl_bone_info_new ("hip", 3);
    g_autoptr(GrlBoneInfo) bone2 = grl_bone_info_copy (bone1);

    g_assert_nonnull (bone2);

    /* The copy is an independent allocation. */
    g_assert_true (bone1 != bone2);

    /* All fields are equal. */
    g_assert_cmpstr (grl_bone_info_get_name (bone2), ==, "hip");
    g_assert_cmpstr (grl_bone_info_get_name (bone1), ==,
                     grl_bone_info_get_name (bone2));
    g_assert_cmpint (grl_bone_info_get_parent (bone2), ==, 3);
    g_assert_cmpint (grl_bone_info_get_parent (bone1), ==,
                     grl_bone_info_get_parent (bone2));
}

static void
test_bone_info_copy_independent (void)
{
    /* The boxed-type copy func (used by g_boxed_copy) produces a distinct
     * pointer with equal contents. */
    g_autoptr(GrlBoneInfo) bone1 = grl_bone_info_new ("forearm.R", 5);
    GrlBoneInfo           *bone2 = g_boxed_copy (GRL_TYPE_BONE_INFO, bone1);

    g_assert_nonnull (bone2);
    g_assert_true ((gpointer)bone1 != (gpointer)bone2);
    g_assert_cmpstr (grl_bone_info_get_name (bone2), ==, "forearm.R");
    g_assert_cmpint (grl_bone_info_get_parent (bone2), ==, 5);

    g_boxed_free (GRL_TYPE_BONE_INFO, bone2);
}

/*
 * Test accessors
 */

static void
test_bone_info_get_name (void)
{
    g_autoptr(GrlBoneInfo) bone = grl_bone_info_new ("forearm.L", 5);

    g_assert_cmpstr (grl_bone_info_get_name (bone), ==, "forearm.L");
}

static void
test_bone_info_get_parent (void)
{
    g_autoptr(GrlBoneInfo) bone = grl_bone_info_new ("hand.L", 6);

    g_assert_cmpint (grl_bone_info_get_parent (bone), ==, 6);
}

/*
 * Test GValue round-trip: store/retrieve a GrlBoneInfo through a GValue using
 * the boxed-type machinery, verifying the fields survive a copy in and out.
 */

static void
test_bone_info_gvalue_roundtrip (void)
{
    g_autoptr(GrlBoneInfo) original = grl_bone_info_new ("clavicle.L", 9);
    GValue                 value = G_VALUE_INIT;
    GrlBoneInfo           *fetched;

    g_value_init (&value, GRL_TYPE_BONE_INFO);

    /* set_boxed copies the value into the GValue. */
    g_value_set_boxed (&value, original);

    /* get_boxed returns the GValue's internal copy (transfer none). */
    fetched = g_value_get_boxed (&value);
    g_assert_nonnull (fetched);
    g_assert_true ((gpointer)fetched != (gpointer)original);
    g_assert_cmpstr (grl_bone_info_get_name (fetched), ==, "clavicle.L");
    g_assert_cmpint (grl_bone_info_get_parent (fetched), ==, 9);

    g_value_unset (&value);
}

/*
 * Test g_autoptr cleanup compiles and runs (no leak, no crash).
 */

static void
test_bone_info_autoptr_cleanup (void)
{
    g_autoptr(GrlBoneInfo) bone = grl_bone_info_new ("toe", 12);

    g_assert_nonnull (bone);
    /* bone is freed by grl_bone_info_free() when this scope exits. */
}

/*
 * Test registration
 */

int
main (int   argc,
      char *argv[])
{
    g_test_init (&argc, &argv, NULL);

    /* GType */
    g_test_add_func ("/bone-info/type", test_bone_info_type);

    /* Constructors */
    g_test_add_func ("/bone-info/new", test_bone_info_new);
    g_test_add_func ("/bone-info/new-root", test_bone_info_new_root);
    g_test_add_func ("/bone-info/new-parent-zero", test_bone_info_new_parent_zero);
    g_test_add_func ("/bone-info/new-parent-large", test_bone_info_new_parent_large);
    g_test_add_func ("/bone-info/new-null-name", test_bone_info_new_null_name);
    g_test_add_func ("/bone-info/new-empty-name", test_bone_info_new_empty_name);
    g_test_add_func ("/bone-info/new-name-exactly-31", test_bone_info_new_name_exactly_31);
    g_test_add_func ("/bone-info/new-truncates", test_bone_info_new_truncates);
    g_test_add_func ("/bone-info/copy", test_bone_info_copy);
    g_test_add_func ("/bone-info/copy-independent", test_bone_info_copy_independent);

    /* Accessors */
    g_test_add_func ("/bone-info/get-name", test_bone_info_get_name);
    g_test_add_func ("/bone-info/get-parent", test_bone_info_get_parent);

    /* GValue / boxed integration */
    g_test_add_func ("/bone-info/gvalue-roundtrip", test_bone_info_gvalue_roundtrip);
    g_test_add_func ("/bone-info/autoptr-cleanup", test_bone_info_autoptr_cleanup);

    return g_test_run ();
}

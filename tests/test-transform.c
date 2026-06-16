/* test-transform.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Unit tests for GrlTransform.
 */

#include <glib.h>
#include "src/graphics/grl-transform.h"
#include "src/math/grl-vector3.h"
#include "src/math/grl-quaternion.h"

#define EPSILON 0.0001f

/*
 * Test GType registration: GrlTransform is a registered GBoxed value type.
 */

static void
test_transform_type (void)
{
    GType type = grl_transform_get_type ();

    g_assert_true (type != G_TYPE_INVALID);
    g_assert_cmpstr (g_type_name (type), ==, "GrlTransform");

    g_assert_true (G_TYPE_IS_BOXED (type));
    g_assert_false (G_TYPE_IS_OBJECT (type));
}

/*
 * Test constructors
 */

static void
test_transform_new (void)
{
    g_autoptr(GrlVector3)    translation = grl_vector3_new (1.0f, 2.0f, 3.0f);
    g_autoptr(GrlQuaternion) rotation = grl_quaternion_new (0.0f, 0.0f, 0.0f, 1.0f);
    g_autoptr(GrlVector3)    scale = grl_vector3_new (4.0f, 5.0f, 6.0f);
    g_autoptr(GrlTransform)  xf = grl_transform_new (translation, rotation, scale);

    g_assert_nonnull (xf);

    g_assert_cmpfloat_with_epsilon (xf->translation.x, 1.0f, EPSILON);
    g_assert_cmpfloat_with_epsilon (xf->translation.y, 2.0f, EPSILON);
    g_assert_cmpfloat_with_epsilon (xf->translation.z, 3.0f, EPSILON);

    g_assert_cmpfloat_with_epsilon (xf->rotation.x, 0.0f, EPSILON);
    g_assert_cmpfloat_with_epsilon (xf->rotation.y, 0.0f, EPSILON);
    g_assert_cmpfloat_with_epsilon (xf->rotation.z, 0.0f, EPSILON);
    g_assert_cmpfloat_with_epsilon (xf->rotation.w, 1.0f, EPSILON);

    g_assert_cmpfloat_with_epsilon (xf->scale.x, 4.0f, EPSILON);
    g_assert_cmpfloat_with_epsilon (xf->scale.y, 5.0f, EPSILON);
    g_assert_cmpfloat_with_epsilon (xf->scale.z, 6.0f, EPSILON);
}

static void
test_transform_new_null_components (void)
{
    /* NULL components fall back to defaults: translation (0,0,0),
     * rotation (0,0,0,1), scale (1,1,1). */
    g_autoptr(GrlTransform) xf = grl_transform_new (NULL, NULL, NULL);

    g_assert_nonnull (xf);

    g_assert_cmpfloat_with_epsilon (xf->translation.x, 0.0f, EPSILON);
    g_assert_cmpfloat_with_epsilon (xf->translation.y, 0.0f, EPSILON);
    g_assert_cmpfloat_with_epsilon (xf->translation.z, 0.0f, EPSILON);

    g_assert_cmpfloat_with_epsilon (xf->rotation.x, 0.0f, EPSILON);
    g_assert_cmpfloat_with_epsilon (xf->rotation.y, 0.0f, EPSILON);
    g_assert_cmpfloat_with_epsilon (xf->rotation.z, 0.0f, EPSILON);
    g_assert_cmpfloat_with_epsilon (xf->rotation.w, 1.0f, EPSILON);

    g_assert_cmpfloat_with_epsilon (xf->scale.x, 1.0f, EPSILON);
    g_assert_cmpfloat_with_epsilon (xf->scale.y, 1.0f, EPSILON);
    g_assert_cmpfloat_with_epsilon (xf->scale.z, 1.0f, EPSILON);
}

static void
test_transform_new_partial_null (void)
{
    /* An explicit translation with NULL rotation/scale keeps the translation
     * and uses identity rotation / unit scale for the missing components. */
    g_autoptr(GrlVector3)   translation = grl_vector3_new (-3.0f, 7.5f, 0.0f);
    g_autoptr(GrlTransform) xf = grl_transform_new (translation, NULL, NULL);

    g_assert_nonnull (xf);

    g_assert_cmpfloat_with_epsilon (xf->translation.x, -3.0f, EPSILON);
    g_assert_cmpfloat_with_epsilon (xf->translation.y, 7.5f, EPSILON);
    g_assert_cmpfloat_with_epsilon (xf->translation.z, 0.0f, EPSILON);

    g_assert_cmpfloat_with_epsilon (xf->rotation.w, 1.0f, EPSILON);
    g_assert_cmpfloat_with_epsilon (xf->scale.x, 1.0f, EPSILON);
    g_assert_cmpfloat_with_epsilon (xf->scale.y, 1.0f, EPSILON);
    g_assert_cmpfloat_with_epsilon (xf->scale.z, 1.0f, EPSILON);
}

static void
test_transform_new_extreme_values (void)
{
    /* Negative, zero, and large float values are stored verbatim. */
    g_autoptr(GrlVector3)    translation = grl_vector3_new (-1000000.0f, 0.0f, 1000000.0f);
    g_autoptr(GrlQuaternion) rotation = grl_quaternion_new (-1.0f, 0.0f, 0.0f, 0.0f);
    g_autoptr(GrlVector3)    scale = grl_vector3_new (0.0f, -2.5f, 123456.0f);
    g_autoptr(GrlTransform)  xf = grl_transform_new (translation, rotation, scale);

    g_assert_nonnull (xf);

    g_assert_cmpfloat_with_epsilon (xf->translation.x, -1000000.0f, 1.0f);
    g_assert_cmpfloat_with_epsilon (xf->translation.y, 0.0f, EPSILON);
    g_assert_cmpfloat_with_epsilon (xf->translation.z, 1000000.0f, 1.0f);

    g_assert_cmpfloat_with_epsilon (xf->rotation.x, -1.0f, EPSILON);

    g_assert_cmpfloat_with_epsilon (xf->scale.x, 0.0f, EPSILON);
    g_assert_cmpfloat_with_epsilon (xf->scale.y, -2.5f, EPSILON);
    g_assert_cmpfloat_with_epsilon (xf->scale.z, 123456.0f, 1.0f);
}

static void
test_transform_new_identity (void)
{
    g_autoptr(GrlTransform) xf = grl_transform_new_identity ();

    g_assert_nonnull (xf);

    /* Identity translation is zero */
    g_assert_cmpfloat_with_epsilon (xf->translation.x, 0.0f, EPSILON);
    g_assert_cmpfloat_with_epsilon (xf->translation.y, 0.0f, EPSILON);
    g_assert_cmpfloat_with_epsilon (xf->translation.z, 0.0f, EPSILON);

    /* Identity rotation is the (0, 0, 0, 1) quaternion */
    g_assert_cmpfloat_with_epsilon (xf->rotation.x, 0.0f, EPSILON);
    g_assert_cmpfloat_with_epsilon (xf->rotation.y, 0.0f, EPSILON);
    g_assert_cmpfloat_with_epsilon (xf->rotation.z, 0.0f, EPSILON);
    g_assert_cmpfloat_with_epsilon (xf->rotation.w, 1.0f, EPSILON);

    /* Identity scale is one on every axis */
    g_assert_cmpfloat_with_epsilon (xf->scale.x, 1.0f, EPSILON);
    g_assert_cmpfloat_with_epsilon (xf->scale.y, 1.0f, EPSILON);
    g_assert_cmpfloat_with_epsilon (xf->scale.z, 1.0f, EPSILON);
}

static void
test_transform_copy (void)
{
    g_autoptr(GrlVector3)    translation = grl_vector3_new (7.0f, 8.0f, 9.0f);
    g_autoptr(GrlQuaternion) rotation = grl_quaternion_new (0.0f, 1.0f, 0.0f, 0.0f);
    g_autoptr(GrlVector3)    scale = grl_vector3_new (2.0f, 2.0f, 2.0f);
    g_autoptr(GrlTransform)  xf1 = grl_transform_new (translation, rotation, scale);
    g_autoptr(GrlTransform)  xf2 = grl_transform_copy (xf1);

    g_assert_nonnull (xf2);

    /* The copy is an independent allocation. */
    g_assert_true (xf1 != xf2);

    /* All components are equal to the source. */
    g_assert_cmpfloat_with_epsilon (xf2->translation.x, xf1->translation.x, EPSILON);
    g_assert_cmpfloat_with_epsilon (xf2->translation.y, xf1->translation.y, EPSILON);
    g_assert_cmpfloat_with_epsilon (xf2->translation.z, xf1->translation.z, EPSILON);
    g_assert_cmpfloat_with_epsilon (xf2->rotation.x, xf1->rotation.x, EPSILON);
    g_assert_cmpfloat_with_epsilon (xf2->rotation.y, xf1->rotation.y, EPSILON);
    g_assert_cmpfloat_with_epsilon (xf2->rotation.z, xf1->rotation.z, EPSILON);
    g_assert_cmpfloat_with_epsilon (xf2->rotation.w, xf1->rotation.w, EPSILON);
    g_assert_cmpfloat_with_epsilon (xf2->scale.x, xf1->scale.x, EPSILON);
    g_assert_cmpfloat_with_epsilon (xf2->scale.y, xf1->scale.y, EPSILON);
    g_assert_cmpfloat_with_epsilon (xf2->scale.z, xf1->scale.z, EPSILON);
}

/*
 * Test accessors (round-trip values)
 */

static void
test_transform_get_translation (void)
{
    g_autoptr(GrlVector3)    translation = grl_vector3_new (10.0f, 20.0f, 30.0f);
    g_autoptr(GrlQuaternion) rotation = grl_quaternion_new (0.0f, 0.0f, 0.0f, 1.0f);
    g_autoptr(GrlVector3)    scale = grl_vector3_new (1.0f, 1.0f, 1.0f);
    g_autoptr(GrlTransform)  xf = grl_transform_new (translation, rotation, scale);
    g_autoptr(GrlVector3)    out = grl_transform_get_translation (xf);

    g_assert_nonnull (out);
    g_assert_cmpfloat_with_epsilon (out->x, 10.0f, EPSILON);
    g_assert_cmpfloat_with_epsilon (out->y, 20.0f, EPSILON);
    g_assert_cmpfloat_with_epsilon (out->z, 30.0f, EPSILON);
}

static void
test_transform_get_rotation (void)
{
    g_autoptr(GrlVector3)    translation = grl_vector3_new (0.0f, 0.0f, 0.0f);
    g_autoptr(GrlQuaternion) rotation = grl_quaternion_new (0.1f, 0.2f, 0.3f, 0.4f);
    g_autoptr(GrlVector3)    scale = grl_vector3_new (1.0f, 1.0f, 1.0f);
    g_autoptr(GrlTransform)  xf = grl_transform_new (translation, rotation, scale);
    g_autoptr(GrlQuaternion) out = grl_transform_get_rotation (xf);

    g_assert_nonnull (out);
    g_assert_cmpfloat_with_epsilon (out->x, 0.1f, EPSILON);
    g_assert_cmpfloat_with_epsilon (out->y, 0.2f, EPSILON);
    g_assert_cmpfloat_with_epsilon (out->z, 0.3f, EPSILON);
    g_assert_cmpfloat_with_epsilon (out->w, 0.4f, EPSILON);
}

static void
test_transform_get_scale (void)
{
    g_autoptr(GrlVector3)    translation = grl_vector3_new (0.0f, 0.0f, 0.0f);
    g_autoptr(GrlQuaternion) rotation = grl_quaternion_new (0.0f, 0.0f, 0.0f, 1.0f);
    g_autoptr(GrlVector3)    scale = grl_vector3_new (1.5f, 2.5f, 3.5f);
    g_autoptr(GrlTransform)  xf = grl_transform_new (translation, rotation, scale);
    g_autoptr(GrlVector3)    out = grl_transform_get_scale (xf);

    g_assert_nonnull (out);
    g_assert_cmpfloat_with_epsilon (out->x, 1.5f, EPSILON);
    g_assert_cmpfloat_with_epsilon (out->y, 2.5f, EPSILON);
    g_assert_cmpfloat_with_epsilon (out->z, 3.5f, EPSILON);
}

static void
test_transform_accessors_are_copies (void)
{
    /* Each accessor returns a freshly allocated copy: mutating the returned
     * vector must not change the transform. */
    g_autoptr(GrlVector3)    translation = grl_vector3_new (1.0f, 1.0f, 1.0f);
    g_autoptr(GrlTransform)  xf = grl_transform_new (translation, NULL, NULL);
    g_autoptr(GrlVector3)    out = grl_transform_get_translation (xf);
    g_autoptr(GrlVector3)    out2 = NULL;

    g_assert_nonnull (out);
    out->x = 999.0f;

    out2 = grl_transform_get_translation (xf);
    g_assert_cmpfloat_with_epsilon (out2->x, 1.0f, EPSILON);
}

/*
 * Test GValue round-trip through the boxed-type machinery.
 */

static void
test_transform_gvalue_roundtrip (void)
{
    g_autoptr(GrlVector3)    translation = grl_vector3_new (11.0f, 22.0f, 33.0f);
    g_autoptr(GrlQuaternion) rotation = grl_quaternion_new (0.0f, 0.0f, 1.0f, 0.0f);
    g_autoptr(GrlVector3)    scale = grl_vector3_new (0.5f, 1.5f, 2.5f);
    g_autoptr(GrlTransform)  original = grl_transform_new (translation, rotation, scale);
    GValue                   value = G_VALUE_INIT;
    GrlTransform            *fetched;

    g_value_init (&value, GRL_TYPE_TRANSFORM);
    g_value_set_boxed (&value, original);

    fetched = g_value_get_boxed (&value);
    g_assert_nonnull (fetched);
    g_assert_true ((gpointer)fetched != (gpointer)original);

    g_assert_cmpfloat_with_epsilon (fetched->translation.x, 11.0f, EPSILON);
    g_assert_cmpfloat_with_epsilon (fetched->translation.y, 22.0f, EPSILON);
    g_assert_cmpfloat_with_epsilon (fetched->translation.z, 33.0f, EPSILON);
    g_assert_cmpfloat_with_epsilon (fetched->rotation.z, 1.0f, EPSILON);
    g_assert_cmpfloat_with_epsilon (fetched->scale.x, 0.5f, EPSILON);
    g_assert_cmpfloat_with_epsilon (fetched->scale.y, 1.5f, EPSILON);
    g_assert_cmpfloat_with_epsilon (fetched->scale.z, 2.5f, EPSILON);

    g_value_unset (&value);
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
    g_test_add_func ("/transform/type", test_transform_type);

    /* Constructors */
    g_test_add_func ("/transform/new", test_transform_new);
    g_test_add_func ("/transform/new-null-components", test_transform_new_null_components);
    g_test_add_func ("/transform/new-partial-null", test_transform_new_partial_null);
    g_test_add_func ("/transform/new-extreme-values", test_transform_new_extreme_values);
    g_test_add_func ("/transform/new-identity", test_transform_new_identity);
    g_test_add_func ("/transform/copy", test_transform_copy);

    /* Accessors */
    g_test_add_func ("/transform/get-translation", test_transform_get_translation);
    g_test_add_func ("/transform/get-rotation", test_transform_get_rotation);
    g_test_add_func ("/transform/get-scale", test_transform_get_scale);
    g_test_add_func ("/transform/accessors-are-copies", test_transform_accessors_are_copies);

    /* GValue / boxed integration */
    g_test_add_func ("/transform/gvalue-roundtrip", test_transform_gvalue_roundtrip);

    return g_test_run ();
}

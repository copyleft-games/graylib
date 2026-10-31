/* grl-type-check-private.h
 *
 * Copyright 2026 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Cached instance type checks for graylib's per-object hot paths.
 *
 * G_TYPE_CHECK_INSTANCE_TYPE() only has an inline fast path for an exact
 * type match. A subclass instance (every game object is one) or an
 * interface check falls through to g_type_check_instance_is_a(), a call
 * into the type system per accessor per object per frame. Conformance is
 * immutable once true -- a type never stops being a subtype, or stops
 * implementing an interface it implements -- so remembering the last
 * instance type that passed is exact: the check still runs, and still
 * fails, for anything that does not conform.
 *
 * Each cache is one GType-sized word read and written atomically, so it is
 * thread-safe without a lock; a lost race only costs a full check.
 */

#pragma once

#include <glib-object.h>

G_BEGIN_DECLS

/*
 * grl_type_check_instance_cached:
 * @instance: (nullable): a #GTypeInstance, or %NULL
 * @type: the type @instance must conform to
 * @cache: the call site's (or file's) cache word, zero-initialised
 *
 * Same result as G_TYPE_CHECK_INSTANCE_TYPE (@instance, @type), skipping
 * the type system call when @instance's type matches the cached one.
 *
 * Returns: %TRUE if @instance is non-%NULL and conforms to @type
 */
static inline gboolean
grl_type_check_instance_cached (gconstpointer  instance,
                                GType          type,
                                gsize         *cache)
{
    const GTypeInstance *inst = instance;
    GType                itype;

    if (G_UNLIKELY (inst == NULL || inst->g_class == NULL))
        return FALSE;

    itype = inst->g_class->g_type;
    if (G_LIKELY (itype == type ||
                  itype == (GType) g_atomic_pointer_get (cache)))
        return TRUE;

    /* Miss: ask the type system, and remember a pass. */
    if (!g_type_check_instance_is_a ((GTypeInstance *) inst, type))
        return FALSE;
    g_atomic_pointer_set (cache, (gsize) itype);
    return TRUE;
}

G_END_DECLS

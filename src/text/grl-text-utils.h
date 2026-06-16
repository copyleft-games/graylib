/* grl-text-utils.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Text/string utility functions wrapping raylib's text management API.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "../grl-version.h"

G_BEGIN_DECLS

/*
 * =============================================================================
 * Text query functions
 * =============================================================================
 */

GRL_AVAILABLE_IN_ALL
guint           grl_text_length         (const gchar        *text);

GRL_AVAILABLE_IN_ALL
gboolean        grl_text_is_equal       (const gchar        *text1,
                                         const gchar        *text2);

GRL_AVAILABLE_IN_ALL
gint            grl_text_find_index      (const gchar        *text,
                                         const gchar        *find);

GRL_AVAILABLE_IN_ALL
gint            grl_text_to_integer      (const gchar        *text);

GRL_AVAILABLE_IN_ALL
gfloat          grl_text_to_float        (const gchar        *text);

/*
 * =============================================================================
 * Text transformation functions
 *
 * All return newly allocated strings the caller must free with g_free()
 * (GStrv results with g_strfreev()).
 * =============================================================================
 */

GRL_AVAILABLE_IN_ALL
gchar *         grl_text_subtext         (const gchar        *text,
                                         gint                position,
                                         gint                length);

GRL_AVAILABLE_IN_ALL
gchar *         grl_text_remove_spaces   (const gchar        *text);

GRL_AVAILABLE_IN_ALL
gchar *         grl_text_get_between     (const gchar        *text,
                                         const gchar        *begin,
                                         const gchar        *end);

GRL_AVAILABLE_IN_ALL
gchar *         grl_text_replace         (const gchar        *text,
                                         const gchar        *search,
                                         const gchar        *replacement);

GRL_AVAILABLE_IN_ALL
gchar *         grl_text_replace_between (const gchar        *text,
                                         const gchar        *begin,
                                         const gchar        *end,
                                         const gchar        *replacement);

GRL_AVAILABLE_IN_ALL
gchar *         grl_text_insert          (const gchar        *text,
                                         const gchar        *insert,
                                         gint                position);

GRL_AVAILABLE_IN_ALL
gchar *         grl_text_join            (const gchar * const *lines,
                                         const gchar          *delimiter);

GRL_AVAILABLE_IN_ALL
gchar **        grl_text_split           (const gchar        *text,
                                         gchar               delimiter);

/*
 * =============================================================================
 * Case conversion functions
 * =============================================================================
 */

GRL_AVAILABLE_IN_ALL
gchar *         grl_text_to_upper        (const gchar        *text);

GRL_AVAILABLE_IN_ALL
gchar *         grl_text_to_lower        (const gchar        *text);

GRL_AVAILABLE_IN_ALL
gchar *         grl_text_to_pascal       (const gchar        *text);

GRL_AVAILABLE_IN_ALL
gchar *         grl_text_to_snake        (const gchar        *text);

GRL_AVAILABLE_IN_ALL
gchar *         grl_text_to_camel        (const gchar        *text);

G_END_DECLS

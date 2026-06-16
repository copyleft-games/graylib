/* grl-fs.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Filesystem helper functions for Graylib.
 *
 * These functions wrap raylib's filesystem API but return GLib-idiomatic
 * types (gboolean, gchar*, GStrv, gint64, ...) rather than exposing
 * raylib's internal static buffers. Returned strings and string arrays
 * are owned copies that the caller must free.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib.h>
#include "../grl-version.h"

G_BEGIN_DECLS

/*
 * Existence and type queries
 */

GRL_AVAILABLE_IN_ALL
gboolean        grl_fs_file_exists              (const gchar    *path);

GRL_AVAILABLE_IN_ALL
gboolean        grl_fs_directory_exists         (const gchar    *path);

GRL_AVAILABLE_IN_ALL
gboolean        grl_fs_is_path_file             (const gchar    *path);

GRL_AVAILABLE_IN_ALL
gboolean        grl_fs_is_file_extension        (const gchar    *path,
                                                 const gchar    *ext);

GRL_AVAILABLE_IN_ALL
gboolean        grl_fs_is_file_name_valid       (const gchar    *file_name);

/*
 * File operations
 */

GRL_AVAILABLE_IN_ALL
gboolean        grl_fs_file_rename              (const gchar    *path,
                                                 const gchar    *new_path);

GRL_AVAILABLE_IN_ALL
gboolean        grl_fs_file_remove              (const gchar    *path);

GRL_AVAILABLE_IN_ALL
gboolean        grl_fs_file_copy                (const gchar    *src_path,
                                                 const gchar    *dst_path);

GRL_AVAILABLE_IN_ALL
gboolean        grl_fs_file_move                (const gchar    *src_path,
                                                 const gchar    *dst_path);

/*
 * File text editing
 */

GRL_AVAILABLE_IN_ALL
gboolean        grl_fs_file_text_replace        (const gchar    *path,
                                                 const gchar    *search,
                                                 const gchar    *replacement);

GRL_AVAILABLE_IN_ALL
gint            grl_fs_file_text_find_index     (const gchar    *path,
                                                 const gchar    *search);

/*
 * File metadata
 */

GRL_AVAILABLE_IN_ALL
gint            grl_fs_get_file_length          (const gchar    *path);

GRL_AVAILABLE_IN_ALL
gint64          grl_fs_get_file_mod_time        (const gchar    *path);

/*
 * Path string accessors (return owned copies)
 */

GRL_AVAILABLE_IN_ALL
gchar *         grl_fs_get_file_extension       (const gchar    *path);

GRL_AVAILABLE_IN_ALL
gchar *         grl_fs_get_file_name            (const gchar    *path);

GRL_AVAILABLE_IN_ALL
gchar *         grl_fs_get_file_name_without_ext (const gchar   *path);

GRL_AVAILABLE_IN_ALL
gchar *         grl_fs_get_directory_path       (const gchar    *path);

GRL_AVAILABLE_IN_ALL
gchar *         grl_fs_get_prev_directory_path  (const gchar    *path);

GRL_AVAILABLE_IN_ALL
gchar *         grl_fs_get_working_directory    (void);

GRL_AVAILABLE_IN_ALL
gchar *         grl_fs_get_application_directory (void);

/*
 * Directory operations
 */

GRL_AVAILABLE_IN_ALL
gboolean        grl_fs_make_directory           (const gchar    *path);

GRL_AVAILABLE_IN_ALL
gboolean        grl_fs_change_directory         (const gchar    *path);

GRL_AVAILABLE_IN_ALL
GStrv           grl_fs_load_directory_files     (const gchar    *dir_path);

GRL_AVAILABLE_IN_ALL
GStrv           grl_fs_load_directory_files_ex  (const gchar    *base_path,
                                                 const gchar    *filter,
                                                 gboolean        scan_subdirs);

GRL_AVAILABLE_IN_ALL
guint           grl_fs_get_directory_file_count (const gchar    *dir_path);

GRL_AVAILABLE_IN_ALL
guint           grl_fs_get_directory_file_count_ex (const gchar *base_path,
                                                    const gchar *filter,
                                                    gboolean     scan_subdirs);

G_END_DECLS

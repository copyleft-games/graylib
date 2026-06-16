/* grl-fs.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Filesystem helper functions implementation.
 */

#include "config.h"
#include "grl-fs.h"
#include <raylib.h>
#include <glib/gstdio.h>

/**
 * SECTION:grl-fs
 * @title: Filesystem Functions
 * @short_description: GLib-idiomatic wrappers over raylib's filesystem API
 *
 * This module wraps raylib's filesystem API (FileExists, GetFileName,
 * LoadDirectoryFiles, ...) as plain functions that return GLib-native
 * types: #gboolean for success flags, owned #gchar strings (rather than
 * raylib's internal static buffers), and %NULL-terminated string arrays
 * (#GStrv) for directory listings.
 *
 * Because raylib's path accessors (grl_fs_get_file_name() and friends)
 * return pointers into shared static storage, this module always hands
 * back freshly allocated copies. Directory listings are deep-copied into
 * a #GStrv and raylib's #FilePathList is released immediately, so the
 * static-buffer hazards stay contained inside this translation unit.
 *
 * Reading file contents is intentionally not wrapped here: use GLib's
 * g_file_get_contents() or #GFile, which provide better error reporting
 * and ownership semantics than raylib's LoadFileData/LoadFileText.
 *
 * Example:
 * |[<!-- language="C" -->
 * g_auto(GStrv) files = grl_fs_load_directory_files ("assets");
 * for (gchar **p = files; p != NULL && *p != NULL; p++)
 *     g_print ("%s\n", *p);
 * ]|
 */

/*
 * Existence and type queries
 */

/**
 * grl_fs_file_exists:
 * @path: (type filename): Path to a file.
 *
 * Checks whether a regular file exists at @path.
 *
 * Returns: %TRUE if the file exists.
 */
gboolean
grl_fs_file_exists (const gchar *path)
{
    unsigned char raw;

    g_return_val_if_fail (path != NULL, FALSE);

    raw = FileExists (path);
    return raw != 0;
}

/**
 * grl_fs_directory_exists:
 * @path: (type filename): Path to a directory.
 *
 * Checks whether a directory exists at @path.
 *
 * Returns: %TRUE if the directory exists.
 */
gboolean
grl_fs_directory_exists (const gchar *path)
{
    unsigned char raw;

    g_return_val_if_fail (path != NULL, FALSE);

    raw = DirectoryExists (path);
    return raw != 0;
}

/**
 * grl_fs_is_path_file:
 * @path: (type filename): Path to test.
 *
 * Checks whether @path refers to a file (rather than a directory).
 *
 * Returns: %TRUE if @path is a file.
 */
gboolean
grl_fs_is_path_file (const gchar *path)
{
    unsigned char raw;

    g_return_val_if_fail (path != NULL, FALSE);

    raw = IsPathFile (path);
    return raw != 0;
}

/**
 * grl_fs_is_file_extension:
 * @path: (type filename): Path or file name to test.
 * @ext: File extension to match, including the dot (e.g. ".png").
 *
 * Checks whether @path has the file extension @ext.
 *
 * Returns: %TRUE if @path ends with @ext.
 */
gboolean
grl_fs_is_file_extension (const gchar *path,
                          const gchar *ext)
{
    unsigned char raw;

    g_return_val_if_fail (path != NULL, FALSE);
    g_return_val_if_fail (ext != NULL, FALSE);

    raw = IsFileExtension (path, ext);
    return raw != 0;
}

/**
 * grl_fs_is_file_name_valid:
 * @file_name: A file name (not a full path).
 *
 * Checks whether @file_name is valid for the current platform/OS.
 *
 * Returns: %TRUE if @file_name is valid.
 */
gboolean
grl_fs_is_file_name_valid (const gchar *file_name)
{
    unsigned char raw;

    g_return_val_if_fail (file_name != NULL, FALSE);

    raw = IsFileNameValid (file_name);
    return raw != 0;
}

/*
 * File operations
 */

/**
 * grl_fs_file_rename:
 * @path: (type filename): Existing file path.
 * @new_path: (type filename): New file path.
 *
 * Renames the file at @path to @new_path, if it exists.
 *
 * Returns: %TRUE on success.
 */
gboolean
grl_fs_file_rename (const gchar *path,
                    const gchar *new_path)
{
    g_return_val_if_fail (path != NULL, FALSE);
    g_return_val_if_fail (new_path != NULL, FALSE);

    /* raylib returns 0 on success, non-zero on failure. */
    return FileRename (path, new_path) == 0;
}

/**
 * grl_fs_file_remove:
 * @path: (type filename): File path to remove.
 *
 * Removes the file at @path, if it exists.
 *
 * Returns: %TRUE on success.
 */
gboolean
grl_fs_file_remove (const gchar *path)
{
    g_return_val_if_fail (path != NULL, FALSE);

    /* raylib returns 0 on success, non-zero on failure. */
    return FileRemove (path) == 0;
}

/**
 * grl_fs_file_copy:
 * @src_path: (type filename): Source file path.
 * @dst_path: (type filename): Destination file path.
 *
 * Copies the file at @src_path to @dst_path. Intermediate directories of
 * @dst_path are created if they do not exist.
 *
 * Returns: %TRUE on success.
 */
gboolean
grl_fs_file_copy (const gchar *src_path,
                  const gchar *dst_path)
{
    unsigned char raw;

    g_return_val_if_fail (src_path != NULL, FALSE);
    g_return_val_if_fail (dst_path != NULL, FALSE);

    /* raylib 6.0's FileCopy() returns the (truthy) SaveFileData() result on
     * success and 0/-1 on failure, so success is non-zero here. */
    raw = (FileCopy (src_path, dst_path) > 0);
    return raw != 0;
}

/**
 * grl_fs_file_move:
 * @src_path: (type filename): Source file path.
 * @dst_path: (type filename): Destination file path.
 *
 * Moves the file at @src_path to @dst_path. Intermediate directories of
 * @dst_path are created if they do not exist.
 *
 * Returns: %TRUE on success.
 */
gboolean
grl_fs_file_move (const gchar *src_path,
                  const gchar *dst_path)
{
    g_return_val_if_fail (src_path != NULL, FALSE);
    g_return_val_if_fail (dst_path != NULL, FALSE);

    /* raylib 6.0's FileMove() is unreliable: its internal `FileCopy(...) == 0`
     * success test is inverted, so it leaves the source in place and reports
     * failure even when the copy succeeded.  Perform the move directly instead:
     * try an atomic rename first, then fall back to copy + remove. */
    if (g_rename (src_path, dst_path) == 0)
        return TRUE;

    if (!grl_fs_file_copy (src_path, dst_path))
        return FALSE;

    return grl_fs_file_remove (src_path);
}

/*
 * File text editing
 */

/**
 * grl_fs_file_text_replace:
 * @path: (type filename): Path to an existing text file.
 * @search: Text to search for.
 * @replacement: Replacement text.
 *
 * Replaces occurrences of @search with @replacement in the existing text
 * file at @path.
 *
 * Returns: %TRUE on success.
 */
gboolean
grl_fs_file_text_replace (const gchar *path,
                          const gchar *search,
                          const gchar *replacement)
{
    unsigned char raw;

    g_return_val_if_fail (path != NULL, FALSE);
    g_return_val_if_fail (search != NULL, FALSE);
    g_return_val_if_fail (replacement != NULL, FALSE);

    /* raylib 6.0's FileTextReplace() returns the (truthy) SaveFileText()
     * result on success, so success is non-zero here. */
    raw = (FileTextReplace (path, search, replacement) > 0);
    return raw != 0;
}

/**
 * grl_fs_file_text_find_index:
 * @path: (type filename): Path to an existing text file.
 * @search: Text to search for.
 *
 * Finds the byte index of the first occurrence of @search in the text
 * file at @path.
 *
 * Returns: The index of @search, or a negative value if not found.
 */
gint
grl_fs_file_text_find_index (const gchar *path,
                             const gchar *search)
{
    g_return_val_if_fail (path != NULL, -1);
    g_return_val_if_fail (search != NULL, -1);

    return FileTextFindIndex (path, search);
}

/*
 * File metadata
 */

/**
 * grl_fs_get_file_length:
 * @path: (type filename): Path to a file.
 *
 * Gets the length of the file at @path in bytes.
 *
 * Returns: The file length in bytes, or 0 if it cannot be determined.
 */
gint
grl_fs_get_file_length (const gchar *path)
{
    g_return_val_if_fail (path != NULL, 0);

    return GetFileLength (path);
}

/**
 * grl_fs_get_file_mod_time:
 * @path: (type filename): Path to a file.
 *
 * Gets the last modification time of the file at @path, as a Unix
 * timestamp (seconds since the epoch).
 *
 * Returns: The modification time, or 0 if it cannot be determined.
 */
gint64
grl_fs_get_file_mod_time (const gchar *path)
{
    g_return_val_if_fail (path != NULL, 0);

    return (gint64) GetFileModTime (path);
}

/*
 * Path string accessors
 *
 * raylib returns pointers into shared static storage for these; we always
 * hand back freshly allocated copies (transfer full).
 */

/**
 * grl_fs_get_file_extension:
 * @path: (type filename): Path or file name.
 *
 * Extracts the file extension of @path, including the leading dot
 * (e.g. ".png").
 *
 * Returns: (transfer full) (nullable): A newly allocated extension string,
 *   or %NULL if @path has no extension. Free with g_free().
 */
gchar *
grl_fs_get_file_extension (const gchar *path)
{
    const char *ext;

    g_return_val_if_fail (path != NULL, NULL);

    ext = GetFileExtension (path);
    return ext != NULL ? g_strdup (ext) : NULL;
}

/**
 * grl_fs_get_file_name:
 * @path: (type filename): A file path.
 *
 * Extracts the file name component of @path (the part after the last
 * directory separator).
 *
 * Returns: (transfer full) (nullable): A newly allocated file name string,
 *   or %NULL. Free with g_free().
 */
gchar *
grl_fs_get_file_name (const gchar *path)
{
    const char *name;

    g_return_val_if_fail (path != NULL, NULL);

    name = GetFileName (path);
    return name != NULL ? g_strdup (name) : NULL;
}

/**
 * grl_fs_get_file_name_without_ext:
 * @path: (type filename): A file path.
 *
 * Extracts the file name component of @path with its extension stripped.
 *
 * Returns: (transfer full) (nullable): A newly allocated file name string
 *   without its extension, or %NULL. Free with g_free().
 */
gchar *
grl_fs_get_file_name_without_ext (const gchar *path)
{
    const char *name;

    g_return_val_if_fail (path != NULL, NULL);

    name = GetFileNameWithoutExt (path);
    return name != NULL ? g_strdup (name) : NULL;
}

/**
 * grl_fs_get_directory_path:
 * @path: (type filename): A file path.
 *
 * Extracts the directory portion of @path (everything up to, but not
 * including, the final path component).
 *
 * Returns: (transfer full) (nullable): A newly allocated directory path,
 *   or %NULL. Free with g_free().
 */
gchar *
grl_fs_get_directory_path (const gchar *path)
{
    const char *dir;

    g_return_val_if_fail (path != NULL, NULL);

    dir = GetDirectoryPath (path);
    return dir != NULL ? g_strdup (dir) : NULL;
}

/**
 * grl_fs_get_prev_directory_path:
 * @path: (type filename): A directory path.
 *
 * Gets the parent directory of @path.
 *
 * Returns: (transfer full) (nullable): A newly allocated parent directory
 *   path, or %NULL. Free with g_free().
 */
gchar *
grl_fs_get_prev_directory_path (const gchar *path)
{
    const char *dir;

    g_return_val_if_fail (path != NULL, NULL);

    dir = GetPrevDirectoryPath (path);
    return dir != NULL ? g_strdup (dir) : NULL;
}

/**
 * grl_fs_get_working_directory:
 *
 * Gets the current working directory.
 *
 * Returns: (transfer full) (nullable): A newly allocated path string, or
 *   %NULL. Free with g_free().
 */
gchar *
grl_fs_get_working_directory (void)
{
    const char *dir = GetWorkingDirectory ();
    return dir != NULL ? g_strdup (dir) : NULL;
}

/**
 * grl_fs_get_application_directory:
 *
 * Gets the directory containing the running application's executable.
 *
 * Returns: (transfer full) (nullable): A newly allocated path string, or
 *   %NULL. Free with g_free().
 */
gchar *
grl_fs_get_application_directory (void)
{
    const char *dir = GetApplicationDirectory ();
    return dir != NULL ? g_strdup (dir) : NULL;
}

/*
 * Directory operations
 */

/**
 * grl_fs_make_directory:
 * @path: (type filename): Directory path to create.
 *
 * Creates the directory @path, including any missing parent directories.
 *
 * Returns: %TRUE on success.
 */
gboolean
grl_fs_make_directory (const gchar *path)
{
    g_return_val_if_fail (path != NULL, FALSE);

    /* raylib returns 0 on success, non-zero on failure. */
    return MakeDirectory (path) == 0;
}

/**
 * grl_fs_change_directory:
 * @path: (type filename): Directory path to change into.
 *
 * Changes the current working directory to @path.
 *
 * Returns: %TRUE on success.
 */
gboolean
grl_fs_change_directory (const gchar *path)
{
    unsigned char raw;

    g_return_val_if_fail (path != NULL, FALSE);

    raw = ChangeDirectory (path);
    return raw != 0;
}

/*
 * Helper: deep-copy a raylib FilePathList into a freshly allocated GStrv
 * and release the raylib-owned list. The returned array is NULL-terminated
 * and must be freed with g_strfreev().
 */
static GStrv
grl_fs_file_path_list_to_strv (FilePathList list)
{
    GStrv  result;
    guint  count;
    guint  i;

    count = list.count;
    result = g_new0 (gchar *, (gsize) count + 1);

    for (i = 0; i < count; i++)
        result[i] = g_strdup (list.paths[i] != NULL ? list.paths[i] : "");

    /* result[count] is already NULL from g_new0. */

    UnloadDirectoryFiles (list);

    return result;
}

/**
 * grl_fs_load_directory_files:
 * @dir_path: (type filename): Directory to list.
 *
 * Lists the files and subdirectories directly inside @dir_path (no
 * recursion).
 *
 * Returns: (transfer full) (array zero-terminated=1): A newly allocated,
 *   %NULL-terminated array of path strings. Free with g_strfreev().
 */
GStrv
grl_fs_load_directory_files (const gchar *dir_path)
{
    FilePathList list;

    g_return_val_if_fail (dir_path != NULL, NULL);

    list = LoadDirectoryFiles (dir_path);
    return grl_fs_file_path_list_to_strv (list);
}

/**
 * grl_fs_load_directory_files_ex:
 * @base_path: (type filename): Base directory to list.
 * @filter: (nullable): Extension/type filter such as "*.*", "FILES*" or
 *   "DIRS*", or %NULL for no filtering.
 * @scan_subdirs: Whether to recurse into subdirectories.
 *
 * Lists files under @base_path with optional extension filtering and
 * optional recursive scanning of subdirectories.
 *
 * Returns: (transfer full) (array zero-terminated=1): A newly allocated,
 *   %NULL-terminated array of path strings. Free with g_strfreev().
 */
GStrv
grl_fs_load_directory_files_ex (const gchar *base_path,
                                const gchar *filter,
                                gboolean     scan_subdirs)
{
    FilePathList list;

    g_return_val_if_fail (base_path != NULL, NULL);

    list = LoadDirectoryFilesEx (base_path, filter, scan_subdirs ? true : false);
    return grl_fs_file_path_list_to_strv (list);
}

/**
 * grl_fs_get_directory_file_count:
 * @dir_path: (type filename): Directory to count.
 *
 * Counts the files and subdirectories directly inside @dir_path (no
 * recursion).
 *
 * Returns: The number of entries.
 */
guint
grl_fs_get_directory_file_count (const gchar *dir_path)
{
    g_return_val_if_fail (dir_path != NULL, 0);

    return GetDirectoryFileCount (dir_path);
}

/**
 * grl_fs_get_directory_file_count_ex:
 * @base_path: (type filename): Base directory to count.
 * @filter: (nullable): Extension/type filter such as "*.*" or "DIR" (to
 *   include directories), or %NULL for no filtering.
 * @scan_subdirs: Whether to recurse into subdirectories.
 *
 * Counts files under @base_path with optional extension filtering and
 * optional recursive scanning of subdirectories.
 *
 * Returns: The number of entries.
 */
guint
grl_fs_get_directory_file_count_ex (const gchar *base_path,
                                    const gchar *filter,
                                    gboolean     scan_subdirs)
{
    g_return_val_if_fail (base_path != NULL, 0);

    return GetDirectoryFileCountEx (base_path, filter,
                                    scan_subdirs ? true : false);
}

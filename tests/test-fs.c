/* test-fs.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Unit tests for the grl_fs_* filesystem helpers.
 *
 * These tests run headless: they use real temporary files and directories
 * created via GLib (g_dir_make_tmp, g_file_set_contents) and exercise every
 * public grl_fs_* function in src/system/grl-fs.h, including edge cases.
 *
 * Expected return values are pinned against the ACTUAL behaviour of the
 * bundled raylib 6.0 implementation (deps/raylib/src/rcore.c), not the
 * idealised docs: e.g. GetFileExtension() includes the leading dot and
 * returns NULL when there is none, IsFileExtension() is case-insensitive,
 * GetDirectoryPath() of a bare filename returns "./", the "*.*" filter counts
 * directories, and a NULL filter counts files only.
 */

#include <glib.h>
#include <glib/gstdio.h>
#include <string.h>
#include <stdlib.h>
#include "src/system/grl-fs.h"

/*
 * Helpers
 */

/* Create a temporary directory; abort the test on failure. */
static gchar *
make_tmp_dir (void)
{
    g_autoptr(GError) error = NULL;
    gchar            *dir;

    dir = g_dir_make_tmp ("graylib-fs-test-XXXXXX", &error);
    g_assert_no_error (error);
    g_assert_nonnull (dir);

    return dir;
}

/* Write @contents to @path; abort on failure. */
static void
write_file (const gchar *path,
            const gchar *contents)
{
    g_autoptr(GError) error = NULL;

    g_assert_true (g_file_set_contents (path, contents, -1, &error));
    g_assert_no_error (error);
}

/* Recursively remove a directory tree (files one level deep + the dir). The
 * test creates only shallow trees, so this handles a single subdir level. */
static void
remove_dir_recursive (const gchar *dir)
{
    g_autoptr(GDir) d = NULL;
    const gchar    *name;

    d = g_dir_open (dir, 0, NULL);
    if (d == NULL)
        return;

    while ((name = g_dir_read_name (d)) != NULL)
    {
        g_autofree gchar *child = g_build_filename (dir, name, NULL);

        if (g_file_test (child, G_FILE_TEST_IS_DIR))
            remove_dir_recursive (child);
        else
            g_remove (child);
    }

    g_rmdir (dir);
}

/*
 * Existence / type queries
 */

static void
test_fs_existence_queries (void)
{
    g_autofree gchar *dir = make_tmp_dir ();
    g_autofree gchar *file = g_build_filename (dir, "file.txt", NULL);
    g_autofree gchar *subdir = g_build_filename (dir, "subdir", NULL);
    g_autofree gchar *missing = g_build_filename (dir, "nope.xyz", NULL);

    write_file (file, "hello");
    g_assert_cmpint (g_mkdir (subdir, 0755), ==, 0);

    /* file_exists: true for a regular file, true for a directory (access()
     * succeeds for both), false for a missing path. */
    g_assert_true (grl_fs_file_exists (file));
    g_assert_true (grl_fs_file_exists (subdir));
    g_assert_false (grl_fs_file_exists (missing));

    /* directory_exists: true only for a directory. */
    g_assert_true (grl_fs_directory_exists (dir));
    g_assert_true (grl_fs_directory_exists (subdir));
    g_assert_false (grl_fs_directory_exists (file));
    g_assert_false (grl_fs_directory_exists (missing));

    /* is_path_file: true only for a regular file (S_ISREG). */
    g_assert_true (grl_fs_is_path_file (file));
    g_assert_false (grl_fs_is_path_file (subdir));
    g_assert_false (grl_fs_is_path_file (missing));

    g_remove (file);
    g_rmdir (subdir);
    g_rmdir (dir);
}

static void
test_fs_is_file_extension (void)
{
    /* Matching, including the dot. */
    g_assert_true (grl_fs_is_file_extension ("a.png", ".png"));
    g_assert_true (grl_fs_is_file_extension ("/some/dir/a.png", ".png"));

    /* raylib lowercases both sides: matching is CASE-INSENSITIVE. */
    g_assert_true (grl_fs_is_file_extension ("a.PNG", ".png"));
    g_assert_true (grl_fs_is_file_extension ("a.png", ".PNG"));

    /* An ext provided without the leading dot also matches. */
    g_assert_true (grl_fs_is_file_extension ("a.png", "png"));

    /* A ';'-separated list matches any member. */
    g_assert_true (grl_fs_is_file_extension ("a.png", ".png;.jpg"));
    g_assert_true (grl_fs_is_file_extension ("a.jpg", ".png;.jpg"));

    /* Non-matching extension. */
    g_assert_false (grl_fs_is_file_extension ("a.png", ".jpg"));

    /* Multi-dot name uses the LAST extension. */
    g_assert_true (grl_fs_is_file_extension ("a.tar.gz", ".gz"));
    g_assert_false (grl_fs_is_file_extension ("a.tar.gz", ".tar"));

    /* No extension at all: never matches. */
    g_assert_false (grl_fs_is_file_extension ("noext", ".png"));
}

static void
test_fs_is_file_name_valid (void)
{
    /* Plain valid names. */
    g_assert_true (grl_fs_is_file_name_valid ("hello.txt"));
    g_assert_true (grl_fs_is_file_name_valid ("a_file-1.2.3.dat"));

    /* Reserved / invalid characters make a name invalid. */
    g_assert_false (grl_fs_is_file_name_valid ("bad/name"));
    g_assert_false (grl_fs_is_file_name_valid ("bad\\name"));
    g_assert_false (grl_fs_is_file_name_valid ("bad:name"));
    g_assert_false (grl_fs_is_file_name_valid ("bad?name"));
    g_assert_false (grl_fs_is_file_name_valid ("bad*name"));

    /* An all-periods name is invalid. */
    g_assert_false (grl_fs_is_file_name_valid (".."));
    g_assert_false (grl_fs_is_file_name_valid ("..."));

    /* Quirk: raylib treats the empty string as VALID (it skips the
     * validation loop entirely and leaves the initial value TRUE). */
    g_assert_true (grl_fs_is_file_name_valid (""));
}

/*
 * File operations: rename / remove
 */

static void
test_fs_file_rename (void)
{
    g_autofree gchar *dir = make_tmp_dir ();
    g_autofree gchar *src = g_build_filename (dir, "old.txt", NULL);
    g_autofree gchar *dst = g_build_filename (dir, "new.txt", NULL);

    write_file (src, "data");

    /* Success path: rename the existing file. */
    g_assert_true (grl_fs_file_rename (src, dst));
    g_assert_false (grl_fs_file_exists (src));
    g_assert_true (grl_fs_file_exists (dst));

    /* Failure path: renaming a non-existent file returns FALSE (raylib
     * returns -1 when the source does not exist). */
    g_assert_false (grl_fs_file_rename (src, dst));

    g_remove (dst);
    g_rmdir (dir);
}

static void
test_fs_file_remove (void)
{
    g_autofree gchar *dir = make_tmp_dir ();
    g_autofree gchar *file = g_build_filename (dir, "victim.txt", NULL);

    write_file (file, "bye");

    /* Success path. */
    g_assert_true (grl_fs_file_exists (file));
    g_assert_true (grl_fs_file_remove (file));
    g_assert_false (grl_fs_file_exists (file));

    /* Failure path: removing a missing file returns FALSE. */
    g_assert_false (grl_fs_file_remove (file));

    g_rmdir (dir);
}

/*
 * File operations: copy
 */

static void
test_fs_file_copy (void)
{
    g_autoptr(GError) error = NULL;
    g_autofree gchar *dir = make_tmp_dir ();
    g_autofree gchar *src = g_build_filename (dir, "src.bin", NULL);
    g_autofree gchar *dst = g_build_filename (dir, "dst.bin", NULL);
    g_autofree gchar *back = NULL;
    gsize             len = 0;

    /* raylib's FileCopy succeeds only when the source has > 0 bytes. */
    write_file (src, "known-payload-bytes");

    g_assert_false (grl_fs_file_exists (dst));
    g_assert_true (grl_fs_file_copy (src, dst));
    g_assert_true (grl_fs_file_exists (dst));

    /* Round-trip: destination contents must equal the source. */
    g_assert_true (g_file_get_contents (dst, &back, &len, &error));
    g_assert_no_error (error);
    g_assert_cmpstr (back, ==, "known-payload-bytes");
    g_assert_cmpuint (len, ==, strlen ("known-payload-bytes"));

    g_remove (dst);
    g_remove (src);
    g_rmdir (dir);
}

static void
test_fs_file_copy_creates_dirs (void)
{
    g_autofree gchar *dir = make_tmp_dir ();
    g_autofree gchar *src = g_build_filename (dir, "src.bin", NULL);
    g_autofree gchar *nested = g_build_filename (dir, "a", "b", NULL);
    g_autofree gchar *dst = g_build_filename (dir, "a", "b", "dst.bin", NULL);

    write_file (src, "payload");

    /* The intermediate "a/b" directory does not exist yet; FileCopy creates
     * the destination's parent directories. */
    g_assert_false (grl_fs_directory_exists (nested));
    g_assert_true (grl_fs_file_copy (src, dst));
    g_assert_true (grl_fs_directory_exists (nested));
    g_assert_true (grl_fs_file_exists (dst));

    g_remove (src);
    remove_dir_recursive (dir);
}

/*
 * File operations: move
 */

static void
test_fs_file_move (void)
{
    g_autofree gchar *dir = make_tmp_dir ();
    g_autofree gchar *src = g_build_filename (dir, "src.bin", NULL);
    g_autofree gchar *dst = g_build_filename (dir, "moved.bin", NULL);
    g_autoptr(GError) error = NULL;
    g_autofree gchar *back = NULL;

    write_file (src, "moveme");

    /* The wrapper routes around raylib's broken FileMove via g_rename
     * (or copy + remove): dst exists with the contents, src is gone. */
    g_assert_true (grl_fs_file_move (src, dst));
    g_assert_false (grl_fs_file_exists (src));
    g_assert_true (grl_fs_file_exists (dst));

    g_assert_true (g_file_get_contents (dst, &back, NULL, &error));
    g_assert_no_error (error);
    g_assert_cmpstr (back, ==, "moveme");

    g_remove (dst);
    g_rmdir (dir);
}

/*
 * File text editing
 */

static void
test_fs_file_text_replace (void)
{
    g_autoptr(GError) error = NULL;
    g_autofree gchar *dir = make_tmp_dir ();
    g_autofree gchar *file = g_build_filename (dir, "text.txt", NULL);
    g_autofree gchar *back = NULL;

    write_file (file, "the quick brown FOX jumps");

    g_assert_true (grl_fs_file_text_replace (file, "FOX", "cat"));

    g_assert_true (g_file_get_contents (file, &back, NULL, &error));
    g_assert_no_error (error);
    g_assert_nonnull (strstr (back, "cat"));
    g_assert_null (strstr (back, "FOX"));

    g_remove (file);
    g_rmdir (dir);
}

static void
test_fs_file_text_find_index (void)
{
    g_autofree gchar *dir = make_tmp_dir ();
    g_autofree gchar *file = g_build_filename (dir, "find.txt", NULL);

    write_file (file, "hello world");

    /* "world" begins at byte index 6. */
    g_assert_cmpint (grl_fs_file_text_find_index (file, "world"), ==, 6);

    /* The first token is at index 0. */
    g_assert_cmpint (grl_fs_file_text_find_index (file, "hello"), ==, 0);

    /* A missing token returns a negative index. */
    g_assert_cmpint (grl_fs_file_text_find_index (file, "absent"), <, 0);

    /* A missing file returns a negative index. */
    g_remove (file);
    g_assert_cmpint (grl_fs_file_text_find_index (file, "hello"), <, 0);

    g_rmdir (dir);
}

/*
 * File metadata
 */

static void
test_fs_get_file_length (void)
{
    g_autofree gchar *dir = make_tmp_dir ();
    g_autofree gchar *file = g_build_filename (dir, "len.bin", NULL);
    const gchar      *contents = "0123456789ABCDEF";

    write_file (file, contents);

    g_assert_cmpint (grl_fs_get_file_length (file),
                     ==,
                     (gint) strlen (contents));

    /* A missing file reports length 0. */
    g_remove (file);
    g_assert_cmpint (grl_fs_get_file_length (file), ==, 0);

    g_rmdir (dir);
}

static void
test_fs_get_file_mod_time (void)
{
    g_autofree gchar *dir = make_tmp_dir ();
    g_autofree gchar *file = g_build_filename (dir, "mtime.bin", NULL);

    write_file (file, "stamp");

    /* An existing file has a positive modification time. */
    g_assert_cmpint (grl_fs_get_file_mod_time (file), >, 0);

    /* A missing file reports 0. */
    g_remove (file);
    g_assert_cmpint (grl_fs_get_file_mod_time (file), ==, 0);

    g_rmdir (dir);
}

/*
 * Path-string accessors (operate purely on the string argument)
 */

static void
test_fs_get_file_extension (void)
{
    g_autofree gchar *png = grl_fs_get_file_extension ("/some/dir/sprite.png");
    g_autofree gchar *targz = grl_fs_get_file_extension ("a.tar.gz");
    gchar            *noext;
    gchar            *hidden;

    /* raylib returns the extension INCLUDING the leading dot. */
    g_assert_nonnull (png);
    g_assert_cmpstr (png, ==, ".png");

    /* Multi-dot: only the last extension. */
    g_assert_nonnull (targz);
    g_assert_cmpstr (targz, ==, ".gz");

    /* No extension: GetFileExtension returns NULL, so the wrapper returns
     * NULL too (not ""). */
    noext = grl_fs_get_file_extension ("noext");
    g_assert_null (noext);

    /* A leading-dot-only name (e.g. ".hidden") has no extension per raylib
     * (the dot is the first char), so the wrapper returns NULL. */
    hidden = grl_fs_get_file_extension (".hidden");
    g_assert_null (hidden);
}

static void
test_fs_get_file_name (void)
{
    g_autofree gchar *name = grl_fs_get_file_name ("/some/dir/sprite.png");
    g_autofree gchar *bare = grl_fs_get_file_name ("sprite.png");
    g_autofree gchar *nested = grl_fs_get_file_name ("a/b/c/d.txt");

    /* Normal path: component after the last separator. */
    g_assert_nonnull (name);
    g_assert_cmpstr (name, ==, "sprite.png");

    /* No directory: the whole string is the file name. */
    g_assert_nonnull (bare);
    g_assert_cmpstr (bare, ==, "sprite.png");

    /* Deeply nested. */
    g_assert_nonnull (nested);
    g_assert_cmpstr (nested, ==, "d.txt");
}

static void
test_fs_get_file_name_without_ext (void)
{
    g_autofree gchar *name = grl_fs_get_file_name_without_ext ("/some/dir/sprite.png");
    g_autofree gchar *bare = grl_fs_get_file_name_without_ext ("sprite.png");
    g_autofree gchar *targz = grl_fs_get_file_name_without_ext ("/x/a.tar.gz");

    g_assert_nonnull (name);
    g_assert_cmpstr (name, ==, "sprite");

    g_assert_nonnull (bare);
    g_assert_cmpstr (bare, ==, "sprite");

    /* Strips only the final extension. */
    g_assert_nonnull (targz);
    g_assert_cmpstr (targz, ==, "a.tar");
}

static void
test_fs_get_directory_path (void)
{
    g_autofree gchar *nested = grl_fs_get_directory_path ("/some/dir/sprite.png");
    g_autofree gchar *bare = grl_fs_get_directory_path ("sprite.png");
    g_autofree gchar *root = grl_fs_get_directory_path ("/file.txt");

    /* Absolute nested path: everything up to the last separator. */
    g_assert_nonnull (nested);
    g_assert_cmpstr (nested, ==, "/some/dir");

    /* A bare filename with no separator yields "./" (raylib prepends the
     * current-directory marker for relative inputs). */
    g_assert_nonnull (bare);
    g_assert_cmpstr (bare, ==, "./");

    /* A file in the root directory: the single leading slash. */
    g_assert_nonnull (root);
    g_assert_cmpstr (root, ==, "/");
}

static void
test_fs_get_prev_directory_path (void)
{
    g_autofree gchar *prev = grl_fs_get_prev_directory_path ("/some/dir");
    g_autofree gchar *deeper = grl_fs_get_prev_directory_path ("/a/b/c");
    g_autofree gchar *shortp = grl_fs_get_prev_directory_path ("/a");

    /* Parent directory of a nested path. */
    g_assert_nonnull (prev);
    g_assert_cmpstr (prev, ==, "/some");

    g_assert_nonnull (deeper);
    g_assert_cmpstr (deeper, ==, "/a/b");

    /* A path of length <= 3 is returned unchanged (root-ish). */
    g_assert_nonnull (shortp);
    g_assert_cmpstr (shortp, ==, "/a");
}

static void
test_fs_get_working_directory (void)
{
    g_autofree gchar *cwd = grl_fs_get_working_directory ();

    g_assert_nonnull (cwd);
    g_assert_cmpuint (strlen (cwd), >, 0);
}

static void
test_fs_get_application_directory (void)
{
    g_autofree gchar *appdir = grl_fs_get_application_directory ();

    g_assert_nonnull (appdir);
    g_assert_cmpuint (strlen (appdir), >, 0);
}

/*
 * Directory operations
 */

static void
test_fs_make_directory (void)
{
    g_autofree gchar *dir = make_tmp_dir ();
    g_autofree gchar *nested = g_build_filename (dir, "x", "y", "z", NULL);

    /* Create a nested path under the temp dir (parents are created). */
    g_assert_false (grl_fs_directory_exists (nested));
    g_assert_true (grl_fs_make_directory (nested));
    g_assert_true (grl_fs_directory_exists (nested));

    /* Already-exists: raylib returns 0 (success), so the wrapper is TRUE. */
    g_assert_true (grl_fs_make_directory (nested));
    g_assert_true (grl_fs_directory_exists (nested));

    remove_dir_recursive (dir);
}

static void
test_fs_change_directory (void)
{
    g_autofree gchar *orig = g_get_current_dir ();
    g_autofree gchar *dir = make_tmp_dir ();
    g_autofree gchar *now = NULL;
    g_autofree gchar *real_dir = NULL;
    g_autofree gchar *real_now = NULL;

    g_assert_nonnull (orig);

    /* Change into the temp dir, then verify the working directory reflects
     * it. Compare canonicalised paths because /tmp may be a symlink (macOS,
     * some Linux setups) and getcwd resolves it. */
    g_assert_true (grl_fs_change_directory (dir));

    now = grl_fs_get_working_directory ();
    g_assert_nonnull (now);

    real_dir = realpath (dir, NULL);
    real_now = realpath (now, NULL);
    g_assert_nonnull (real_dir);
    g_assert_nonnull (real_now);
    g_assert_cmpstr (real_now, ==, real_dir);

    /* CRITICAL: restore the original working directory so sibling tests are
     * not affected by the CWD mutation. */
    g_assert_true (grl_fs_change_directory (orig));

    g_rmdir (dir);
}

static void
test_fs_load_directory_files (void)
{
    g_auto(GStrv) files = NULL;
    g_autofree gchar *dir = make_tmp_dir ();
    g_autofree gchar *a = g_build_filename (dir, "a.txt", NULL);
    g_autofree gchar *b = g_build_filename (dir, "b.txt", NULL);
    g_autofree gchar *c = g_build_filename (dir, "c.dat", NULL);

    write_file (a, "1");
    write_file (b, "2");
    write_file (c, "3");

    files = grl_fs_load_directory_files (dir);

    /* Non-NULL, NULL-terminated, exactly 3 entries (no recursion, no
     * subdirs present). */
    g_assert_nonnull (files);
    g_assert_cmpuint (g_strv_length (files), ==, 3);
    g_assert_null (files[3]);

    g_remove (a);
    g_remove (b);
    g_remove (c);
    g_rmdir (dir);
}

static void
test_fs_load_directory_files_ex (void)
{
    g_auto(GStrv) txt = NULL;
    g_auto(GStrv) rec = NULL;
    g_autofree gchar *dir = make_tmp_dir ();
    g_autofree gchar *a = g_build_filename (dir, "a.txt", NULL);
    g_autofree gchar *b = g_build_filename (dir, "b.txt", NULL);
    g_autofree gchar *c = g_build_filename (dir, "c.dat", NULL);
    g_autofree gchar *sub = g_build_filename (dir, "sub", NULL);
    g_autofree gchar *d = g_build_filename (dir, "sub", "d.txt", NULL);

    write_file (a, "1");
    write_file (b, "2");
    write_file (c, "3");
    g_assert_cmpint (g_mkdir (sub, 0755), ==, 0);
    write_file (d, "4");

    /* ".txt" filter, no recursion: only a.txt and b.txt. Note the filter
     * uses raylib's IsFileExtension convention (".txt"), NOT a glob. */
    txt = grl_fs_load_directory_files_ex (dir, ".txt", FALSE);
    g_assert_nonnull (txt);
    g_assert_cmpuint (g_strv_length (txt), ==, 2);
    g_assert_null (txt[2]);

    /* ".txt" filter, recursive: a.txt, b.txt and sub/d.txt. */
    rec = grl_fs_load_directory_files_ex (dir, ".txt", TRUE);
    g_assert_nonnull (rec);
    g_assert_cmpuint (g_strv_length (rec), ==, 3);

    g_remove (a);
    g_remove (b);
    g_remove (c);
    g_remove (d);
    g_rmdir (sub);
    g_rmdir (dir);
}

static void
test_fs_get_directory_file_count (void)
{
    g_autofree gchar *dir = make_tmp_dir ();
    g_autofree gchar *a = g_build_filename (dir, "a.txt", NULL);
    g_autofree gchar *b = g_build_filename (dir, "b.txt", NULL);
    g_autofree gchar *c = g_build_filename (dir, "c.dat", NULL);
    g_autofree gchar *sub = g_build_filename (dir, "sub", NULL);

    write_file (a, "1");
    write_file (b, "2");
    write_file (c, "3");
    g_assert_cmpint (g_mkdir (sub, 0755), ==, 0);

    /* The plain count uses raylib's "*.*" filter, which counts both files
     * and directories: 3 files + 1 subdir == 4. */
    g_assert_cmpuint (grl_fs_get_directory_file_count (dir), ==, 4);

    g_remove (a);
    g_remove (b);
    g_remove (c);
    g_rmdir (sub);
    g_rmdir (dir);
}

static void
test_fs_get_directory_file_count_ex (void)
{
    g_autofree gchar *dir = make_tmp_dir ();
    g_autofree gchar *a = g_build_filename (dir, "a.txt", NULL);
    g_autofree gchar *b = g_build_filename (dir, "b.txt", NULL);
    g_autofree gchar *c = g_build_filename (dir, "c.dat", NULL);
    g_autofree gchar *sub = g_build_filename (dir, "sub", NULL);
    g_autofree gchar *d = g_build_filename (dir, "sub", "d.txt", NULL);

    write_file (a, "1");
    write_file (b, "2");
    write_file (c, "3");
    g_assert_cmpint (g_mkdir (sub, 0755), ==, 0);
    write_file (d, "4");

    /* NULL filter counts files only (the directory is excluded): 3. */
    g_assert_cmpuint (grl_fs_get_directory_file_count_ex (dir, NULL, FALSE),
                      ==, 3);

    /* ".txt" filter, no recursion: a.txt, b.txt -> 2. */
    g_assert_cmpuint (grl_fs_get_directory_file_count_ex (dir, ".txt", FALSE),
                      ==, 2);

    /* ".txt" filter, recursive: a.txt, b.txt, sub/d.txt -> 3. */
    g_assert_cmpuint (grl_fs_get_directory_file_count_ex (dir, ".txt", TRUE),
                      ==, 3);

    g_remove (a);
    g_remove (b);
    g_remove (c);
    g_remove (d);
    g_rmdir (sub);
    g_rmdir (dir);
}

/*
 * Test registration
 */

int
main (int   argc,
      char *argv[])
{
    g_test_init (&argc, &argv, NULL);

    /* Existence / type */
    g_test_add_func ("/fs/existence-queries", test_fs_existence_queries);
    g_test_add_func ("/fs/is-file-extension", test_fs_is_file_extension);
    g_test_add_func ("/fs/is-file-name-valid", test_fs_is_file_name_valid);

    /* File operations */
    g_test_add_func ("/fs/file-rename", test_fs_file_rename);
    g_test_add_func ("/fs/file-remove", test_fs_file_remove);
    g_test_add_func ("/fs/file-copy", test_fs_file_copy);
    g_test_add_func ("/fs/file-copy-creates-dirs", test_fs_file_copy_creates_dirs);
    g_test_add_func ("/fs/file-move", test_fs_file_move);

    /* File text editing */
    g_test_add_func ("/fs/file-text-replace", test_fs_file_text_replace);
    g_test_add_func ("/fs/file-text-find-index", test_fs_file_text_find_index);

    /* File metadata */
    g_test_add_func ("/fs/get-file-length", test_fs_get_file_length);
    g_test_add_func ("/fs/get-file-mod-time", test_fs_get_file_mod_time);

    /* Path-string accessors */
    g_test_add_func ("/fs/get-file-extension", test_fs_get_file_extension);
    g_test_add_func ("/fs/get-file-name", test_fs_get_file_name);
    g_test_add_func ("/fs/get-file-name-without-ext", test_fs_get_file_name_without_ext);
    g_test_add_func ("/fs/get-directory-path", test_fs_get_directory_path);
    g_test_add_func ("/fs/get-prev-directory-path", test_fs_get_prev_directory_path);
    g_test_add_func ("/fs/get-working-directory", test_fs_get_working_directory);
    g_test_add_func ("/fs/get-application-directory", test_fs_get_application_directory);

    /* Directory operations */
    g_test_add_func ("/fs/make-directory", test_fs_make_directory);
    g_test_add_func ("/fs/change-directory", test_fs_change_directory);
    g_test_add_func ("/fs/load-directory-files", test_fs_load_directory_files);
    g_test_add_func ("/fs/load-directory-files-ex", test_fs_load_directory_files_ex);
    g_test_add_func ("/fs/get-directory-file-count", test_fs_get_directory_file_count);
    g_test_add_func ("/fs/get-directory-file-count-ex", test_fs_get_directory_file_count_ex);

    return g_test_run ();
}

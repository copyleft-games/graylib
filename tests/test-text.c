/* test-text.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Unit tests for the grl_text_* string utilities.
 *
 * These tests pin down the *actual* behaviour of raylib's text API as
 * wrapped by graylib (TextLength, TextToInteger, TextToSnake, ...). Where
 * raylib's behaviour is quirky (e.g. TextToInteger stops at the first
 * non-digit, TextToSnake only treats ASCII upper-case as a word boundary)
 * the expected values below were derived from reading
 * deps/raylib/src/rtext.c and are asserted exactly.
 */

#include <glib.h>
#include "src/text/grl-text-utils.h"

#define EPSILON 0.0001f

/*
 * =============================================================================
 * Query functions
 * =============================================================================
 */

static void
test_text_length (void)
{
    /* TextLength is a byte count to the terminating NUL. */
    g_assert_cmpuint (grl_text_length (""), ==, 0);
    g_assert_cmpuint (grl_text_length ("hello"), ==, 5);
    g_assert_cmpuint (grl_text_length ("a"), ==, 1);

    /* Multibyte UTF-8: "é" is U+00E9 -> 0xC3 0xA9 (2 bytes); the string
     * "héllo" is h(1) + é(2) + llo(3) = 6 bytes, NOT 5 characters. */
    g_assert_cmpuint (grl_text_length ("\xC3\xA9"), ==, 2);
    g_assert_cmpuint (grl_text_length ("h\xC3\xA9llo"), ==, 6);
}

static void
test_text_is_equal (void)
{
    g_assert_true (grl_text_is_equal ("abc", "abc"));
    g_assert_true (grl_text_is_equal ("", ""));
    g_assert_false (grl_text_is_equal ("abc", "abd"));
    g_assert_false (grl_text_is_equal ("abc", "ab"));
    g_assert_false (grl_text_is_equal ("", "x"));
    g_assert_false (grl_text_is_equal ("x", ""));
    /* Case sensitive (byte-for-byte). */
    g_assert_false (grl_text_is_equal ("abc", "ABC"));
}

static void
test_text_find_index (void)
{
    /* Match at the start -> index 0. */
    g_assert_cmpint (grl_text_find_index ("hello", "he"), ==, 0);
    /* Match in the middle. */
    g_assert_cmpint (grl_text_find_index ("hello", "ll"), ==, 2);
    /* Match at the end. */
    g_assert_cmpint (grl_text_find_index ("hello", "lo"), ==, 3);
    /* Not found -> -1. */
    g_assert_cmpint (grl_text_find_index ("hello", "xyz"), ==, -1);
    /* Empty needle: strstr() returns the haystack, so index 0. */
    g_assert_cmpint (grl_text_find_index ("hello", ""), ==, 0);
}

static void
test_text_to_integer (void)
{
    g_assert_cmpint (grl_text_to_integer ("42"), ==, 42);
    g_assert_cmpint (grl_text_to_integer ("-7"), ==, -7);
    g_assert_cmpint (grl_text_to_integer ("+5"), ==, 5);
    g_assert_cmpint (grl_text_to_integer ("0"), ==, 0);

    /* Leading whitespace is NOT skipped: the digit scan stops on the first
     * non-digit (the space), so "  12" parses as 0. */
    g_assert_cmpint (grl_text_to_integer ("  12"), ==, 0);
    /* Non-numeric text parses as 0. */
    g_assert_cmpint (grl_text_to_integer ("abc"), ==, 0);
    g_assert_cmpint (grl_text_to_integer (""), ==, 0);
    /* Parsing stops at the first non-digit, keeping the leading digits. */
    g_assert_cmpint (grl_text_to_integer ("3x"), ==, 3);
    g_assert_cmpint (grl_text_to_integer ("123abc"), ==, 123);
}

static void
test_text_to_float (void)
{
    g_assert_cmpfloat_with_epsilon (grl_text_to_float ("3.14"), 3.14f, EPSILON);
    g_assert_cmpfloat_with_epsilon (grl_text_to_float ("-2.5"), -2.5f, EPSILON);
    g_assert_cmpfloat_with_epsilon (grl_text_to_float ("0"), 0.0f, EPSILON);
    g_assert_cmpfloat_with_epsilon (grl_text_to_float ("0.0"), 0.0f, EPSILON);
    g_assert_cmpfloat_with_epsilon (grl_text_to_float ("3.5"), 3.5f, EPSILON);
    g_assert_cmpfloat_with_epsilon (grl_text_to_float ("42"), 42.0f, EPSILON);
}

/*
 * =============================================================================
 * Transformation functions
 * =============================================================================
 */

static void
test_text_subtext (void)
{
    g_autofree gchar *normal = NULL;
    g_autofree gchar *at_zero = NULL;
    g_autofree gchar *clamped = NULL;
    g_autofree gchar *past_end = NULL;
    g_autofree gchar *zero_len = NULL;

    /* Normal slice: 5 bytes from position 6 of "hello world". */
    normal = grl_text_subtext ("hello world", 6, 5);
    g_assert_nonnull (normal);
    g_assert_cmpstr (normal, ==, "world");

    /* Position 0. */
    at_zero = grl_text_subtext ("hello", 0, 3);
    g_assert_nonnull (at_zero);
    g_assert_cmpstr (at_zero, ==, "hel");

    /* Length longer than remaining bytes -> clamped to the remainder. */
    clamped = grl_text_subtext ("hello", 3, 100);
    g_assert_nonnull (clamped);
    g_assert_cmpstr (clamped, ==, "lo");

    /* Position past the end -> empty string (position >= textLength). */
    past_end = grl_text_subtext ("hello", 99, 3);
    g_assert_nonnull (past_end);
    g_assert_cmpstr (past_end, ==, "");

    /* Zero length -> empty string. */
    zero_len = grl_text_subtext ("hello", 0, 0);
    g_assert_nonnull (zero_len);
    g_assert_cmpstr (zero_len, ==, "");
}

static void
test_text_remove_spaces (void)
{
    g_autofree gchar *mid = NULL;
    g_autofree gchar *edges = NULL;
    g_autofree gchar *none = NULL;
    g_autofree gchar *empty = NULL;

    mid = grl_text_remove_spaces ("a b c");
    g_assert_nonnull (mid);
    g_assert_cmpstr (mid, ==, "abc");

    edges = grl_text_remove_spaces ("  trim  me  ");
    g_assert_nonnull (edges);
    g_assert_cmpstr (edges, ==, "trimme");

    none = grl_text_remove_spaces ("nospaces");
    g_assert_nonnull (none);
    g_assert_cmpstr (none, ==, "nospaces");

    empty = grl_text_remove_spaces ("");
    g_assert_nonnull (empty);
    g_assert_cmpstr (empty, ==, "");
}

static void
test_text_get_between (void)
{
    g_autofree gchar *single = NULL;
    g_autofree gchar *word = NULL;
    g_autofree gchar *no_begin = NULL;
    g_autofree gchar *no_end = NULL;
    g_autofree gchar *empty_between = NULL;

    /* "[x]" between "[" and "]" -> "x". */
    single = grl_text_get_between ("[x]", "[", "]");
    g_assert_nonnull (single);
    g_assert_cmpstr (single, ==, "x");

    word = grl_text_get_between ("foo(bar)baz", "(", ")");
    g_assert_nonnull (word);
    g_assert_cmpstr (word, ==, "bar");

    /* Begin marker absent -> empty string. */
    no_begin = grl_text_get_between ("hello", "[", "]");
    g_assert_nonnull (no_begin);
    g_assert_cmpstr (no_begin, ==, "");

    /* Begin present but end absent -> empty string. */
    no_end = grl_text_get_between ("[hello", "[", "]");
    g_assert_nonnull (no_end);
    g_assert_cmpstr (no_end, ==, "");

    /* Empty span -> empty string. */
    empty_between = grl_text_get_between ("[]", "[", "]");
    g_assert_nonnull (empty_between);
    g_assert_cmpstr (empty_between, ==, "");
}

static void
test_text_replace (void)
{
    g_autofree gchar *single = NULL;
    g_autofree gchar *multiple = NULL;
    g_autofree gchar *no_match = NULL;
    g_autofree gchar *with_empty = NULL;

    /* Single occurrence. */
    single = grl_text_replace ("hello world", "world", "there");
    g_assert_nonnull (single);
    g_assert_cmpstr (single, ==, "hello there");

    /* Multiple occurrences are all replaced. */
    multiple = grl_text_replace ("aaa", "a", "b");
    g_assert_nonnull (multiple);
    g_assert_cmpstr (multiple, ==, "bbb");

    /* No match -> unchanged copy of the original. */
    no_match = grl_text_replace ("hello", "xyz", "abc");
    g_assert_nonnull (no_match);
    g_assert_cmpstr (no_match, ==, "hello");

    /* Replace with empty string -> deletion of every match. */
    with_empty = grl_text_replace ("a-b-c", "-", "");
    g_assert_nonnull (with_empty);
    g_assert_cmpstr (with_empty, ==, "abc");
}

static void
test_text_replace_between (void)
{
    g_autofree gchar *basic = NULL;
    g_autofree gchar *delete_span = NULL;
    g_autofree gchar *empty_repl = NULL;
    gchar            *no_match;

    /* Replace the span between the markers, markers preserved. */
    basic = grl_text_replace_between ("[x]", "[", "]", "Y");
    g_assert_nonnull (basic);
    g_assert_cmpstr (basic, ==, "[Y]");

    /* NULL replacement removes the span (markers preserved). */
    delete_span = grl_text_replace_between ("foo(bar)baz", "(", ")", NULL);
    g_assert_nonnull (delete_span);
    g_assert_cmpstr (delete_span, ==, "foo()baz");

    /* Empty-string replacement also collapses the span. */
    empty_repl = grl_text_replace_between ("foo(bar)baz", "(", ")", "");
    g_assert_nonnull (empty_repl);
    g_assert_cmpstr (empty_repl, ==, "foo()baz");

    /* No match -> NULL (begin or end not found). */
    no_match = grl_text_replace_between ("hello", "[", "]", "Y");
    g_assert_null (no_match);
}

static void
test_text_insert (void)
{
    g_autofree gchar *append = NULL;
    g_autofree gchar *append_empty = NULL;
    g_autofree gchar *prepend = NULL;
    g_autofree gchar *middle = NULL;

    /* Insert at the end (position == strlen(text)) -> append. This is the
     * only fully well-defined case: raylib's TextInsertAlloc has a known
     * bug for position != textLen (its tail-copy loop indexes the source
     * with the destination index, reading past the source NUL), so the
     * bytes AFTER the insertion point are non-deterministic garbage. We
     * therefore assert the append case exactly and only the deterministic
     * leading bytes for the prepend/middle cases. */
    append = grl_text_insert ("hello", " world", 5);
    g_assert_nonnull (append);
    g_assert_cmpstr (append, ==, "hello world");

    /* Empty insert at the end is a no-op copy. */
    append_empty = grl_text_insert ("abc", "", 3);
    g_assert_nonnull (append_empty);
    g_assert_cmpstr (append_empty, ==, "abc");

    /* Insert at position 0 -> prepend.  grl_text_insert() splices with GLib
     * (raylib's TextInsertAlloc is OOB-buggy for position != textLen), so the
     * full result is well-defined. */
    prepend = grl_text_insert ("world", "hello ", 0);
    g_assert_nonnull (prepend);
    g_assert_cmpstr (prepend, ==, "hello world");

    /* Insert in the middle. */
    middle = grl_text_insert ("ac", "b", 1);
    g_assert_nonnull (middle);
    g_assert_cmpstr (middle, ==, "abc");
}

static void
test_text_join (void)
{
    const gchar *three[]  = { "a", "b", "c", NULL };
    const gchar *single[] = { "only", NULL };
    const gchar *empty[]  = { NULL };
    g_autofree gchar *joined = NULL;
    g_autofree gchar *one = NULL;
    g_autofree gchar *none = NULL;

    joined = grl_text_join (three, ",");
    g_assert_nonnull (joined);
    g_assert_cmpstr (joined, ==, "a,b,c");

    /* Single element: no delimiter is appended. */
    one = grl_text_join (single, ",");
    g_assert_nonnull (one);
    g_assert_cmpstr (one, ==, "only");

    /* Empty (zero-length) list -> empty string. */
    none = grl_text_join (empty, ",");
    g_assert_nonnull (none);
    g_assert_cmpstr (none, ==, "");
}

static void
test_text_split (void)
{
    g_auto(GStrv) basic = NULL;
    g_auto(GStrv) no_delim = NULL;
    g_auto(GStrv) trailing = NULL;
    g_auto(GStrv) consecutive = NULL;
    g_auto(GStrv) empty = NULL;

    /* Three elements. */
    basic = grl_text_split ("a,b,c", ',');
    g_assert_nonnull (basic);
    g_assert_cmpuint (g_strv_length (basic), ==, 3);
    g_assert_cmpstr (basic[0], ==, "a");
    g_assert_cmpstr (basic[1], ==, "b");
    g_assert_cmpstr (basic[2], ==, "c");
    g_assert_null (basic[3]);

    /* No delimiter present -> one element (the whole string). */
    no_delim = grl_text_split ("abc", ',');
    g_assert_nonnull (no_delim);
    g_assert_cmpuint (g_strv_length (no_delim), ==, 1);
    g_assert_cmpstr (no_delim[0], ==, "abc");
    g_assert_null (no_delim[1]);

    /* Trailing delimiter -> an empty trailing field: "a","b","". */
    trailing = grl_text_split ("a,b,", ',');
    g_assert_nonnull (trailing);
    g_assert_cmpuint (g_strv_length (trailing), ==, 3);
    g_assert_cmpstr (trailing[0], ==, "a");
    g_assert_cmpstr (trailing[1], ==, "b");
    g_assert_cmpstr (trailing[2], ==, "");
    g_assert_null (trailing[3]);

    /* Consecutive delimiters -> an empty field between: "a","","b". */
    consecutive = grl_text_split ("a,,b", ',');
    g_assert_nonnull (consecutive);
    g_assert_cmpuint (g_strv_length (consecutive), ==, 3);
    g_assert_cmpstr (consecutive[0], ==, "a");
    g_assert_cmpstr (consecutive[1], ==, "");
    g_assert_cmpstr (consecutive[2], ==, "b");
    g_assert_null (consecutive[3]);

    /* Empty string -> one (empty) element. */
    empty = grl_text_split ("", ',');
    g_assert_nonnull (empty);
    g_assert_cmpuint (g_strv_length (empty), ==, 1);
    g_assert_cmpstr (empty[0], ==, "");
    g_assert_null (empty[1]);
}

/*
 * =============================================================================
 * Case conversion functions
 * =============================================================================
 */

static void
test_text_to_upper (void)
{
    g_autofree gchar *mixed = NULL;
    g_autofree gchar *already = NULL;
    g_autofree gchar *empty = NULL;

    mixed = grl_text_to_upper ("Hello World 123");
    g_assert_nonnull (mixed);
    g_assert_cmpstr (mixed, ==, "HELLO WORLD 123");

    already = grl_text_to_upper ("ALLCAPS");
    g_assert_nonnull (already);
    g_assert_cmpstr (already, ==, "ALLCAPS");

    empty = grl_text_to_upper ("");
    g_assert_nonnull (empty);
    g_assert_cmpstr (empty, ==, "");
}

static void
test_text_to_lower (void)
{
    g_autofree gchar *mixed = NULL;
    g_autofree gchar *already = NULL;
    g_autofree gchar *empty = NULL;

    mixed = grl_text_to_lower ("Hello World 123");
    g_assert_nonnull (mixed);
    g_assert_cmpstr (mixed, ==, "hello world 123");

    already = grl_text_to_lower ("alllower");
    g_assert_nonnull (already);
    g_assert_cmpstr (already, ==, "alllower");

    empty = grl_text_to_lower ("");
    g_assert_nonnull (empty);
    g_assert_cmpstr (empty, ==, "");
}

static void
test_text_to_pascal (void)
{
    g_autofree gchar *snake_in = NULL;
    g_autofree gchar *single = NULL;
    g_autofree gchar *spaces = NULL;

    /* underscore_separated -> PascalCase: each '_' is dropped and the
     * following character is upper-cased; the first character is also
     * upper-cased. */
    snake_in = grl_text_to_pascal ("hello_world");
    g_assert_nonnull (snake_in);
    g_assert_cmpstr (snake_in, ==, "HelloWorld");

    single = grl_text_to_pascal ("hello");
    g_assert_nonnull (single);
    g_assert_cmpstr (single, ==, "Hello");

    /* raylib only treats '_' as a word boundary; spaces are copied through
     * verbatim and only the first character is upper-cased. */
    spaces = grl_text_to_pascal ("hello world");
    g_assert_nonnull (spaces);
    g_assert_cmpstr (spaces, ==, "Hello world");
}

static void
test_text_to_snake (void)
{
    g_autofree gchar *camel_in = NULL;
    g_autofree gchar *pascal_in = NULL;
    g_autofree gchar *spaces = NULL;
    g_autofree gchar *plain = NULL;

    /* camelCase -> an '_' is inserted before each (non-leading) upper-case
     * letter, which is then lower-cased. */
    camel_in = grl_text_to_snake ("helloWorld");
    g_assert_nonnull (camel_in);
    g_assert_cmpstr (camel_in, ==, "hello_world");

    /* A leading upper-case letter does NOT get a leading underscore. */
    pascal_in = grl_text_to_snake ("HelloWorld");
    g_assert_nonnull (pascal_in);
    g_assert_cmpstr (pascal_in, ==, "hello_world");

    /* Spaces are not word boundaries: "hello world" has no upper-case, so
     * it is copied through unchanged (space preserved, no underscore). */
    spaces = grl_text_to_snake ("hello world");
    g_assert_nonnull (spaces);
    g_assert_cmpstr (spaces, ==, "hello world");

    plain = grl_text_to_snake ("hello");
    g_assert_nonnull (plain);
    g_assert_cmpstr (plain, ==, "hello");
}

static void
test_text_to_camel (void)
{
    g_autofree gchar *snake_in = NULL;
    g_autofree gchar *pascal_in = NULL;
    g_autofree gchar *spaces = NULL;

    /* hello_world -> helloWorld: '_' dropped, next char upper-cased, first
     * char forced lower-case. */
    snake_in = grl_text_to_camel ("hello_world");
    g_assert_nonnull (snake_in);
    g_assert_cmpstr (snake_in, ==, "helloWorld");

    /* A leading upper-case letter is lower-cased. */
    pascal_in = grl_text_to_camel ("HelloWorld");
    g_assert_nonnull (pascal_in);
    g_assert_cmpstr (pascal_in, ==, "helloWorld");

    /* Spaces are not word boundaries; only the first char is lower-cased. */
    spaces = grl_text_to_camel ("Hello world");
    g_assert_nonnull (spaces);
    g_assert_cmpstr (spaces, ==, "hello world");
}

/*
 * =============================================================================
 * Test registration
 * =============================================================================
 */

int
main (int   argc,
      char *argv[])
{
    g_test_init (&argc, &argv, NULL);

    /* Query */
    g_test_add_func ("/text/length", test_text_length);
    g_test_add_func ("/text/is-equal", test_text_is_equal);
    g_test_add_func ("/text/find-index", test_text_find_index);
    g_test_add_func ("/text/to-integer", test_text_to_integer);
    g_test_add_func ("/text/to-float", test_text_to_float);

    /* Transformation */
    g_test_add_func ("/text/subtext", test_text_subtext);
    g_test_add_func ("/text/remove-spaces", test_text_remove_spaces);
    g_test_add_func ("/text/get-between", test_text_get_between);
    g_test_add_func ("/text/replace", test_text_replace);
    g_test_add_func ("/text/replace-between", test_text_replace_between);
    g_test_add_func ("/text/insert", test_text_insert);
    g_test_add_func ("/text/join", test_text_join);
    g_test_add_func ("/text/split", test_text_split);

    /* Case conversion */
    g_test_add_func ("/text/to-upper", test_text_to_upper);
    g_test_add_func ("/text/to-lower", test_text_to_lower);
    g_test_add_func ("/text/to-pascal", test_text_to_pascal);
    g_test_add_func ("/text/to-snake", test_text_to_snake);
    g_test_add_func ("/text/to-camel", test_text_to_camel);

    return g_test_run ();
}

/* grl-text-utils.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Text/string utility functions wrapping raylib's text management API.
 */

#include "config.h"
#include "grl-text-utils.h"
#include <raylib.h>
#include <string.h>

/**
 * SECTION:grl-text-utils
 * @title: Text Utilities
 * @short_description: String query, transformation and case conversion helpers
 *
 * Wrappers around raylib's text string management API
 * (TextSubtext, TextReplace, TextSplit, TextToUpper, ...).
 *
 * raylib's text functions return pointers into reused internal static
 * buffers (or, for the `*Alloc` variants, freshly heap-allocated memory
 * that must be released with raylib's MemFree()). Neither form is safe to
 * expose across the GObject Introspection boundary, so every function in
 * this module returns an *owned copy*:
 *
 * - functions returning #gchar* hand back memory the caller must free with
 *   g_free();
 * - functions returning a #GStrv hand back a %NULL-terminated array the
 *   caller must free with g_strfreev().
 *
 * raylib's TextFormat() (sprintf-style varargs over a static buffer) is
 * intentionally not wrapped: g_strdup_printf() is the idiomatic GLib
 * replacement.
 */

/*
 * =============================================================================
 * Text query functions
 * =============================================================================
 */

/**
 * grl_text_length:
 * @text: text to measure
 *
 * Gets the byte length of @text, scanning for the terminating NUL.
 *
 * Returns: The number of bytes in @text, or 0 if @text is %NULL.
 */
guint
grl_text_length (const gchar *text)
{
    g_return_val_if_fail (text != NULL, 0);

    return TextLength (text);
}

/**
 * grl_text_is_equal:
 * @text1: first string
 * @text2: second string
 *
 * Checks whether two strings are byte-for-byte equal.
 *
 * Returns: %TRUE if @text1 and @text2 are equal.
 */
gboolean
grl_text_is_equal (const gchar *text1,
                   const gchar *text2)
{
    unsigned char raw;

    g_return_val_if_fail (text1 != NULL, FALSE);
    g_return_val_if_fail (text2 != NULL, FALSE);

    raw = TextIsEqual (text1, text2);

    return raw != 0;
}

/**
 * grl_text_find_index:
 * @text: text to search within
 * @find: substring to search for
 *
 * Finds the byte index of the first occurrence of @find within @text.
 *
 * Returns: The byte index of the first match, or -1 if not found.
 */
gint
grl_text_find_index (const gchar *text,
                     const gchar *find)
{
    g_return_val_if_fail (text != NULL, -1);
    g_return_val_if_fail (find != NULL, -1);

    return TextFindIndex (text, find);
}

/**
 * grl_text_to_integer:
 * @text: text containing an integer value
 *
 * Parses an integer value from @text.
 *
 * Returns: The integer value parsed from @text, or 0 if @text is %NULL.
 */
gint
grl_text_to_integer (const gchar *text)
{
    g_return_val_if_fail (text != NULL, 0);

    return TextToInteger (text);
}

/**
 * grl_text_to_float:
 * @text: text containing a floating-point value
 *
 * Parses a floating-point value from @text.
 *
 * Returns: The float value parsed from @text, or 0.0 if @text is %NULL.
 */
gfloat
grl_text_to_float (const gchar *text)
{
    g_return_val_if_fail (text != NULL, 0.0f);

    return TextToFloat (text);
}

/*
 * =============================================================================
 * Text transformation functions
 * =============================================================================
 */

/**
 * grl_text_subtext:
 * @text: source text
 * @position: byte offset of the first character to copy
 * @length: number of bytes to copy
 *
 * Extracts a substring of @text starting at @position for @length bytes.
 *
 * Returns: (transfer full): A newly allocated substring; free with g_free().
 */
gchar *
grl_text_subtext (const gchar *text,
                  gint         position,
                  gint         length)
{
    /* TextSubtext returns a pointer into a reused static buffer. */
    g_return_val_if_fail (text != NULL, NULL);

    return g_strdup (TextSubtext (text, position, length));
}

/**
 * grl_text_remove_spaces:
 * @text: source text
 *
 * Returns a copy of @text with all space characters removed.
 *
 * Returns: (transfer full): A newly allocated string; free with g_free().
 */
gchar *
grl_text_remove_spaces (const gchar *text)
{
    /* TextRemoveSpaces returns a pointer into a reused static buffer. */
    g_return_val_if_fail (text != NULL, NULL);

    return g_strdup (TextRemoveSpaces (text));
}

/**
 * grl_text_get_between:
 * @text: source text
 * @begin: opening delimiter
 * @end: closing delimiter
 *
 * Extracts the text found between the first occurrence of @begin and the
 * following occurrence of @end.
 *
 * Returns: (transfer full): A newly allocated string (empty if no match);
 *   free with g_free().
 */
gchar *
grl_text_get_between (const gchar *text,
                      const gchar *begin,
                      const gchar *end)
{
    /* GetTextBetween returns a (non-const) pointer into a static buffer,
     * NOT heap memory, so it must not be MemFree'd. */
    g_return_val_if_fail (text != NULL, NULL);
    g_return_val_if_fail (begin != NULL, NULL);
    g_return_val_if_fail (end != NULL, NULL);

    return g_strdup (GetTextBetween (text, begin, end));
}

/**
 * grl_text_replace:
 * @text: source text
 * @search: substring to search for
 * @replacement: (nullable): replacement string, or %NULL to delete matches
 *
 * Replaces every occurrence of @search in @text with @replacement.
 *
 * Returns: (transfer full) (nullable): A newly allocated string; free with
 *   g_free(). May be %NULL if @search is empty.
 */
gchar *
grl_text_replace (const gchar *text,
                  const gchar *search,
                  const gchar *replacement)
{
    char  *raw;
    gchar *result;

    g_return_val_if_fail (text != NULL, NULL);
    g_return_val_if_fail (search != NULL, NULL);

    /* TextReplaceAlloc returns heap memory that must be MemFree'd. */
    raw = TextReplaceAlloc (text, search, replacement);
    if (raw == NULL)
        return NULL;

    result = g_strdup (raw);
    MemFree (raw);

    return result;
}

/**
 * grl_text_replace_between:
 * @text: source text
 * @begin: opening delimiter
 * @end: closing delimiter
 * @replacement: (nullable): replacement string, or %NULL to delete the span
 *
 * Replaces the text found between @begin and @end with @replacement. When
 * @replacement is %NULL the span (between the delimiters) is removed.
 *
 * Returns: (transfer full) (nullable): A newly allocated string; free with
 *   g_free(). May be %NULL if no match is found.
 */
gchar *
grl_text_replace_between (const gchar *text,
                          const gchar *begin,
                          const gchar *end,
                          const gchar *replacement)
{
    char  *raw;
    gchar *result;

    g_return_val_if_fail (text != NULL, NULL);
    g_return_val_if_fail (begin != NULL, NULL);
    g_return_val_if_fail (end != NULL, NULL);

    /* TextReplaceBetweenAlloc returns heap memory that must be MemFree'd. */
    raw = TextReplaceBetweenAlloc (text, begin, end, replacement);
    if (raw == NULL)
        return NULL;

    result = g_strdup (raw);
    MemFree (raw);

    return result;
}

/**
 * grl_text_insert:
 * @text: source text
 * @insert: text to insert
 * @position: byte offset at which to insert @insert
 *
 * Inserts @insert into @text at byte offset @position.
 *
 * Returns: (transfer full) (nullable): A newly allocated string; free with
 *   g_free().
 */
gchar *
grl_text_insert (const gchar *text,
                 const gchar *insert,
                 gint         position)
{
    gsize text_len;
    gint  pos;

    g_return_val_if_fail (text != NULL, NULL);
    g_return_val_if_fail (insert != NULL, NULL);

    /* raylib 6.0's TextInsertAlloc() reads out of bounds for any position
     * other than the end of the string (its tail-copy loop indexes the source
     * with the destination offset), so splice with GLib instead.  @position is
     * a byte offset, clamped into [0, len]. */
    text_len = strlen (text);
    pos = CLAMP (position, 0, (gint) text_len);

    return g_strdup_printf ("%.*s%s%s", pos, text, insert, text + pos);
}

/**
 * grl_text_join:
 * @lines: (array zero-terminated=1): %NULL-terminated array of strings to join
 * @delimiter: delimiter inserted between each joined string
 *
 * Joins the strings in @lines into a single string, inserting @delimiter
 * between consecutive elements.
 *
 * Returns: (transfer full): A newly allocated string; free with g_free().
 */
gchar *
grl_text_join (const gchar * const *lines,
               const gchar         *delimiter)
{
    gchar *result;
    char **list;
    guint  count;
    guint  i;

    g_return_val_if_fail (lines != NULL, NULL);
    g_return_val_if_fail (delimiter != NULL, NULL);

    count = g_strv_length ((gchar **) lines);

    /* TextJoin takes a (char **) and a count; build a non-const view of the
     * incoming GStrv. The elements are not modified, only read. */
    list = g_new (char *, count > 0 ? count : 1);
    for (i = 0; i < count; i++)
        list[i] = (char *) lines[i];

    /* TextJoin returns a pointer into a reused static buffer. */
    result = g_strdup (TextJoin (list, (int) count, delimiter));

    g_free (list);

    return result;
}

/**
 * grl_text_split:
 * @text: source text
 * @delimiter: single-byte delimiter to split on
 *
 * Splits @text into substrings on each occurrence of @delimiter.
 *
 * Returns: (transfer full) (array zero-terminated=1): A newly allocated
 *   %NULL-terminated array of strings; free with g_strfreev().
 */
gchar **
grl_text_split (const gchar *text,
                gchar        delimiter)
{
    gchar      **result;
    const char **parts;
    int          count;
    int          i;

    g_return_val_if_fail (text != NULL, NULL);

    /* TextSplit returns pointers into static buffers; deep-copy each part. */
    count = 0;
    parts = (const char **) TextSplit (text, delimiter, &count);

    result = g_new0 (gchar *, (gsize) count + 1);
    for (i = 0; i < count; i++)
        result[i] = g_strdup (parts[i]);
    result[count] = NULL;

    return result;
}

/*
 * =============================================================================
 * Case conversion functions
 * =============================================================================
 */

/**
 * grl_text_to_upper:
 * @text: source text
 *
 * Returns an upper-case copy of @text.
 *
 * Returns: (transfer full): A newly allocated string; free with g_free().
 */
gchar *
grl_text_to_upper (const gchar *text)
{
    /* TextToUpper returns a pointer into a reused static buffer. */
    g_return_val_if_fail (text != NULL, NULL);

    return g_strdup (TextToUpper (text));
}

/**
 * grl_text_to_lower:
 * @text: source text
 *
 * Returns a lower-case copy of @text.
 *
 * Returns: (transfer full): A newly allocated string; free with g_free().
 */
gchar *
grl_text_to_lower (const gchar *text)
{
    /* TextToLower returns a pointer into a reused static buffer. */
    g_return_val_if_fail (text != NULL, NULL);

    return g_strdup (TextToLower (text));
}

/**
 * grl_text_to_pascal:
 * @text: source text
 *
 * Returns a PascalCase copy of @text.
 *
 * Returns: (transfer full): A newly allocated string; free with g_free().
 */
gchar *
grl_text_to_pascal (const gchar *text)
{
    /* TextToPascal returns a pointer into a reused static buffer. */
    g_return_val_if_fail (text != NULL, NULL);

    return g_strdup (TextToPascal (text));
}

/**
 * grl_text_to_snake:
 * @text: source text
 *
 * Returns a snake_case copy of @text.
 *
 * Returns: (transfer full): A newly allocated string; free with g_free().
 */
gchar *
grl_text_to_snake (const gchar *text)
{
    /* TextToSnake returns a pointer into a reused static buffer. */
    g_return_val_if_fail (text != NULL, NULL);

    return g_strdup (TextToSnake (text));
}

/**
 * grl_text_to_camel:
 * @text: source text
 *
 * Returns a camelCase copy of @text.
 *
 * Returns: (transfer full): A newly allocated string; free with g_free().
 */
gchar *
grl_text_to_camel (const gchar *text)
{
    /* TextToCamel returns a pointer into a reused static buffer. */
    g_return_val_if_fail (text != NULL, NULL);

    return g_strdup (TextToCamel (text));
}

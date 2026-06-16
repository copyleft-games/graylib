# Text — String Utilities

The `grl_text_*` module provides string-utility functions wrapping raylib's text
management API, returning GLib-idiomatic types.

> **Owned copies.** Every transformation/case function returns a **newly
> allocated** result that the caller owns: `gchar *` results are freed with
> `g_free()`, and `GStrv` (`gchar **`) results with `g_strfreev()`. (raylib's own
> text helpers return pointers into a rotating internal static buffer; this
> wrapper copies out of it so the result is yours.)

> **`TextFormat` is intentionally not wrapped.** raylib's `TextFormat()` is a
> `printf`-style formatter over a static buffer. There is no `grl_text_format()`
> — use GLib's `g_strdup_printf()` (or `g_strdup_vprintf()`) instead, which
> returns an owned string and is variadic-safe.

## Query Functions

These read from the input string and return scalars (no allocation).

```c
guint    grl_text_length     (const gchar *text);
gboolean grl_text_is_equal   (const gchar *text1, const gchar *text2);
gint     grl_text_find_index (const gchar *text,  const gchar *find);
gint     grl_text_to_integer (const gchar *text);
gfloat   grl_text_to_float   (const gchar *text);
```

| Function | Description |
|----------|-------------|
| `length(text)` | Length of `text` in bytes |
| `is_equal(text1, text2)` | Whether the two strings are equal |
| `find_index(text, find)` | Index of the first occurrence of `find` in `text` (`-1` if absent) |
| `to_integer(text)` | Parse `text` as an integer |
| `to_float(text)` | Parse `text` as a float |

## Transformation Functions

All return a newly allocated string (`gchar *`, free with `g_free()`), except
`grl_text_split()`, which returns a `GStrv` (free with `g_strfreev()`).

```c
gchar *  grl_text_subtext         (const gchar *text, gint position, gint length);
gchar *  grl_text_remove_spaces   (const gchar *text);
gchar *  grl_text_get_between     (const gchar *text, const gchar *begin, const gchar *end);
gchar *  grl_text_replace         (const gchar *text, const gchar *search, const gchar *replacement);
gchar *  grl_text_replace_between (const gchar *text, const gchar *begin, const gchar *end,
                                   const gchar *replacement);
gchar *  grl_text_insert          (const gchar *text, const gchar *insert, gint position);
gchar *  grl_text_join            (const gchar * const *lines, const gchar *delimiter);
gchar ** grl_text_split           (const gchar *text, gchar delimiter);
```

| Function | Description |
|----------|-------------|
| `subtext(text, position, length)` | Extract a substring |
| `remove_spaces(text)` | Remove all spaces |
| `get_between(text, begin, end)` | Extract the text between the `begin` and `end` delimiters |
| `replace(text, search, replacement)` | Replace all occurrences of `search` |
| `replace_between(text, begin, end, replacement)` | Replace the text between `begin` and `end` delimiters |
| `insert(text, insert, position)` | Insert `insert` at `position` |
| `join(lines, delimiter)` | Join a `NULL`-terminated array of strings with `delimiter` |
| `split(text, delimiter)` | Split `text` on a single-char `delimiter` (returns a `GStrv`) |

## Case Conversion Functions

All return a newly allocated string (`gchar *`, free with `g_free()`).

```c
gchar * grl_text_to_upper  (const gchar *text);
gchar * grl_text_to_lower  (const gchar *text);
gchar * grl_text_to_pascal (const gchar *text);
gchar * grl_text_to_snake  (const gchar *text);
gchar * grl_text_to_camel  (const gchar *text);
```

| Function | Description |
|----------|-------------|
| `to_upper(text)` | Convert to `UPPERCASE` |
| `to_lower(text)` | Convert to `lowercase` |
| `to_pascal(text)` | Convert to `PascalCase` |
| `to_snake(text)` | Convert to `snake_case` |
| `to_camel(text)` | Convert to `camelCase` |

## Example

```c
#include <graylib.h>

void
demo (void)
{
    g_autofree gchar *upper = grl_text_to_upper ("hello world");
    g_autofree gchar *snake = grl_text_to_snake ("MyVariableName");
    g_auto(GStrv) parts = grl_text_split ("a,b,c", ',');

    g_print ("%s\n", upper);  /* HELLO WORLD */
    g_print ("%s\n", snake);  /* my_variable_name */

    for (gsize i = 0; parts != NULL && parts[i] != NULL; i++)
        g_print ("part %zu: %s\n", i, parts[i]);

    /* For printf-style formatting, use GLib, not grl_text_*: */
    g_autofree gchar *msg = g_strdup_printf ("score: %d", 42);
    g_print ("%s\n", msg);
}
```

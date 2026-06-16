# System — Filesystem Helpers

The `grl_fs_*` module provides filesystem helper functions. They are thin
wrappers over raylib's path utilities, but return GLib-idiomatic types
(`gboolean`, `gchar *`, `GStrv`, `gint64`, ...) instead of exposing raylib's
internal static buffers.

> **Prefer GLib/GIO for file contents.** These helpers are convenient for path
> manipulation, existence/type queries, and lightweight file operations. For
> reading and writing file *contents*, GLib's `GFile` / GIO API (`g_file_get_contents()`,
> `g_file_set_contents()`, `GFileInputStream`, async I/O, etc.) is the preferred,
> more capable choice.

> **Owned strings.** Every accessor that returns `gchar *` returns an **owned
> copy** — free it with `g_free()`. Functions returning `GStrv` (a
> `NULL`-terminated `gchar **`) must be freed with `g_strfreev()`.

## Existence & Type Queries

```c
gboolean grl_fs_file_exists        (const gchar *path);
gboolean grl_fs_directory_exists   (const gchar *path);
gboolean grl_fs_is_path_file       (const gchar *path);
gboolean grl_fs_is_file_extension  (const gchar *path, const gchar *ext);
gboolean grl_fs_is_file_name_valid (const gchar *file_name);
```

| Function | Description |
|----------|-------------|
| `file_exists(path)` | Whether a file exists at `path` |
| `directory_exists(path)` | Whether a directory exists at `path` |
| `is_path_file(path)` | Whether `path` refers to a file (vs a directory) |
| `is_file_extension(path, ext)` | Whether `path` has the extension `ext` (e.g. `".png"`) |
| `is_file_name_valid(file_name)` | Whether `file_name` is a valid filename |

## File Operations

```c
gboolean grl_fs_file_rename (const gchar *path,     const gchar *new_path);
gboolean grl_fs_file_remove (const gchar *path);
gboolean grl_fs_file_copy   (const gchar *src_path, const gchar *dst_path);
gboolean grl_fs_file_move   (const gchar *src_path, const gchar *dst_path);
```

| Function | Description |
|----------|-------------|
| `file_rename(path, new_path)` | Rename a file |
| `file_remove(path)` | Remove a file |
| `file_copy(src, dst)` | Copy a file |
| `file_move(src, dst)` | Move a file |

### File Text Editing

```c
gboolean grl_fs_file_text_replace    (const gchar *path,
                                      const gchar *search,
                                      const gchar *replacement);
gint     grl_fs_file_text_find_index (const gchar *path,
                                      const gchar *search);
```

| Function | Description |
|----------|-------------|
| `file_text_replace(path, search, replacement)` | Replace text within a file in place |
| `file_text_find_index(path, search)` | Find the index of `search` within the file's text (`-1` if not found) |

## File Metadata

```c
gint   grl_fs_get_file_length   (const gchar *path);
gint64 grl_fs_get_file_mod_time (const gchar *path);
```

| Function | Description |
|----------|-------------|
| `get_file_length(path)` | File size in bytes |
| `get_file_mod_time(path)` | Last modification time (UNIX timestamp) |

## Path String Accessors

All of these return an **owned `gchar *`** — free with `g_free()`.

```c
gchar * grl_fs_get_file_extension        (const gchar *path);
gchar * grl_fs_get_file_name             (const gchar *path);
gchar * grl_fs_get_file_name_without_ext (const gchar *path);
gchar * grl_fs_get_directory_path        (const gchar *path);
gchar * grl_fs_get_prev_directory_path   (const gchar *path);
gchar * grl_fs_get_working_directory     (void);
gchar * grl_fs_get_application_directory (void);
```

| Function | Description |
|----------|-------------|
| `get_file_extension(path)` | The file extension (including the leading dot) |
| `get_file_name(path)` | The final path component (filename with extension) |
| `get_file_name_without_ext(path)` | The filename with the extension stripped |
| `get_directory_path(path)` | The directory portion of `path` |
| `get_prev_directory_path(path)` | The parent directory of `path` |
| `get_working_directory()` | The current working directory |
| `get_application_directory()` | The directory containing the running executable |

## Directory Operations

```c
gboolean grl_fs_make_directory             (const gchar *path);
gboolean grl_fs_change_directory           (const gchar *path);

GStrv    grl_fs_load_directory_files       (const gchar *dir_path);
GStrv    grl_fs_load_directory_files_ex    (const gchar *base_path,
                                            const gchar *filter,
                                            gboolean     scan_subdirs);

guint    grl_fs_get_directory_file_count   (const gchar *dir_path);
guint    grl_fs_get_directory_file_count_ex (const gchar *base_path,
                                            const gchar *filter,
                                            gboolean     scan_subdirs);
```

| Function | Description |
|----------|-------------|
| `make_directory(path)` | Create a directory |
| `change_directory(path)` | Change the process working directory |
| `load_directory_files(dir_path)` | List the files in a directory (returns a `GStrv`) |
| `load_directory_files_ex(base, filter, scan_subdirs)` | List files matching `filter`, optionally recursing into subdirectories (returns a `GStrv`) |
| `get_directory_file_count(dir_path)` | Count files in a directory |
| `get_directory_file_count_ex(base, filter, scan_subdirs)` | Count files matching `filter`, optionally recursive |

The two directory-listing functions return a `GStrv` — free it with
`g_strfreev()`:

```c
g_auto(GStrv) files = grl_fs_load_directory_files_ex ("assets", ".png", TRUE);
for (gsize i = 0; files != NULL && files[i] != NULL; i++)
    g_print ("%s\n", files[i]);
```

## Example

```c
#include <graylib.h>

void
inspect (const gchar *path)
{
    g_autofree gchar *name = NULL;
    g_autofree gchar *ext = NULL;

    if (!grl_fs_file_exists (path))
    {
        g_print ("%s does not exist\n", path);
        return;
    }

    name = grl_fs_get_file_name (path);
    ext = grl_fs_get_file_extension (path);

    g_print ("%s: %d bytes, name=%s, ext=%s\n",
             path, grl_fs_get_file_length (path), name, ext);
}
```

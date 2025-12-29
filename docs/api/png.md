# PNG Manipulation

This document covers PNG chunk manipulation and indexed PNG support using rpng integration.

## Overview

Graylib provides low-level PNG chunk access for reading and writing metadata, as well as support for indexed (palette-based) PNG images. This is useful for:

- Embedding metadata in PNG files
- Reading image properties
- Working with indexed/paletted images for retro-style games
- Validating PNG file integrity

## GrlPngChunk

A PNG file consists of chunks, each with a 4-character type code, data, and CRC checksum.

### Structure

```c
struct _GrlPngChunk
{
    gint      length;       /* Data length in bytes */
    gchar     type[4];      /* Chunk type FOURCC (e.g., "tEXt", "gAMA") */
    guint8   *data;         /* Chunk data pointer */
    guint32   crc;          /* 32-bit CRC checksum */
};
```

### Common Chunk Types

| Type | Description |
|------|-------------|
| IHDR | Image header (dimensions, bit depth) |
| PLTE | Palette for indexed images |
| IDAT | Image data |
| IEND | End marker |
| tEXt | Text metadata |
| gAMA | Gamma correction |
| sRGB | sRGB color space |
| tIME | Last modification time |
| bKGD | Background color |

### Creating Chunks

```c
/* Create a custom chunk */
const guint8 custom_data[] = { 0x01, 0x02, 0x03 };
g_autoptr(GrlPngChunk) chunk = grl_png_chunk_new ("prIv", custom_data, 3);

/* Get chunk type as string */
g_autofree gchar *type = grl_png_chunk_get_type_string (chunk);
g_print ("Chunk type: %s\n", type);
```

## Reading Chunks

### Count Chunks

```c
GError *error = NULL;
gint count = grl_png_chunk_count ("image.png", &error);
if (count < 0)
{
    g_printerr ("Error: %s\n", error->message);
    g_clear_error (&error);
}
else
{
    g_print ("PNG has %d chunks\n", count);
}
```

### Read Specific Chunk

```c
GError *error = NULL;
g_autoptr(GrlPngChunk) chunk = grl_png_chunk_read ("image.png", "tEXt", &error);
if (chunk != NULL)
{
    g_print ("Found tEXt chunk with %d bytes\n", chunk->length);
}
```

### Read All Chunks

```c
GError *error = NULL;
gint count;
GrlPngChunk **chunks = grl_png_chunk_read_all ("image.png", &count, &error);

if (chunks != NULL)
{
    for (gint i = 0; i < count; i++)
    {
        g_autofree gchar *type = grl_png_chunk_get_type_string (chunks[i]);
        g_print ("Chunk %d: %s (%d bytes)\n", i, type, chunks[i]->length);
        grl_png_chunk_free (chunks[i]);
    }
    g_free (chunks);
}
```

## Writing Chunks

### Write Custom Chunk

```c
GError *error = NULL;
const guint8 data[] = { 'h', 'e', 'l', 'l', 'o' };
g_autoptr(GrlPngChunk) chunk = grl_png_chunk_new ("prIv", data, 5);

if (!grl_png_chunk_write ("image.png", chunk, &error))
{
    g_printerr ("Write failed: %s\n", error->message);
    g_clear_error (&error);
}
```

### Write Text Metadata

```c
GError *error = NULL;

/* Add author information */
if (!grl_png_chunk_write_text ("image.png", "Author", "John Doe", &error))
{
    g_printerr ("Error: %s\n", error->message);
    g_clear_error (&error);
}

/* Add description */
grl_png_chunk_write_text ("image.png", "Description", "Game sprite sheet", NULL);
```

### Write Gamma

```c
/* Standard gamma value */
grl_png_chunk_write_gamma ("image.png", 2.2f, NULL);
```

### Write sRGB

```c
/* Rendering intents:
 * 0 = Perceptual (photographs)
 * 1 = Relative colorimetric (logos)
 * 2 = Saturation (charts)
 * 3 = Absolute colorimetric (proofing)
 */
grl_png_chunk_write_srgb ("image.png", 0, NULL);
```

### Write Timestamp

```c
/* Set modification time */
grl_png_chunk_write_time ("image.png",
                          2025, 12, 25,  /* Year, month, day */
                          12, 30, 0,     /* Hour, minute, second */
                          NULL);
```

### Remove Chunks

```c
/* Remove all tEXt chunks */
grl_png_chunk_remove ("image.png", "tEXt", NULL);
```

## Validation

```c
GError *error = NULL;
if (grl_png_chunk_check_valid ("image.png", &error))
{
    g_print ("PNG file is valid\n");
}
else
{
    g_printerr ("PNG validation failed: %s\n", error->message);
    g_clear_error (&error);
}
```

## GrlPngPalette

Color palettes for indexed PNG images. Each pixel in an indexed image is an 8-bit index into the palette.

### Structure

```c
struct _GrlPngPalette
{
    gint       color_count;   /* Number of colors (1-256) */
    GrlColor  *colors;        /* Array of palette colors */
};
```

### Creating Palettes

```c
/* Create from existing colors */
GrlColor colors[4] = {
    { 0, 0, 0, 255 },       /* Black */
    { 255, 255, 255, 255 }, /* White */
    { 255, 0, 0, 255 },     /* Red */
    { 0, 255, 0, 255 }      /* Green */
};
g_autoptr(GrlPngPalette) palette = grl_png_palette_new (colors, 4);

/* Create empty palette */
g_autoptr(GrlPngPalette) empty = grl_png_palette_new_empty (256);
```

### Working with Colors

```c
/* Get a color from palette */
g_autoptr(GrlColor) color = grl_png_palette_get_color (palette, 2);
if (color != NULL)
{
    g_print ("Color 2: R=%d G=%d B=%d\n", color->r, color->g, color->b);
}

/* Set a color in palette */
g_autoptr(GrlColor) blue = grl_color_new (0, 0, 255, 255);
grl_png_palette_set_color (palette, 3, blue);
```

## Indexed PNG Images

Load and save palette-based PNG images:

### Loading Indexed PNG

```c
GError *error = NULL;
GrlPngPalette *palette = NULL;

g_autoptr(GrlImage) image = grl_image_new_from_png_indexed (
    "indexed.png",
    &palette,
    &error
);

if (image == NULL)
{
    g_printerr ("Load failed: %s\n", error->message);
    g_clear_error (&error);
}
else
{
    g_print ("Loaded %dx%d indexed image with %d colors\n",
             grl_image_get_width (image),
             grl_image_get_height (image),
             palette->color_count);

    /* Use the palette... */
    grl_png_palette_free (palette);
}
```

### Saving Indexed PNG

```c
GError *error = NULL;

/* Create a palette */
GrlColor colors[4] = {
    { 0, 0, 0, 255 },
    { 255, 0, 0, 255 },
    { 0, 255, 0, 255 },
    { 0, 0, 255, 255 }
};
g_autoptr(GrlPngPalette) palette = grl_png_palette_new (colors, 4);

/* Save image using palette */
if (!grl_image_save_as_png_indexed (image, "output.png", palette, &error))
{
    g_printerr ("Save failed: %s\n", error->message);
    g_clear_error (&error);
}
```

## Complete Example

```c
#include <graylib.h>

int
main (int argc, char *argv[])
{
    GError *error = NULL;

    /* Read PNG metadata */
    gint chunk_count = grl_png_chunk_count ("sprite.png", &error);
    if (chunk_count >= 0)
    {
        g_print ("Sprite has %d chunks\n", chunk_count);
    }

    /* Read all text chunks */
    gint count;
    GrlPngChunk **chunks = grl_png_chunk_read_all ("sprite.png", &count, NULL);
    if (chunks != NULL)
    {
        for (gint i = 0; i < count; i++)
        {
            if (memcmp (chunks[i]->type, "tEXt", 4) == 0)
            {
                /* tEXt format: keyword\0text */
                gchar *keyword = (gchar *)chunks[i]->data;
                g_print ("Metadata: %s\n", keyword);
            }
            grl_png_chunk_free (chunks[i]);
        }
        g_free (chunks);
    }

    /* Add our own metadata */
    grl_png_chunk_write_text ("sprite.png", "Software", "Graylib", NULL);
    grl_png_chunk_write_gamma ("sprite.png", 2.2f, NULL);

    /* Validate the modified file */
    if (grl_png_chunk_check_valid ("sprite.png", &error))
    {
        g_print ("File is valid!\n");
    }
    else
    {
        g_printerr ("Validation error: %s\n", error->message);
        g_clear_error (&error);
    }

    return 0;
}
```

## Function Reference

### GrlPngChunk Functions

| Function | Description |
|----------|-------------|
| `grl_png_chunk_new()` | Create a new chunk |
| `grl_png_chunk_copy()` | Copy a chunk |
| `grl_png_chunk_free()` | Free a chunk |
| `grl_png_chunk_get_type_string()` | Get chunk type as string |
| `grl_png_chunk_count()` | Count chunks in file |
| `grl_png_chunk_read()` | Read specific chunk type |
| `grl_png_chunk_read_all()` | Read all chunks |
| `grl_png_chunk_write()` | Write chunk to file |
| `grl_png_chunk_remove()` | Remove chunks by type |
| `grl_png_chunk_write_text()` | Write text metadata |
| `grl_png_chunk_write_gamma()` | Write gamma value |
| `grl_png_chunk_write_srgb()` | Write sRGB intent |
| `grl_png_chunk_write_time()` | Write timestamp |
| `grl_png_chunk_check_valid()` | Validate PNG CRCs |

### GrlPngPalette Functions

| Function | Description |
|----------|-------------|
| `grl_png_palette_new()` | Create from color array |
| `grl_png_palette_new_empty()` | Create empty palette |
| `grl_png_palette_copy()` | Copy a palette |
| `grl_png_palette_free()` | Free a palette |
| `grl_png_palette_get_color()` | Get color at index |
| `grl_png_palette_set_color()` | Set color at index |

### GrlImage Indexed PNG Functions

| Function | Description |
|----------|-------------|
| `grl_image_new_from_png_indexed()` | Load indexed PNG with palette |
| `grl_image_save_as_png_indexed()` | Save as indexed PNG |

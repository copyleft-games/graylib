# Graphics Types

This document covers the graphics resource types added in Phase 2: images, textures, and fonts.

## GrlImage

CPU-side image type for loading and manipulating pixel data before uploading to the GPU.

### Loading Images

```c
/* Load from file */
g_autoptr(GrlImage) img = grl_image_new_from_file ("sprite.png");

/* Load from memory */
g_autoptr(GrlImage) img = grl_image_new_from_memory (".png", data, data_size);

/* Capture screenshot */
g_autoptr(GrlImage) screenshot = grl_image_new_from_screen ();
```

### Generating Images

```c
/* Solid color */
g_autoptr(GrlColor) red = grl_color_new_red ();
g_autoptr(GrlImage) img = grl_image_new_color (256, 256, red);

/* Gradients */
g_autoptr(GrlImage) linear = grl_image_new_gradient_linear (256, 256, 0, start, end);
g_autoptr(GrlImage) radial = grl_image_new_gradient_radial (256, 256, 0.5f, inner, outer);

/* Patterns */
g_autoptr(GrlImage) checker = grl_image_new_checked (256, 256, 8, 8, color1, color2);
g_autoptr(GrlImage) noise = grl_image_new_white_noise (256, 256, 1.0f);
g_autoptr(GrlImage) perlin = grl_image_new_perlin_noise (256, 256, 0, 0, 4.0f);
g_autoptr(GrlImage) cells = grl_image_new_cellular (256, 256, 16);
```

### Image Transformations

```c
/* Resize (bilinear interpolation) */
grl_image_resize (img, 128, 128);

/* Resize (nearest neighbor - preserves pixels) */
grl_image_resize_nearest (img, 128, 128);

/* Resize canvas (add padding) */
grl_image_resize_canvas (img, 300, 300, 50, 50, bg_color);

/* Crop to region */
grl_image_crop (img, rect);

/* Flip */
grl_image_flip_vertical (img);
grl_image_flip_horizontal (img);

/* Rotate (90 degree increments) */
grl_image_rotate (img, 90);   /* or 180, 270 */
grl_image_rotate_cw (img);    /* 90 degrees clockwise */
grl_image_rotate_ccw (img);   /* 90 degrees counter-clockwise */
```

### Color Operations

```c
/* Apply color tint */
grl_image_color_tint (img, tint_color);

/* Invert colors */
grl_image_color_invert (img);

/* Convert to grayscale */
grl_image_color_grayscale (img);

/* Adjust contrast (-100 to 100) */
grl_image_color_contrast (img, 20.0f);

/* Adjust brightness (-255 to 255) */
grl_image_color_brightness (img, 40);

/* Replace a specific color */
grl_image_color_replace (img, old_color, new_color);
```

### Alpha Operations

```c
/* Premultiply alpha */
grl_image_alpha_premultiply (img);

/* Crop to non-transparent region */
grl_image_alpha_crop (img, 0.0f);

/* Clear pixels matching color to transparent */
grl_image_alpha_clear (img, color, 0.1f);

/* Apply alpha mask from another image */
grl_image_alpha_mask (img, mask_img);
```

### Drawing on Images

```c
/* Clear with background color */
grl_image_clear_background (img, color);

/* Draw primitives */
grl_image_draw_pixel (img, x, y, color);
grl_image_draw_line (img, x1, y1, x2, y2, color);
grl_image_draw_circle (img, cx, cy, radius, color);
grl_image_draw_rectangle (img, rect, color);

/* Composite another image */
grl_image_draw_image (img, src, src_rect, dst_rect, tint);

/* Draw text (using default font) */
grl_image_draw_text (img, "Hello", x, y, 20, color);
```

### Exporting Images

```c
/* Save to file (format determined by extension) */
grl_image_export (img, "output.png");

/* Export to memory buffer */
gsize size;
guint8 *data = grl_image_export_to_memory (img, ".png", &size);
g_free (data);
```

## GrlTexture

GPU texture for efficient rendering. Textures are created from images or loaded directly.

### Creating Textures

```c
/* Load directly from file */
g_autoptr(GrlTexture) tex = grl_texture_new_from_file ("sprite.png");

/* Create from image (recommended for manipulation) */
g_autoptr(GrlImage) img = grl_image_new_from_file ("sprite.png");
grl_image_resize (img, 64, 64);
g_autoptr(GrlTexture) tex = grl_texture_new_from_image (img);

/* Load from memory */
g_autoptr(GrlTexture) tex = grl_texture_new_from_memory (".png", data, size);
```

### Texture Properties

```c
gint width = grl_texture_get_width (tex);
gint height = grl_texture_get_height (tex);
gint mipmaps = grl_texture_get_mipmaps (tex);
GrlPixelFormat format = grl_texture_get_format (tex);
gboolean valid = grl_texture_is_valid (tex);
```

### Texture Configuration

```c
/* Generate mipmaps for smooth scaling */
grl_texture_gen_mipmaps (tex);

/* Set filtering mode */
grl_texture_set_filter (tex, GRL_TEXTURE_FILTER_POINT);     /* Pixelated */
grl_texture_set_filter (tex, GRL_TEXTURE_FILTER_BILINEAR);  /* Smooth */
grl_texture_set_filter (tex, GRL_TEXTURE_FILTER_TRILINEAR); /* Smooth + mipmaps */

/* Set wrapping mode */
grl_texture_set_wrap (tex, GRL_TEXTURE_WRAP_REPEAT);        /* Tile */
grl_texture_set_wrap (tex, GRL_TEXTURE_WRAP_CLAMP);         /* Extend edges */
grl_texture_set_wrap (tex, GRL_TEXTURE_WRAP_MIRROR_REPEAT); /* Mirror + tile */
```

### Updating Textures

```c
/* Update entire texture from image */
grl_texture_update (tex, new_image);

/* Update rectangular region */
grl_texture_update_rec (tex, rect, pixel_data);
```

### Downloading Texture Data

```c
/* Download from GPU to create image */
g_autoptr(GrlImage) img = grl_texture_to_image (tex);
grl_image_export (img, "screenshot.png");
```

## GrlFont

Font type for text rendering. Supports TTF, OTF, FNT, and sprite fonts.

### Loading Fonts

```c
/* Use default raylib font */
g_autoptr(GrlFont) font = grl_font_new_default ();

/* Load from file */
g_autoptr(GrlFont) font = grl_font_new_from_file ("fonts/arial.ttf");

/* Load with specific size and characters */
g_autoptr(GrlFont) font = grl_font_new_from_file_ex ("fonts/arial.ttf", 32, NULL, 0);

/* Load from sprite image (XNA-style) */
g_autoptr(GrlFont) font = grl_font_new_from_image (img, magenta_key, 32);

/* Load from memory */
g_autoptr(GrlFont) font = grl_font_new_from_memory (".ttf", data, size, 24, NULL, 0);
```

### Font Properties

```c
gint base_size = grl_font_get_base_size (font);
gint glyph_count = grl_font_get_glyph_count (font);
gint padding = grl_font_get_glyph_padding (font);
gboolean valid = grl_font_is_valid (font);
```

### Text Measurement

```c
/* Measure text dimensions */
g_autoptr(GrlVector2) size = grl_font_measure_text (font, "Hello World", 24.0f, 2.0f);
g_print ("Text size: %.0f x %.0f\n", size->x, size->y);

/* Measure with default font */
gint width = grl_measure_text ("Hello", 20);
```

### Glyph Information

```c
/* Get glyph index for a character */
gint index = grl_font_get_glyph_index (font, 'A');

/* Get glyph rectangle in font atlas */
g_autoptr(GrlRectangle) rec = grl_font_get_glyph_atlas_rec (font, 'A');
```

## Drawing Textures

All drawing must occur between `grl_window_begin_drawing()` and `grl_window_end_drawing()`.

```c
/* Simple draw at position */
grl_draw_texture (tex, 100, 100, white);
grl_draw_texture_v (tex, position, white);

/* Draw with rotation and scale */
grl_draw_texture_ex (tex, position, 45.0f, 2.0f, white);

/* Draw portion of texture (sprite sheets) */
grl_draw_texture_rec (tex, source_rect, position, white);

/* Full control: source, dest, origin, rotation */
grl_draw_texture_pro (tex, source, dest, origin, rotation, white);
```

## Drawing Text

```c
/* Draw with default font */
grl_draw_text ("Hello World!", 100, 100, 20, white);

/* Draw FPS counter */
grl_draw_fps (10, 10);

/* Draw with custom font */
grl_draw_text_ex (font, "Hello", position, 24.0f, 2.0f, white);

/* Draw with rotation */
grl_draw_text_pro (font, "Rotated", position, origin, 45.0f, 24.0f, 2.0f, white);

/* Draw single Unicode character */
grl_draw_text_codepoint (font, 0x263A, position, 32.0f, white);  /* Smiley face */
```

## Complete Example

```c
#include <graylib.h>

int
main (int argc, char *argv[])
{
    g_autoptr(GrlApplication) app = grl_application_new ();
    g_autoptr(GrlWindow) window = grl_window_new (800, 600, "Graphics Demo");
    g_autoptr(GrlColor) bg = grl_color_new (40, 40, 60, 255);
    g_autoptr(GrlColor) white = grl_color_new_white ();

    /* Load and configure texture */
    g_autoptr(GrlTexture) tex = grl_texture_new_from_file ("sprite.png");
    grl_texture_set_filter (tex, GRL_TEXTURE_FILTER_POINT);

    /* Load custom font */
    g_autoptr(GrlFont) font = grl_font_new_from_file ("fonts/pixel.ttf");

    g_autoptr(GrlVector2) pos = grl_vector2_new (400.0f, 300.0f);

    grl_window_set_target_fps (window, 60);

    while (!grl_window_should_close (window))
    {
        grl_window_begin_drawing (window);
        grl_draw_clear_background (bg);

        /* Draw texture centered */
        grl_draw_texture_v (tex, pos, white);

        /* Draw text with custom font */
        grl_draw_text_ex (font, "Graylib Graphics!", pos, 32.0f, 2.0f, white);

        grl_draw_fps (10, 10);

        grl_window_end_drawing (window);
    }

    return 0;
}
```

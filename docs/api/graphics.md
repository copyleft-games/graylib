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

## GrlShader

Custom shader programs for advanced rendering effects.

### Creating Shaders

```c
/* Load from files */
g_autoptr(GError) error = NULL;
g_autoptr(GrlShader) shader = grl_shader_new_from_file (
    "shaders/vertex.vs",
    "shaders/fragment.fs",
    &error
);

/* Use default vertex shader with custom fragment */
g_autoptr(GrlShader) shader = grl_shader_new_from_file (
    NULL,                    /* Use default vertex shader */
    "shaders/fragment.fs",
    &error
);

/* Create from code strings */
const gchar *fs_code =
    "#version 330\n"
    "in vec2 fragTexCoord;\n"
    "in vec4 fragColor;\n"
    "uniform sampler2D texture0;\n"
    "uniform float time;\n"
    "out vec4 finalColor;\n"
    "void main() {\n"
    "    vec4 texColor = texture(texture0, fragTexCoord);\n"
    "    finalColor = texColor * fragColor;\n"
    "}\n";

g_autoptr(GrlShader) shader = grl_shader_new_from_memory (
    NULL,      /* Default vertex shader */
    fs_code,
    &error
);
```

### Using Shaders

```c
/* Check if valid */
if (!grl_shader_is_valid (shader))
{
    g_printerr ("Shader failed to compile\n");
    return;
}

/* Begin shader mode */
grl_shader_begin (shader);

/* Draw with shader applied */
grl_draw_texture (texture, x, y, white);

/* End shader mode */
grl_shader_end (shader);
```

### Setting Uniform Values

```c
/* Get uniform location (cache this for performance) */
gint time_loc = grl_shader_get_location (shader, "time");
gint resolution_loc = grl_shader_get_location (shader, "resolution");
gint color_loc = grl_shader_get_location (shader, "tintColor");

/* Set values */
grl_shader_set_value_float (shader, time_loc, elapsed_time);
grl_shader_set_value_vec2 (shader, resolution_loc, 800.0f, 600.0f);
grl_shader_set_value_vec4 (shader, color_loc, 1.0f, 0.5f, 0.2f, 1.0f);

/* Set texture uniform */
gint tex_loc = grl_shader_get_location (shader, "noiseTexture");
grl_shader_set_value_texture (shader, tex_loc, noise_texture);
```

### Shader Methods

| Method | Description |
|--------|-------------|
| `is_valid()` | Check if shader compiled successfully |
| `begin()` | Begin using this shader for drawing |
| `end()` | Return to default shader |
| `get_location(name)` | Get uniform location by name |
| `get_location_attrib(name)` | Get attribute location by name |
| `set_value_float(loc, value)` | Set float uniform |
| `set_value_int(loc, value)` | Set integer uniform |
| `set_value_vec2(loc, x, y)` | Set vec2 uniform |
| `set_value_vec3(loc, x, y, z)` | Set vec3 uniform |
| `set_value_vec4(loc, x, y, z, w)` | Set vec4 uniform |
| `set_value_texture(loc, texture)` | Set sampler2D uniform |

### Example: Grayscale Shader

```c
const gchar *grayscale_fs =
    "#version 330\n"
    "in vec2 fragTexCoord;\n"
    "in vec4 fragColor;\n"
    "uniform sampler2D texture0;\n"
    "out vec4 finalColor;\n"
    "void main() {\n"
    "    vec4 texColor = texture(texture0, fragTexCoord);\n"
    "    float gray = dot(texColor.rgb, vec3(0.299, 0.587, 0.114));\n"
    "    finalColor = vec4(gray, gray, gray, texColor.a) * fragColor;\n"
    "}\n";

g_autoptr(GrlShader) grayscale = grl_shader_new_from_memory (NULL, grayscale_fs, NULL);

/* In render loop */
grl_shader_begin (grayscale);
grl_draw_texture (sprite, x, y, white);
grl_shader_end (grayscale);
```

## GrlRenderTexture

Off-screen rendering target (framebuffer). Useful for post-processing, minimap rendering, or caching complex scenes.

### Creating Render Textures

```c
/* Create render texture matching window size */
g_autoptr(GrlRenderTexture) target = grl_render_texture_new (800, 600);

/* Check if valid */
if (!grl_render_texture_is_valid (target))
{
    g_printerr ("Failed to create render texture\n");
}

/* Get dimensions */
gint width = grl_render_texture_get_width (target);
gint height = grl_render_texture_get_height (target);
```

### Rendering to Texture

```c
/* Begin rendering to texture */
grl_render_texture_begin (target);

/* Clear the render texture */
g_autoptr(GrlColor) bg = grl_color_new (0, 0, 0, 255);
grl_window_clear_background (window, bg);

/* Draw scene to render texture */
grl_draw_circle (400, 300, 100, red);
grl_draw_rectangle (100, 100, 200, 150, blue);

/* End rendering to texture */
grl_render_texture_end (target);
```

### Drawing Render Texture to Screen

```c
/* Get the texture from render target */
g_autoptr(GrlTexture) tex = grl_render_texture_get_texture (target);

/* Draw to screen (note: Y is flipped in OpenGL framebuffers) */
g_autoptr(GrlRectangle) source = grl_rectangle_new (0, 600, 800, -600);  /* Flip Y */
g_autoptr(GrlRectangle) dest = grl_rectangle_new (0, 0, 800, 600);
g_autoptr(GrlVector2) origin = grl_vector2_new (0, 0);
g_autoptr(GrlColor) white = grl_color_new_white ();

grl_draw_texture_pro (tex, source, dest, origin, 0.0f, white);
```

### Methods

| Method | Description |
|--------|-------------|
| `is_valid()` | Check if render texture is valid |
| `get_width()` | Get width in pixels |
| `get_height()` | Get height in pixels |
| `begin()` | Begin rendering to this texture |
| `end()` | End rendering and return to screen |
| `get_texture()` | Get the color buffer as a texture |

### Example: Post-Processing Effect

```c
#include <graylib.h>

int
main (int argc, char *argv[])
{
    g_autoptr(GrlWindow) window = grl_window_new (800, 600, "Post-Processing");
    grl_window_set_target_fps (window, 60);

    /* Create render texture for scene */
    g_autoptr(GrlRenderTexture) scene_target = grl_render_texture_new (800, 600);

    /* Load post-processing shader */
    g_autoptr(GrlShader) blur_shader = grl_shader_new_from_file (
        NULL, "shaders/blur.fs", NULL);
    gint resolution_loc = grl_shader_get_location (blur_shader, "resolution");

    g_autoptr(GrlColor) bg = grl_color_new (30, 30, 45, 255);
    g_autoptr(GrlColor) white = grl_color_new_white ();
    g_autoptr(GrlColor) red = grl_color_new_red ();

    while (!grl_window_should_close (window))
    {
        /* 1. Render scene to texture */
        grl_render_texture_begin (scene_target);
        grl_window_clear_background (window, bg);
        grl_draw_circle (400, 300, 100, red);
        grl_render_texture_end (scene_target);

        /* 2. Draw scene texture to screen with shader */
        grl_window_begin_drawing (window);

        grl_shader_begin (blur_shader);
        grl_shader_set_value_vec2 (blur_shader, resolution_loc, 800.0f, 600.0f);

        g_autoptr(GrlTexture) scene_tex = grl_render_texture_get_texture (scene_target);
        g_autoptr(GrlRectangle) src = grl_rectangle_new (0, 600, 800, -600);
        g_autoptr(GrlRectangle) dst = grl_rectangle_new (0, 0, 800, 600);
        g_autoptr(GrlVector2) origin = grl_vector2_new (0, 0);
        grl_draw_texture_pro (scene_tex, src, dst, origin, 0.0f, white);

        grl_shader_end (blur_shader);

        grl_draw_fps (10, 10);
        grl_window_end_drawing (window);
    }

    return 0;
}
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

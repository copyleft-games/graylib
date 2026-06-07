# Enumerations

This document provides a reference for all enumeration types in Graylib.

## Window Configuration

### GrlConfigFlags

Window configuration flags. Can be combined with bitwise OR.

```c
typedef enum
{
    GRL_FLAG_VSYNC_HINT               = 0x00000040,
    GRL_FLAG_FULLSCREEN_MODE          = 0x00000002,
    GRL_FLAG_WINDOW_RESIZABLE         = 0x00000004,
    GRL_FLAG_WINDOW_UNDECORATED       = 0x00000008,
    GRL_FLAG_WINDOW_HIDDEN            = 0x00000080,
    GRL_FLAG_WINDOW_MINIMIZED         = 0x00000200,
    GRL_FLAG_WINDOW_MAXIMIZED         = 0x00000400,
    GRL_FLAG_WINDOW_UNFOCUSED         = 0x00000800,
    GRL_FLAG_WINDOW_TOPMOST           = 0x00001000,
    GRL_FLAG_WINDOW_ALWAYS_RUN        = 0x00000100,
    GRL_FLAG_WINDOW_TRANSPARENT       = 0x00000010,
    GRL_FLAG_WINDOW_HIGHDPI           = 0x00002000,
    GRL_FLAG_WINDOW_MOUSE_PASSTHROUGH = 0x00004000,
    GRL_FLAG_BORDERLESS_WINDOWED_MODE = 0x00008000,
    GRL_FLAG_MSAA_4X_HINT             = 0x00000020,
    GRL_FLAG_INTERLACED_HINT          = 0x00010000
} GrlConfigFlags;
```

| Flag | Description |
|------|-------------|
| `GRL_FLAG_VSYNC_HINT` | Enable V-Sync on GPU |
| `GRL_FLAG_FULLSCREEN_MODE` | Run in fullscreen mode |
| `GRL_FLAG_WINDOW_RESIZABLE` | Allow window resizing |
| `GRL_FLAG_WINDOW_UNDECORATED` | Disable window decoration (borders/title) |
| `GRL_FLAG_WINDOW_HIDDEN` | Start with hidden window |
| `GRL_FLAG_WINDOW_MINIMIZED` | Start minimized |
| `GRL_FLAG_WINDOW_MAXIMIZED` | Start maximized |
| `GRL_FLAG_WINDOW_UNFOCUSED` | Start without focus |
| `GRL_FLAG_WINDOW_TOPMOST` | Window always on top |
| `GRL_FLAG_WINDOW_ALWAYS_RUN` | Keep running while minimized |
| `GRL_FLAG_WINDOW_TRANSPARENT` | Enable transparent framebuffer |
| `GRL_FLAG_WINDOW_HIGHDPI` | Support HighDPI displays |
| `GRL_FLAG_WINDOW_MOUSE_PASSTHROUGH` | Allow mouse click-through |
| `GRL_FLAG_BORDERLESS_WINDOWED_MODE` | Borderless windowed mode |
| `GRL_FLAG_MSAA_4X_HINT` | Enable 4x MSAA anti-aliasing |
| `GRL_FLAG_INTERLACED_HINT` | Enable interlaced video format |

### GrlLogLevel

Trace log level for controlling output verbosity.

```c
typedef enum
{
    GRL_LOG_ALL = 0,
    GRL_LOG_TRACE,
    GRL_LOG_DEBUG,
    GRL_LOG_INFO,
    GRL_LOG_WARNING,
    GRL_LOG_ERROR,
    GRL_LOG_FATAL,
    GRL_LOG_NONE
} GrlLogLevel;
```

| Level | Description |
|-------|-------------|
| `GRL_LOG_ALL` | Display all logs |
| `GRL_LOG_TRACE` | Internal trace logging |
| `GRL_LOG_DEBUG` | Debug information |
| `GRL_LOG_INFO` | Program execution info |
| `GRL_LOG_WARNING` | Recoverable failures |
| `GRL_LOG_ERROR` | Unrecoverable failures |
| `GRL_LOG_FATAL` | Fatal errors (abort) |
| `GRL_LOG_NONE` | Disable all logging |

## Keyboard Input

### GrlKey

Keyboard keys (US keyboard layout). Only commonly used keys are shown below.

```c
typedef enum
{
    GRL_KEY_NULL            = 0,
    /* Letters */
    GRL_KEY_A               = 65,
    GRL_KEY_B               = 66,
    /* ... through GRL_KEY_Z = 90 */

    /* Numbers */
    GRL_KEY_ZERO            = 48,
    GRL_KEY_ONE             = 49,
    /* ... through GRL_KEY_NINE = 57 */

    /* Function keys */
    GRL_KEY_SPACE           = 32,
    GRL_KEY_ESCAPE          = 256,
    GRL_KEY_ENTER           = 257,
    GRL_KEY_TAB             = 258,
    GRL_KEY_BACKSPACE       = 259,
    GRL_KEY_INSERT          = 260,
    GRL_KEY_DELETE          = 261,

    /* Arrow keys */
    GRL_KEY_RIGHT           = 262,
    GRL_KEY_LEFT            = 263,
    GRL_KEY_DOWN            = 264,
    GRL_KEY_UP              = 265,

    /* F keys */
    GRL_KEY_F1              = 290,
    /* ... through GRL_KEY_F12 = 301 */

    /* Modifiers */
    GRL_KEY_LEFT_SHIFT      = 340,
    GRL_KEY_LEFT_CONTROL    = 341,
    GRL_KEY_LEFT_ALT        = 342,
    GRL_KEY_LEFT_SUPER      = 343,
    GRL_KEY_RIGHT_SHIFT     = 344,
    GRL_KEY_RIGHT_CONTROL   = 345,
    GRL_KEY_RIGHT_ALT       = 346,
    GRL_KEY_RIGHT_SUPER     = 347,

    /* Keypad */
    GRL_KEY_KP_0            = 320,
    /* ... through GRL_KEY_KP_9 = 329 */
    GRL_KEY_KP_ENTER        = 335,

    /* Symbols */
    GRL_KEY_APOSTROPHE      = 39,   /* ' */
    GRL_KEY_COMMA           = 44,   /* , */
    GRL_KEY_MINUS           = 45,   /* - */
    GRL_KEY_PERIOD          = 46,   /* . */
    GRL_KEY_SLASH           = 47,   /* / */
    GRL_KEY_SEMICOLON       = 59,   /* ; */
    GRL_KEY_EQUAL           = 61,   /* = */
    GRL_KEY_LEFT_BRACKET    = 91,   /* [ */
    GRL_KEY_BACKSLASH       = 92,   /* \ */
    GRL_KEY_RIGHT_BRACKET   = 93,   /* ] */
    GRL_KEY_GRAVE           = 96    /* ` */
} GrlKey;
```

**Usage:**
```c
if (grl_input_is_key_pressed (GRL_KEY_SPACE))
{
    player_jump ();
}

if (grl_input_is_key_down (GRL_KEY_LEFT_SHIFT))
{
    player_run ();
}
```

## Mouse Input

### GrlMouseButton

Mouse buttons.

```c
typedef enum
{
    GRL_MOUSE_BUTTON_LEFT    = 0,
    GRL_MOUSE_BUTTON_RIGHT   = 1,
    GRL_MOUSE_BUTTON_MIDDLE  = 2,
    GRL_MOUSE_BUTTON_SIDE    = 3,
    GRL_MOUSE_BUTTON_EXTRA   = 4,
    GRL_MOUSE_BUTTON_FORWARD = 5,
    GRL_MOUSE_BUTTON_BACK    = 6
} GrlMouseButton;
```

| Button | Description |
|--------|-------------|
| `GRL_MOUSE_BUTTON_LEFT` | Primary click |
| `GRL_MOUSE_BUTTON_RIGHT` | Secondary click (context menu) |
| `GRL_MOUSE_BUTTON_MIDDLE` | Middle click (scroll wheel press) |
| `GRL_MOUSE_BUTTON_SIDE` | Side button |
| `GRL_MOUSE_BUTTON_EXTRA` | Extra button |
| `GRL_MOUSE_BUTTON_FORWARD` | Forward navigation button |
| `GRL_MOUSE_BUTTON_BACK` | Back navigation button |

### GrlMouseCursor

Mouse cursor shapes.

```c
typedef enum
{
    GRL_MOUSE_CURSOR_DEFAULT       = 0,
    GRL_MOUSE_CURSOR_ARROW         = 1,
    GRL_MOUSE_CURSOR_IBEAM         = 2,
    GRL_MOUSE_CURSOR_CROSSHAIR     = 3,
    GRL_MOUSE_CURSOR_POINTING_HAND = 4,
    GRL_MOUSE_CURSOR_RESIZE_EW     = 5,
    GRL_MOUSE_CURSOR_RESIZE_NS     = 6,
    GRL_MOUSE_CURSOR_RESIZE_NWSE   = 7,
    GRL_MOUSE_CURSOR_RESIZE_NESW   = 8,
    GRL_MOUSE_CURSOR_RESIZE_ALL    = 9,
    GRL_MOUSE_CURSOR_NOT_ALLOWED   = 10
} GrlMouseCursor;
```

| Cursor | Description |
|--------|-------------|
| `GRL_MOUSE_CURSOR_DEFAULT` | Default system cursor |
| `GRL_MOUSE_CURSOR_ARROW` | Arrow pointer |
| `GRL_MOUSE_CURSOR_IBEAM` | Text editing cursor |
| `GRL_MOUSE_CURSOR_CROSSHAIR` | Crosshair |
| `GRL_MOUSE_CURSOR_POINTING_HAND` | Clickable link cursor |
| `GRL_MOUSE_CURSOR_RESIZE_EW` | Horizontal resize |
| `GRL_MOUSE_CURSOR_RESIZE_NS` | Vertical resize |
| `GRL_MOUSE_CURSOR_RESIZE_NWSE` | Diagonal resize (NW-SE) |
| `GRL_MOUSE_CURSOR_RESIZE_NESW` | Diagonal resize (NE-SW) |
| `GRL_MOUSE_CURSOR_RESIZE_ALL` | Omnidirectional resize |
| `GRL_MOUSE_CURSOR_NOT_ALLOWED` | Operation not allowed |

## Gamepad Input

### GrlGamepadButton

Gamepad buttons with common controller mappings.

```c
typedef enum
{
    GRL_GAMEPAD_BUTTON_UNKNOWN = 0,
    /* D-pad */
    GRL_GAMEPAD_BUTTON_LEFT_FACE_UP,
    GRL_GAMEPAD_BUTTON_LEFT_FACE_RIGHT,
    GRL_GAMEPAD_BUTTON_LEFT_FACE_DOWN,
    GRL_GAMEPAD_BUTTON_LEFT_FACE_LEFT,
    /* Face buttons */
    GRL_GAMEPAD_BUTTON_RIGHT_FACE_UP,      /* Y / Triangle */
    GRL_GAMEPAD_BUTTON_RIGHT_FACE_RIGHT,   /* B / Circle */
    GRL_GAMEPAD_BUTTON_RIGHT_FACE_DOWN,    /* A / Cross */
    GRL_GAMEPAD_BUTTON_RIGHT_FACE_LEFT,    /* X / Square */
    /* Shoulder buttons */
    GRL_GAMEPAD_BUTTON_LEFT_TRIGGER_1,     /* L1 / LB */
    GRL_GAMEPAD_BUTTON_LEFT_TRIGGER_2,     /* L2 / LT */
    GRL_GAMEPAD_BUTTON_RIGHT_TRIGGER_1,    /* R1 / RB */
    GRL_GAMEPAD_BUTTON_RIGHT_TRIGGER_2,    /* R2 / RT */
    /* Middle buttons */
    GRL_GAMEPAD_BUTTON_MIDDLE_LEFT,        /* Select / Back */
    GRL_GAMEPAD_BUTTON_MIDDLE,             /* Guide / Home */
    GRL_GAMEPAD_BUTTON_MIDDLE_RIGHT,       /* Start */
    /* Thumbstick clicks */
    GRL_GAMEPAD_BUTTON_LEFT_THUMB,         /* L3 */
    GRL_GAMEPAD_BUTTON_RIGHT_THUMB         /* R3 */
} GrlGamepadButton;
```

### GrlGamepadAxis

Gamepad analog axes.

```c
typedef enum
{
    GRL_GAMEPAD_AXIS_LEFT_X        = 0,
    GRL_GAMEPAD_AXIS_LEFT_Y        = 1,
    GRL_GAMEPAD_AXIS_RIGHT_X       = 2,
    GRL_GAMEPAD_AXIS_RIGHT_Y       = 3,
    GRL_GAMEPAD_AXIS_LEFT_TRIGGER  = 4,
    GRL_GAMEPAD_AXIS_RIGHT_TRIGGER = 5
} GrlGamepadAxis;
```

| Axis | Range | Description |
|------|-------|-------------|
| `GRL_GAMEPAD_AXIS_LEFT_X` | -1.0 to 1.0 | Left stick horizontal |
| `GRL_GAMEPAD_AXIS_LEFT_Y` | -1.0 to 1.0 | Left stick vertical |
| `GRL_GAMEPAD_AXIS_RIGHT_X` | -1.0 to 1.0 | Right stick horizontal |
| `GRL_GAMEPAD_AXIS_RIGHT_Y` | -1.0 to 1.0 | Right stick vertical |
| `GRL_GAMEPAD_AXIS_LEFT_TRIGGER` | 0.0 to 1.0 | Left trigger pressure |
| `GRL_GAMEPAD_AXIS_RIGHT_TRIGGER` | 0.0 to 1.0 | Right trigger pressure |

## Graphics

### GrlFillRule

Fill rule for `grl_image_fill_path`.  Controls how self-intersecting or
nested subpaths determine the filled region.

```c
typedef enum
{
    GRL_FILL_RULE_NONZERO  = 0,  /* nick: "nonzero"  */
    GRL_FILL_RULE_EVEN_ODD       /* nick: "even-odd" */
} GrlFillRule;
```

| Value | GType nick | Description |
|-------|------------|-------------|
| `GRL_FILL_RULE_NONZERO` | `"nonzero"` | Fill a region if its winding count is non-zero.  Standard SVG/PDF behaviour.  A CCW inner contour inside a CW outer contour creates a hole. |
| `GRL_FILL_RULE_EVEN_ODD` | `"even-odd"` | Fill a region if the ray-crossing count is odd (alternating inside/outside). |

Used with `grl_image_fill_path (image, path, GRL_FILL_RULE_NONZERO, color)`.

### GrlBlendMode

Color blending modes for rendering.

```c
typedef enum
{
    GRL_BLEND_ALPHA = 0,
    GRL_BLEND_ADDITIVE,
    GRL_BLEND_MULTIPLIED,
    GRL_BLEND_ADD_COLORS,
    GRL_BLEND_SUBTRACT_COLORS,
    GRL_BLEND_ALPHA_PREMULTIPLY,
    GRL_BLEND_CUSTOM,
    GRL_BLEND_CUSTOM_SEPARATE
} GrlBlendMode;
```

| Mode | Description |
|------|-------------|
| `GRL_BLEND_ALPHA` | Standard alpha blending (default) |
| `GRL_BLEND_ADDITIVE` | Add source and destination colors |
| `GRL_BLEND_MULTIPLIED` | Multiply colors together |
| `GRL_BLEND_ADD_COLORS` | Alternative additive blend |
| `GRL_BLEND_SUBTRACT_COLORS` | Subtract source from destination |
| `GRL_BLEND_ALPHA_PREMULTIPLY` | Premultiplied alpha blending |
| `GRL_BLEND_CUSTOM` | Custom blend mode |
| `GRL_BLEND_CUSTOM_SEPARATE` | Custom with separate RGB/alpha |

**Usage:**
```c
grl_set_blend_mode (GRL_BLEND_ADDITIVE);
grl_draw_texture (glow_texture, x, y, white);
grl_set_blend_mode (GRL_BLEND_ALPHA);  /* Reset to default */
```

### GrlImageBlendMode

Per-image CPU blend mode for `grl_image_draw_*` primitives. Distinct from
`GrlBlendMode` (which controls GPU/texture blending). All modes except
`GRL_IMAGE_BLEND_REPLACE` require an `R8G8B8A8` image; on other formats drawing
falls back to `GRL_IMAGE_BLEND_REPLACE`.

```c
typedef enum
{
    GRL_IMAGE_BLEND_REPLACE = 0,
    GRL_IMAGE_BLEND_OVER,
    GRL_IMAGE_BLEND_ADD,
    GRL_IMAGE_BLEND_MULTIPLY,
    GRL_IMAGE_BLEND_SUBTRACT
} GrlImageBlendMode;
```

| Mode | Description |
|------|-------------|
| `GRL_IMAGE_BLEND_REPLACE` | Overwrite destination, ignore alpha (default; legacy behavior) |
| `GRL_IMAGE_BLEND_OVER` | Standard straight-alpha source-over compositing |
| `GRL_IMAGE_BLEND_ADD` | Saturating additive blend (glow) |
| `GRL_IMAGE_BLEND_MULTIPLY` | Multiplicative blend |
| `GRL_IMAGE_BLEND_SUBTRACT` | Saturating subtractive blend |

**Usage:**
```c
grl_image_set_blend_mode (img, GRL_IMAGE_BLEND_ADD);
grl_image_draw_gradient_radial (img, cx, cy, r, hot, edge);
grl_image_set_blend_mode (img, GRL_IMAGE_BLEND_REPLACE);  /* Reset to default */
```

### GrlImageColorSpace

Colour space in which `grl_image_draw_*` compositing math is performed, set per
image with `grl_image_set_blend_color_space()`. Controls only the *blended*
modes; `GRL_IMAGE_BLEND_REPLACE` always overwrites and is unaffected.

```c
typedef enum
{
    GRL_IMAGE_COLOR_SPACE_GAMMA = 0,
    GRL_IMAGE_COLOR_SPACE_LINEAR
} GrlImageColorSpace;
```

| Value | Description |
|-------|-------------|
| `GRL_IMAGE_COLOR_SPACE_GAMMA` | Blend directly on 8-bit sRGB values (default; byte-for-byte identical to legacy behaviour) |
| `GRL_IMAGE_COLOR_SPACE_LINEAR` | Decode sRGB→linear, blend, re-encode; physically-correct edges that do not darken or shift hue at partial coverage. Requires an `R8G8B8A8` image (falls back to gamma otherwise) |

**Usage:**
```c
grl_image_set_blend_color_space (img, GRL_IMAGE_COLOR_SPACE_LINEAR);
grl_image_set_blend_mode (img, GRL_IMAGE_BLEND_OVER);
/* ...anti-aliased / alpha-blended draws now composite in linear light... */
grl_image_set_blend_color_space (img, GRL_IMAGE_COLOR_SPACE_GAMMA);  /* default */
```

### GrlPorterDuffOp

Porter-Duff compositing operator for `grl_image_composite()`. These are the
twelve standard operators from the 1984 paper *Compositing Digital Images* by
Thomas Porter and Tom Duff. The operation formula for each pixel is:

> `out_premul = Fa × src_premul + Fb × dst_premul`
> `out_alpha  = Fa × sa + Fb × da`

where `sa` and `da` are the source / destination alpha values in [0, 1].

```c
typedef enum
{
    GRL_PORTER_DUFF_CLEAR    = 0,
    GRL_PORTER_DUFF_SRC,
    GRL_PORTER_DUFF_DST,
    GRL_PORTER_DUFF_SRC_OVER,
    GRL_PORTER_DUFF_DST_OVER,
    GRL_PORTER_DUFF_SRC_IN,
    GRL_PORTER_DUFF_DST_IN,
    GRL_PORTER_DUFF_SRC_OUT,
    GRL_PORTER_DUFF_DST_OUT,
    GRL_PORTER_DUFF_SRC_ATOP,
    GRL_PORTER_DUFF_DST_ATOP,
    GRL_PORTER_DUFF_XOR
} GrlPorterDuffOp;
```

| Operator | Fa | Fb | Description |
|----------|----|----|-------------|
| `GRL_PORTER_DUFF_CLEAR` | 0 | 0 | Both cleared — result is transparent black |
| `GRL_PORTER_DUFF_SRC` | 1 | 0 | Source replaces destination |
| `GRL_PORTER_DUFF_DST` | 0 | 1 | Destination unchanged (no-op) |
| `GRL_PORTER_DUFF_SRC_OVER` | 1 | 1−sa | Standard alpha compositing (source over destination) |
| `GRL_PORTER_DUFF_DST_OVER` | 1−da | 1 | Source placed *behind* destination — use for drop shadows |
| `GRL_PORTER_DUFF_SRC_IN` | da | 0 | Source trimmed to destination's shape |
| `GRL_PORTER_DUFF_DST_IN` | 0 | sa | Destination trimmed to source's shape |
| `GRL_PORTER_DUFF_SRC_OUT` | 1−da | 0 | Source shown only outside destination's shape |
| `GRL_PORTER_DUFF_DST_OUT` | 0 | 1−sa | Destination shown only outside source's shape |
| `GRL_PORTER_DUFF_SRC_ATOP` | da | 1−sa | Source atop destination; destination visible elsewhere |
| `GRL_PORTER_DUFF_DST_ATOP` | 1−da | sa | Destination atop source; source visible elsewhere |
| `GRL_PORTER_DUFF_XOR` | 1−da | 1−sa | Each shows only where the other is absent |

> **Alias note:** `GRL_PORTER_DUFF_DST_OVER` is called `DEST_OVER` in some
> other APIs (e.g. the HTML Canvas 2D specification); the two names refer to
> the identical operation.

**Usage:**
```c
/* Composite a sprite onto a canvas */
grl_image_composite (canvas, sprite, GRL_PORTER_DUFF_SRC_OVER, x, y);

/* Place a blurred shadow behind existing content */
grl_image_composite (canvas, shadow, GRL_PORTER_DUFF_DST_OVER, sx, sy);
```

### GrlGifQuantizer

Palette-building strategy for `GrlGifWriter` (see `grl_image_set...` setters on
the writer). The default is byte-for-byte compatible with the original encoder.

| Value | Nick | Description |
|-------|------|-------------|
| `GRL_GIF_QUANTIZER_WEB_SAFE` | `"web-safe"` | Fixed 6×6×6 216-colour web-safe palette (default; byte-identical output) |
| `GRL_GIF_QUANTIZER_MEDIAN_CUT` | `"median-cut"` | Adaptive median-cut; builds the palette from image data |

### GrlGifDither

Dithering mode for `GrlGifWriter`.

| Value | Nick | Description |
|-------|------|-------------|
| `GRL_GIF_DITHER_NONE` | `"none"` | No dithering; direct nearest-colour mapping (default) |
| `GRL_GIF_DITHER_FLOYD_STEINBERG` | `"floyd-steinberg"` | Floyd–Steinberg error diffusion; only active with `MEDIAN_CUT` |

### GrlGifPaletteScope

Palette scope for `GrlGifWriter` in median-cut mode.

| Value | Nick | Description |
|-------|------|-------------|
| `GRL_GIF_PALETTE_SCOPE_GLOBAL` | `"global"` | Single palette from the first frame, shared by all frames (default) |
| `GRL_GIF_PALETTE_SCOPE_PER_FRAME` | `"per-frame"` | Per-frame Local Color Table; each frame independently quantised |

### GrlGradientAxis

Interpolation axis for `grl_image_draw_gradient_rect()`.

```c
typedef enum
{
    GRL_GRADIENT_AXIS_HORIZONTAL = 0,
    GRL_GRADIENT_AXIS_VERTICAL
} GrlGradientAxis;
```

| Value | Description |
|-------|-------------|
| `GRL_GRADIENT_AXIS_HORIZONTAL` | Interpolate left-to-right |
| `GRL_GRADIENT_AXIS_VERTICAL` | Interpolate top-to-bottom |

### GrlTextureFilter

Texture filtering modes.

```c
typedef enum
{
    GRL_TEXTURE_FILTER_POINT = 0,
    GRL_TEXTURE_FILTER_BILINEAR,
    GRL_TEXTURE_FILTER_TRILINEAR,
    GRL_TEXTURE_FILTER_ANISOTROPIC_4X,
    GRL_TEXTURE_FILTER_ANISOTROPIC_8X,
    GRL_TEXTURE_FILTER_ANISOTROPIC_16X
} GrlTextureFilter;
```

| Filter | Description |
|--------|-------------|
| `GRL_TEXTURE_FILTER_POINT` | Nearest-neighbor (pixelated, good for pixel art) |
| `GRL_TEXTURE_FILTER_BILINEAR` | Linear interpolation (smooth) |
| `GRL_TEXTURE_FILTER_TRILINEAR` | Linear with mipmaps |
| `GRL_TEXTURE_FILTER_ANISOTROPIC_4X` | 4x anisotropic filtering |
| `GRL_TEXTURE_FILTER_ANISOTROPIC_8X` | 8x anisotropic filtering |
| `GRL_TEXTURE_FILTER_ANISOTROPIC_16X` | 16x anisotropic filtering |

### GrlTextureWrap

Texture wrapping modes.

```c
typedef enum
{
    GRL_TEXTURE_WRAP_REPEAT = 0,
    GRL_TEXTURE_WRAP_CLAMP,
    GRL_TEXTURE_WRAP_MIRROR_REPEAT,
    GRL_TEXTURE_WRAP_MIRROR_CLAMP
} GrlTextureWrap;
```

| Mode | Description |
|------|-------------|
| `GRL_TEXTURE_WRAP_REPEAT` | Tile texture infinitely |
| `GRL_TEXTURE_WRAP_CLAMP` | Clamp to edge pixels |
| `GRL_TEXTURE_WRAP_MIRROR_REPEAT` | Mirror and repeat |
| `GRL_TEXTURE_WRAP_MIRROR_CLAMP` | Mirror and clamp |

### GrlPixelFormat

Pixel formats for images and textures.

```c
typedef enum
{
    /* Uncompressed formats */
    GRL_PIXELFORMAT_UNCOMPRESSED_GRAYSCALE = 1,    /* 8 bpp */
    GRL_PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA,        /* 16 bpp */
    GRL_PIXELFORMAT_UNCOMPRESSED_R5G6B5,            /* 16 bpp RGB */
    GRL_PIXELFORMAT_UNCOMPRESSED_R8G8B8,            /* 24 bpp RGB */
    GRL_PIXELFORMAT_UNCOMPRESSED_R5G5B5A1,          /* 16 bpp RGBA */
    GRL_PIXELFORMAT_UNCOMPRESSED_R4G4B4A4,          /* 16 bpp RGBA */
    GRL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,          /* 32 bpp RGBA */
    GRL_PIXELFORMAT_UNCOMPRESSED_R32,               /* 32 bpp float */
    GRL_PIXELFORMAT_UNCOMPRESSED_R32G32B32,         /* 96 bpp RGB float */
    GRL_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32,      /* 128 bpp RGBA float */
    GRL_PIXELFORMAT_UNCOMPRESSED_R16,               /* 16 bpp half float */
    GRL_PIXELFORMAT_UNCOMPRESSED_R16G16B16,         /* 48 bpp RGB half */
    GRL_PIXELFORMAT_UNCOMPRESSED_R16G16B16A16,      /* 64 bpp RGBA half */

    /* Compressed formats */
    GRL_PIXELFORMAT_COMPRESSED_DXT1_RGB,            /* 4 bpp */
    GRL_PIXELFORMAT_COMPRESSED_DXT1_RGBA,           /* 4 bpp */
    GRL_PIXELFORMAT_COMPRESSED_DXT3_RGBA,           /* 8 bpp */
    GRL_PIXELFORMAT_COMPRESSED_DXT5_RGBA,           /* 8 bpp */
    GRL_PIXELFORMAT_COMPRESSED_ETC1_RGB,            /* 4 bpp */
    GRL_PIXELFORMAT_COMPRESSED_ETC2_RGB,            /* 4 bpp */
    GRL_PIXELFORMAT_COMPRESSED_ETC2_EAC_RGBA,       /* 8 bpp */
    GRL_PIXELFORMAT_COMPRESSED_PVRT_RGB,            /* 4 bpp */
    GRL_PIXELFORMAT_COMPRESSED_PVRT_RGBA,           /* 4 bpp */
    GRL_PIXELFORMAT_COMPRESSED_ASTC_4x4_RGBA,       /* 8 bpp */
    GRL_PIXELFORMAT_COMPRESSED_ASTC_8x8_RGBA        /* 2 bpp */
} GrlPixelFormat;
```

## Camera

### GrlCameraMode

Camera system modes for 3D cameras.

```c
typedef enum
{
    GRL_CAMERA_CUSTOM = 0,
    GRL_CAMERA_FREE,
    GRL_CAMERA_ORBITAL,
    GRL_CAMERA_FIRST_PERSON,
    GRL_CAMERA_THIRD_PERSON
} GrlCameraMode;
```

| Mode | Description |
|------|-------------|
| `GRL_CAMERA_CUSTOM` | Manual camera control |
| `GRL_CAMERA_FREE` | Free-flying camera |
| `GRL_CAMERA_ORBITAL` | Orbit around target point |
| `GRL_CAMERA_FIRST_PERSON` | First-person view |
| `GRL_CAMERA_THIRD_PERSON` | Third-person follow camera |

### GrlCameraProjection

Camera projection types.

```c
typedef enum
{
    GRL_CAMERA_PERSPECTIVE = 0,
    GRL_CAMERA_ORTHOGRAPHIC
} GrlCameraProjection;
```

| Projection | Description |
|------------|-------------|
| `GRL_CAMERA_PERSPECTIVE` | 3D perspective (objects shrink with distance) |
| `GRL_CAMERA_ORTHOGRAPHIC` | 2D-like view (no perspective distortion) |

## Touch/Gesture

### GrlGesture

Touch gestures. Can be combined with bitwise OR for multi-gesture detection.

```c
typedef enum
{
    GRL_GESTURE_NONE        = 0,
    GRL_GESTURE_TAP         = 1,
    GRL_GESTURE_DOUBLETAP   = 2,
    GRL_GESTURE_HOLD        = 4,
    GRL_GESTURE_DRAG        = 8,
    GRL_GESTURE_SWIPE_RIGHT = 16,
    GRL_GESTURE_SWIPE_LEFT  = 32,
    GRL_GESTURE_SWIPE_UP    = 64,
    GRL_GESTURE_SWIPE_DOWN  = 128,
    GRL_GESTURE_PINCH_IN    = 256,
    GRL_GESTURE_PINCH_OUT   = 512
} GrlGesture;
```

| Gesture | Description |
|---------|-------------|
| `GRL_GESTURE_NONE` | No gesture detected |
| `GRL_GESTURE_TAP` | Single tap |
| `GRL_GESTURE_DOUBLETAP` | Double tap |
| `GRL_GESTURE_HOLD` | Touch and hold |
| `GRL_GESTURE_DRAG` | Touch and drag |
| `GRL_GESTURE_SWIPE_RIGHT` | Swipe right |
| `GRL_GESTURE_SWIPE_LEFT` | Swipe left |
| `GRL_GESTURE_SWIPE_UP` | Swipe up |
| `GRL_GESTURE_SWIPE_DOWN` | Swipe down |
| `GRL_GESTURE_PINCH_IN` | Pinch inward (zoom out) |
| `GRL_GESTURE_PINCH_OUT` | Pinch outward (zoom in) |

**Usage:**
```c
/* Enable gesture detection */
grl_input_set_gestures_enabled (GRL_GESTURE_TAP |
                                 GRL_GESTURE_SWIPE_LEFT |
                                 GRL_GESTURE_SWIPE_RIGHT);

/* Check for gestures */
GrlGesture gesture = grl_input_get_gesture_detected ();
if (gesture == GRL_GESTURE_SWIPE_LEFT)
{
    next_page ();
}
```

## Material Maps

### GrlMaterialMapType

Material map types for PBR rendering. See [Models](models.md) for details.

```c
typedef enum
{
    GRL_MATERIAL_MAP_ALBEDO,
    GRL_MATERIAL_MAP_METALNESS,
    GRL_MATERIAL_MAP_NORMAL,
    GRL_MATERIAL_MAP_ROUGHNESS,
    GRL_MATERIAL_MAP_OCCLUSION,
    GRL_MATERIAL_MAP_EMISSION,
    GRL_MATERIAL_MAP_HEIGHT,
    GRL_MATERIAL_MAP_CUBEMAP,
    GRL_MATERIAL_MAP_IRRADIANCE,
    GRL_MATERIAL_MAP_PREFILTER,
    GRL_MATERIAL_MAP_BRDF
} GrlMaterialMapType;
```

| Map Type | Description |
|----------|-------------|
| `GRL_MATERIAL_MAP_ALBEDO` | Base color / diffuse |
| `GRL_MATERIAL_MAP_METALNESS` | Metallic factor |
| `GRL_MATERIAL_MAP_NORMAL` | Surface normals |
| `GRL_MATERIAL_MAP_ROUGHNESS` | Surface roughness |
| `GRL_MATERIAL_MAP_OCCLUSION` | Ambient occlusion |
| `GRL_MATERIAL_MAP_EMISSION` | Self-illumination |
| `GRL_MATERIAL_MAP_HEIGHT` | Displacement/height |
| `GRL_MATERIAL_MAP_CUBEMAP` | Environment reflection |
| `GRL_MATERIAL_MAP_IRRADIANCE` | Diffuse IBL |
| `GRL_MATERIAL_MAP_PREFILTER` | Specular IBL |
| `GRL_MATERIAL_MAP_BRDF` | BRDF lookup table |

## RLGL (Low-Level OpenGL)

### GrlRlglDrawMode

Primitive draw modes for RLGL immediate mode rendering.

```c
typedef enum
{
    GRL_RLGL_LINES     = 0x0001,
    GRL_RLGL_TRIANGLES = 0x0004,
    GRL_RLGL_QUADS     = 0x0007
} GrlRlglDrawMode;
```

| Mode | Description |
|------|-------------|
| `GRL_RLGL_LINES` | Draw lines |
| `GRL_RLGL_TRIANGLES` | Draw triangles |
| `GRL_RLGL_QUADS` | Draw quads (converted to triangles internally) |

### GrlRlglMatrixMode

Matrix mode for RLGL matrix stack operations.

```c
typedef enum
{
    GRL_RLGL_MODELVIEW  = 0x1700,
    GRL_RLGL_PROJECTION = 0x1701,
    GRL_RLGL_TEXTURE    = 0x1702
} GrlRlglMatrixMode;
```

| Mode | Description |
|------|-------------|
| `GRL_RLGL_MODELVIEW` | Model-view transformation matrix |
| `GRL_RLGL_PROJECTION` | Projection matrix |
| `GRL_RLGL_TEXTURE` | Texture coordinate matrix |

### GrlRlglCullMode

Face culling modes.

```c
typedef enum
{
    GRL_RLGL_CULL_FRONT = 0,
    GRL_RLGL_CULL_BACK  = 1
} GrlRlglCullMode;
```

| Mode | Description |
|------|-------------|
| `GRL_RLGL_CULL_FRONT` | Cull front-facing polygons |
| `GRL_RLGL_CULL_BACK` | Cull back-facing polygons |

### GrlRlglGlVersion

OpenGL version identifiers.

```c
typedef enum
{
    GRL_RLGL_OPENGL_11    = 1,
    GRL_RLGL_OPENGL_21    = 2,
    GRL_RLGL_OPENGL_33    = 3,
    GRL_RLGL_OPENGL_43    = 4,
    GRL_RLGL_OPENGL_ES_20 = 5,
    GRL_RLGL_OPENGL_ES_30 = 6
} GrlRlglGlVersion;
```

| Version | Description |
|---------|-------------|
| `GRL_RLGL_OPENGL_11` | OpenGL 1.1 |
| `GRL_RLGL_OPENGL_21` | OpenGL 2.1 |
| `GRL_RLGL_OPENGL_33` | OpenGL 3.3 Core |
| `GRL_RLGL_OPENGL_43` | OpenGL 4.3 |
| `GRL_RLGL_OPENGL_ES_20` | OpenGL ES 2.0 |
| `GRL_RLGL_OPENGL_ES_30` | OpenGL ES 3.0 |

### GrlRlglFramebufferAttachType

Framebuffer attachment types.

```c
typedef enum
{
    GRL_RLGL_FRAMEBUFFER_ATTACH_COLOR_CHANNEL0 = 0,
    GRL_RLGL_FRAMEBUFFER_ATTACH_COLOR_CHANNEL1 = 1,
    GRL_RLGL_FRAMEBUFFER_ATTACH_COLOR_CHANNEL2 = 2,
    GRL_RLGL_FRAMEBUFFER_ATTACH_COLOR_CHANNEL3 = 3,
    GRL_RLGL_FRAMEBUFFER_ATTACH_COLOR_CHANNEL4 = 4,
    GRL_RLGL_FRAMEBUFFER_ATTACH_COLOR_CHANNEL5 = 5,
    GRL_RLGL_FRAMEBUFFER_ATTACH_COLOR_CHANNEL6 = 6,
    GRL_RLGL_FRAMEBUFFER_ATTACH_COLOR_CHANNEL7 = 7,
    GRL_RLGL_FRAMEBUFFER_ATTACH_DEPTH          = 100,
    GRL_RLGL_FRAMEBUFFER_ATTACH_STENCIL        = 200
} GrlRlglFramebufferAttachType;
```

| Type | Description |
|------|-------------|
| `GRL_RLGL_FRAMEBUFFER_ATTACH_COLOR_CHANNEL0-7` | Color attachment slots 0-7 |
| `GRL_RLGL_FRAMEBUFFER_ATTACH_DEPTH` | Depth buffer attachment |
| `GRL_RLGL_FRAMEBUFFER_ATTACH_STENCIL` | Stencil buffer attachment |

### GrlRlglFramebufferTexType

Framebuffer texture types.

```c
typedef enum
{
    GRL_RLGL_FRAMEBUFFER_TEX_CUBEMAP_POSITIVE_X = 0,
    GRL_RLGL_FRAMEBUFFER_TEX_CUBEMAP_NEGATIVE_X = 1,
    GRL_RLGL_FRAMEBUFFER_TEX_CUBEMAP_POSITIVE_Y = 2,
    GRL_RLGL_FRAMEBUFFER_TEX_CUBEMAP_NEGATIVE_Y = 3,
    GRL_RLGL_FRAMEBUFFER_TEX_CUBEMAP_POSITIVE_Z = 4,
    GRL_RLGL_FRAMEBUFFER_TEX_CUBEMAP_NEGATIVE_Z = 5,
    GRL_RLGL_FRAMEBUFFER_TEX_TEXTURE2D          = 100,
    GRL_RLGL_FRAMEBUFFER_TEX_RENDERBUFFER       = 200
} GrlRlglFramebufferTexType;
```

| Type | Description |
|------|-------------|
| `GRL_RLGL_FRAMEBUFFER_TEX_CUBEMAP_*` | Cubemap face attachments |
| `GRL_RLGL_FRAMEBUFFER_TEX_TEXTURE2D` | 2D texture attachment |
| `GRL_RLGL_FRAMEBUFFER_TEX_RENDERBUFFER` | Renderbuffer attachment |

### GrlRlglShaderType

Shader types for compilation.

```c
typedef enum
{
    GRL_RLGL_SHADER_FRAGMENT = 0x8B30,
    GRL_RLGL_SHADER_VERTEX   = 0x8B31,
    GRL_RLGL_SHADER_COMPUTE  = 0x91B9
} GrlRlglShaderType;
```

| Type | Description |
|------|-------------|
| `GRL_RLGL_SHADER_FRAGMENT` | Fragment (pixel) shader |
| `GRL_RLGL_SHADER_VERTEX` | Vertex shader |
| `GRL_RLGL_SHADER_COMPUTE` | Compute shader |

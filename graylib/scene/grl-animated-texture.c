/* grl-animated-texture.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Animated texture from spritesheet.
 */

#include "config.h"
#include "grl-animated-texture.h"
#include <gio/gio.h>

/**
 * SECTION:grl-animated-texture
 * @title: GrlAnimatedTexture
 * @short_description: Animated texture from spritesheet
 *
 * #GrlAnimatedTexture provides frame-based animation from a spritesheet.
 * It automatically calculates frame rectangles based on frame dimensions
 * and advances through frames based on time.
 *
 * # Creating an Animation
 *
 * |[<!-- language="C" -->
 * g_autoptr(GrlAnimatedTexture) anim = NULL;
 *
 * // Load a spritesheet with 32x32 frames
 * anim = grl_animated_texture_new_from_file ("walk.png", 32, 32, NULL);
 *
 * // Configure animation
 * grl_animated_texture_set_fps (anim, 12.0f);
 * grl_animated_texture_set_looping (anim, TRUE);
 * grl_animated_texture_play (anim);
 * ]|
 *
 * # Using with GrlSprite
 *
 * |[<!-- language="C" -->
 * // In your update function
 * grl_animated_texture_update (anim, delta);
 *
 * // Apply current frame to sprite
 * g_autoptr(GrlRectangle) rect = grl_animated_texture_get_current_rect (anim);
 * grl_sprite_set_source_rect (sprite, rect);
 * ]|
 */

struct _GrlAnimatedTexture
{
    GObject      parent_instance;

    GrlTexture  *texture;
    gint         frame_width;
    gint         frame_height;
    gint         cols;           /* Frames per row */
    gint         rows;           /* Number of rows */
    gint         frame_count;    /* Total frames to use */
    gint         max_frames;     /* Maximum possible frames */

    gint         current_frame;
    gfloat       frame_timer;    /* Time accumulator */
    gfloat       fps;
    gboolean     looping;
    gboolean     playing;
    gboolean     finished;
};

G_DEFINE_TYPE (GrlAnimatedTexture, grl_animated_texture, G_TYPE_OBJECT)

enum
{
    PROP_0,
    PROP_TEXTURE,
    PROP_FRAME_WIDTH,
    PROP_FRAME_HEIGHT,
    PROP_FRAME_COUNT,
    PROP_CURRENT_FRAME,
    PROP_FPS,
    PROP_LOOPING,
    PROP_PLAYING,
    N_PROPS
};

static GParamSpec *properties[N_PROPS];

enum
{
    SIGNAL_FRAME_CHANGED,
    SIGNAL_ANIMATION_FINISHED,
    N_SIGNALS
};

static guint signals[N_SIGNALS];

/*
 * Private helpers
 */

static void
calculate_frame_layout (GrlAnimatedTexture *self)
{
    gint tex_width, tex_height;

    if (self->texture == NULL || self->frame_width <= 0 || self->frame_height <= 0)
    {
        self->cols = 0;
        self->rows = 0;
        self->max_frames = 0;
        self->frame_count = 0;
        return;
    }

    tex_width = grl_texture_get_width (self->texture);
    tex_height = grl_texture_get_height (self->texture);

    self->cols = tex_width / self->frame_width;
    self->rows = tex_height / self->frame_height;
    self->max_frames = self->cols * self->rows;

    /* Default to using all frames */
    if (self->frame_count <= 0 || self->frame_count > self->max_frames)
        self->frame_count = self->max_frames;
}

/*
 * GObject implementation
 */

static void
grl_animated_texture_finalize (GObject *object)
{
    GrlAnimatedTexture *self = GRL_ANIMATED_TEXTURE (object);

    g_clear_object (&self->texture);

    G_OBJECT_CLASS (grl_animated_texture_parent_class)->finalize (object);
}

static void
grl_animated_texture_get_property (GObject    *object,
                                   guint       prop_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{
    GrlAnimatedTexture *self = GRL_ANIMATED_TEXTURE (object);

    switch (prop_id)
    {
    case PROP_TEXTURE:
        g_value_set_object (value, self->texture);
        break;

    case PROP_FRAME_WIDTH:
        g_value_set_int (value, self->frame_width);
        break;

    case PROP_FRAME_HEIGHT:
        g_value_set_int (value, self->frame_height);
        break;

    case PROP_FRAME_COUNT:
        g_value_set_int (value, self->frame_count);
        break;

    case PROP_CURRENT_FRAME:
        g_value_set_int (value, self->current_frame);
        break;

    case PROP_FPS:
        g_value_set_float (value, self->fps);
        break;

    case PROP_LOOPING:
        g_value_set_boolean (value, self->looping);
        break;

    case PROP_PLAYING:
        g_value_set_boolean (value, self->playing);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
grl_animated_texture_set_property (GObject      *object,
                                   guint         prop_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
    GrlAnimatedTexture *self = GRL_ANIMATED_TEXTURE (object);

    switch (prop_id)
    {
    case PROP_TEXTURE:
        g_clear_object (&self->texture);
        self->texture = g_value_dup_object (value);
        calculate_frame_layout (self);
        break;

    case PROP_FRAME_WIDTH:
        self->frame_width = g_value_get_int (value);
        calculate_frame_layout (self);
        break;

    case PROP_FRAME_HEIGHT:
        self->frame_height = g_value_get_int (value);
        calculate_frame_layout (self);
        break;

    case PROP_FRAME_COUNT:
        grl_animated_texture_set_frame_count (self, g_value_get_int (value));
        break;

    case PROP_CURRENT_FRAME:
        grl_animated_texture_set_current_frame (self, g_value_get_int (value));
        break;

    case PROP_FPS:
        grl_animated_texture_set_fps (self, g_value_get_float (value));
        break;

    case PROP_LOOPING:
        grl_animated_texture_set_looping (self, g_value_get_boolean (value));
        break;

    case PROP_PLAYING:
        if (g_value_get_boolean (value))
            grl_animated_texture_play (self);
        else
            grl_animated_texture_pause (self);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
grl_animated_texture_class_init (GrlAnimatedTextureClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = grl_animated_texture_finalize;
    object_class->get_property = grl_animated_texture_get_property;
    object_class->set_property = grl_animated_texture_set_property;

    /**
     * GrlAnimatedTexture:texture:
     *
     * The spritesheet texture.
     */
    properties[PROP_TEXTURE] =
        g_param_spec_object ("texture",
                             "Texture",
                             "The spritesheet texture",
                             GRL_TYPE_TEXTURE,
                             G_PARAM_READWRITE |
                             G_PARAM_CONSTRUCT_ONLY |
                             G_PARAM_STATIC_STRINGS);

    /**
     * GrlAnimatedTexture:frame-width:
     *
     * The width of each frame in pixels.
     */
    properties[PROP_FRAME_WIDTH] =
        g_param_spec_int ("frame-width",
                          "Frame Width",
                          "Width of each frame",
                          1, G_MAXINT, 32,
                          G_PARAM_READWRITE |
                          G_PARAM_CONSTRUCT_ONLY |
                          G_PARAM_STATIC_STRINGS);

    /**
     * GrlAnimatedTexture:frame-height:
     *
     * The height of each frame in pixels.
     */
    properties[PROP_FRAME_HEIGHT] =
        g_param_spec_int ("frame-height",
                          "Frame Height",
                          "Height of each frame",
                          1, G_MAXINT, 32,
                          G_PARAM_READWRITE |
                          G_PARAM_CONSTRUCT_ONLY |
                          G_PARAM_STATIC_STRINGS);

    /**
     * GrlAnimatedTexture:frame-count:
     *
     * The number of frames to use in the animation.
     */
    properties[PROP_FRAME_COUNT] =
        g_param_spec_int ("frame-count",
                          "Frame Count",
                          "Number of frames in animation",
                          0, G_MAXINT, 0,
                          G_PARAM_READWRITE |
                          G_PARAM_EXPLICIT_NOTIFY |
                          G_PARAM_STATIC_STRINGS);

    /**
     * GrlAnimatedTexture:current-frame:
     *
     * The current frame index (0-based).
     */
    properties[PROP_CURRENT_FRAME] =
        g_param_spec_int ("current-frame",
                          "Current Frame",
                          "Current frame index",
                          0, G_MAXINT, 0,
                          G_PARAM_READWRITE |
                          G_PARAM_EXPLICIT_NOTIFY |
                          G_PARAM_STATIC_STRINGS);

    /**
     * GrlAnimatedTexture:fps:
     *
     * The animation speed in frames per second.
     */
    properties[PROP_FPS] =
        g_param_spec_float ("fps",
                            "FPS",
                            "Frames per second",
                            0.001f, 1000.0f, 10.0f,
                            G_PARAM_READWRITE |
                            G_PARAM_EXPLICIT_NOTIFY |
                            G_PARAM_STATIC_STRINGS);

    /**
     * GrlAnimatedTexture:looping:
     *
     * Whether the animation loops.
     */
    properties[PROP_LOOPING] =
        g_param_spec_boolean ("looping",
                              "Looping",
                              "Whether animation loops",
                              TRUE,
                              G_PARAM_READWRITE |
                              G_PARAM_EXPLICIT_NOTIFY |
                              G_PARAM_STATIC_STRINGS);

    /**
     * GrlAnimatedTexture:playing:
     *
     * Whether the animation is currently playing.
     */
    properties[PROP_PLAYING] =
        g_param_spec_boolean ("playing",
                              "Playing",
                              "Whether animation is playing",
                              FALSE,
                              G_PARAM_READWRITE |
                              G_PARAM_EXPLICIT_NOTIFY |
                              G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (object_class, N_PROPS, properties);

    /**
     * GrlAnimatedTexture::frame-changed:
     * @self: The #GrlAnimatedTexture
     * @frame: The new frame index
     *
     * Emitted when the current frame changes.
     */
    signals[SIGNAL_FRAME_CHANGED] =
        g_signal_new ("frame-changed",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0, NULL, NULL, NULL,
                      G_TYPE_NONE, 1, G_TYPE_INT);

    /**
     * GrlAnimatedTexture::animation-finished:
     * @self: The #GrlAnimatedTexture
     *
     * Emitted when a non-looping animation finishes.
     */
    signals[SIGNAL_ANIMATION_FINISHED] =
        g_signal_new ("animation-finished",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0, NULL, NULL, NULL,
                      G_TYPE_NONE, 0);
}

static void
grl_animated_texture_init (GrlAnimatedTexture *self)
{
    self->texture = NULL;
    self->frame_width = 32;
    self->frame_height = 32;
    self->cols = 0;
    self->rows = 0;
    self->frame_count = 0;
    self->max_frames = 0;
    self->current_frame = 0;
    self->frame_timer = 0.0f;
    self->fps = 10.0f;
    self->looping = TRUE;
    self->playing = FALSE;
    self->finished = FALSE;
}

/*
 * Public API
 */

/**
 * grl_animated_texture_new:
 * @texture: (transfer none): The spritesheet texture
 * @frame_width: Width of each frame
 * @frame_height: Height of each frame
 *
 * Creates a new animated texture from a spritesheet.
 * Frames are extracted left-to-right, top-to-bottom.
 *
 * Returns: (transfer full): A new #GrlAnimatedTexture
 */
GrlAnimatedTexture *
grl_animated_texture_new (GrlTexture *texture,
                          gint        frame_width,
                          gint        frame_height)
{
    g_return_val_if_fail (GRL_IS_TEXTURE (texture), NULL);
    g_return_val_if_fail (frame_width > 0, NULL);
    g_return_val_if_fail (frame_height > 0, NULL);

    return g_object_new (GRL_TYPE_ANIMATED_TEXTURE,
                         "texture", texture,
                         "frame-width", frame_width,
                         "frame-height", frame_height,
                         NULL);
}

/**
 * grl_animated_texture_new_from_file:
 * @filename: Path to the spritesheet file
 * @frame_width: Width of each frame
 * @frame_height: Height of each frame
 * @error: (nullable): Return location for error, or %NULL
 *
 * Creates a new animated texture by loading a spritesheet from file.
 *
 * Returns: (transfer full) (nullable): A new #GrlAnimatedTexture, or %NULL on error
 */
GrlAnimatedTexture *
grl_animated_texture_new_from_file (const gchar  *filename,
                                    gint          frame_width,
                                    gint          frame_height,
                                    GError      **error)
{
    g_autoptr(GrlTexture) texture = NULL;

    g_return_val_if_fail (filename != NULL, NULL);
    g_return_val_if_fail (frame_width > 0, NULL);
    g_return_val_if_fail (frame_height > 0, NULL);

    texture = grl_texture_new_from_file (filename);

    if (texture == NULL)
    {
        g_set_error (error,
                     G_IO_ERROR,
                     G_IO_ERROR_FAILED,
                     "Failed to load texture from '%s'",
                     filename);
        return NULL;
    }

    return grl_animated_texture_new (texture, frame_width, frame_height);
}

/**
 * grl_animated_texture_get_texture:
 * @self: A #GrlAnimatedTexture
 *
 * Gets the underlying spritesheet texture.
 *
 * Returns: (transfer none): The texture
 */
GrlTexture *
grl_animated_texture_get_texture (GrlAnimatedTexture *self)
{
    g_return_val_if_fail (GRL_IS_ANIMATED_TEXTURE (self), NULL);

    return self->texture;
}

/**
 * grl_animated_texture_get_frame_count:
 * @self: A #GrlAnimatedTexture
 *
 * Gets the total number of frames in the animation.
 *
 * Returns: The frame count
 */
gint
grl_animated_texture_get_frame_count (GrlAnimatedTexture *self)
{
    g_return_val_if_fail (GRL_IS_ANIMATED_TEXTURE (self), 0);

    return self->frame_count;
}

/**
 * grl_animated_texture_get_frame_width:
 * @self: A #GrlAnimatedTexture
 *
 * Gets the width of each frame.
 *
 * Returns: The frame width in pixels
 */
gint
grl_animated_texture_get_frame_width (GrlAnimatedTexture *self)
{
    g_return_val_if_fail (GRL_IS_ANIMATED_TEXTURE (self), 0);

    return self->frame_width;
}

/**
 * grl_animated_texture_get_frame_height:
 * @self: A #GrlAnimatedTexture
 *
 * Gets the height of each frame.
 *
 * Returns: The frame height in pixels
 */
gint
grl_animated_texture_get_frame_height (GrlAnimatedTexture *self)
{
    g_return_val_if_fail (GRL_IS_ANIMATED_TEXTURE (self), 0);

    return self->frame_height;
}

/**
 * grl_animated_texture_set_frame_count:
 * @self: A #GrlAnimatedTexture
 * @count: The number of frames to use
 *
 * Sets the number of frames to use. This is useful when the
 * spritesheet contains more frames than needed.
 */
void
grl_animated_texture_set_frame_count (GrlAnimatedTexture *self,
                                      gint                count)
{
    g_return_if_fail (GRL_IS_ANIMATED_TEXTURE (self));

    /* Clamp to valid range */
    if (count <= 0)
        count = self->max_frames;

    if (count > self->max_frames)
        count = self->max_frames;

    if (self->frame_count == count)
        return;

    self->frame_count = count;

    /* Clamp current frame if needed */
    if (self->current_frame >= self->frame_count)
        self->current_frame = self->frame_count - 1;

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_FRAME_COUNT]);
}

/**
 * grl_animated_texture_get_current_frame:
 * @self: A #GrlAnimatedTexture
 *
 * Gets the current frame index (0-based).
 *
 * Returns: The current frame index
 */
gint
grl_animated_texture_get_current_frame (GrlAnimatedTexture *self)
{
    g_return_val_if_fail (GRL_IS_ANIMATED_TEXTURE (self), 0);

    return self->current_frame;
}

/**
 * grl_animated_texture_set_current_frame:
 * @self: A #GrlAnimatedTexture
 * @frame: The frame index to set
 *
 * Sets the current frame index. The index is clamped to valid range.
 */
void
grl_animated_texture_set_current_frame (GrlAnimatedTexture *self,
                                        gint                frame)
{
    g_return_if_fail (GRL_IS_ANIMATED_TEXTURE (self));

    /* Clamp to valid range */
    if (frame < 0)
        frame = 0;

    if (frame >= self->frame_count)
        frame = self->frame_count - 1;

    if (frame < 0)
        frame = 0;

    if (self->current_frame == frame)
        return;

    self->current_frame = frame;
    self->frame_timer = 0.0f;

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_CURRENT_FRAME]);
    g_signal_emit (self, signals[SIGNAL_FRAME_CHANGED], 0, frame);
}

/**
 * grl_animated_texture_get_frame_rect:
 * @self: A #GrlAnimatedTexture
 * @frame: The frame index
 *
 * Gets the source rectangle for a specific frame.
 *
 * Returns: (transfer full): The frame's source rectangle
 */
GrlRectangle *
grl_animated_texture_get_frame_rect (GrlAnimatedTexture *self,
                                     gint                frame)
{
    gint row, col;
    gfloat x, y;

    g_return_val_if_fail (GRL_IS_ANIMATED_TEXTURE (self), NULL);

    if (self->cols <= 0 || frame < 0 || frame >= self->frame_count)
        return grl_rectangle_new_empty ();

    /* Calculate row and column */
    row = frame / self->cols;
    col = frame % self->cols;

    /* Calculate pixel position */
    x = (gfloat)(col * self->frame_width);
    y = (gfloat)(row * self->frame_height);

    return grl_rectangle_new (x, y,
                              (gfloat)self->frame_width,
                              (gfloat)self->frame_height);
}

/**
 * grl_animated_texture_get_current_rect:
 * @self: A #GrlAnimatedTexture
 *
 * Gets the source rectangle for the current frame.
 *
 * Returns: (transfer full): The current frame's source rectangle
 */
GrlRectangle *
grl_animated_texture_get_current_rect (GrlAnimatedTexture *self)
{
    g_return_val_if_fail (GRL_IS_ANIMATED_TEXTURE (self), NULL);

    return grl_animated_texture_get_frame_rect (self, self->current_frame);
}

/**
 * grl_animated_texture_get_fps:
 * @self: A #GrlAnimatedTexture
 *
 * Gets the animation speed in frames per second.
 *
 * Returns: The FPS
 */
gfloat
grl_animated_texture_get_fps (GrlAnimatedTexture *self)
{
    g_return_val_if_fail (GRL_IS_ANIMATED_TEXTURE (self), 10.0f);

    return self->fps;
}

/**
 * grl_animated_texture_set_fps:
 * @self: A #GrlAnimatedTexture
 * @fps: Frames per second
 *
 * Sets the animation speed in frames per second.
 */
void
grl_animated_texture_set_fps (GrlAnimatedTexture *self,
                              gfloat              fps)
{
    g_return_if_fail (GRL_IS_ANIMATED_TEXTURE (self));
    g_return_if_fail (fps > 0.0f);

    if (self->fps == fps)
        return;

    self->fps = fps;

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_FPS]);
}

/**
 * grl_animated_texture_get_looping:
 * @self: A #GrlAnimatedTexture
 *
 * Gets whether the animation loops.
 *
 * Returns: %TRUE if looping
 */
gboolean
grl_animated_texture_get_looping (GrlAnimatedTexture *self)
{
    g_return_val_if_fail (GRL_IS_ANIMATED_TEXTURE (self), TRUE);

    return self->looping;
}

/**
 * grl_animated_texture_set_looping:
 * @self: A #GrlAnimatedTexture
 * @looping: Whether to loop
 *
 * Sets whether the animation should loop.
 */
void
grl_animated_texture_set_looping (GrlAnimatedTexture *self,
                                  gboolean            looping)
{
    g_return_if_fail (GRL_IS_ANIMATED_TEXTURE (self));

    looping = !!looping;

    if (self->looping == looping)
        return;

    self->looping = looping;

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_LOOPING]);
}

/**
 * grl_animated_texture_get_playing:
 * @self: A #GrlAnimatedTexture
 *
 * Gets whether the animation is currently playing.
 *
 * Returns: %TRUE if playing
 */
gboolean
grl_animated_texture_get_playing (GrlAnimatedTexture *self)
{
    g_return_val_if_fail (GRL_IS_ANIMATED_TEXTURE (self), FALSE);

    return self->playing;
}

/**
 * grl_animated_texture_play:
 * @self: A #GrlAnimatedTexture
 *
 * Starts or resumes the animation.
 */
void
grl_animated_texture_play (GrlAnimatedTexture *self)
{
    g_return_if_fail (GRL_IS_ANIMATED_TEXTURE (self));

    if (self->playing)
        return;

    self->playing = TRUE;
    self->finished = FALSE;

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_PLAYING]);
}

/**
 * grl_animated_texture_pause:
 * @self: A #GrlAnimatedTexture
 *
 * Pauses the animation.
 */
void
grl_animated_texture_pause (GrlAnimatedTexture *self)
{
    g_return_if_fail (GRL_IS_ANIMATED_TEXTURE (self));

    if (!self->playing)
        return;

    self->playing = FALSE;

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_PLAYING]);
}

/**
 * grl_animated_texture_stop:
 * @self: A #GrlAnimatedTexture
 *
 * Stops the animation and resets to frame 0.
 */
void
grl_animated_texture_stop (GrlAnimatedTexture *self)
{
    g_return_if_fail (GRL_IS_ANIMATED_TEXTURE (self));

    self->playing = FALSE;
    self->finished = FALSE;
    self->current_frame = 0;
    self->frame_timer = 0.0f;

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_PLAYING]);
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_CURRENT_FRAME]);
}

/**
 * grl_animated_texture_update:
 * @self: A #GrlAnimatedTexture
 * @delta: Time delta in seconds
 *
 * Updates the animation. Call this each frame to advance
 * the animation based on elapsed time.
 */
void
grl_animated_texture_update (GrlAnimatedTexture *self,
                             gfloat              delta)
{
    gfloat frame_duration;
    gint old_frame;

    g_return_if_fail (GRL_IS_ANIMATED_TEXTURE (self));

    if (!self->playing || self->finished || self->frame_count <= 1)
        return;

    /* Calculate frame duration */
    frame_duration = 1.0f / self->fps;

    /* Accumulate time */
    self->frame_timer += delta;

    /* Check if we need to advance frames */
    old_frame = self->current_frame;

    while (self->frame_timer >= frame_duration)
    {
        self->frame_timer -= frame_duration;
        self->current_frame++;

        /* Handle end of animation */
        if (self->current_frame >= self->frame_count)
        {
            if (self->looping)
            {
                self->current_frame = 0;
            }
            else
            {
                self->current_frame = self->frame_count - 1;
                self->finished = TRUE;
                self->playing = FALSE;

                g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_PLAYING]);
                g_signal_emit (self, signals[SIGNAL_ANIMATION_FINISHED], 0);
                break;
            }
        }
    }

    /* Emit frame changed if frame changed */
    if (self->current_frame != old_frame)
    {
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_CURRENT_FRAME]);
        g_signal_emit (self, signals[SIGNAL_FRAME_CHANGED], 0, self->current_frame);
    }
}

/**
 * grl_animated_texture_is_finished:
 * @self: A #GrlAnimatedTexture
 *
 * Checks if a non-looping animation has finished.
 *
 * Returns: %TRUE if the animation has finished
 */
gboolean
grl_animated_texture_is_finished (GrlAnimatedTexture *self)
{
    g_return_val_if_fail (GRL_IS_ANIMATED_TEXTURE (self), FALSE);

    return self->finished;
}

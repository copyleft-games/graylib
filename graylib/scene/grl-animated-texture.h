/* grl-animated-texture.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Animated texture from spritesheet.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "../grl-version.h"
#include "../graphics/grl-texture.h"
#include "../math/grl-rectangle.h"

G_BEGIN_DECLS

#define GRL_TYPE_ANIMATED_TEXTURE (grl_animated_texture_get_type ())

GRL_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (GrlAnimatedTexture, grl_animated_texture, GRL, ANIMATED_TEXTURE, GObject)

/*
 * Constructors
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
GRL_AVAILABLE_IN_ALL
GrlAnimatedTexture * grl_animated_texture_new           (GrlTexture  *texture,
                                                         gint         frame_width,
                                                         gint         frame_height);

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
GRL_AVAILABLE_IN_ALL
GrlAnimatedTexture * grl_animated_texture_new_from_file (const gchar *filename,
                                                         gint         frame_width,
                                                         gint         frame_height,
                                                         GError     **error);

/*
 * Texture access
 */

/**
 * grl_animated_texture_get_texture:
 * @self: A #GrlAnimatedTexture
 *
 * Gets the underlying spritesheet texture.
 *
 * Returns: (transfer none): The texture
 */
GRL_AVAILABLE_IN_ALL
GrlTexture *    grl_animated_texture_get_texture        (GrlAnimatedTexture *self);

/*
 * Frame configuration
 */

/**
 * grl_animated_texture_get_frame_count:
 * @self: A #GrlAnimatedTexture
 *
 * Gets the total number of frames in the animation.
 *
 * Returns: The frame count
 */
GRL_AVAILABLE_IN_ALL
gint            grl_animated_texture_get_frame_count    (GrlAnimatedTexture *self);

/**
 * grl_animated_texture_get_frame_width:
 * @self: A #GrlAnimatedTexture
 *
 * Gets the width of each frame.
 *
 * Returns: The frame width in pixels
 */
GRL_AVAILABLE_IN_ALL
gint            grl_animated_texture_get_frame_width    (GrlAnimatedTexture *self);

/**
 * grl_animated_texture_get_frame_height:
 * @self: A #GrlAnimatedTexture
 *
 * Gets the height of each frame.
 *
 * Returns: The frame height in pixels
 */
GRL_AVAILABLE_IN_ALL
gint            grl_animated_texture_get_frame_height   (GrlAnimatedTexture *self);

/**
 * grl_animated_texture_set_frame_count:
 * @self: A #GrlAnimatedTexture
 * @count: The number of frames to use
 *
 * Sets the number of frames to use. This is useful when the
 * spritesheet contains more frames than needed or when
 * only a portion should be used.
 */
GRL_AVAILABLE_IN_ALL
void            grl_animated_texture_set_frame_count    (GrlAnimatedTexture *self,
                                                         gint                count);

/*
 * Current frame
 */

/**
 * grl_animated_texture_get_current_frame:
 * @self: A #GrlAnimatedTexture
 *
 * Gets the current frame index (0-based).
 *
 * Returns: The current frame index
 */
GRL_AVAILABLE_IN_ALL
gint            grl_animated_texture_get_current_frame  (GrlAnimatedTexture *self);

/**
 * grl_animated_texture_set_current_frame:
 * @self: A #GrlAnimatedTexture
 * @frame: The frame index to set
 *
 * Sets the current frame index. The index is clamped to valid range.
 */
GRL_AVAILABLE_IN_ALL
void            grl_animated_texture_set_current_frame  (GrlAnimatedTexture *self,
                                                         gint                frame);

/**
 * grl_animated_texture_get_frame_rect:
 * @self: A #GrlAnimatedTexture
 * @frame: The frame index
 *
 * Gets the source rectangle for a specific frame.
 *
 * Returns: (transfer full): The frame's source rectangle
 */
GRL_AVAILABLE_IN_ALL
GrlRectangle *  grl_animated_texture_get_frame_rect     (GrlAnimatedTexture *self,
                                                         gint                frame);

/**
 * grl_animated_texture_get_current_rect:
 * @self: A #GrlAnimatedTexture
 *
 * Gets the source rectangle for the current frame.
 *
 * Returns: (transfer full): The current frame's source rectangle
 */
GRL_AVAILABLE_IN_ALL
GrlRectangle *  grl_animated_texture_get_current_rect   (GrlAnimatedTexture *self);

/*
 * Animation control
 */

/**
 * grl_animated_texture_get_fps:
 * @self: A #GrlAnimatedTexture
 *
 * Gets the animation speed in frames per second.
 *
 * Returns: The FPS
 */
GRL_AVAILABLE_IN_ALL
gfloat          grl_animated_texture_get_fps            (GrlAnimatedTexture *self);

/**
 * grl_animated_texture_set_fps:
 * @self: A #GrlAnimatedTexture
 * @fps: Frames per second
 *
 * Sets the animation speed in frames per second.
 */
GRL_AVAILABLE_IN_ALL
void            grl_animated_texture_set_fps            (GrlAnimatedTexture *self,
                                                         gfloat              fps);

/**
 * grl_animated_texture_get_looping:
 * @self: A #GrlAnimatedTexture
 *
 * Gets whether the animation loops.
 *
 * Returns: %TRUE if looping
 */
GRL_AVAILABLE_IN_ALL
gboolean        grl_animated_texture_get_looping        (GrlAnimatedTexture *self);

/**
 * grl_animated_texture_set_looping:
 * @self: A #GrlAnimatedTexture
 * @looping: Whether to loop
 *
 * Sets whether the animation should loop.
 */
GRL_AVAILABLE_IN_ALL
void            grl_animated_texture_set_looping        (GrlAnimatedTexture *self,
                                                         gboolean            looping);

/**
 * grl_animated_texture_get_playing:
 * @self: A #GrlAnimatedTexture
 *
 * Gets whether the animation is currently playing.
 *
 * Returns: %TRUE if playing
 */
GRL_AVAILABLE_IN_ALL
gboolean        grl_animated_texture_get_playing        (GrlAnimatedTexture *self);

/**
 * grl_animated_texture_play:
 * @self: A #GrlAnimatedTexture
 *
 * Starts or resumes the animation.
 */
GRL_AVAILABLE_IN_ALL
void            grl_animated_texture_play               (GrlAnimatedTexture *self);

/**
 * grl_animated_texture_pause:
 * @self: A #GrlAnimatedTexture
 *
 * Pauses the animation.
 */
GRL_AVAILABLE_IN_ALL
void            grl_animated_texture_pause              (GrlAnimatedTexture *self);

/**
 * grl_animated_texture_stop:
 * @self: A #GrlAnimatedTexture
 *
 * Stops the animation and resets to frame 0.
 */
GRL_AVAILABLE_IN_ALL
void            grl_animated_texture_stop               (GrlAnimatedTexture *self);

/**
 * grl_animated_texture_update:
 * @self: A #GrlAnimatedTexture
 * @delta: Time delta in seconds
 *
 * Updates the animation. Call this each frame to advance
 * the animation based on elapsed time.
 */
GRL_AVAILABLE_IN_ALL
void            grl_animated_texture_update             (GrlAnimatedTexture *self,
                                                         gfloat              delta);

/**
 * grl_animated_texture_is_finished:
 * @self: A #GrlAnimatedTexture
 *
 * Checks if a non-looping animation has finished.
 *
 * Returns: %TRUE if the animation has finished
 */
GRL_AVAILABLE_IN_ALL
gboolean        grl_animated_texture_is_finished        (GrlAnimatedTexture *self);

G_END_DECLS

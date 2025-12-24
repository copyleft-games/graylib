/* graylib.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Main header file for Graylib - a GObject wrapper around raylib.
 *
 * Include this header to use Graylib in your application:
 *
 * |[<!-- language="C" -->
 * #include <graylib.h>
 *
 * int
 * main (int argc, char *argv[])
 * {
 *     g_autoptr(GrlApplication) app = NULL;
 *     g_autoptr(GrlWindow) window = NULL;
 *     g_autoptr(GrlColor) bg_color = NULL;
 *
 *     app = grl_application_new ();
 *     window = grl_window_new (800, 600, "My Game");
 *     bg_color = grl_color_new (40, 40, 60, 255);
 *
 *     grl_window_set_target_fps (window, 60);
 *
 *     while (!grl_window_should_close (window))
 *     {
 *         grl_window_begin_drawing (window);
 *         grl_draw_clear_background (bg_color);
 *         grl_window_end_drawing (window);
 *     }
 *
 *     return 0;
 * }
 * ]|
 */

#pragma once

#define GRAYLIB_INSIDE

/* Version information */
#include "grl-version.h"

/* Forward declarations */
#include "grl-types.h"

/* Enumerations */
#include "grl-enums.h"

/* Math types (GBoxed) */
#include "math/grl-vector2.h"
#include "math/grl-vector3.h"
#include "math/grl-vector4.h"
#include "math/grl-matrix.h"
#include "math/grl-bounding-box.h"
#include "math/grl-color.h"
#include "math/grl-rectangle.h"

/* Core module */
#include "core/grl-application.h"
#include "core/grl-window.h"
#include "core/grl-input.h"

/* Graphics module */
#include "graphics/grl-image.h"
#include "graphics/grl-texture.h"
#include "graphics/grl-font.h"
#include "graphics/grl-camera2d.h"
#include "graphics/grl-camera3d.h"
#include "graphics/grl-shader.h"
#include "graphics/grl-render-texture.h"
#include "graphics/grl-mesh.h"
#include "graphics/grl-material.h"
#include "graphics/grl-model.h"
#include "graphics/grl-model-animation.h"

/* Drawing module */
#include "drawing/grl-draw.h"

/* Audio module */
#include "audio/grl-audio-device.h"
#include "audio/grl-sound.h"
#include "audio/grl-music.h"

/* Scene module */
#include "scene/grl-drawable.h"
#include "scene/grl-updatable.h"
#include "scene/grl-collidable.h"
#include "scene/grl-entity.h"
#include "scene/grl-sprite.h"
#include "scene/grl-animated-texture.h"
#include "scene/grl-scene.h"
#include "scene/grl-scene-manager.h"

/* Collision module */
#include "collision/grl-collision.h"

/*
 * Future includes:
 *
 * #include "core/grl-timing.h"
 */

#undef GRAYLIB_INSIDE

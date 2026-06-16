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
#include "math/grl-quaternion.h"
#include "math/grl-bounding-box.h"
#include "math/grl-color.h"
#include "math/grl-rectangle.h"

/* Math utilities */
#include "math/grl-math-utils.h"

/* Core module */
#include "core/grl-application.h"
#include "core/grl-window.h"
#include "core/grl-input.h"

/* Graphics module */
#include "graphics/grl-image.h"
#include "graphics/grl-path.h"
#include "graphics/grl-svg.h"
#include "graphics/grl-gif-writer.h"
#include "graphics/grl-image-accumulator.h"
#include "graphics/grl-image-temporal.h"
#include "graphics/grl-image-font.h"
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
#include "graphics/grl-png.h"
#include "graphics/grl-bone-info.h"
#include "graphics/grl-transform.h"

/* Drawing module */
#include "drawing/grl-draw.h"

/* Audio module */
#include "audio/grl-audio-device.h"
#include "audio/grl-wave.h"
#include "audio/grl-sound.h"
#include "audio/grl-music.h"
#include "audio/grl-audio-stream.h"

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

/* UI module */
#include "ui/grl-ui-enums.h"
#include "ui/grl-ui-control.h"
#include "ui/grl-ui-style.h"
#include "ui/grl-ui-label.h"
#include "ui/grl-ui-button.h"
#include "ui/grl-ui-checkbox.h"
#include "ui/grl-ui-slider.h"
#include "ui/grl-ui-textbox.h"
#include "ui/grl-ui-toggle.h"
#include "ui/grl-ui-progressbar.h"
#include "ui/grl-ui-spinner.h"
#include "ui/grl-ui-valuebox.h"
#include "ui/grl-ui-combobox.h"
#include "ui/grl-ui-dropdownbox.h"
#include "ui/grl-ui-togglegroup.h"
#include "ui/grl-ui-listview.h"
#include "ui/grl-ui-colorpicker.h"
#include "ui/grl-ui-panel.h"
#include "ui/grl-ui-groupbox.h"
#include "ui/grl-ui-windowbox.h"

/* Resources module */
#include "resources/grl-resource-enums.h"
#include "resources/grl-resource-chunk-info.h"
#include "resources/grl-resource-pack.h"

/* RLGL module (low-level OpenGL abstraction) */
#include "rlgl/grl-rlgl.h"

/* Software renderer module (rlsw CPU rasterizer) */
#include "rlsw/grl-software-renderer.h"

/* System module (filesystem helpers) */
#include "system/grl-fs.h"

/* Text module (string utilities) */
#include "text/grl-text-utils.h"

/* Utility functions */
#include "grl-utils.h"

/*
 * Future includes:
 *
 * #include "core/grl-timing.h"
 */

#undef GRAYLIB_INSIDE

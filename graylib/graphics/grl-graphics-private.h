/* grl-graphics-private.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Private/internal header for graphics module.
 * Contains internal constructors and utilities shared between
 * graphics types but not exposed in the public API.
 */

#pragma once

#include <glib-object.h>
#include "grl-image.h"
#include "grl-texture.h"

G_BEGIN_DECLS

/*
 * Internal constructor for GrlImage from raylib Image handle.
 * Used when we need to create a GrlImage from raylib operations
 * that return an Image struct directly.
 *
 * The returned GrlImage takes ownership of the handle data.
 */
GrlImage *  grl_image_new_from_raw_handle   (gpointer handle);

G_END_DECLS

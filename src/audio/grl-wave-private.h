/* grl-wave-private.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Private header for GrlWave internals - for use within the audio module only.
 */

#pragma once

#include "grl-wave.h"
#include <raylib.h>

G_BEGIN_DECLS

/*
 * Internal accessor for GrlWave's raylib Wave structure.
 * This allows other audio module components to convert GrlWave to Sound.
 *
 * Returns a copy of the internal Wave, caller does NOT own the data pointer.
 */
Wave    _grl_wave_get_raylib_wave   (const GrlWave *self);

G_END_DECLS

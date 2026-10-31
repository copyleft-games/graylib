/* SPDX-License-Identifier: AGPL-3.0-or-later
 * Public SDK ownership adaptation, never an AudioBuffer layout dependency.
 */
#pragma once
#include <stdbool.h>
#include <glib.h>
#include <raylib.h>

#ifndef GRL_SDK_WAV_EXTRA_FREE
#define GRL_SDK_WAV_EXTRA_FREE (0)
#endif

/* The saved pointer is captured only by a constructor known to load WAV on
 * the fingerprinted SDK. A fixed/unknown SDK supplies NULL. Keeping this
 * adapter injectable lets hermetic tests prove exactly one deallocation. */
static inline void
grl_audio_unload_music_owned (Music music,
                              gpointer extra_context,
                              void (*unload_music) (Music),
                              void (*free_context) (void *))
{
	unload_music (music);
	if (extra_context != NULL)
		free_context (extra_context);
}

static inline gpointer
grl_audio_wav_extra_context (Music music,
                             gboolean known_wav,
                             gboolean source_capability)
{
	return known_wav && source_capability ? music.ctxData : NULL;
}

/* test-audio-stream.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Unit tests for GrlAudioStream.
 *
 * Note: Tests that require an audio device are skipped.
 * GrlAudioStream requires audio device initialization to create
 * instances, so most functional tests are not possible without
 * hardware. These tests focus on GType registration.
 */

#include <glib.h>
#include "src/audio/grl-audio-stream.h"

/*
 * Test GType registration
 */

static void
test_audio_stream_type (void)
{
    GType type = grl_audio_stream_get_type ();

    g_assert_true (type != G_TYPE_INVALID);
    g_assert_cmpstr (g_type_name (type), ==, "GrlAudioStream");
}

static void
test_audio_stream_type_is_object (void)
{
    GType type = grl_audio_stream_get_type ();

    g_assert_true (g_type_is_a (type, G_TYPE_OBJECT));
}

static void
test_audio_stream_type_is_final (void)
{
    GType type = grl_audio_stream_get_type ();

    g_assert_true (G_TYPE_IS_FINAL (type));
}

/*
 * Test registration
 *
 * Note: Constructor tests (grl_audio_stream_new) and playback tests
 * (play, stop, pause, resume, is_playing) require an initialized
 * audio device and are not tested here. Property tests (volume, pitch,
 * pan, sample_rate, sample_size, channels) also require a valid
 * GrlAudioStream instance.
 */

int
main (int   argc,
      char *argv[])
{
    g_test_init (&argc, &argv, NULL);

    /* Type registration */
    g_test_add_func ("/audio-stream/type", test_audio_stream_type);
    g_test_add_func ("/audio-stream/type-is-object", test_audio_stream_type_is_object);
    g_test_add_func ("/audio-stream/type-is-final", test_audio_stream_type_is_final);

    return g_test_run ();
}

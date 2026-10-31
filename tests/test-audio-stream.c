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
 * hardware.  The headless tests cover GType registration and the
 * callback bookkeeping on an unloaded object (g_object_new); the
 * device test (/audio-stream/callback-device) runs only with
 * GRL_TEST_AUDIO_DEVICE=1.
 */

#include <math.h>
#include <glib.h>
#include <raylib.h>
#include "src/audio/grl-audio-stream.h"
#include "src/audio/grl-audio-device.h"

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
 * Callback streaming
 */

/* counts its calls and frames (from the device thread: atomics) */
typedef struct
{
    gint      calls;
    gint      frames;
    gint      tag;
    gint     *destroyed_tag;   /* set to tag by the destroy notify */
    gfloat    phase;
} CallbackProbe;

static void
probe_fill (gpointer buffer,
            guint    n_frames,
            gpointer user_data)
{
    CallbackProbe *probe = user_data;
    gfloat        *out = buffer;
    guint          i;

    /* a quiet 440 Hz sine, 32-bit float stereo */
    for (i = 0; i < n_frames; i++)
    {
        gfloat v = 0.05f * sinf (probe->phase);

        out[2 * i] = v;
        out[2 * i + 1] = v;
        probe->phase += 2.0f * (gfloat)G_PI * 440.0f / 44100.0f;
    }
    g_atomic_int_inc (&probe->calls);
    (void)g_atomic_int_add (&probe->frames, (gint)n_frames);
}

static void
probe_destroy (gpointer user_data)
{
    CallbackProbe *probe = user_data;

    if (probe->destroyed_tag != NULL)
        *probe->destroyed_tag = probe->tag;
}

/* set, replace, clear and dispose on an unloaded object: the destroy
 * notifies run in order and the push API stays inert */
static void
test_audio_stream_callback_bookkeeping (void)
{
    GrlAudioStream *stream;
    CallbackProbe   first = { 0 };
    CallbackProbe   second = { 0 };
    gint            destroyed;
    gfloat          block[8] = { 0 };

    destroyed = 0;
    first.tag = 1;
    first.destroyed_tag = &destroyed;
    second.tag = 2;
    second.destroyed_tag = &destroyed;

    stream = g_object_new (GRL_TYPE_AUDIO_STREAM, NULL);
    g_assert_false (grl_audio_stream_has_callback (stream));

    grl_audio_stream_set_callback (stream, probe_fill, &first, probe_destroy);
    g_assert_true (grl_audio_stream_has_callback (stream));
    g_assert_cmpint (destroyed, ==, 0);

    /* pushing is a no-op with a callback, and nothing reads as processed */
    grl_audio_stream_update (stream, block, 4);
    g_assert_false (grl_audio_stream_is_processed (stream));

    /* replacing destroys the old data */
    grl_audio_stream_set_callback (stream, probe_fill, &second, probe_destroy);
    g_assert_cmpint (destroyed, ==, 1);
    g_assert_true (grl_audio_stream_has_callback (stream));

    /* clearing destroys the current data */
    grl_audio_stream_set_callback (stream, NULL, NULL, NULL);
    g_assert_cmpint (destroyed, ==, 2);
    g_assert_false (grl_audio_stream_has_callback (stream));

    /* dispose drops a callback too */
    destroyed = 0;
    grl_audio_stream_set_callback (stream, probe_fill, &first, probe_destroy);
    g_object_unref (stream);
    g_assert_cmpint (destroyed, ==, 1);

    /* an unloaded object never ran the callback */
    g_assert_cmpint (first.calls, ==, 0);
    g_assert_cmpint (second.calls, ==, 0);
}

/* slots go back: far more set/clear cycles than there are slots */
static void
test_audio_stream_callback_slots (void)
{
    GrlAudioStream *streams[GRL_AUDIO_STREAM_MAX_CALLBACKS];
    CallbackProbe   probe = { 0 };
    guint           round;
    guint           i;

    for (round = 0; round < 4; round++)
    {
        for (i = 0; i < GRL_AUDIO_STREAM_MAX_CALLBACKS; i++)
        {
            streams[i] = g_object_new (GRL_TYPE_AUDIO_STREAM, NULL);
            grl_audio_stream_set_callback (streams[i], probe_fill, &probe, NULL);
            g_assert_true (grl_audio_stream_has_callback (streams[i]));
        }
        for (i = 0; i < GRL_AUDIO_STREAM_MAX_CALLBACKS; i++)
            g_object_unref (streams[i]);
    }
}

/* every slot taken: a warning, the data destroyed at once, no callback */
static void
test_audio_stream_callback_exhausted (void)
{
    if (g_test_subprocess ())
    {
        GrlAudioStream *streams[GRL_AUDIO_STREAM_MAX_CALLBACKS];
        GrlAudioStream *extra;
        CallbackProbe   probe = { 0 };
        gint            destroyed = 0;
        guint           i;

        probe.tag = 7;
        probe.destroyed_tag = &destroyed;
        for (i = 0; i < GRL_AUDIO_STREAM_MAX_CALLBACKS; i++)
        {
            streams[i] = g_object_new (GRL_TYPE_AUDIO_STREAM, NULL);
            grl_audio_stream_set_callback (streams[i], probe_fill, &probe, NULL);
        }
        extra = g_object_new (GRL_TYPE_AUDIO_STREAM, NULL);
        g_log_set_always_fatal (G_LOG_FATAL_MASK);
        grl_audio_stream_set_callback (extra, probe_fill, &probe, probe_destroy);
        g_assert_false (grl_audio_stream_has_callback (extra));
        g_assert_cmpint (destroyed, ==, 7);
        g_object_unref (extra);
        for (i = 0; i < GRL_AUDIO_STREAM_MAX_CALLBACKS; i++)
            g_object_unref (streams[i]);
        return;
    }
    g_test_trap_subprocess (NULL, 0, G_TEST_SUBPROCESS_DEFAULT);
    g_test_trap_assert_passed ();
    g_test_trap_assert_stderr ("*callback slots are taken*");
}

/*
 * The device pulls the callback on its own thread while the stream
 * plays; once the callback is removed it never runs again.
 */
static void
test_audio_stream_callback_device (void)
{
    GrlAudioDevice *device;
    GrlAudioStream *stream;
    CallbackProbe   probe = { 0 };
    gint            destroyed = 0;
    gint            frames_after;
    gint64          deadline;

    if (g_strcmp0 (g_getenv ("GRL_TEST_AUDIO_DEVICE"), "1") != 0)
    {
        g_test_skip ("set GRL_TEST_AUDIO_DEVICE=1 to test with an audio device");
        return;
    }
    SetTraceLogLevel (LOG_WARNING);
    device = grl_audio_device_get_default ();
    grl_audio_device_init_audio (device);
    if (!grl_audio_device_is_ready (device))
    {
        g_test_skip ("no audio device");
        return;
    }
    grl_audio_device_set_master_volume (device, 0.0f);

    stream = grl_audio_stream_new (44100, 32, 2);
    g_assert_nonnull (stream);
    probe.tag = 3;
    probe.destroyed_tag = &destroyed;
    grl_audio_stream_set_callback (stream, probe_fill, &probe, probe_destroy);
    g_assert_false (grl_audio_stream_is_processed (stream));

    /* not called before play */
    g_usleep (50000);
    g_assert_cmpint (g_atomic_int_get (&probe.calls), ==, 0);

    grl_audio_stream_play (stream);
    deadline = g_get_monotonic_time () + 3 * G_USEC_PER_SEC;
    while (g_atomic_int_get (&probe.frames) < 44100 / 4 && g_get_monotonic_time () < deadline)
        g_usleep (10000);
    g_assert_cmpint (g_atomic_int_get (&probe.frames), >=, 44100 / 4);

    /* removed: destroy ran, and no call follows */
    grl_audio_stream_set_callback (stream, NULL, NULL, NULL);
    g_assert_cmpint (destroyed, ==, 3);
    frames_after = g_atomic_int_get (&probe.frames);
    g_usleep (200000);
    g_assert_cmpint (g_atomic_int_get (&probe.frames), ==, frames_after);

    grl_audio_stream_stop (stream);
    g_object_unref (stream);
    grl_audio_device_close (device);
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

    /* Callback streaming */
    g_test_add_func ("/audio-stream/callback-bookkeeping", test_audio_stream_callback_bookkeeping);
    g_test_add_func ("/audio-stream/callback-slots", test_audio_stream_callback_slots);
    g_test_add_func ("/audio-stream/callback-exhausted", test_audio_stream_callback_exhausted);
    g_test_add_func ("/audio-stream/callback-device", test_audio_stream_callback_device);

    return g_test_run ();
}

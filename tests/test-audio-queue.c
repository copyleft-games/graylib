/* test-audio-queue.c
 *
 * Copyright 2026 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Tests for graylib's audio command thread.  With an audio server (a
 * PipeWire or PulseAudio socket in XDG_RUNTIME_DIR, or
 * GRL_TEST_AUDIO_DEVICE=1; =0 skips):
 *
 * - /audio-queue/lock-hammer: a stream callback holds raylib's audio
 *   lock for LOCK_HOLD_US of every device period while the test thread
 *   hammers sound calls; a direct raylib call waits for the lock,
 *   graylib's must not, and is_playing() must follow play / stop / pause
 *   / resume at once and see a sound's natural end.
 * - /audio-queue/release-order: objects dropped with calls still queued,
 *   then the device closed.
 *
 * Also built with ThreadSanitizer: make -C tests tsan (see tests/Makefile).
 */

#define _GNU_SOURCE   /* RUSAGE_THREAD */
#include <math.h>
#include <string.h>
#include <time.h>
#ifdef __linux__
#include <sys/resource.h>
#endif
#include <glib.h>
#include <gio/gio.h>
#include <raylib.h>
#include "src/audio/grl-audio-device.h"
#include "src/audio/grl-audio-stream.h"
#include "src/audio/grl-sound.h"
#include "src/audio/grl-wave.h"

/* ThreadSanitizer's runtime takes its own locks inside intercepted calls:
 * under it only the race checks count, not the latencies */
#if defined(__SANITIZE_THREAD__)
#define UNDER_TSAN (1)
#elif defined(__has_feature)
#if __has_feature(thread_sanitizer)
#define UNDER_TSAN (1)
#endif
#endif
#ifndef UNDER_TSAN
#define UNDER_TSAN (0)
#endif

#define TONE_RATE      (44100)
#define LOCK_HOLD_US   (6000)
#define HAMMER_ROUNDS  (1500)
#define N_SOUNDS       (8)

/* ---- helpers ------------------------------------------------------------ */

static gint64
now_ns (void)
{
    struct timespec ts;

    clock_gettime (CLOCK_MONOTONIC, &ts);
    return (gint64)ts.tv_sec * G_GINT64_CONSTANT (1000000000) + ts.tv_nsec;
}

static int
compare_gint64 (const void *a,
                const void *b)
{
    gint64 da = *(const gint64 *)a;
    gint64 db = *(const gint64 *)b;

    return (da > db) - (da < db);
}

/* whether an audio server looks reachable (or the caller insists) */
static gboolean
audio_server_expected (void)
{
    const gchar      *forced;
    const gchar      *runtime;
    g_autofree gchar *pipewire = NULL;
    g_autofree gchar *pulse = NULL;

    forced = g_getenv ("GRL_TEST_AUDIO_DEVICE");
    if (g_strcmp0 (forced, "1") == 0)
        return TRUE;
    if (g_strcmp0 (forced, "0") == 0)
        return FALSE;
    runtime = g_getenv ("PIPEWIRE_RUNTIME_DIR");
    if (runtime == NULL)
        runtime = g_getenv ("XDG_RUNTIME_DIR");
    if (runtime == NULL)
        return FALSE;
    pipewire = g_build_filename (runtime, "pipewire-0", NULL);
    pulse = g_build_filename (runtime, "pulse", "native", NULL);
    return g_file_test (pipewire, G_FILE_TEST_EXISTS) || g_file_test (pulse, G_FILE_TEST_EXISTS);
}

/* opens the default device, or skips the test; FALSE when skipped */
static gboolean
open_device_or_skip (GrlAudioDevice **out)
{
    GrlAudioDevice *device;

    if (!audio_server_expected ())
    {
        g_test_skip ("no audio server (set GRL_TEST_AUDIO_DEVICE=1 to force)");
        return FALSE;
    }
    SetTraceLogLevel (LOG_WARNING);
    device = grl_audio_device_get_default ();
    grl_audio_device_init_audio (device);
    if (!grl_audio_device_is_ready (device))
    {
        g_test_skip ("the audio device did not open");
        return FALSE;
    }
    grl_audio_device_set_master_volume (device, 0.0f);
    *out = device;
    return TRUE;
}

/* a quiet 16-bit mono tone of @seconds */
static GrlSound *
make_tone (gdouble seconds,
           gdouble hz)
{
    g_autoptr(GrlWave) wave = NULL;
    g_autofree gint16 *pcm = NULL;
    guint              frames;
    guint              i;

    frames = (guint)(TONE_RATE * seconds);
    pcm = g_new (gint16, frames);
    for (i = 0; i < frames; i++)
        pcm[i] = (gint16)(2000.0 * sin (2.0 * G_PI * hz * i / TONE_RATE));
    wave = grl_wave_new_from_samples (TONE_RATE, 16, 1, (const guint8 *)pcm, frames * sizeof (gint16));
    g_assert_nonnull (wave);
    return grl_sound_new_from_grl_wave (wave);
}

/* a stream callback that sits on raylib's audio lock (the mixer holds it
 * while it runs callbacks), the worst case of a busy synth */
typedef struct
{
    gint calls;   /* atomic */
} LockHog;

static void
lock_hog_fill (gpointer buffer,
               guint    frames,
               gpointer user_data)
{
    LockHog *hog = user_data;

    memset (buffer, 0, (gsize)frames * 2 * sizeof (gfloat));
    g_usleep (LOCK_HOLD_US);
    g_atomic_int_inc (&hog->calls);
}

/* ---- with a device ------------------------------------------------------ */

/*
 * Context switches of the calling thread: voluntary ones mean it waited
 * (a futex on a lock), involuntary ones that the scheduler preempted it
 * (a loaded machine; not graylib's doing).  Linux only; elsewhere both
 * read 0 and only the latencies are checked.
 */
static void
thread_switches (glong *voluntary,
                 glong *involuntary)
{
#ifdef RUSAGE_THREAD
    struct rusage usage;

    getrusage (RUSAGE_THREAD, &usage);
    *voluntary = usage.ru_nvcsw;
    *involuntary = usage.ru_nivcsw;
#else
    *voluntary = 0;
    *involuntary = 0;
#endif
}

/* times one call into lat; tallies the calls that blocked or were preempted */
#define TIMED(call) \
    G_STMT_START { \
        glong  vol0_, inv0_, vol1_, inv1_; \
        gint64 t0_, d_; \
        thread_switches (&vol0_, &inv0_); \
        t0_ = now_ns (); \
        call; \
        d_ = now_ns () - t0_; \
        thread_switches (&vol1_, &inv1_); \
        g_array_append_val (lat, d_); \
        if (vol1_ != vol0_) \
            blocked++; \
        else if (inv1_ != inv0_) \
            preempted++; \
        else if (d_ > clean_max) \
            clean_max = d_; \
    } G_STMT_END

/*
 * Calls on the test thread while the mixer sits on raylib's lock.  The
 * direct raylib call shows the stall (it must wait for the hog); the
 * graylib calls go through the queue and must not.
 */
static void
test_lock_hammer (void)
{
    GrlAudioDevice *device;
    GrlAudioStream *hog_stream;
    GrlSound       *sounds[N_SOUNDS];
    GrlSound       *probe;
    LockHog         hog = { 0 };
    GArray         *lat;
    gint64          direct_max;
    gint64          t0;
    gint64          deadline;
    gint64         *v;
    gint64          clean_max = 0;
    guint           blocked = 0;
    guint           preempted = 0;
    guint           round;
    guint           i;
    guint           n;

    if (!open_device_or_skip (&device))
        return;

    for (i = 0; i < N_SOUNDS; i++)
    {
        sounds[i] = make_tone (0.2, 300.0 + 40.0 * i);
        g_assert_nonnull (sounds[i]);
    }
    probe = make_tone (0.05, 880.0);

    /* the hog: from now on the lock is held most of the time */
    hog_stream = grl_audio_stream_new (TONE_RATE, 32, 2);
    g_assert_nonnull (hog_stream);
    grl_audio_stream_set_callback (hog_stream, lock_hog_fill, &hog, NULL);
    grl_audio_stream_play (hog_stream);
    g_assert_true (grl_audio_stream_is_playing (hog_stream));
    deadline = g_get_monotonic_time () + 3 * G_USEC_PER_SEC;
    while (g_atomic_int_get (&hog.calls) < 3 && g_get_monotonic_time () < deadline)
        g_usleep (1000);
    g_assert_cmpint (g_atomic_int_get (&hog.calls), >=, 3);

    /* 1. a direct raylib call waits for the lock: the stall exists */
    {
        Wave   wave;
        gint16 silence[256];
        Sound  raw;

        memset (silence, 0, sizeof (silence));
        wave.frameCount = G_N_ELEMENTS (silence);
        wave.sampleRate = TONE_RATE;
        wave.sampleSize = 16;
        wave.channels = 1;
        wave.data = silence;
        raw = LoadSoundFromWave (wave);
        direct_max = 0;
        for (round = 0; round < 40; round++)
        {
            t0 = now_ns ();
            SetSoundVolume (raw, 0.5f);
            direct_max = MAX (direct_max, now_ns () - t0);
            g_usleep (500);
        }
        UnloadSound (raw);
    }
    g_test_message ("direct raylib calls under the hog: max %.2f ms", direct_max / 1e6);

    /* 2. graylib's calls: never a wait on the lock */
    lat = g_array_sized_new (FALSE, FALSE, sizeof (gint64), HAMMER_ROUNDS * N_SOUNDS * 6);
    for (round = 0; round < HAMMER_ROUNDS; round++)
    {
        for (i = 0; i < N_SOUNDS; i++)
        {
            gboolean playing = FALSE;

            TIMED (grl_sound_set_volume (sounds[i], (round % 10) / 10.0f));
            TIMED (grl_sound_set_pitch (sounds[i], 0.8f + (round % 5) / 10.0f));
            TIMED (grl_sound_set_pan (sounds[i], ((round + i) % 3) - 1.0f));
            TIMED (grl_sound_play (sounds[i]));
            /* coherent at once, whatever the thread has applied */
            TIMED (playing = grl_sound_is_playing (sounds[i]));
            g_assert_true (playing);

            if (round % 7 == 0)
            {
                TIMED (grl_sound_stop (sounds[i]));
                g_assert_false (grl_sound_is_playing (sounds[i]));
            }
        }
        if (round % 50 == 0)
            g_usleep (1000);
    }
    n = lat->len;
    v = (gint64 *)lat->data;
    qsort (v, n, sizeof (gint64), compare_gint64);
    g_test_message ("%u graylib calls under the hog: median %.2f us, p99 %.2f us, p99.9 %.2f us, max %.2f us; "
                    "%u blocked, %u preempted, max of the rest %.2f us",
                    n, v[n / 2] / 1e3, v[(n * 99) / 100] / 1e3, v[(n * 999) / 1000] / 1e3, v[n - 1] / 1e3,
                    blocked, preempted, clean_max / 1e3);

    /* the hog really held the lock: a direct call waited for it */
    g_assert_cmpint (direct_max, >, 1000000);
    /* graylib's calls never block: no call gave the CPU up (a futex wait
     * on raylib's lock, or any other lock, is a voluntary switch), and
     * the ones the scheduler did not preempt are far under a lock wait */
    if (!UNDER_TSAN)
    {
        g_assert_cmpuint (blocked, ==, 0);
        g_assert_cmpint (clean_max, <, 1000000);
        g_assert_cmpint (v[(n * 99) / 100], <, 200000);
    }
    g_array_unref (lat);

    /* 3. pause / resume / stop are reported at once */
    grl_sound_play (probe);
    g_assert_true (grl_sound_is_playing (probe));
    grl_sound_pause (probe);
    g_assert_false (grl_sound_is_playing (probe));
    grl_sound_resume (probe);
    g_assert_true (grl_sound_is_playing (probe));
    grl_sound_stop (probe);
    g_assert_false (grl_sound_is_playing (probe));
    /* resuming a stopped sound does not make it play */
    grl_sound_pause (probe);
    grl_sound_resume (probe);
    g_assert_false (grl_sound_is_playing (probe));

    /* 4. a 50 ms sound's natural end shows up through the thread */
    grl_sound_play (probe);
    g_assert_true (grl_sound_is_playing (probe));
    deadline = g_get_monotonic_time () + 3 * G_USEC_PER_SEC;
    while (grl_sound_is_playing (probe) && g_get_monotonic_time () < deadline)
        g_usleep (2000);
    g_assert_false (grl_sound_is_playing (probe));

    /* 5. the hog's own mirror and its removal */
    grl_audio_stream_pause (hog_stream);
    g_assert_false (grl_audio_stream_is_playing (hog_stream));
    grl_audio_stream_resume (hog_stream);
    g_assert_true (grl_audio_stream_is_playing (hog_stream));
    grl_audio_stream_set_callback (hog_stream, NULL, NULL, NULL);
    grl_audio_stream_stop (hog_stream);
    g_assert_false (grl_audio_stream_is_playing (hog_stream));

    g_object_unref (hog_stream);
    for (i = 0; i < N_SOUNDS; i++)
        g_object_unref (sounds[i]);
    g_object_unref (probe);
    grl_audio_device_close (device);
}

/* objects dropped with their calls still queued, then the device closed */
static void
test_release_order (void)
{
    GrlAudioDevice *device;
    GrlSound       *source;
    GrlSound       *alias;
    guint           i;

    if (!open_device_or_skip (&device))
        return;

    for (i = 0; i < 50; i++)
    {
        source = make_tone (0.1, 500.0);
        alias = grl_sound_new_alias (source);
        g_assert_nonnull (alias);
        grl_sound_play (alias);
        grl_sound_play (source);
        grl_sound_set_pitch (alias, 1.5f);
        /* the alias unloads before its source, in queue order */
        g_object_unref (alias);
        g_object_unref (source);
    }
    /* dropped playing, its unload still queued when the device closes:
     * the close applies it first (raylib's lock lives in the device) */
    source = make_tone (0.5, 500.0);
    grl_sound_play (source);
    g_object_unref (source);
    grl_audio_device_close (device);
    g_assert_false (grl_audio_device_is_ready (device));
}

/* a do-nothing per-voice processor */
static void
noop_process (gfloat   *frames,
              guint     n_frames,
              guint     channels,
              gpointer  user_data)
{
}

/*
 * upstream's ownership rules through the queue: flattened alias roots,
 * checked writes validated at once and queued, independent voices from
 * the retained snapshot, a playing voice with a processor disposed.
 */
static void
test_owned_voices (void)
{
    GrlAudioDevice    *device;
    GrlSound          *source;
    GrlSound          *alias;
    GrlSound          *alias2;
    GrlSound          *voice;
    g_autoptr(GError)  error = NULL;
    g_autofree guint8 *bytes = NULL;
    gsize              frame_bytes;

    if (!open_device_or_skip (&device))
        return;

    source = make_tone (0.2, 440.0);
    alias = grl_sound_new_alias (source);
    alias2 = grl_sound_new_alias (alias);
    g_assert_nonnull (alias2);
    /* the caller drops the root and the first alias; alias2 keeps the root */
    g_object_unref (source);
    g_object_unref (alias);
    grl_sound_play (alias2);
    g_assert_true (grl_sound_is_playing (alias2));

    /* a wrong size fails at once, nothing queued */
    frame_bytes = 2 * sizeof (gfloat);   /* raylib's playback format: f32 stereo */
    bytes = g_malloc0 (frame_bytes * 64);
    g_assert_false (grl_sound_update_checked (alias2, bytes, frame_bytes * 64 - 1, 64, &error));
    g_assert_error (error, G_IO_ERROR, G_IO_ERROR_INVALID_ARGUMENT);
    g_clear_error (&error);
    g_assert_true (grl_sound_is_playing (alias2));

    /* a valid write is queued; raylib's update stops the voice, reported at once */
    g_assert_true (grl_sound_update_checked (alias2, bytes, frame_bytes * 64, 64, &error));
    g_assert_no_error (error);
    g_assert_false (grl_sound_is_playing (alias2));

    /* an independent voice from the (updated) root snapshot */
    voice = grl_sound_new_voice (alias2, &error);
    g_assert_no_error (error);
    g_assert_nonnull (voice);
    g_assert_false (grl_sound_is_alias (voice));
    g_object_unref (alias2);   /* the root goes now; the voice is independent */
    g_assert_cmpuint (grl_sound_add_processor (voice, noop_process, NULL, NULL), >, 0);
    grl_sound_play (voice);
    g_assert_true (grl_sound_is_playing (voice));
    /* disposed playing, with a processor: stopped (through the queue), then detached */
    g_object_unref (voice);

    grl_audio_device_close (device);
}

int
main (int   argc,
      char *argv[])
{
    g_test_init (&argc, &argv, NULL);

    g_test_add_func ("/audio-queue/lock-hammer", test_lock_hammer);
    g_test_add_func ("/audio-queue/release-order", test_release_order);
    g_test_add_func ("/audio-queue/owned-voices", test_owned_voices);

    return g_test_run ();
}

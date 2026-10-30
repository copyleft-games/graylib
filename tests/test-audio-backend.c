/* test-audio-backend.c
 *
 * Copyright 2026 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Tests for the backend GrlAudioDevice opens the device through.
 *
 * Headless: backend names, the preference and its environment override,
 * the 'none' backend (no device, every call inert).  With an audio server
 * (a PipeWire or PulseAudio socket in XDG_RUNTIME_DIR, or
 * GRL_TEST_AUDIO_DEVICE=1; =0 skips), /audio-backend/init-stress opens
 * and closes the device again and again through the default order (ALSA
 * first), playing between: miniaudio's PulseAudio backend crashed there
 * now and then.
 */

#include <math.h>
#include <string.h>
#include <glib.h>
#include <raylib.h>
#include "src/grl-enums.h"
#include "src/audio/grl-audio-device.h"
#include "src/audio/grl-sound.h"
#include "src/audio/grl-wave.h"

#define TONE_RATE      (44100)
#define STRESS_CYCLES  (12)

/* ---- helpers ------------------------------------------------------------ */

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

/* ---- headless ----------------------------------------------------------- */

static void
test_backend_parse (void)
{
    GrlAudioBackend backend;

    g_assert_true (grl_audio_backend_parse ("auto", &backend));
    g_assert_cmpint (backend, ==, GRL_AUDIO_BACKEND_AUTO);
    g_assert_true (grl_audio_backend_parse ("ALSA", &backend));
    g_assert_cmpint (backend, ==, GRL_AUDIO_BACKEND_ALSA);
    g_assert_true (grl_audio_backend_parse ("pipewire", &backend));
    g_assert_cmpint (backend, ==, GRL_AUDIO_BACKEND_ALSA);
    g_assert_true (grl_audio_backend_parse ("pulse", &backend));
    g_assert_cmpint (backend, ==, GRL_AUDIO_BACKEND_PULSEAUDIO);
    g_assert_true (grl_audio_backend_parse ("PulseAudio", &backend));
    g_assert_cmpint (backend, ==, GRL_AUDIO_BACKEND_PULSEAUDIO);
    g_assert_true (grl_audio_backend_parse ("off", &backend));
    g_assert_cmpint (backend, ==, GRL_AUDIO_BACKEND_NONE);
    g_assert_true (grl_audio_backend_parse ("0", &backend));
    g_assert_cmpint (backend, ==, GRL_AUDIO_BACKEND_NONE);
    g_assert_false (grl_audio_backend_parse ("jack", &backend));
    g_assert_false (grl_audio_backend_parse ("", &backend));
    g_assert_false (grl_audio_backend_parse (NULL, &backend));

    g_assert_true (G_TYPE_IS_ENUM (GRL_TYPE_AUDIO_BACKEND));
}

/* the preference, and the environment winning over it */
static void
test_backend_preference (void)
{
    g_autofree gchar *saved = NULL;

    saved = g_strdup (g_getenv ("GRL_AUDIO_BACKEND"));
    g_unsetenv ("GRL_AUDIO_BACKEND");

    g_assert_cmpint (grl_audio_device_get_preferred_backend (), ==, GRL_AUDIO_BACKEND_AUTO);
    grl_audio_device_set_preferred_backend (GRL_AUDIO_BACKEND_PULSEAUDIO);
    g_assert_cmpint (grl_audio_device_get_preferred_backend (), ==, GRL_AUDIO_BACKEND_PULSEAUDIO);

    g_setenv ("GRL_AUDIO_BACKEND", "alsa", TRUE);
    g_assert_cmpint (grl_audio_device_get_preferred_backend (), ==, GRL_AUDIO_BACKEND_ALSA);
    g_setenv ("GRL_AUDIO_BACKEND", "none", TRUE);
    g_assert_cmpint (grl_audio_device_get_preferred_backend (), ==, GRL_AUDIO_BACKEND_NONE);
    /* an unknown name falls back to auto (with a message) */
    g_setenv ("GRL_AUDIO_BACKEND", "bogus", TRUE);
    g_assert_cmpint (grl_audio_device_get_preferred_backend (), ==, GRL_AUDIO_BACKEND_AUTO);

    g_unsetenv ("GRL_AUDIO_BACKEND");
    grl_audio_device_set_preferred_backend (GRL_AUDIO_BACKEND_AUTO);
    if (saved != NULL)
        g_setenv ("GRL_AUDIO_BACKEND", saved, TRUE);
}

/*
 * The 'none' backend: no device opens, the game keeps going and every
 * call on an object that could not load is inert.
 */
static void
test_backend_none (void)
{
    GrlAudioDevice *device;
    GrlSound       *sound;

    grl_audio_device_set_preferred_backend (GRL_AUDIO_BACKEND_NONE);
    device = grl_audio_device_get_default ();
    g_assert_nonnull (device);
    g_assert_false (grl_audio_device_init_audio (device));
    g_assert_false (grl_audio_device_is_ready (device));

    /* an unloaded sound: everything inert, nothing queued */
    sound = g_object_new (GRL_TYPE_SOUND, NULL);
    grl_sound_play (sound);
    grl_sound_set_volume (sound, 0.3f);
    grl_sound_pause (sound);
    grl_sound_resume (sound);
    g_assert_false (grl_sound_is_playing (sound));
    grl_sound_stop (sound);
    g_object_unref (sound);

    grl_audio_device_close (device);
    grl_audio_device_set_preferred_backend (GRL_AUDIO_BACKEND_AUTO);
}

/* ---- with a device ------------------------------------------------------ */

/*
 * Repeated open / close through the default backend order (ALSA first).
 * miniaudio's PulseAudio backend crashed here now and then; ALSA must
 * open every time.
 */
static void
test_init_stress (void)
{
    GrlAudioDevice *device;
    guint           cycle;

    if (!open_device_or_skip (&device))
        return;
    grl_audio_device_close (device);

    for (cycle = 0; cycle < STRESS_CYCLES; cycle++)
    {
        GrlSound *sound;

        g_assert_true (grl_audio_device_init_audio (device));
        g_assert_true (grl_audio_device_is_ready (device));
        grl_audio_device_set_master_volume (device, 0.0f);

        sound = make_tone (0.05, 440.0);
        grl_sound_play (sound);
        g_assert_true (grl_sound_is_playing (sound));
        grl_sound_set_volume (sound, 0.5f);
        g_object_unref (sound);

        grl_audio_device_close (device);
        g_assert_false (grl_audio_device_is_ready (device));
    }
}

int
main (int   argc,
      char *argv[])
{
    g_test_init (&argc, &argv, NULL);

    g_test_add_func ("/audio-backend/parse", test_backend_parse);
    g_test_add_func ("/audio-backend/preference", test_backend_preference);
    g_test_add_func ("/audio-backend/none", test_backend_none);
    g_test_add_func ("/audio-backend/init-stress", test_init_stress);

    return g_test_run ();
}

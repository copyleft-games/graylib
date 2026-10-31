/* test-music.c
 *
 * Copyright 2026 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Unit tests for GrlMusic's background streaming.
 *
 * Headless: the switch, its property and the inert calls on an unloaded
 * object (g_object_new).  With GRL_TEST_AUDIO_DEVICE=1 a short generated
 * WAV plays with the streaming thread refilling it (grl_music_update()
 * is never called): position, pause, seek, the end of a non-looping
 * track, stop and closing the device under a playing track.
 */

#include <math.h>
#include <string.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <raylib.h>
#include "src/audio/grl-music.h"
#include "src/audio/grl-audio-device.h"

#define WAV_RATE (44100)

/* counts notify::background-streaming */
static void
on_notify (GObject    *object,
           GParamSpec *pspec,
           gpointer    user_data)
{
	(void)object;
	(void)pspec;
    (*(guint *)user_data)++;
}

static void
test_music_background_switch (void)
{
    GrlMusic *music;
    guint     notifies;
    gboolean  value;

    music = g_object_new (GRL_TYPE_MUSIC, NULL);
    notifies = 0;
    g_signal_connect (music, "notify::background-streaming", G_CALLBACK (on_notify), &notifies);

    g_assert_false (grl_music_get_background_streaming (music));
    grl_music_set_background_streaming (music, TRUE);
    g_assert_true (grl_music_get_background_streaming (music));
    g_assert_cmpuint (notifies, ==, 1);

    /* the same value again: no notify */
    grl_music_set_background_streaming (music, TRUE);
    g_assert_cmpuint (notifies, ==, 1);

    g_object_get (music, "background-streaming", &value, NULL);
    g_assert_true (value);
    g_object_set (music, "background-streaming", FALSE, NULL);
    g_assert_false (grl_music_get_background_streaming (music));
    g_assert_cmpuint (notifies, ==, 2);

    /* unloaded: every call is inert, also in the background */
    grl_music_set_background_streaming (music, TRUE);
    grl_music_play (music);
    grl_music_update (music);
    g_assert_false (grl_music_is_playing (music));
    g_assert_cmpfloat (grl_music_get_time_played (music), ==, 0.0f);
    grl_music_pause (music);
    grl_music_resume (music);
    grl_music_seek (music, 1.0f);
    grl_music_set_looping (music, TRUE);
    grl_music_stop (music);

    g_object_unref (music);
}

/* a 16-bit stereo WAV of @seconds of a quiet tone, in a temporary file */
static gchar *
write_wav (gdouble seconds)
{
    GByteArray *out;
    gchar      *path;
    guint32     frames;
    guint32     bytes;
    guint32     v;
    guint32     i;
    guint8      header[44];
    gint        fd;

    frames = (guint32)(seconds * WAV_RATE);
    bytes = frames * 4;
    memcpy (header, "RIFF", 4);
    v = 36 + bytes;
    memcpy (header + 4, &v, 4);
    memcpy (header + 8, "WAVEfmt ", 8);
    v = 16;
    memcpy (header + 16, &v, 4);
    header[20] = 1; header[21] = 0;
    header[22] = 2; header[23] = 0;
    v = WAV_RATE;
    memcpy (header + 24, &v, 4);
    v = WAV_RATE * 4;
    memcpy (header + 28, &v, 4);
    header[32] = 4; header[33] = 0;
    header[34] = 16; header[35] = 0;
    memcpy (header + 36, "data", 4);
    memcpy (header + 40, &bytes, 4);

    out = g_byte_array_new ();
    g_byte_array_append (out, header, 44);
    for (i = 0; i < frames; i++)
    {
        gint16 s = (gint16)(1000.0 * sin (2.0 * G_PI * 440.0 * (gdouble)i / WAV_RATE));

        g_byte_array_append (out, (const guint8 *)&s, 2);
        g_byte_array_append (out, (const guint8 *)&s, 2);
    }
    fd = g_file_open_tmp ("grl-test-music-XXXXXX.wav", &path, NULL);
    g_assert_cmpint (fd, >=, 0);
    g_assert_true (g_close (fd, NULL));
    g_assert_true (g_file_set_contents (path, (const gchar *)out->data, out->len, NULL));
    g_byte_array_unref (out);
    return path;
}

/* waits up to @seconds for the music's playing state to become @playing */
static gboolean
wait_playing (GrlMusic *music,
              gboolean  playing,
              gdouble   seconds)
{
    gint64 deadline;

    deadline = g_get_monotonic_time () + (gint64)(seconds * G_USEC_PER_SEC);
    while (g_get_monotonic_time () < deadline)
    {
        if (grl_music_is_playing (music) == playing)
            return TRUE;
        g_usleep (5000);
    }
    return grl_music_is_playing (music) == playing;
}

static void
test_music_background_device (void)
{
    GrlAudioDevice   *device;
    GrlMusic         *music;
    g_autofree gchar *path = NULL;
    g_autoptr(GError) error = NULL;
    gfloat            t0;
    gfloat            t1;

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

    path = write_wav (1.5);
    music = grl_music_new_from_file (path, &error);
    g_assert_no_error (error);
    grl_music_set_background_streaming (music, TRUE);
    grl_music_set_looping (music, FALSE);

    /* played and refilled by the thread alone (no grl_music_update ()) */
    grl_music_play (music);
    g_assert_true (grl_music_is_playing (music));
    g_usleep (400000);
    t0 = grl_music_get_time_played (music);
    g_assert_cmpfloat (t0, >, 0.1f);
    g_assert_true (grl_music_is_playing (music));

    /* paused: reported stopped, the position holds */
    grl_music_pause (music);
    g_assert_false (grl_music_is_playing (music));
    t0 = grl_music_get_time_played (music);
    g_usleep (200000);
    t1 = grl_music_get_time_played (music);
    g_assert_cmpfloat (fabsf (t1 - t0), <, 0.05f);
    grl_music_resume (music);
    g_assert_true (grl_music_is_playing (music));

    /* a seek back is seen at once */
    grl_music_seek (music, 0.2f);
    t0 = grl_music_get_time_played (music);
    g_assert_cmpfloat (t0, <, 0.6f);

    /* the end of a non-looping track is detected by the thread */
    g_assert_true (wait_playing (music, FALSE, 4.0));

    /* again from the start, then stopped */
    grl_music_play (music);
    g_assert_true (grl_music_is_playing (music));
    g_usleep (100000);
    grl_music_stop (music);
    g_assert_false (grl_music_is_playing (music));
    g_assert_cmpfloat (grl_music_get_time_played (music), <, 0.01f);

    /* switching off hands the refills back to the caller */
    grl_music_set_background_streaming (music, FALSE);
    grl_music_play (music);
    grl_music_update (music);
    g_assert_true (grl_music_is_playing (music));
    grl_music_set_background_streaming (music, TRUE);
    g_assert_true (grl_music_is_playing (music));

    /* closing the device under a playing track stops the thread first */
    grl_audio_device_close (device);
    g_assert_false (grl_audio_device_is_ready (device));

    /* raylib needs its device to unload the music (its lock lives there):
     * reopen, then dispose of a playing background track */
    grl_audio_device_init_audio (device);
    grl_music_play (music);
    g_object_unref (music);
    grl_audio_device_close (device);
    (void)g_unlink (path);
}

int
main (int   argc,
      char *argv[])
{
    g_test_init (&argc, &argv, NULL);

    g_test_add_func ("/music/background-switch", test_music_background_switch);
    g_test_add_func ("/music/background-device", test_music_background_device);

    return g_test_run ();
}

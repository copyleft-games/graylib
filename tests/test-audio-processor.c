/* test-audio-processor.c
 *
 * Copyright 2026 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Unit tests for the per-voice audio processors of GrlSound, GrlMusic
 * and GrlAudioStream.
 *
 * Headless: the voices are unloaded objects (g_object_new), which keep
 * their processors without a raylib stream; *_process() runs them as the
 * device would.  The trampoline slots are process-wide, so every test
 * checks that it gave back what it took.
 */

#include <string.h>
#include <glib.h>
#include <glib-object.h>
#include "src/audio/grl-sound.h"
#include "src/audio/grl-music.h"
#include "src/audio/grl-audio-stream.h"

#define EPSILON 0.0001f

/* a processor that scales the block and records its calls */
typedef struct
{
    gfloat  gain;
    guint   calls;
    guint   last_frames;
    guint   last_channels;
    gint   *order;      /* shared call log */
    guint  *n_order;
    gint    tag;
    gboolean *destroyed;
} Probe;

static void
probe_process (gfloat   *frames,
               guint     n_frames,
               guint     channels,
               gpointer  user_data)
{
    Probe *probe;
    guint  i;

    probe = (Probe *)user_data;
    probe->calls++;
    probe->last_frames = n_frames;
    probe->last_channels = channels;
    for (i = 0; i < n_frames * channels; i++)
        frames[i] *= probe->gain;
    if (probe->order != NULL)
        probe->order[(*probe->n_order)++] = probe->tag;
}

static void
probe_destroy (gpointer user_data)
{
    Probe *probe;

    probe = (Probe *)user_data;
    if (probe->destroyed != NULL)
        *probe->destroyed = TRUE;
}

static void
fill (gfloat *frames,
      guint   n_frames)
{
    guint i;

    for (i = 0; i < n_frames * GRL_AUDIO_PROCESSOR_CHANNELS; i++)
        frames[i] = 1.0f;
}

static void
test_processor_constants (void)
{
    g_assert_cmpuint (GRL_AUDIO_PROCESSOR_CHANNELS, ==, 2);
    g_assert_cmpuint (grl_audio_processor_get_n_free_slots (), ==, GRL_AUDIO_PROCESSOR_MAX_SLOTS);
}

static void
test_processor_sound_order (void)
{
    g_autoptr(GrlSound) sound = NULL;
    gfloat   frames[8 * GRL_AUDIO_PROCESSOR_CHANNELS];
    gint     order[8];
    guint    n_order;
    Probe    first;
    Probe    second;
    guint    id1;
    guint    id2;

    sound = g_object_new (GRL_TYPE_SOUND, NULL);
    n_order = 0;
    memset (&first, 0, sizeof (first));
    memset (&second, 0, sizeof (second));
    first.gain = 0.5f;
    first.order = order;
    first.n_order = &n_order;
    first.tag = 1;
    second.gain = 3.0f;
    second.order = order;
    second.n_order = &n_order;
    second.tag = 2;

    g_assert_cmpuint (grl_sound_get_n_processors (sound), ==, 0);
    id1 = grl_sound_add_processor (sound, probe_process, &first, NULL);
    id2 = grl_sound_add_processor (sound, probe_process, &second, NULL);
    g_assert_cmpuint (id1, !=, 0);
    g_assert_cmpuint (id2, !=, 0);
    g_assert_cmpuint (id1, !=, id2);
    g_assert_cmpuint (grl_sound_get_n_processors (sound), ==, 2);
    g_assert_cmpuint (grl_audio_processor_get_n_free_slots (), ==, GRL_AUDIO_PROCESSOR_MAX_SLOTS - 2);

    /* both run, in the order they were added, on the stereo block */
    fill (frames, 8);
    grl_sound_process (sound, frames, 8);
    g_assert_cmpuint (n_order, ==, 2);
    g_assert_cmpint (order[0], ==, 1);
    g_assert_cmpint (order[1], ==, 2);
    g_assert_cmpuint (first.last_frames, ==, 8);
    g_assert_cmpuint (first.last_channels, ==, 2);
    g_assert_cmpfloat_with_epsilon (frames[0], 1.5f, EPSILON);
    g_assert_cmpfloat_with_epsilon (frames[15], 1.5f, EPSILON);

    /* an empty block runs nothing */
    grl_sound_process (sound, frames, 0);
    g_assert_cmpuint (first.calls, ==, 1);

    /* removed: no longer runs, the slot is free again */
    g_assert_true (grl_sound_remove_processor (sound, id1));
    g_assert_false (grl_sound_remove_processor (sound, id1));
    g_assert_false (grl_sound_remove_processor (sound, 0));
    g_assert_cmpuint (grl_sound_get_n_processors (sound), ==, 1);
    fill (frames, 8);
    grl_sound_process (sound, frames, 8);
    g_assert_cmpuint (first.calls, ==, 1);
    g_assert_cmpuint (second.calls, ==, 2);
    g_assert_cmpfloat_with_epsilon (frames[3], 3.0f, EPSILON);

    grl_sound_clear_processors (sound);
    g_assert_cmpuint (grl_sound_get_n_processors (sound), ==, 0);
    g_assert_cmpuint (grl_audio_processor_get_n_free_slots (), ==, GRL_AUDIO_PROCESSOR_MAX_SLOTS);
}

static void
test_processor_destroy_on_dispose (void)
{
    GrlSound      *sound;
    GrlMusic      *music;
    GrlAudioStream *stream;
    gboolean       destroyed[3] = { FALSE, FALSE, FALSE };
    Probe          probes[3];
    guint          i;

    memset (probes, 0, sizeof (probes));
    for (i = 0; i < 3; i++)
    {
        probes[i].gain = 1.0f;
        probes[i].destroyed = &destroyed[i];
    }

    sound = g_object_new (GRL_TYPE_SOUND, NULL);
    music = g_object_new (GRL_TYPE_MUSIC, NULL);
    stream = g_object_new (GRL_TYPE_AUDIO_STREAM, NULL);
    g_assert_cmpuint (grl_sound_add_processor (sound, probe_process, &probes[0], probe_destroy), !=, 0);
    g_assert_cmpuint (grl_music_add_processor (music, probe_process, &probes[1], probe_destroy), !=, 0);
    g_assert_cmpuint (grl_audio_stream_add_processor (stream, probe_process, &probes[2], probe_destroy), !=, 0);
    g_assert_cmpuint (grl_audio_processor_get_n_free_slots (), ==, GRL_AUDIO_PROCESSOR_MAX_SLOTS - 3);

    /* the destroy notify runs when the voice goes, the slots come back */
    g_object_unref (sound);
    g_object_unref (music);
    g_object_unref (stream);
    for (i = 0; i < 3; i++)
        g_assert_true (destroyed[i]);
    g_assert_cmpuint (grl_audio_processor_get_n_free_slots (), ==, GRL_AUDIO_PROCESSOR_MAX_SLOTS);
}

static void
test_processor_remove_destroys (void)
{
    g_autoptr(GrlMusic) music = NULL;
    gfloat   frames[4 * GRL_AUDIO_PROCESSOR_CHANNELS];
    gboolean destroyed;
    Probe    probe;
    guint    id;

    music = g_object_new (GRL_TYPE_MUSIC, NULL);
    destroyed = FALSE;
    memset (&probe, 0, sizeof (probe));
    probe.gain = 2.0f;
    probe.destroyed = &destroyed;

    id = grl_music_add_processor (music, probe_process, &probe, probe_destroy);
    fill (frames, 4);
    grl_music_process (music, frames, 4);
    g_assert_cmpfloat_with_epsilon (frames[7], 2.0f, EPSILON);
    g_assert_false (destroyed);
    g_assert_true (grl_music_remove_processor (music, id));
    g_assert_true (destroyed);
    g_assert_cmpuint (grl_music_get_n_processors (music), ==, 0);
}

static void
test_processor_stream_clear (void)
{
    g_autoptr(GrlAudioStream) stream = NULL;
    gboolean destroyed[2] = { FALSE, FALSE };
    Probe    probes[2];
    guint    i;

    stream = g_object_new (GRL_TYPE_AUDIO_STREAM, NULL);
    memset (probes, 0, sizeof (probes));
    for (i = 0; i < 2; i++)
    {
        probes[i].gain = 1.0f;
        probes[i].destroyed = &destroyed[i];
        g_assert_cmpuint (grl_audio_stream_add_processor (stream, probe_process, &probes[i], probe_destroy), !=, 0);
    }
    g_assert_cmpuint (grl_audio_stream_get_n_processors (stream), ==, 2);
    grl_audio_stream_clear_processors (stream);
    g_assert_true (destroyed[0]);
    g_assert_true (destroyed[1]);
    g_assert_cmpuint (grl_audio_stream_get_n_processors (stream), ==, 0);
    g_assert_cmpuint (grl_audio_processor_get_n_free_slots (), ==, GRL_AUDIO_PROCESSOR_MAX_SLOTS);
}

static void
test_processor_slots_exhausted (void)
{
    g_autoptr(GrlSound) sound = NULL;
    gboolean destroyed;
    Probe    probe;
    Probe    late;
    guint    i;
    guint    id;

    sound = g_object_new (GRL_TYPE_SOUND, NULL);
    memset (&probe, 0, sizeof (probe));
    probe.gain = 1.0f;
    for (i = 0; i < GRL_AUDIO_PROCESSOR_MAX_SLOTS; i++)
        g_assert_cmpuint (grl_sound_add_processor (sound, probe_process, &probe, NULL), !=, 0);
    g_assert_cmpuint (grl_audio_processor_get_n_free_slots (), ==, 0);

    /* one too many: refused with a warning, its data released at once */
    destroyed = FALSE;
    memset (&late, 0, sizeof (late));
    late.destroyed = &destroyed;
    g_test_expect_message (NULL, G_LOG_LEVEL_WARNING, "*audio processor slots are taken*");
    id = grl_sound_add_processor (sound, probe_process, &late, probe_destroy);
    g_test_assert_expected_messages ();
    g_assert_cmpuint (id, ==, 0);
    g_assert_true (destroyed);
    g_assert_cmpuint (grl_sound_get_n_processors (sound), ==, GRL_AUDIO_PROCESSOR_MAX_SLOTS);

    grl_sound_clear_processors (sound);
    g_assert_cmpuint (grl_audio_processor_get_n_free_slots (), ==, GRL_AUDIO_PROCESSOR_MAX_SLOTS);
}

int
main (int   argc,
      char *argv[])
{
    g_test_init (&argc, &argv, NULL);

    g_test_add_func ("/audio-processor/constants", test_processor_constants);
    g_test_add_func ("/audio-processor/sound-order", test_processor_sound_order);
    g_test_add_func ("/audio-processor/destroy-on-dispose", test_processor_destroy_on_dispose);
    g_test_add_func ("/audio-processor/remove-destroys", test_processor_remove_destroys);
    g_test_add_func ("/audio-processor/stream-clear", test_processor_stream_clear);
    g_test_add_func ("/audio-processor/slots-exhausted", test_processor_slots_exhausted);

    return g_test_run ();
}

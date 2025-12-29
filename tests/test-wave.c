/* test-wave.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Unit tests for GrlWave.
 *
 * Note: Tests that require an audio device or file I/O are skipped.
 * These tests focus on the GBoxed type mechanics and property accessors.
 */

#include <glib.h>
#include <math.h>
#include "src/audio/grl-wave.h"

#define EPSILON 0.0001f

static gboolean
float_equal (gfloat a, gfloat b)
{
    return fabsf (a - b) < EPSILON;
}

/*
 * Test GType registration
 */

static void
test_wave_type (void)
{
    GType type = grl_wave_get_type ();

    g_assert_true (type != G_TYPE_INVALID);
    g_assert_cmpstr (g_type_name (type), ==, "GrlWave");
}

/*
 * Test constructors
 */

static void
test_wave_new_from_samples (void)
{
    /* Create a simple 16-bit mono wave with 100 frames at 44100 Hz */
    guint sample_rate = 44100;
    guint sample_size = 16;
    guint channels = 1;
    gsize frame_count = 100;
    gsize data_size = frame_count * (sample_size / 8) * channels;
    gint16 *samples = g_new0 (gint16, frame_count);
    g_autoptr(GrlWave) wave = NULL;
    gsize i;

    /* Fill with a simple pattern */
    for (i = 0; i < frame_count; i++)
    {
        samples[i] = (gint16)(i * 100);
    }

    wave = grl_wave_new_from_samples (sample_rate, sample_size, channels,
                                      (const guint8 *)samples, data_size);

    g_assert_nonnull (wave);
    g_free (samples);
}

static void
test_wave_new_from_samples_stereo (void)
{
    /* Create a 16-bit stereo wave */
    guint sample_rate = 48000;
    guint sample_size = 16;
    guint channels = 2;
    gsize frame_count = 50;
    gsize data_size = frame_count * (sample_size / 8) * channels;
    gint16 *samples = g_new0 (gint16, frame_count * channels);
    g_autoptr(GrlWave) wave = NULL;

    wave = grl_wave_new_from_samples (sample_rate, sample_size, channels,
                                      (const guint8 *)samples, data_size);

    g_assert_nonnull (wave);
    g_free (samples);
}

/*
 * Test copy/free
 */

static void
test_wave_copy (void)
{
    guint sample_rate = 44100;
    guint sample_size = 16;
    guint channels = 1;
    gsize frame_count = 100;
    gsize data_size = frame_count * (sample_size / 8) * channels;
    gint16 *samples = g_new0 (gint16, frame_count);
    g_autoptr(GrlWave) wave1 = NULL;
    g_autoptr(GrlWave) wave2 = NULL;

    wave1 = grl_wave_new_from_samples (sample_rate, sample_size, channels,
                                       (const guint8 *)samples, data_size);
    g_assert_nonnull (wave1);

    wave2 = grl_wave_copy (wave1);
    g_assert_nonnull (wave2);
    g_assert_true (wave1 != wave2);

    /* Verify copy has same properties */
    g_assert_cmpuint (grl_wave_get_sample_rate (wave1), ==, grl_wave_get_sample_rate (wave2));
    g_assert_cmpuint (grl_wave_get_sample_size (wave1), ==, grl_wave_get_sample_size (wave2));
    g_assert_cmpuint (grl_wave_get_channels (wave1), ==, grl_wave_get_channels (wave2));
    g_assert_cmpuint (grl_wave_get_frame_count (wave1), ==, grl_wave_get_frame_count (wave2));

    g_free (samples);
}

static void
test_wave_free_null (void)
{
    /* Should not crash when passed NULL */
    grl_wave_free (NULL);
}

/*
 * Test property accessors
 */

static void
test_wave_get_frame_count (void)
{
    guint sample_rate = 44100;
    guint sample_size = 16;
    guint channels = 1;
    gsize frame_count = 256;
    gsize data_size = frame_count * (sample_size / 8) * channels;
    gint16 *samples = g_new0 (gint16, frame_count);
    g_autoptr(GrlWave) wave = NULL;

    wave = grl_wave_new_from_samples (sample_rate, sample_size, channels,
                                      (const guint8 *)samples, data_size);

    g_assert_cmpuint (grl_wave_get_frame_count (wave), ==, frame_count);

    g_free (samples);
}

static void
test_wave_get_sample_rate (void)
{
    guint sample_rate = 48000;
    guint sample_size = 16;
    guint channels = 1;
    gsize frame_count = 100;
    gsize data_size = frame_count * (sample_size / 8) * channels;
    gint16 *samples = g_new0 (gint16, frame_count);
    g_autoptr(GrlWave) wave = NULL;

    wave = grl_wave_new_from_samples (sample_rate, sample_size, channels,
                                      (const guint8 *)samples, data_size);

    g_assert_cmpuint (grl_wave_get_sample_rate (wave), ==, sample_rate);

    g_free (samples);
}

static void
test_wave_get_sample_size (void)
{
    guint sample_rate = 44100;
    guint sample_size = 16;
    guint channels = 1;
    gsize frame_count = 100;
    gsize data_size = frame_count * (sample_size / 8) * channels;
    gint16 *samples = g_new0 (gint16, frame_count);
    g_autoptr(GrlWave) wave = NULL;

    wave = grl_wave_new_from_samples (sample_rate, sample_size, channels,
                                      (const guint8 *)samples, data_size);

    g_assert_cmpuint (grl_wave_get_sample_size (wave), ==, sample_size);

    g_free (samples);
}

static void
test_wave_get_channels_mono (void)
{
    guint sample_rate = 44100;
    guint sample_size = 16;
    guint channels = 1;
    gsize frame_count = 100;
    gsize data_size = frame_count * (sample_size / 8) * channels;
    gint16 *samples = g_new0 (gint16, frame_count);
    g_autoptr(GrlWave) wave = NULL;

    wave = grl_wave_new_from_samples (sample_rate, sample_size, channels,
                                      (const guint8 *)samples, data_size);

    g_assert_cmpuint (grl_wave_get_channels (wave), ==, 1);

    g_free (samples);
}

static void
test_wave_get_channels_stereo (void)
{
    guint sample_rate = 44100;
    guint sample_size = 16;
    guint channels = 2;
    gsize frame_count = 100;
    gsize data_size = frame_count * (sample_size / 8) * channels;
    gint16 *samples = g_new0 (gint16, frame_count * channels);
    g_autoptr(GrlWave) wave = NULL;

    wave = grl_wave_new_from_samples (sample_rate, sample_size, channels,
                                      (const guint8 *)samples, data_size);

    g_assert_cmpuint (grl_wave_get_channels (wave), ==, 2);

    g_free (samples);
}

static void
test_wave_get_duration (void)
{
    guint sample_rate = 44100;
    guint sample_size = 16;
    guint channels = 1;
    gsize frame_count = 44100; /* 1 second worth of frames */
    gsize data_size = frame_count * (sample_size / 8) * channels;
    gint16 *samples = g_new0 (gint16, frame_count);
    g_autoptr(GrlWave) wave = NULL;
    gfloat duration;

    wave = grl_wave_new_from_samples (sample_rate, sample_size, channels,
                                      (const guint8 *)samples, data_size);

    duration = grl_wave_get_duration (wave);
    g_assert_true (float_equal (duration, 1.0f));

    g_free (samples);
}

static void
test_wave_get_duration_half_second (void)
{
    guint sample_rate = 44100;
    guint sample_size = 16;
    guint channels = 1;
    gsize frame_count = 22050; /* 0.5 second worth of frames */
    gsize data_size = frame_count * (sample_size / 8) * channels;
    gint16 *samples = g_new0 (gint16, frame_count);
    g_autoptr(GrlWave) wave = NULL;
    gfloat duration;

    wave = grl_wave_new_from_samples (sample_rate, sample_size, channels,
                                      (const guint8 *)samples, data_size);

    duration = grl_wave_get_duration (wave);
    g_assert_true (float_equal (duration, 0.5f));

    g_free (samples);
}

static void
test_wave_is_valid (void)
{
    guint sample_rate = 44100;
    guint sample_size = 16;
    guint channels = 1;
    gsize frame_count = 100;
    gsize data_size = frame_count * (sample_size / 8) * channels;
    gint16 *samples = g_new0 (gint16, frame_count);
    g_autoptr(GrlWave) wave = NULL;

    wave = grl_wave_new_from_samples (sample_rate, sample_size, channels,
                                      (const guint8 *)samples, data_size);

    g_assert_true (grl_wave_is_valid (wave));

    g_free (samples);
}

/*
 * Test manipulation functions
 */

static void
test_wave_crop (void)
{
    guint sample_rate = 44100;
    guint sample_size = 16;
    guint channels = 1;
    gsize frame_count = 100;
    gsize data_size = frame_count * (sample_size / 8) * channels;
    gint16 *samples = g_new0 (gint16, frame_count);
    g_autoptr(GrlWave) wave = NULL;
    g_autoptr(GrlWave) cropped = NULL;

    wave = grl_wave_new_from_samples (sample_rate, sample_size, channels,
                                      (const guint8 *)samples, data_size);

    cropped = grl_wave_crop (wave, 10, 50);
    g_assert_nonnull (cropped);
    g_assert_cmpuint (grl_wave_get_frame_count (cropped), ==, 40); /* 50 - 10 */

    g_free (samples);
}

static void
test_wave_format (void)
{
    guint sample_rate = 44100;
    guint sample_size = 16;
    guint channels = 1;
    gsize frame_count = 100;
    gsize data_size = frame_count * (sample_size / 8) * channels;
    gint16 *samples = g_new0 (gint16, frame_count);
    g_autoptr(GrlWave) wave = NULL;
    g_autoptr(GrlWave) formatted = NULL;

    wave = grl_wave_new_from_samples (sample_rate, sample_size, channels,
                                      (const guint8 *)samples, data_size);

    /* Convert to 22050 Hz, 8-bit, stereo */
    formatted = grl_wave_format (wave, 22050, 8, 2);
    g_assert_nonnull (formatted);
    g_assert_cmpuint (grl_wave_get_sample_rate (formatted), ==, 22050);
    g_assert_cmpuint (grl_wave_get_sample_size (formatted), ==, 8);
    g_assert_cmpuint (grl_wave_get_channels (formatted), ==, 2);

    g_free (samples);
}

/*
 * Test sample access
 */

static void
test_wave_load_samples (void)
{
    guint sample_rate = 44100;
    guint sample_size = 16;
    guint channels = 1;
    gsize frame_count = 100;
    gsize data_size = frame_count * (sample_size / 8) * channels;
    gint16 *samples = g_new0 (gint16, frame_count);
    g_autoptr(GrlWave) wave = NULL;
    gfloat *loaded_samples = NULL;
    gsize loaded_count = 0;

    wave = grl_wave_new_from_samples (sample_rate, sample_size, channels,
                                      (const guint8 *)samples, data_size);

    loaded_samples = grl_wave_load_samples (wave, &loaded_count);
    g_assert_nonnull (loaded_samples);
    g_assert_cmpuint (loaded_count, ==, frame_count * channels);

    g_free (loaded_samples);
    g_free (samples);
}

/*
 * Test registration
 */

int
main (int   argc,
      char *argv[])
{
    g_test_init (&argc, &argv, NULL);

    /* Type registration */
    g_test_add_func ("/wave/type", test_wave_type);

    /* Constructors */
    g_test_add_func ("/wave/new-from-samples", test_wave_new_from_samples);
    g_test_add_func ("/wave/new-from-samples-stereo", test_wave_new_from_samples_stereo);

    /* Copy/Free */
    g_test_add_func ("/wave/copy", test_wave_copy);
    g_test_add_func ("/wave/free-null", test_wave_free_null);

    /* Property accessors */
    g_test_add_func ("/wave/get-frame-count", test_wave_get_frame_count);
    g_test_add_func ("/wave/get-sample-rate", test_wave_get_sample_rate);
    g_test_add_func ("/wave/get-sample-size", test_wave_get_sample_size);
    g_test_add_func ("/wave/get-channels-mono", test_wave_get_channels_mono);
    g_test_add_func ("/wave/get-channels-stereo", test_wave_get_channels_stereo);
    g_test_add_func ("/wave/get-duration", test_wave_get_duration);
    g_test_add_func ("/wave/get-duration-half-second", test_wave_get_duration_half_second);
    g_test_add_func ("/wave/is-valid", test_wave_is_valid);

    /* Manipulation */
    g_test_add_func ("/wave/crop", test_wave_crop);
    g_test_add_func ("/wave/format", test_wave_format);

    /* Sample access */
    g_test_add_func ("/wave/load-samples", test_wave_load_samples);

    return g_test_run ();
}

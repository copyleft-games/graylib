/* SPDX-License-Identifier: AGPL-3.0-or-later
 * Hermetic native adapters. FakeBuffer is test-owned, NOT an SDK layout.
 * Real public Wave conversion remains device-free; no audio device is opened.
 */
#include "src/audio/grl-audio-cleanup-private.h"
#include <graylib.h>
#include <gio/gio.h>
#include <glib/gstdio.h>
#include <string.h>

typedef struct {
	Wave wave;
	gboolean borrowed;
} FakeBuffer;

static FakeBuffer *last_buffer;
static guint allocated_buffers, freed_buffers;
static guint context_frees;
static gboolean fixed_sdk;
static gboolean large_native_sound;
static gboolean device_ready = TRUE;

/* This fixture models only public Sound/Wave contracts. The production
 * wrapper treats stream.buffer as opaque and never sees FakeBuffer fields. */
Sound
LoadSoundFromWave (Wave wave)
{
	Sound result = { 0 };
	FakeBuffer *buffer = g_new0 (FakeBuffer, 1);

	buffer->wave = WaveCopy (wave);
	WaveFormat (&buffer->wave, 44100, 32, 2);
	result.frameCount = buffer->wave.frameCount;
	if (large_native_sound)
		result.frameCount = GRL_SOUND_VOICE_MAX_PCM_BYTES / 8 + 1;
	result.stream.sampleRate = 44100;
	result.stream.sampleSize = 32;
	result.stream.channels = 2;
	result.stream.buffer = (rAudioBuffer *)buffer;
	last_buffer = buffer;
	allocated_buffers++;
	return result;
}

void
UnloadSound (Sound sound)
{
	FakeBuffer *buffer = (FakeBuffer *)sound.stream.buffer;

	g_assert_false (buffer->borrowed);
	UnloadWave (buffer->wave);
	g_free (buffer);
	freed_buffers++;
}

Sound
LoadSoundAlias (Sound source)
{
	Sound result = source;
	FakeBuffer *original = (FakeBuffer *)source.stream.buffer;
	FakeBuffer *buffer = g_new0 (FakeBuffer, 1);

	buffer->wave = original->wave;
	buffer->borrowed = TRUE;
	result.stream.buffer = (rAudioBuffer *)buffer;
	last_buffer = buffer;
	allocated_buffers++;
	return result;
}

void
UnloadSoundAlias (Sound alias)
{
	FakeBuffer *buffer = (FakeBuffer *)alias.stream.buffer;

	g_assert_true (buffer->borrowed);
	g_free (buffer);
	freed_buffers++;
}

void
UpdateSound (Sound sound, const void *data, int frames)
{
	FakeBuffer *buffer = (FakeBuffer *)sound.stream.buffer;

	g_assert_cmpuint ((guint)frames, <=, sound.frameCount);
	memcpy (buffer->wave.data, data, (gsize)frames * 2 * sizeof (gfloat));
}

bool
IsAudioDeviceReady (void)
{
	return device_ready ? true : false;
}

/* No native mixer is involved in these metadata-only device adapters. */
void SetSoundVolume (Sound sound, float value) { (void)sound; (void)value; }
void SetSoundPitch (Sound sound, float value) { (void)sound; (void)value; }
void SetSoundPan (Sound sound, float value) { (void)sound; (void)value; }

static void
free_context (void *context)
{
	context_frees++;
	g_assert_cmpuint (context_frees, ==, 1);
	g_free (context);
}

static void
unload_music (Music music)
{
	if (fixed_sdk)
		free_context (music.ctxData);
}

/* A simulated future SDK frees the context itself: capability FALSE must
 * leave it alone. The known old SDK delegates exactly that one free to us. */
static void
test_context_deallocation (void)
{
	Music music = { 0 };
	gpointer extra;
	guint mode;

	for (mode = 0; mode < 3; mode++) {
		context_frees = 0;
		music.ctxData = g_malloc0 (8);
		fixed_sdk = mode != 0;
		extra = grl_audio_wav_extra_context (music, mode != 2, mode != 1);
		grl_audio_unload_music_owned (music, extra, unload_music, free_context);
		g_assert_cmpuint (context_frees, ==, 1);
	}
}

static const gfloat initial[8] = { .1f, .2f, .3f, .4f, .5f, .6f, .7f, .8f };
static const gfloat changed[8] = { .8f, .7f, .6f, .5f, .4f, .3f, .2f, .1f };

/* Explicit independent policy retains the initial snapshot; native aliases
 * still share writes, keep the source alive, and expose updated snapshots. */
static void
test_voice_and_alias_ownership (void)
{
	g_autoptr(GrlSound) source = NULL;
	g_autoptr(GrlSound) voice = NULL;
	g_autoptr(GrlSound) alias = NULL;
	g_autoptr(GrlSound) updated = NULL;
	g_autoptr(GrlSound) child = NULL;
	g_autoptr(GError) error = NULL;
	g_autoptr(GrlSound) retained_source = NULL;
	FakeBuffer *voice_buffer;
	FakeBuffer *base_buffer;
	GWeakRef weak;

	source = grl_sound_new_from_wave ((const guint8 *)initial, sizeof initial, 44100, 32, 2);
	g_assert_nonnull (source);
	base_buffer = last_buffer;
	voice = grl_sound_new_voice (source, &error);
	g_assert_no_error (error);
	g_assert_nonnull (voice);
	g_assert_false (grl_sound_is_alias (voice));
	voice_buffer = last_buffer;
	alias = grl_sound_new_alias (source);
	g_assert_nonnull (alias);
	g_assert_true (grl_sound_is_alias (alias));
	g_assert_true (grl_sound_update_checked (alias, (const guint8 *)changed, sizeof changed, 4, &error));
	g_assert_no_error (error);
	g_assert_cmpmem (base_buffer->wave.data, sizeof changed, changed, sizeof changed);
	g_assert_cmpmem (voice_buffer->wave.data, sizeof initial, initial, sizeof initial);
	updated = grl_sound_new_voice (alias, &error);
	g_assert_no_error (error);
	g_assert_cmpmem (last_buffer->wave.data, sizeof changed, changed, sizeof changed);
	/* A later voice-of-voice still sees the old immutable snapshot; partial
	 * shared writes preserve the untouched tail and publish a new snapshot. */
	child = grl_sound_new_voice (voice, &error);
	g_assert_no_error (error);
	g_assert_cmpmem (last_buffer->wave.data, sizeof initial, initial, sizeof initial);
	g_assert_true (grl_sound_update_checked (alias, (const guint8 *)initial, sizeof initial / 2, 2, &error));
	g_assert_no_error (error);
	g_clear_object (&updated);
	updated = grl_sound_new_voice (alias, &error);
	g_assert_no_error (error);
	g_assert_cmpmem (last_buffer->wave.data, sizeof initial / 2, initial, sizeof initial / 2);
	g_assert_cmpmem ((const guint8 *)last_buffer->wave.data + sizeof initial / 2, sizeof changed / 2,
	                 (const guint8 *)changed + sizeof changed / 2, sizeof changed / 2);
	grl_sound_set_volume (voice, .25f);
	grl_sound_set_pitch (voice, 1.5f);
	grl_sound_set_pan (voice, -.5f);
	g_assert_cmpfloat (grl_sound_get_volume (source), ==, 1.0f);
	g_assert_cmpfloat (grl_sound_get_pitch (source), ==, 1.0f);
	g_assert_cmpfloat (grl_sound_get_pan (source), ==, 0.0f);
	g_weak_ref_init (&weak, source);
	g_clear_object (&source);
	retained_source = g_weak_ref_get (&weak);
	g_assert_nonnull (retained_source);
	g_clear_object (&retained_source);
	g_clear_object (&alias);
	retained_source = g_weak_ref_get (&weak);
	g_assert_null (retained_source);
	g_weak_ref_clear (&weak);
	g_assert_cmpmem (voice_buffer->wave.data, sizeof initial, initial, sizeof initial);
}

/* Bad cardinalities never reach the adapter or change any retained bytes. */
static void
test_checked_update_rejection (void)
{
	g_autoptr(GrlSound) source = grl_sound_new_from_wave ((const guint8 *)initial, sizeof initial, 44100, 32, 2);
	g_autoptr(GError) error = NULL;
	FakeBuffer *buffer = last_buffer;

	g_assert_false (grl_sound_update_checked (source, (const guint8 *)changed, sizeof changed - 1, 4, &error));
	g_assert_error (error, G_IO_ERROR, G_IO_ERROR_INVALID_ARGUMENT);
	g_clear_error (&error);
	g_assert_false (grl_sound_update_checked (source, (const guint8 *)changed, sizeof changed, G_MAXUINT, &error));
	g_assert_error (error, G_IO_ERROR, G_IO_ERROR_INVALID_ARGUMENT);
	g_clear_error (&error);
	g_assert_cmpmem (buffer->wave.data, sizeof initial, initial, sizeof initial);
	g_assert_null (grl_sound_new_from_wave ((const guint8 *)initial, sizeof initial, 44100, 0, 2));
	g_assert_null (grl_sound_new_from_wave ((const guint8 *)initial, sizeof initial - 1, 44100, 32, 2));
	device_ready = FALSE;
	g_assert_false (grl_sound_update_checked (source, (const guint8 *)changed, sizeof changed, 4, &error));
	g_assert_error (error, G_IO_ERROR, G_IO_ERROR_NOT_INITIALIZED);
	g_clear_error (&error);
	g_assert_null (grl_sound_new_voice (source, &error));
	g_assert_error (error, G_IO_ERROR, G_IO_ERROR_NOT_INITIALIZED);
	device_ready = TRUE;
}

static void
test_unavailable_snapshot (void)
{
	g_autoptr(GrlSound) source = g_object_new (GRL_TYPE_SOUND, NULL);
	g_autoptr(GError) error = NULL;

	g_assert_null (grl_sound_new_voice (source, &error));
	g_assert_error (error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED);
	g_clear_error (&error);
	large_native_sound = TRUE;
	g_clear_object (&source);
	source = grl_sound_new_from_wave ((const guint8 *)initial, sizeof initial, 44100, 32, 2);
	large_native_sound = FALSE;
	g_assert_nonnull (source);
	g_assert_null (grl_sound_new_voice (source, &error));
	g_assert_error (error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED);
}

/* Exercise each decoder/borrowed-data route a hundred times. Real public
 * Wave file/memory decoding is used; only device-facing Sound APIs are fake. */
static void
test_constructor_smoke (void)
{
	g_autoptr(GError) error = NULL;
	g_autofree gchar *directory = g_dir_make_tmp ("grl-audio-owned-XXXXXX", &error);
	g_autofree gchar *path = NULL;
	g_autofree gchar *encoded = NULL;
	g_autoptr(GrlWave) decoded = NULL;
	gsize encoded_size = 0;
	Wave wave;
	guint mode, iteration;

	g_assert_no_error (error);
	path = g_build_filename (directory, "fixture.wav", NULL);
	wave.frameCount = 4;
	wave.sampleRate = 44100;
	wave.sampleSize = 32;
	wave.channels = 2;
	wave.data = (gpointer)initial;
	g_assert_true (ExportWave (wave, path));
	g_assert_true (g_file_get_contents (path, &encoded, &encoded_size, &error));
	g_assert_no_error (error);
	decoded = grl_wave_new_from_memory (".wav", (const guint8 *)encoded, encoded_size, &error);
	g_assert_no_error (error);
	for (mode = 0; mode < 4; mode++) {
		for (iteration = 0; iteration < 100; iteration++) {
			g_autoptr(GrlSound) source = NULL;
			g_autoptr(GrlSound) voice = NULL;

			switch (mode) {
			case 0: source = grl_sound_new_from_file (path, &error); break;
			case 1: source = grl_sound_new_from_memory (".wav", (const guint8 *)encoded, encoded_size, &error); break;
			case 2: source = grl_sound_new_from_wave ((const guint8 *)initial, sizeof initial, 44100, 32, 2); break;
			case 3: source = grl_sound_new_from_grl_wave (decoded); break;
			default: g_assert_not_reached ();
			}
			g_assert_no_error (error);
			g_assert_nonnull (source);
			voice = grl_sound_new_voice (source, &error);
			g_assert_no_error (error);
			g_assert_nonnull (voice);
			g_clear_object (&source);
			g_assert_false (grl_sound_is_alias (voice));
		}
	}
	g_assert_cmpint (g_unlink (path), ==, 0);
	g_assert_cmpint (g_rmdir (directory), ==, 0);
}

int
main (int argc, char **argv)
{
	gint result;

	g_test_init (&argc, &argv, NULL);
	g_test_add_func ("/audio-owned/context-free-once", test_context_deallocation);
	g_test_add_func ("/audio-owned/voice-and-alias", test_voice_and_alias_ownership);
	g_test_add_func ("/audio-owned/checked-update", test_checked_update_rejection);
	g_test_add_func ("/audio-owned/unavailable-snapshot", test_unavailable_snapshot);
	g_test_add_func ("/audio-owned/constructors-100", test_constructor_smoke);
	result = g_test_run ();
	g_assert_cmpuint (allocated_buffers, ==, freed_buffers);
	return result;
}

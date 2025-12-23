# Audio System

This document covers the audio types added in Phase 4: audio device, sound effects, and streaming music.

## GrlAudioDevice

Singleton for managing the audio system. Must be initialized before using any audio features.

### Initialization

```c
/* Get the audio device singleton */
GrlAudioDevice *audio = grl_audio_device_get_default ();

/* Initialize audio system */
if (!grl_audio_device_init_audio (audio))
{
    g_printerr ("Failed to initialize audio\n");
    return 1;
}

/* Audio is ready to use */
g_print ("Audio ready: %s\n",
         grl_audio_device_is_ready (audio) ? "yes" : "no");
```

### Master Volume

```c
/* Set master volume (0.0 to 1.0) */
grl_audio_device_set_master_volume (audio, 0.8f);

/* Get current master volume */
gfloat vol = grl_audio_device_get_master_volume (audio);
```

### Cleanup

```c
/* Close audio device (optional - happens automatically on exit) */
grl_audio_device_close (audio);
```

## GrlSound

Sound effects that are loaded entirely into memory. Best for short sounds like button clicks, explosions, and UI feedback.

### Loading Sounds

```c
g_autoptr(GError) error = NULL;

/* Load from file */
g_autoptr(GrlSound) sfx = grl_sound_new_from_file ("shoot.wav", &error);

if (sfx == NULL)
{
    g_printerr ("Failed to load sound: %s\n", error->message);
    return;
}

/* Create from raw wave data */
g_autoptr(GrlSound) beep = grl_sound_new_from_wave (
    wave_data,      /* Raw PCM data */
    data_size,      /* Size in bytes */
    44100,          /* Sample rate (Hz) */
    16,             /* Sample size (bits) */
    1               /* Channels (1=mono, 2=stereo) */
);
```

### Playback Control

```c
/* Play sound (restarts if already playing) */
grl_sound_play (sfx);

/* Stop playback */
grl_sound_stop (sfx);

/* Pause/Resume */
grl_sound_pause (sfx);
grl_sound_resume (sfx);

/* Check if playing */
if (grl_sound_is_playing (sfx))
{
    g_print ("Sound is playing\n");
}
```

### Audio Properties

```c
/* Volume (0.0 to 1.0) */
grl_sound_set_volume (sfx, 0.8f);
gfloat vol = grl_sound_get_volume (sfx);

/* Pitch (0.5 to 2.0, 1.0 = normal) */
grl_sound_set_pitch (sfx, 1.2f);  /* Slightly higher pitch */
gfloat pitch = grl_sound_get_pitch (sfx);

/* Pan (-1.0=left, 0.0=center, 1.0=right) */
grl_sound_set_pan (sfx, -0.5f);   /* Slightly left */
gfloat pan = grl_sound_get_pan (sfx);
```

### Multi-Sound Support

For sound effects that need to overlap (e.g., rapid gunfire):

```c
/* Play overlapping instances */
grl_sound_play_multi (sfx);
grl_sound_play_multi (sfx);  /* Second instance starts immediately */

/* Stop all instances */
grl_sound_stop_multi (sfx);

/* Get number of playing instances */
gint count = grl_sound_get_sounds_playing (sfx);
```

## GrlMusic

Streaming music that is loaded in chunks from disk. Best for background music and long audio tracks.

### Loading Music

```c
g_autoptr(GError) error = NULL;

/* Load from file (streaming) */
g_autoptr(GrlMusic) music = grl_music_new_from_file ("bgm.ogg", &error);

if (music == NULL)
{
    g_printerr ("Failed to load music: %s\n", error->message);
    return;
}
```

**Supported formats:** WAV, OGG, MP3, FLAC, XM, MOD (depending on raylib build)

### Playback Control

```c
/* Start playback */
grl_music_play (music);

/* Stop and reset position */
grl_music_stop (music);

/* Pause/Resume */
grl_music_pause (music);
grl_music_resume (music);

/* Check if playing */
if (grl_music_is_playing (music))
{
    g_print ("Music is playing\n");
}
```

### Stream Updating (CRITICAL!)

**You MUST call `grl_music_update()` in your main loop while music is playing:**

```c
while (!grl_window_should_close (window))
{
    /* Update music stream buffers - REQUIRED! */
    grl_music_update (music);

    /* Rest of your game loop... */
    grl_window_begin_drawing (window);
    /* ... */
    grl_window_end_drawing (window);
}
```

Without calling `grl_music_update()`, music will stutter or stop.

### Looping

```c
/* Enable looping */
grl_music_set_looping (music, TRUE);

/* Check if looping is enabled */
if (grl_music_get_looping (music))
{
    g_print ("Music will loop\n");
}
```

### Seeking and Time

```c
/* Get total length */
gfloat length = grl_music_get_time_length (music);
g_print ("Music is %.1f seconds long\n", length);

/* Get current position */
gfloat position = grl_music_get_time_played (music);
g_print ("Currently at %.1f seconds\n", position);

/* Seek to specific position */
grl_music_seek (music, 30.0f);  /* Jump to 30 seconds */
```

### Audio Properties

```c
/* Volume (0.0 to 1.0) */
grl_music_set_volume (music, 0.5f);
gfloat vol = grl_music_get_volume (music);

/* Pitch (0.5 to 2.0, 1.0 = normal) */
grl_music_set_pitch (music, 0.9f);  /* Slightly slower */
gfloat pitch = grl_music_get_pitch (music);

/* Pan (-1.0=left, 0.0=center, 1.0=right) */
grl_music_set_pan (music, 0.0f);    /* Center */
gfloat pan = grl_music_get_pan (music);
```

## Complete Example

```c
#include <graylib.h>

int
main (int argc, char *argv[])
{
    g_autoptr(GrlWindow) window = NULL;
    g_autoptr(GrlAudioDevice) audio = NULL;
    g_autoptr(GrlMusic) music = NULL;
    g_autoptr(GrlSound) click = NULL;
    g_autoptr(GrlColor) bg = NULL;
    g_autoptr(GError) error = NULL;

    /* Create window */
    window = grl_window_new (800, 600, "Audio Demo");
    grl_window_set_target_fps (window, 60);

    /* Initialize audio */
    audio = grl_audio_device_get_default ();
    if (!grl_audio_device_init_audio (audio))
    {
        g_printerr ("Failed to init audio\n");
        return 1;
    }

    /* Load audio files */
    music = grl_music_new_from_file ("music.ogg", &error);
    if (music == NULL)
    {
        g_print ("No music file: %s\n", error->message);
        g_clear_error (&error);
    }
    else
    {
        grl_music_set_looping (music, TRUE);
        grl_music_set_volume (music, 0.5f);
        grl_music_play (music);
    }

    click = grl_sound_new_from_file ("click.wav", &error);
    if (click == NULL)
    {
        g_print ("No sound file: %s\n", error->message);
        g_clear_error (&error);
    }

    bg = grl_color_new (30, 30, 45, 255);

    while (!grl_window_should_close (window))
    {
        /* CRITICAL: Update music stream */
        if (music != NULL)
        {
            grl_music_update (music);
        }

        /* Handle input */
        if (grl_input_is_key_pressed (GRL_KEY_SPACE))
        {
            if (music != NULL)
            {
                if (grl_music_is_playing (music))
                    grl_music_pause (music);
                else
                    grl_music_resume (music);
            }
        }

        if (grl_input_is_key_pressed (GRL_KEY_E) && click != NULL)
        {
            grl_sound_play (click);
        }

        /* Draw */
        grl_window_begin_drawing (window);
        grl_window_clear_background (window, bg);

        if (music != NULL)
        {
            gchar text[64];
            g_snprintf (text, sizeof(text), "Time: %.1f / %.1f",
                        grl_music_get_time_played (music),
                        grl_music_get_time_length (music));
            grl_draw_text (text, 20, 20, 20, NULL);
        }

        grl_draw_text ("SPACE: Play/Pause  E: Sound Effect", 20, 50, 16, NULL);
        grl_draw_fps (700, 10);

        grl_window_end_drawing (window);
    }

    return 0;
}
```

## Sound vs Music: When to Use Each

| Feature | GrlSound | GrlMusic |
|---------|----------|----------|
| Memory | Fully loaded | Streamed from disk |
| Latency | Instant | Slight delay |
| Best for | Short effects | Long tracks |
| Overlapping | Supported | Single instance |
| Requires update | No | Yes (every frame) |

**Use GrlSound for:**
- Button clicks
- Explosions
- Footsteps
- UI feedback
- Any sound < 5 seconds

**Use GrlMusic for:**
- Background music
- Ambient sounds
- Voice overs
- Any audio > 10 seconds

## Properties Summary

### GrlAudioDevice Properties

| Property | Type | Access | Description |
|----------|------|--------|-------------|
| `is-ready` | gboolean | Read | Whether audio device is initialized |
| `master-volume` | gfloat | Read/Write | Master volume (0.0-1.0) |

### GrlSound Properties

| Property | Type | Access | Description |
|----------|------|--------|-------------|
| `filename` | gchar* | Read | Source filename |
| `volume` | gfloat | Read/Write | Volume (0.0-1.0) |
| `pitch` | gfloat | Read/Write | Pitch multiplier |
| `pan` | gfloat | Read/Write | Stereo pan (-1.0 to 1.0) |
| `is-playing` | gboolean | Read | Whether sound is playing |

### GrlMusic Properties

| Property | Type | Access | Description |
|----------|------|--------|-------------|
| `filename` | gchar* | Read | Source filename |
| `volume` | gfloat | Read/Write | Volume (0.0-1.0) |
| `pitch` | gfloat | Read/Write | Pitch multiplier |
| `pan` | gfloat | Read/Write | Stereo pan (-1.0 to 1.0) |
| `looping` | gboolean | Read/Write | Whether music loops |
| `is-playing` | gboolean | Read | Whether music is playing |
| `time-length` | gfloat | Read | Total duration in seconds |
| `time-played` | gfloat | Read | Current position in seconds |

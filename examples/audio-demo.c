/* audio-demo.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Audio system demonstration for Graylib.
 *
 * This example shows how to:
 * - Initialize the audio device
 * - Load and play sound effects
 * - Load and stream music
 * - Control volume, pitch, and pan
 * - Handle music looping and seeking
 *
 * Note: This example requires audio files to be present.
 * Place test.wav (short sound effect) and music.ogg (background music)
 * in the same directory as the executable, or modify the paths below.
 */

#include <graylib.h>
#include <stdio.h>

/* UI Layout constants */
#define MARGIN          20
#define BOX_HEIGHT      30
#define TEXT_SIZE       20

int
main (int   argc,
      char *argv[])
{
    g_autoptr(GrlWindow) window = NULL;
    g_autoptr(GrlAudioDevice) audio = NULL;
    g_autoptr(GrlMusic) music = NULL;
    g_autoptr(GrlSound) sound = NULL;
    g_autoptr(GrlColor) bg_color = NULL;
    g_autoptr(GrlColor) text_color = NULL;
    g_autoptr(GrlColor) bar_color = NULL;
    g_autoptr(GrlColor) bar_bg_color = NULL;
    g_autoptr(GError) error = NULL;

    gfloat master_volume;
    gfloat music_volume;
    gfloat music_pitch;
    gfloat music_pan;
    gboolean music_loaded;
    gboolean sound_loaded;

    /* Create the window */
    window = grl_window_new (800, 500, "Graylib - Audio Demo");

    if (!grl_window_is_ready (window))
    {
        g_printerr ("Failed to create window\n");
        return 1;
    }

    grl_window_set_target_fps (window, 60);

    /* Initialize audio device */
    audio = grl_audio_device_get_default ();

    if (!grl_audio_device_init_audio (audio))
    {
        g_printerr ("Failed to initialize audio device\n");
        return 1;
    }

    g_print ("Audio device initialized\n");

    /* Try to load music file */
    music = grl_music_new_from_file ("music.ogg", &error);
    music_loaded = (music != NULL);

    if (!music_loaded)
    {
        g_print ("Could not load music.ogg: %s\n", error->message);
        g_print ("Music playback will be disabled.\n");
        g_clear_error (&error);
    }
    else
    {
        g_print ("Music loaded successfully\n");
        grl_music_set_looping (music, TRUE);
    }

    /* Try to load sound effect */
    sound = grl_sound_new_from_file ("test.wav", &error);
    sound_loaded = (sound != NULL);

    if (!sound_loaded)
    {
        g_print ("Could not load test.wav: %s\n", error->message);
        g_print ("Sound effects will be disabled.\n");
        g_clear_error (&error);
    }
    else
    {
        g_print ("Sound effect loaded successfully\n");
    }

    /* Create colors */
    bg_color = grl_color_new (30, 30, 45, 255);
    text_color = grl_color_new_raywhite ();
    bar_color = grl_color_new (100, 200, 100, 255);
    bar_bg_color = grl_color_new (60, 60, 80, 255);

    /* Initialize audio parameters */
    master_volume = 1.0f;
    music_volume = 0.8f;
    music_pitch = 1.0f;
    music_pan = 0.0f;

    /* Print controls */
    g_print ("\n=== Audio Demo Controls ===\n");
    g_print ("SPACE     - Play/Pause music\n");
    g_print ("S         - Stop music\n");
    g_print ("E         - Play sound effect\n");
    g_print ("L         - Toggle music loop\n");
    g_print ("UP/DOWN   - Master volume\n");
    g_print ("LEFT/RIGHT - Music pan\n");
    g_print ("[/]       - Music pitch\n");
    g_print ("ESC       - Exit\n");
    g_print ("===========================\n\n");

    /* Main game loop */
    while (!grl_window_should_close (window))
    {
        gint width, height;
        gint y_pos;
        gchar info_text[256];
        gfloat time_played;
        gfloat time_length;
        gfloat progress;
        gboolean is_playing;
        gboolean is_looping;

        width = grl_window_get_width (window);
        height = grl_window_get_height (window);

        /* Update music stream (REQUIRED for streaming audio!) */
        if (music_loaded)
        {
            grl_music_update (music);
        }

        /* Handle input */

        /* Space - Play/Pause music */
        if (grl_input_is_key_pressed (GRL_KEY_SPACE) && music_loaded)
        {
            if (grl_music_is_playing (music))
            {
                grl_music_pause (music);
                g_print ("Music paused\n");
            }
            else
            {
                grl_music_play (music);
                g_print ("Music playing\n");
            }
        }

        /* S - Stop music */
        if (grl_input_is_key_pressed (GRL_KEY_S) && music_loaded)
        {
            grl_music_stop (music);
            g_print ("Music stopped\n");
        }

        /* E - Play sound effect */
        if (grl_input_is_key_pressed (GRL_KEY_E) && sound_loaded)
        {
            grl_sound_play (sound);
            g_print ("Sound effect played\n");
        }

        /* L - Toggle loop */
        if (grl_input_is_key_pressed (GRL_KEY_L) && music_loaded)
        {
            gboolean loop = !grl_music_get_looping (music);
            grl_music_set_looping (music, loop);
            g_print ("Music loop: %s\n", loop ? "ON" : "OFF");
        }

        /* UP/DOWN - Master volume */
        if (grl_input_is_key_down (GRL_KEY_UP))
        {
            master_volume += 0.01f;

            if (master_volume > 1.0f)
                master_volume = 1.0f;

            grl_audio_device_set_master_volume (audio, master_volume);
        }

        if (grl_input_is_key_down (GRL_KEY_DOWN))
        {
            master_volume -= 0.01f;

            if (master_volume < 0.0f)
                master_volume = 0.0f;

            grl_audio_device_set_master_volume (audio, master_volume);
        }

        /* LEFT/RIGHT - Pan */
        if (grl_input_is_key_down (GRL_KEY_LEFT) && music_loaded)
        {
            music_pan -= 0.01f;

            if (music_pan < -1.0f)
                music_pan = -1.0f;

            grl_music_set_pan (music, music_pan);
        }

        if (grl_input_is_key_down (GRL_KEY_RIGHT) && music_loaded)
        {
            music_pan += 0.01f;

            if (music_pan > 1.0f)
                music_pan = 1.0f;

            grl_music_set_pan (music, music_pan);
        }

        /* [/] - Pitch */
        if (grl_input_is_key_down (GRL_KEY_LEFT_BRACKET) && music_loaded)
        {
            music_pitch -= 0.005f;

            if (music_pitch < 0.5f)
                music_pitch = 0.5f;

            grl_music_set_pitch (music, music_pitch);
        }

        if (grl_input_is_key_down (GRL_KEY_RIGHT_BRACKET) && music_loaded)
        {
            music_pitch += 0.005f;

            if (music_pitch > 2.0f)
                music_pitch = 2.0f;

            grl_music_set_pitch (music, music_pitch);
        }

        /* Get music state */
        if (music_loaded)
        {
            time_played = grl_music_get_time_played (music);
            time_length = grl_music_get_time_length (music);
            is_playing = grl_music_is_playing (music);
            is_looping = grl_music_get_looping (music);
            progress = (time_length > 0.0f) ? (time_played / time_length) : 0.0f;
        }
        else
        {
            time_played = 0.0f;
            time_length = 0.0f;
            is_playing = FALSE;
            is_looping = FALSE;
            progress = 0.0f;
        }

        /* Begin drawing */
        grl_window_begin_drawing (window);
        grl_window_clear_background (window, bg_color);

        /* Title */
        grl_draw_text ("GRAYLIB AUDIO DEMO", MARGIN, MARGIN, 30, text_color);

        y_pos = MARGIN + 50;

        /* Audio device status */
        g_snprintf (info_text, sizeof (info_text),
                    "Audio Device: %s",
                    grl_audio_device_is_ready (audio) ? "Ready" : "Not Ready");
        grl_draw_text (info_text, MARGIN, y_pos, TEXT_SIZE, text_color);
        y_pos += BOX_HEIGHT;

        /* Master volume bar */
        g_snprintf (info_text, sizeof (info_text), "Master Volume: %.0f%%", master_volume * 100.0f);
        grl_draw_text (info_text, MARGIN, y_pos, TEXT_SIZE, text_color);
        y_pos += 25;
        grl_draw_rectangle (MARGIN, y_pos, width - MARGIN * 2, 15, bar_bg_color);
        grl_draw_rectangle (MARGIN, y_pos,
                            (gint)((width - MARGIN * 2) * master_volume), 15, bar_color);
        y_pos += BOX_HEIGHT;

        /* Separator */
        y_pos += 10;
        grl_draw_text ("--- MUSIC ---", MARGIN, y_pos, TEXT_SIZE, text_color);
        y_pos += BOX_HEIGHT;

        /* Music status */
        if (music_loaded)
        {
            g_snprintf (info_text, sizeof (info_text),
                        "Status: %s  |  Loop: %s",
                        is_playing ? "Playing" : "Stopped/Paused",
                        is_looping ? "ON" : "OFF");
        }
        else
        {
            g_snprintf (info_text, sizeof (info_text), "Music not loaded (place music.ogg in current dir)");
        }
        grl_draw_text (info_text, MARGIN, y_pos, TEXT_SIZE, text_color);
        y_pos += BOX_HEIGHT;

        /* Progress bar */
        if (music_loaded)
        {
            g_snprintf (info_text, sizeof (info_text),
                        "Time: %.1f / %.1f sec", time_played, time_length);
            grl_draw_text (info_text, MARGIN, y_pos, TEXT_SIZE, text_color);
            y_pos += 25;
            grl_draw_rectangle (MARGIN, y_pos, width - MARGIN * 2, 15, bar_bg_color);
            grl_draw_rectangle (MARGIN, y_pos,
                                (gint)((width - MARGIN * 2) * progress), 15, bar_color);
            y_pos += BOX_HEIGHT;
        }
        else
        {
            y_pos += BOX_HEIGHT;
        }

        /* Pan indicator */
        g_snprintf (info_text, sizeof (info_text), "Pan: %.2f", music_pan);
        grl_draw_text (info_text, MARGIN, y_pos, TEXT_SIZE, text_color);
        y_pos += BOX_HEIGHT;

        /* Pitch indicator */
        g_snprintf (info_text, sizeof (info_text), "Pitch: %.2f", music_pitch);
        grl_draw_text (info_text, MARGIN, y_pos, TEXT_SIZE, text_color);
        y_pos += BOX_HEIGHT;

        /* Separator */
        y_pos += 10;
        grl_draw_text ("--- SOUND EFFECTS ---", MARGIN, y_pos, TEXT_SIZE, text_color);
        y_pos += BOX_HEIGHT;

        /* Sound status */
        if (sound_loaded)
        {
            g_snprintf (info_text, sizeof (info_text),
                        "Sound: %s",
                        grl_sound_is_playing (sound) ? "Playing" : "Ready");
        }
        else
        {
            g_snprintf (info_text, sizeof (info_text), "Sound not loaded (place test.wav in current dir)");
        }
        grl_draw_text (info_text, MARGIN, y_pos, TEXT_SIZE, text_color);
        y_pos += BOX_HEIGHT;

        /* Controls reminder */
        y_pos = height - MARGIN - 80;
        {
            g_autoptr(GrlColor) dim_color = grl_color_new (150, 150, 150, 255);

            grl_draw_text ("Controls:", MARGIN, y_pos, 16, dim_color);
            y_pos += 20;
            grl_draw_text ("SPACE=Play/Pause  S=Stop  E=Sound  L=Loop", MARGIN, y_pos, 16, dim_color);
            y_pos += 20;
            grl_draw_text ("UP/DOWN=Volume  LEFT/RIGHT=Pan  [/]=Pitch", MARGIN, y_pos, 16, dim_color);
        }

        /* Draw FPS */
        grl_draw_fps (width - 80, 10);

        grl_window_end_drawing (window);
    }

    /* Cleanup happens automatically via g_autoptr */
    g_print ("Audio demo finished\n");

    return 0;
}

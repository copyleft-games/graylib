/* sketch-template.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Minimal sketch template for the grl-preview-host live-reload tool.
 *
 * Usage:
 *   Copy this file, rename it (e.g. my-sketch.c), and run:
 *
 *     grl-preview-host my-sketch.c
 *
 *   The host compiles and loads your sketch as a shared object, calls
 *   sketch_draw() each frame, and hot-reloads whenever you save the file.
 *
 * Sketch ABI:
 *
 *   REQUIRED:
 *     void sketch_draw (GrlImage *canvas, double t);
 *
 *       Render one frame into @canvas at elapsed time @t (seconds).
 *       The canvas is an RGBA8 GrlImage; you may use any grl_image_draw_*
 *       primitive. Always call grl_image_clear_background() first unless
 *       you intentionally accumulate across frames.
 *
 *   OPTIONAL (all three must be present together or individually):
 *     void sketch_init (void);
 *
 *       Called once after the first successful load. Use to allocate
 *       persistent resources.
 *
 *     void *sketch_state (void);
 *
 *       Called by the host immediately before a hot-reload.  Return a
 *       pointer to your state that you want to survive the reload (e.g.
 *       a heap-allocated struct). The pointer is opaque to the host; it
 *       is passed back to sketch_reload().  Return NULL if you have no
 *       state to preserve.
 *
 *     void sketch_reload (void *state);
 *
 *       Called after a hot-reload instead of sketch_init(). @state is the
 *       value returned by sketch_state() from the previous version of the
 *       sketch. Restore your state from it. You are responsible for
 *       freeing the old state when you no longer need it.
 *
 * State preservation contract:
 *   - If sketch_state is present and returns non-NULL, sketch_reload is
 *     called with that pointer on the next successful reload.
 *   - If sketch_state is absent (or returns NULL) the host calls
 *     sketch_init instead.
 *   - If sketch_reload is absent the host falls back to sketch_init.
 *
 * IMPORTANT — in-process crash note:
 *   The sketch runs inside the host process. A null-pointer dereference,
 *   stack overflow, or any other fatal signal in sketch code will crash
 *   the entire host. Process isolation is future work.
 *
 * Build note:
 *   The host invokes cc with -shared -fPIC and links libgraylib + glib.
 *   Do NOT include a main() function in a sketch file.
 *
 * GObject memory note:
 *   GrlColor, GrlVector2, etc. are GBoxed types. Free them with
 *   grl_color_free() / grl_vector2_free(), NOT g_object_unref().
 *   Use g_autoptr() for automatic cleanup.
 */

/* The preview host compiles sketches with -DGRAYLIB_INSIDE -DGRAYLIB_COMPILATION,
 * which satisfies the umbrella header's direct-include guard. */
#include <graylib.h>
#include <math.h>

/* --------------------------------------------------------------------------
 * Optional persistent state.
 *
 * For a minimal sketch you do not need any of this. The example below uses
 * a simple struct to show the state preservation mechanism.
 * -------------------------------------------------------------------------- */

typedef struct
{
    double phase;      /* animation phase accumulator (radians) */
    int    frame;      /* total frame counter                   */
} SketchState;

static SketchState *g_state = NULL;

/* --------------------------------------------------------------------------
 * sketch_init — called once on first load
 * -------------------------------------------------------------------------- */

void
sketch_init (void)
{
    g_state = g_new0 (SketchState, 1);
    g_state->phase = 0.0;
    g_state->frame = 0;
}

/* --------------------------------------------------------------------------
 * sketch_state — snapshot state before a hot-reload
 * -------------------------------------------------------------------------- */

void *
sketch_state (void)
{
    /* Hand our heap block to the host; we will receive it back in
     * sketch_reload(). Return NULL if you have nothing to preserve. */
    return g_state;
}

/* --------------------------------------------------------------------------
 * sketch_reload — restore state after a hot-reload
 * -------------------------------------------------------------------------- */

void
sketch_reload (void *state)
{
    /* The old version's state block is handed back to us.
     * We can migrate it or just reuse it directly. */
    g_state = (SketchState *)state;
}

/* --------------------------------------------------------------------------
 * sketch_draw — render one frame (REQUIRED)
 *
 * @canvas: RGBA8 GrlImage to draw into. Same object every frame.
 * @t:      Elapsed time in seconds since the host started.
 * -------------------------------------------------------------------------- */

void
sketch_draw (GrlImage *canvas,
             double    t)
{
    int    w, h;
    double cx, cy;
    double orbit_r;
    double ball_x, ball_y;
    double pulse;
    int    ball_r;
    int    trail;

    /* Clear to a dark background each frame */
    {
        g_autoptr (GrlColor) bg = grl_color_new (20, 18, 30, 255);
        grl_image_clear_background (canvas, bg);
    }

    w = grl_image_get_width  (canvas);
    h = grl_image_get_height (canvas);
    cx = w * 0.5;
    cy = h * 0.5;

    /* Update state */
    if (g_state != NULL)
        g_state->frame++;

    /* Compute animation */
    orbit_r = (h < w ? h : w) * 0.28;
    ball_x  = cx + orbit_r * cos (t * 1.3);
    ball_y  = cy + orbit_r * sin (t * 1.3) * 0.6; /* elliptical */
    pulse   = 18.0 + 10.0 * sin (t * 3.7);
    ball_r  = (int)pulse;

    /* Draw faint orbit guide */
    {
        g_autoptr (GrlColor) guide = grl_color_new (60, 55, 80, 120);
        grl_image_draw_ellipse (canvas,
                                (int)cx, (int)cy,
                                (int)orbit_r, (int)(orbit_r * 0.6),
                                guide);
    }

    /* Draw trailing circles at half-step behind */
    for (trail = 3; trail >= 1; trail--)
    {
        double angle_offset = (double)trail * 0.18;
        double tx = cx + orbit_r * cos (t * 1.3 - angle_offset);
        double ty = cy + orbit_r * sin (t * 1.3 - angle_offset) * 0.6;
        guint8 alpha = (guint8)(40 + trail * 20);
        guint8 redness = (guint8)(120 + trail * 20);
        g_autoptr (GrlColor) tc = grl_color_new (redness, 80, 200, alpha);
        grl_image_draw_circle (canvas, (int)tx, (int)ty, ball_r - 4, tc);
    }

    /* Draw the main ball — vivid magenta-ish */
    {
        g_autoptr (GrlColor) ball = grl_color_new (220, 80, 255, 255);
        grl_image_draw_circle (canvas, (int)ball_x, (int)ball_y, ball_r, ball);
    }

    /* Draw a small crosshair at center */
    {
        g_autoptr (GrlColor) cross = grl_color_new (80, 80, 100, 180);
        grl_image_draw_line (canvas,
                             (int)cx - 12, (int)cy,
                             (int)cx + 12, (int)cy,
                             cross);
        grl_image_draw_line (canvas,
                             (int)cx, (int)cy - 12,
                             (int)cx, (int)cy + 12,
                             cross);
    }

    /* Label with elapsed time (bitmap font — works headless and windowed) */
    {
        char   buf[64];
        double whole_secs = (double)(int)t;
        int    millis     = (int)((t - whole_secs) * 1000.0);
        g_autoptr (GrlColor) label = grl_color_new (180, 180, 200, 220);

        g_snprintf (buf, sizeof (buf), "t=%.3fs", t);
        grl_image_draw_text_bitmap (canvas, buf, 8, 8, 10, label);

        if (g_state != NULL)
        {
            g_snprintf (buf, sizeof (buf), "frame %d", g_state->frame);
            grl_image_draw_text_bitmap (canvas, buf, 8, 22, 10, label);
            (void)whole_secs; (void)millis;
        }
    }
}

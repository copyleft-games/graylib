/* grl-image-accumulator.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Float-precision RGBA accumulation buffer for frame averaging.
 *
 * # Internals
 *
 * The accumulation buffer is a contiguous gfloat array of
 * (width * height * 4) elements laid out as (R, G, B, A) tuples in
 * row-major order.  All values are stored normalised to [0, 1]:
 *
 *   - In LINEAR mode each sRGB byte-component is converted to linear light
 *     via srgb_to_linear() before being multiplied by the frame weight.
 *   - In GAMMA mode the raw byte / 255.0 value is multiplied by the weight
 *     directly.
 *   - Alpha is always treated as a linear coverage quantity: byte / 255.0 × weight.
 *
 * resolve() divides by total_weight, converts back (linear->sRGB) if
 * needed, clamps to [0,1] and rounds to guint8.
 *
 * # Canonical motion-blur loop (headless)
 *
 * |[<!-- language="C" -->
 * GrlImageAccumulator *acc = grl_image_accumulator_new (320, 240, TRUE);
 * GrlGifWriter *gif = grl_gif_writer_new ("blur.gif", 320, 240, 0, NULL);
 *
 * // For each output frame:
 * grl_image_accumulator_reset (acc);
 * for (n = 0; n < N_SAMPLES; n++)
 * {
 *     GrlImage *sub = render_sub_frame (t + (n / (gfloat)N_SAMPLES) * delta);
 *     grl_image_accumulator_add (acc, sub, 1.0f);
 *     g_object_unref (sub);
 * }
 * {
 *     GrlImage *avg = grl_image_accumulator_resolve (acc);
 *     grl_gif_writer_add_frame (gif, avg, 4, NULL);
 *     g_object_unref (avg);
 * }
 *
 * grl_gif_writer_close (gif, NULL);
 * g_object_unref (gif);
 * g_object_unref (acc);
 * ]|
 *
 * With N_SAMPLES = 1 the output is identical to the single sub-frame (no
 * blur).  Increasing N_SAMPLES trades render time for smoother motion.
 */

#include "config.h"
#include "grl-image-accumulator.h"
#include "grl-image.h"
#include "../math/grl-color.h"
#include <math.h>
#include <string.h>

struct _GrlImageAccumulator
{
    GObject  parent_instance;

    gint     width;
    gint     height;
    gboolean linear;       /* TRUE = sRGB<->linear conversion on accumulate/resolve */

    gfloat  *buf;          /* width * height * 4 floats (R,G,B,A), normalised [0,1] */
    gfloat   total_weight; /* sum of all weights added so far */
};

G_DEFINE_FINAL_TYPE (GrlImageAccumulator, grl_image_accumulator, G_TYPE_OBJECT)

/* ---------------------------------------------------------------------------
 * sRGB <-> linear helpers (piecewise IEC 61966-2-1)
 * --------------------------------------------------------------------------- */

/*
 * srgb_to_linear:
 * @c: A value in [0, 1] representing an sRGB-encoded channel.
 *
 * Returns the linearised light value in [0, 1].
 */
static gfloat
srgb_to_linear (gfloat c)
{
    if (c <= 0.04045f)
        return c / 12.92f;
    return powf ((c + 0.055f) / 1.055f, 2.4f);
}

/*
 * linear_to_srgb:
 * @c: A linear light value in [0, 1].
 *
 * Returns the sRGB-encoded value in [0, 1].
 */
static gfloat
linear_to_srgb (gfloat c)
{
    if (c <= 0.0031308f)
        return 12.92f * c;
    return 1.055f * powf (c, 1.0f / 2.4f) - 0.055f;
}

/* ---------------------------------------------------------------------------
 * GObject boilerplate
 * --------------------------------------------------------------------------- */

static void
grl_image_accumulator_finalize (GObject *object)
{
    GrlImageAccumulator *self = GRL_IMAGE_ACCUMULATOR (object);

    g_free (self->buf);
    self->buf = NULL;

    G_OBJECT_CLASS (grl_image_accumulator_parent_class)->finalize (object);
}

static void
grl_image_accumulator_class_init (GrlImageAccumulatorClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = grl_image_accumulator_finalize;
}

static void
grl_image_accumulator_init (GrlImageAccumulator *self)
{
    self->width        = 0;
    self->height       = 0;
    self->linear       = FALSE;
    self->buf          = NULL;
    self->total_weight = 0.0f;
}

/* ---------------------------------------------------------------------------
 * Public API
 * --------------------------------------------------------------------------- */

/**
 * grl_image_accumulator_new:
 * @width: Canvas width in pixels.
 * @height: Canvas height in pixels.
 * @linear: %TRUE to accumulate in linear light.
 *
 * Creates a new #GrlImageAccumulator.
 *
 * Returns: (transfer full): A new #GrlImageAccumulator.
 */
GrlImageAccumulator *
grl_image_accumulator_new (gint     width,
                            gint     height,
                            gboolean linear)
{
    GrlImageAccumulator *self;
    gsize                n;

    g_return_val_if_fail (width > 0, NULL);
    g_return_val_if_fail (height > 0, NULL);

    self = g_object_new (GRL_TYPE_IMAGE_ACCUMULATOR, NULL);
    self->width  = width;
    self->height = height;
    self->linear = linear;

    n        = (gsize)width * (gsize)height * 4u;
    self->buf = g_new0 (gfloat, n);

    return self;
}

/**
 * grl_image_accumulator_add:
 * @self: A #GrlImageAccumulator.
 * @frame: The source image.
 * @weight: Contribution weight for this sample.
 *
 * Adds @frame × @weight to the accumulation buffer.
 */
void
grl_image_accumulator_add (GrlImageAccumulator *self,
                            GrlImage            *frame,
                            gfloat               weight)
{
    GrlImage *src;   /* possibly a resampled copy */
    gboolean  need_unref;
    gint      x;
    gint      y;
    gint      w;
    gint      h;
    gint      idx;

    g_return_if_fail (GRL_IS_IMAGE_ACCUMULATOR (self));
    g_return_if_fail (GRL_IS_IMAGE (frame));

    w = grl_image_get_width (frame);
    h = grl_image_get_height (frame);

    /* Resample to canvas size if dimensions differ. */
    if (w != self->width || h != self->height)
    {
        src        = grl_image_scaled_nearest (frame, self->width, self->height);
        need_unref = TRUE;
    }
    else
    {
        src        = frame;
        need_unref = FALSE;
    }

    /* Accumulate pixels. */
    for (y = 0; y < self->height; y++)
    {
        for (x = 0; x < self->width; x++)
        {
            g_autoptr(GrlColor) c  = grl_image_get_pixel (src, x, y);
            gfloat              fr = (gfloat)c->r / 255.0f;
            gfloat              fg = (gfloat)c->g / 255.0f;
            gfloat              fb = (gfloat)c->b / 255.0f;
            gfloat              fa = (gfloat)c->a / 255.0f;

            if (self->linear)
            {
                fr = srgb_to_linear (fr);
                fg = srgb_to_linear (fg);
                fb = srgb_to_linear (fb);
                /* alpha is linear coverage — no conversion */
            }

            idx = (y * self->width + x) * 4;
            self->buf[idx + 0] += fr * weight;
            self->buf[idx + 1] += fg * weight;
            self->buf[idx + 2] += fb * weight;
            self->buf[idx + 3] += fa * weight;
        }
    }

    self->total_weight += weight;

    if (need_unref)
        g_object_unref (src);
}

/**
 * grl_image_accumulator_resolve:
 * @self: A #GrlImageAccumulator.
 *
 * Produces the weighted-average output image, or %NULL if no samples have
 * been added yet.
 *
 * Returns: (transfer full) (nullable): A new #GrlImage, or %NULL.
 */
GrlImage *
grl_image_accumulator_resolve (GrlImageAccumulator *self)
{
    g_autoptr(GrlColor) transparent = NULL;
    GrlImage           *result;
    gfloat              inv_w;
    gint                x;
    gint                y;
    gint                idx;

    g_return_val_if_fail (GRL_IS_IMAGE_ACCUMULATOR (self), NULL);

    if (self->total_weight <= 0.0f)
        return NULL;

    inv_w = 1.0f / self->total_weight;

    transparent = grl_color_new (0, 0, 0, 0);
    result      = grl_image_new_color (self->width, self->height, transparent);

    for (y = 0; y < self->height; y++)
    {
        for (x = 0; x < self->width; x++)
        {
            g_autoptr(GrlColor) c  = NULL;
            gfloat              fr;
            gfloat              fg;
            gfloat              fb;
            gfloat              fa;
            guint8              r;
            guint8              g_ch;
            guint8              b;
            guint8              a;

            idx = (y * self->width + x) * 4;
            fr  = self->buf[idx + 0] * inv_w;
            fg  = self->buf[idx + 1] * inv_w;
            fb  = self->buf[idx + 2] * inv_w;
            fa  = self->buf[idx + 3] * inv_w;

            if (self->linear)
            {
                fr = linear_to_srgb (fr);
                fg = linear_to_srgb (fg);
                fb = linear_to_srgb (fb);
                /* alpha remains linear */
            }

            /* Clamp and convert to guint8. */
            r    = (guint8)(CLAMP (fr * 255.0f + 0.5f, 0.0f, 255.0f));
            g_ch = (guint8)(CLAMP (fg * 255.0f + 0.5f, 0.0f, 255.0f));
            b    = (guint8)(CLAMP (fb * 255.0f + 0.5f, 0.0f, 255.0f));
            a    = (guint8)(CLAMP (fa * 255.0f + 0.5f, 0.0f, 255.0f));

            c = grl_color_new (r, g_ch, b, a);
            grl_image_draw_pixel (result, x, y, c);
        }
    }

    return result;
}

/**
 * grl_image_accumulator_reset:
 * @self: A #GrlImageAccumulator.
 *
 * Zeroes the accumulation buffer and resets the total weight.
 */
void
grl_image_accumulator_reset (GrlImageAccumulator *self)
{
    gsize n;

    g_return_if_fail (GRL_IS_IMAGE_ACCUMULATOR (self));

    n = (gsize)self->width * (gsize)self->height * 4u;
    memset (self->buf, 0, n * sizeof (gfloat));
    self->total_weight = 0.0f;
}

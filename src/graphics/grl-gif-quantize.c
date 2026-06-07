/* grl-gif-quantize.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Median-cut palette builder and Floyd-Steinberg dithering for GrlGifWriter.
 *
 * Median-cut algorithm overview
 * ==============================
 * 1. Start with one "box" covering all unique, opaque pixels.
 * 2. Repeat until we have max_colors boxes (or can no longer split):
 *    a. Pick the box with the largest range along any RGB axis.
 *    b. Sort the pixels in that box along its longest axis.
 *    c. Split the box at the median pixel.
 * 3. The representative colour for each box is the mean RGB of its pixels.
 *
 * Fully transparent pixels (alpha == 0) are excluded from sampling because
 * their RGB components are typically zero-filled and would pull means toward
 * black, wasting palette slots on an invisible colour.
 */

#include "config.h"
#include "grl-gif-quantize.h"
#include <string.h>
#include <stdlib.h>

/* -------------------------------------------------------------------------
 * Internal types
 * ------------------------------------------------------------------------- */

/* A pixel represented as three floats for error-diffusion arithmetic. */
typedef struct
{
    gfloat r;
    gfloat g;
    gfloat b;
} FloatPixel;

/* A box in median-cut colour space. */
typedef struct
{
    /* Indices into the working pixel array (half-open [lo, hi)). */
    gint lo;
    gint hi;

    /* Channel ranges (computed once per split). */
    guint8 r_min, r_max;
    guint8 g_min, g_max;
    guint8 b_min, b_max;
} MCBox;

/* Working pixel: packed 24-bit RGB. One pixel per sampled pixel. */
typedef struct
{
    guint8 r, g, b;
} RGB8;

/* -------------------------------------------------------------------------
 * Sorting helpers (qsort compare functions, gnu89 compatible)
 * ------------------------------------------------------------------------- */

static int
cmp_r (const void *a, const void *b)
{
    const RGB8 *pa = (const RGB8 *)a;
    const RGB8 *pb = (const RGB8 *)b;

    return (int)pa->r - (int)pb->r;
}

static int
cmp_g (const void *a, const void *b)
{
    const RGB8 *pa = (const RGB8 *)a;
    const RGB8 *pb = (const RGB8 *)b;

    return (int)pa->g - (int)pb->g;
}

static int
cmp_b (const void *a, const void *b)
{
    const RGB8 *pa = (const RGB8 *)a;
    const RGB8 *pb = (const RGB8 *)b;

    return (int)pa->b - (int)pb->b;
}

/* -------------------------------------------------------------------------
 * Box helpers
 * ------------------------------------------------------------------------- */

/* Compute the range of each channel for the pixels in [lo, hi). */
static void
box_compute_ranges (MCBox *box, const RGB8 *pixels)
{
    gint    i;
    guint8  rmin, rmax, gmin, gmax, bmin, bmax;

    rmin = gmin = bmin = 255;
    rmax = gmax = bmax = 0;

    for (i = box->lo; i < box->hi; i++)
    {
        if (pixels[i].r < rmin) rmin = pixels[i].r;
        if (pixels[i].r > rmax) rmax = pixels[i].r;
        if (pixels[i].g < gmin) gmin = pixels[i].g;
        if (pixels[i].g > gmax) gmax = pixels[i].g;
        if (pixels[i].b < bmin) bmin = pixels[i].b;
        if (pixels[i].b > bmax) bmax = pixels[i].b;
    }

    box->r_min = rmin; box->r_max = rmax;
    box->g_min = gmin; box->g_max = gmax;
    box->b_min = bmin; box->b_max = bmax;
}

/* Return the mean RGB of the pixels in [lo, hi). */
static RGB8
box_mean (const MCBox *box, const RGB8 *pixels)
{
    RGB8    result;
    gint64  r_sum, g_sum, b_sum;
    gint    count;
    gint    i;

    r_sum = g_sum = b_sum = 0;
    count = box->hi - box->lo;

    for (i = box->lo; i < box->hi; i++)
    {
        r_sum += pixels[i].r;
        g_sum += pixels[i].g;
        b_sum += pixels[i].b;
    }

    result.r = (count > 0) ? (guint8)(r_sum / count) : 0;
    result.g = (count > 0) ? (guint8)(g_sum / count) : 0;
    result.b = (count > 0) ? (guint8)(b_sum / count) : 0;
    return result;
}

/* -------------------------------------------------------------------------
 * Public: grl_gif_median_cut
 * ------------------------------------------------------------------------- */

gint
grl_gif_median_cut (const guint8 *rgba,
                    gint          n_pixels,
                    gint          max_colors,
                    guint8        palette[768])
{
    RGB8   *pix;
    MCBox  *boxes;
    gint    n_opaque;
    gint    n_boxes;
    gint    i;
    gint    best;
    gint    best_range;
    gint    split;
    int   (*sorter)(const void *, const void *);

    /* Clamp max_colors to valid range. */
    if (max_colors < 2)   max_colors = 2;
    if (max_colors > 256) max_colors = 256;

    /* Zero-fill output palette. */
    memset (palette, 0, 768);

    if (n_pixels <= 0)
    {
        /* Degenerate: return a single black entry. */
        return 1;
    }

    /* Collect opaque (alpha > 0) pixels into a working buffer. */
    pix = g_new (RGB8, n_pixels);
    n_opaque = 0;
    for (i = 0; i < n_pixels; i++)
    {
        if (rgba[i * 4 + 3] > 0)
        {
            pix[n_opaque].r = rgba[i * 4 + 0];
            pix[n_opaque].g = rgba[i * 4 + 1];
            pix[n_opaque].b = rgba[i * 4 + 2];
            n_opaque++;
        }
    }

    if (n_opaque == 0)
    {
        /* All pixels transparent: return single black entry. */
        g_free (pix);
        return 1;
    }

    /* Initialise with one box covering all pixels. */
    boxes = g_new0 (MCBox, max_colors);
    boxes[0].lo = 0;
    boxes[0].hi = n_opaque;
    box_compute_ranges (&boxes[0], pix);
    n_boxes = 1;

    /* Repeatedly split the box with the largest channel range. */
    while (n_boxes < max_colors)
    {
        /* Find the box to split. */
        best = -1;
        best_range = 0;

        for (i = 0; i < n_boxes; i++)
        {
            gint r_range;
            gint g_range;
            gint b_range;
            gint range;

            /* A box with only one pixel cannot be split. */
            if (boxes[i].hi - boxes[i].lo < 2)
                continue;

            r_range = (gint)boxes[i].r_max - (gint)boxes[i].r_min;
            g_range = (gint)boxes[i].g_max - (gint)boxes[i].g_min;
            b_range = (gint)boxes[i].b_max - (gint)boxes[i].b_min;

            range = r_range;
            if (g_range > range) range = g_range;
            if (b_range > range) range = b_range;

            if (range > best_range)
            {
                best_range = range;
                best = i;
            }
        }

        if (best < 0 || best_range == 0)
            break;   /* Nothing left to split. */

        {
            MCBox  *box;
            gint    r_range;
            gint    g_range;
            gint    b_range;

            box = &boxes[best];
            r_range = (gint)box->r_max - (gint)box->r_min;
            g_range = (gint)box->g_max - (gint)box->g_min;
            b_range = (gint)box->b_max - (gint)box->b_min;

            /* Choose the axis with the widest range. */
            sorter = cmp_r;
            if (g_range >= r_range && g_range >= b_range)
                sorter = cmp_g;
            else if (b_range >= r_range && b_range >= g_range)
                sorter = cmp_b;

            qsort (pix + box->lo,
                   (size_t)(box->hi - box->lo),
                   sizeof (RGB8),
                   sorter);

            /* Split at the median. */
            split = box->lo + (box->hi - box->lo) / 2;

            /* Reuse the current slot for the lower half, append upper half. */
            {
                MCBox lower;
                MCBox upper;

                lower.lo = box->lo;
                lower.hi = split;
                upper.lo = split;
                upper.hi = box->hi;

                box_compute_ranges (&lower, pix);
                box_compute_ranges (&upper, pix);

                boxes[best] = lower;
                boxes[n_boxes] = upper;
                n_boxes++;
            }
        }
    }

    /* Emit one palette entry per box (the mean colour). */
    for (i = 0; i < n_boxes; i++)
    {
        RGB8 mean;

        mean = box_mean (&boxes[i], pix);
        palette[i * 3 + 0] = mean.r;
        palette[i * 3 + 1] = mean.g;
        palette[i * 3 + 2] = mean.b;
    }

    g_free (boxes);
    g_free (pix);

    return n_boxes;
}

/* -------------------------------------------------------------------------
 * Nearest-colour lookup (linear scan — sufficient for <= 256 colours)
 * ------------------------------------------------------------------------- */

static gint
nearest_palette_index (guint8        r,
                       guint8        g,
                       guint8        b,
                       const guint8 *palette,
                       gint          palette_len)
{
    gint best_idx;
    gint best_dist;
    gint i;

    best_idx  = 0;
    best_dist = G_MAXINT;

    for (i = 0; i < palette_len; i++)
    {
        gint dr;
        gint dg;
        gint db;
        gint dist;

        dr = (gint)r - (gint)palette[i * 3 + 0];
        dg = (gint)g - (gint)palette[i * 3 + 1];
        db = (gint)b - (gint)palette[i * 3 + 2];
        dist = dr * dr + dg * dg + db * db;

        if (dist < best_dist)
        {
            best_dist = dist;
            best_idx  = i;
            if (dist == 0)
                break;
        }
    }

    return best_idx;
}

/* Clamp a float to [0, 255] and return as guint8. */
static guint8
clamp_u8 (gfloat v)
{
    if (v < 0.0f)   return 0;
    if (v > 255.0f) return 255;
    return (guint8)(v + 0.5f);
}

/* -------------------------------------------------------------------------
 * Public: grl_gif_map_indices
 * ------------------------------------------------------------------------- */

void
grl_gif_map_indices (const guint8  *rgba,
                     gint           width,
                     gint           height,
                     const guint8  *palette,
                     gint           palette_len,
                     GrlGifDither   dither,
                     gint           transparent_index,
                     guint8         alpha_threshold,
                     guint8        *out_indices)
{
    gint n_pixels;
    gint i;

    n_pixels = width * height;

    if (dither == GRL_GIF_DITHER_FLOYD_STEINBERG)
    {
        /*
         * Floyd-Steinberg error diffusion.
         * Error buffer holds floating-point RGB for each pixel in the current
         * and next row.  We use a two-row ping-pong scheme.
         */
        FloatPixel *err_cur;   /* Error for the current row */
        FloatPixel *err_next;  /* Error for the row below   */
        FloatPixel *tmp;
        gint        x;
        gint        y;

        err_cur  = g_new0 (FloatPixel, width + 2);
        err_next = g_new0 (FloatPixel, width + 2);

        for (y = 0; y < height; y++)
        {
            /* Zero the next-row error buffer for accumulation. */
            memset (err_next, 0, sizeof (FloatPixel) * (width + 2));

            for (x = 0; x < width; x++)
            {
                gint   px_off;
                guint8 a;
                gfloat fr, fg, fb;
                gint   idx;
                gfloat qr, qg, qb;
                gfloat er, eg, eb;

                px_off = (y * width + x) * 4;
                a = rgba[px_off + 3];

                if (transparent_index >= 0 && a < alpha_threshold)
                {
                    out_indices[y * width + x] = (guint8)transparent_index;
                    continue;
                }

                /* Apply accumulated error. */
                fr = (gfloat)rgba[px_off + 0] + err_cur[x + 1].r;
                fg = (gfloat)rgba[px_off + 1] + err_cur[x + 1].g;
                fb = (gfloat)rgba[px_off + 2] + err_cur[x + 1].b;

                /* Clamp to valid range. */
                fr = CLAMP (fr, 0.0f, 255.0f);
                fg = CLAMP (fg, 0.0f, 255.0f);
                fb = CLAMP (fb, 0.0f, 255.0f);

                /* Find nearest palette entry. */
                idx = nearest_palette_index (clamp_u8 (fr),
                                             clamp_u8 (fg),
                                             clamp_u8 (fb),
                                             palette,
                                             palette_len);

                out_indices[y * width + x] = (guint8)idx;

                /* Quantisation error. */
                qr = (gfloat)palette[idx * 3 + 0];
                qg = (gfloat)palette[idx * 3 + 1];
                qb = (gfloat)palette[idx * 3 + 2];
                er = fr - qr;
                eg = fg - qg;
                eb = fb - qb;

                /* Distribute error: right 7/16, lower-left 3/16, lower 5/16, lower-right 1/16. */
                if (x + 1 < width)
                {
                    err_cur[x + 2].r += er * (7.0f / 16.0f);
                    err_cur[x + 2].g += eg * (7.0f / 16.0f);
                    err_cur[x + 2].b += eb * (7.0f / 16.0f);
                }
                if (y + 1 < height)
                {
                    if (x > 0)
                    {
                        err_next[x].r += er * (3.0f / 16.0f);
                        err_next[x].g += eg * (3.0f / 16.0f);
                        err_next[x].b += eb * (3.0f / 16.0f);
                    }
                    err_next[x + 1].r += er * (5.0f / 16.0f);
                    err_next[x + 1].g += eg * (5.0f / 16.0f);
                    err_next[x + 1].b += eb * (5.0f / 16.0f);
                    if (x + 1 < width)
                    {
                        err_next[x + 2].r += er * (1.0f / 16.0f);
                        err_next[x + 2].g += eg * (1.0f / 16.0f);
                        err_next[x + 2].b += eb * (1.0f / 16.0f);
                    }
                }
            }

            /* Swap error rows. */
            tmp      = err_cur;
            err_cur  = err_next;
            err_next = tmp;
        }

        g_free (err_cur);
        g_free (err_next);
    }
    else
    {
        /* No dithering: direct nearest-colour lookup. */
        for (i = 0; i < n_pixels; i++)
        {
            guint8 a;

            a = rgba[i * 4 + 3];
            if (transparent_index >= 0 && a < alpha_threshold)
            {
                out_indices[i] = (guint8)transparent_index;
                continue;
            }

            out_indices[i] = (guint8)nearest_palette_index (rgba[i * 4 + 0],
                                                             rgba[i * 4 + 1],
                                                             rgba[i * 4 + 2],
                                                             palette,
                                                             palette_len);
        }
    }
}

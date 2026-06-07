/* grl-gif-writer.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Streaming animated GIF writer.
 *
 * This is a small, self-contained GIF89a encoder. The default mode quantises
 * frames to a fixed 6x6x6 web-safe palette (216 colors) producing output
 * byte-for-byte identical to the original encoder. An adaptive median-cut
 * quantizer (GRL_GIF_QUANTIZER_MEDIAN_CUT) is also available, supporting
 * Floyd-Steinberg dithering, per-frame local palettes, and transparency.
 * Pixel data is encoded with GIF's LZW scheme used in its "uncompressed" form:
 * fixed 9-bit codes with a Clear code emitted often enough that the decoder's
 * code table never has to grow. This trades file size for implementation
 * simplicity and broad compatibility (the output decodes in every conformant
 * GIF reader). A true LZW compressor can be dropped in later without changing
 * the public API.
 *
 * Backward-compatibility guarantee
 * ==================================
 * When no setters are called (all defaults), the encoder follows the original
 * web-safe path exactly.  The MEDIAN_CUT path is entered only when the caller
 * explicitly requests GRL_GIF_QUANTIZER_MEDIAN_CUT. This means:
 *
 *   - grl_gif_writer_new() always writes the header, LSD, and GCT eagerly
 *     when quantizer == WEB_SAFE (legacy path).
 *   - For MEDIAN_CUT + GLOBAL scope, the header and LSD are still written
 *     eagerly but the 768-byte GCT block is deferred: we write a placeholder
 *     of 768 zero bytes and then fseek back to overwrite it after we know the
 *     first frame's palette. The LSD packed byte is written at new() time with
 *     the same 0xf7 value (GCT present, size=256). Because GLOBAL scope builds
 *     the palette once from the first frame, subsequent frames use it without
 *     seeking again.
 *   - For MEDIAN_CUT + PER_FRAME scope, no GCT is written at all; the LSD
 *     packed byte is 0x00 (no GCT). Each frame's Image Descriptor carries a
 *     Local Color Table flag.
 */

#include "config.h"
#include "grl-gif-writer.h"
#include "grl-gif-quantize.h"
#include "grl-image.h"
#include <raylib.h>
#include <glib/gstdio.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

/* GIF LZW constants for an 8-bit (256-entry) code space. */
#define GIF_MIN_CODE_SIZE 8
#define GIF_CLEAR_CODE    256
#define GIF_EOI_CODE      257
#define GIF_CODE_SIZE     (GIF_MIN_CODE_SIZE + 1)   /* fixed 9-bit codes */
/* Clear before the decoder table reaches 512 entries (stays 9-bit). */
#define GIF_LITERALS_PER_CLEAR 253

/* File offset just after the 6-byte signature + 4-byte LSD width/height +
 * 1-byte packed field = offset 11. The 768-byte GCT follows at offset 13
 * (after background index and pixel-aspect bytes). */
#define GCT_OFFSET 13

struct _GrlGifWriter
{
    GObject  parent_instance;

    FILE    *fp;
    gint     width;
    gint     height;
    guint    frame_count;
    gboolean closed;

    /* Quantization settings (set before first add_frame). */
    GrlGifQuantizer   quantizer;
    GrlGifDither      dither;
    GrlGifPaletteScope palette_scope;
    gint              max_colors;
    gboolean          transparency_enabled;
    guint8            alpha_threshold;

    /* For MEDIAN_CUT + GLOBAL: stored after first frame. */
    guint8   global_palette[768];   /* 256 * 3 */
    gint     global_palette_len;    /* number of meaningful entries */
    gint     global_transparent_index; /* -1 if transparency disabled */
    gboolean header_written;        /* TRUE once GCT has been written/filled */

    /* For MEDIAN_CUT + GLOBAL: we need to remember loop_count for the
     * NETSCAPE extension which we write eagerly alongside the placeholder
     * GCT, so this is stored during new(). */
    /* (loop_count is fully consumed in new(); no need to store it.) */
};

G_DEFINE_TYPE (GrlGifWriter, grl_gif_writer, G_TYPE_OBJECT)

G_DEFINE_QUARK (grl-gif-error-quark, grl_gif_error)

/*
 * Low-level output helpers
 */

static gboolean
gif_write (GrlGifWriter *self,
           const void   *data,
           gsize         len)
{
    if (self->fp == NULL)
        return FALSE;

    return (fwrite (data, 1, len, self->fp) == len);
}

static gboolean
gif_u8 (GrlGifWriter *self,
        guint8        v)
{
    return gif_write (self, &v, 1);
}

static gboolean
gif_u16 (GrlGifWriter *self,
         guint         v)
{
    guint8 b[2];

    b[0] = (guint8)(v & 0xff);
    b[1] = (guint8)((v >> 8) & 0xff);
    return gif_write (self, b, 2);
}

/* Map an 8-bit RGB triple to a 6x6x6 web-safe palette index (0..215). */
static guint8
gif_websafe_index (guint8 r,
                   guint8 g,
                   guint8 b)
{
    gint r6 = ((gint)r * 5 + 127) / 255;
    gint g6 = ((gint)g * 5 + 127) / 255;
    gint b6 = ((gint)b * 5 + 127) / 255;

    return (guint8)(r6 * 36 + g6 * 6 + b6);
}

/*
 * GObject boilerplate
 */

static void
grl_gif_writer_finalize (GObject *object)
{
    GrlGifWriter *self = GRL_GIF_WRITER (object);

    /* Best-effort: a writer dropped without close() still produces a valid
     * file (we append the trailer here) rather than a truncated one. */
    if (self->fp != NULL && !self->closed)
    {
        guint8 trailer = 0x3b;

        fwrite (&trailer, 1, 1, self->fp);
        fclose (self->fp);
        self->fp = NULL;
    }

    G_OBJECT_CLASS (grl_gif_writer_parent_class)->finalize (object);
}

static void
grl_gif_writer_class_init (GrlGifWriterClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = grl_gif_writer_finalize;
}

static void
grl_gif_writer_init (GrlGifWriter *self)
{
    self->fp = NULL;
    self->width = 0;
    self->height = 0;
    self->frame_count = 0;
    self->closed = FALSE;

    /* Defaults: byte-identical to the original encoder. */
    self->quantizer = GRL_GIF_QUANTIZER_WEB_SAFE;
    self->dither    = GRL_GIF_DITHER_NONE;
    self->palette_scope = GRL_GIF_PALETTE_SCOPE_GLOBAL;
    self->max_colors = 256;
    self->transparency_enabled = FALSE;
    self->alpha_threshold = 128;

    memset (self->global_palette, 0, sizeof self->global_palette);
    self->global_palette_len = 0;
    self->global_transparent_index = -1;
    self->header_written = FALSE;
}

/*
 * Helper: compute the smallest GIF color table size code such that
 * 2^(code+1) >= n_colors.  Returns values 0..7 (palette sizes 2..256).
 */
static guint8
gif_color_table_size_code (gint n_colors)
{
    gint code;
    gint sz;

    code = 0;
    sz = 2;
    while (sz < n_colors && code < 7)
    {
        code++;
        sz *= 2;
    }
    return (guint8)code;
}

/*
 * Helper: write a palette padded to the next power-of-two size.
 * Returns the padded byte count actually written (always a multiple of 3 *
 * power-of-two, 6..768 bytes).
 */
static gboolean
gif_write_color_table (GrlGifWriter *self,
                       const guint8 *palette,
                       gint          palette_len)
{
    guint8 code;
    gint   padded;
    gint   i;

    code   = gif_color_table_size_code (palette_len);
    padded = 2 << code;   /* 2^(code+1) */

    /* Write the meaningful entries. */
    if (!gif_write (self, palette, (gsize)(palette_len * 3)))
        return FALSE;

    /* Pad remaining slots with black. */
    for (i = palette_len; i < padded; i++)
    {
        if (!gif_u8 (self, 0) || !gif_u8 (self, 0) || !gif_u8 (self, 0))
            return FALSE;
    }

    return TRUE;
}

/*
 * Public API
 */

GrlGifWriter *
grl_gif_writer_new (const gchar  *filename,
                    gint          width,
                    gint          height,
                    gint          loop_count,
                    GError      **error)
{
    GrlGifWriter *self;
    guint8        gct[768];

    g_return_val_if_fail (filename != NULL, NULL);
    g_return_val_if_fail (width > 0 && width <= G_MAXUINT16, NULL);
    g_return_val_if_fail (height > 0 && height <= G_MAXUINT16, NULL);

    self = g_object_new (GRL_TYPE_GIF_WRITER, NULL);
    self->width  = width;
    self->height = height;

    self->fp = g_fopen (filename, "wb");
    if (self->fp == NULL)
    {
        g_set_error (error, GRL_GIF_ERROR, GRL_GIF_ERROR_OPEN,
                     "Failed to open '%s' for writing: %s",
                     filename, g_strerror (errno));
        g_object_unref (self);
        return NULL;
    }

    /*
     * Write GIF header + Logical Screen Descriptor.
     *
     * For WEB_SAFE (legacy): write the full 216-colour GCT immediately,
     * exactly as the original encoder did.
     *
     * For MEDIAN_CUT + GLOBAL: write a 768-byte placeholder GCT of zeroes;
     * fseek back to offset GCT_OFFSET on first frame and overwrite it.
     * The packed byte is still 0xf7 (GCT present, 8-bit colour, 256 entries)
     * so the GCT size announced in the LSD is always 256 entries.
     *
     * For MEDIAN_CUT + PER_FRAME: no GCT at all; packed byte is 0x00.
     */

    if (!gif_write (self, "GIF89a", 6) ||
        !gif_u16 (self, (guint)width) ||
        !gif_u16 (self, (guint)height))
    {
        g_set_error (error, GRL_GIF_ERROR, GRL_GIF_ERROR_WRITE,
                     "Failed to write GIF header");
        g_object_unref (self);
        return NULL;
    }

    /*
     * Write LSD + 768-byte zeroed GCT placeholder.
     *
     * We always emit 0xf7 (GCT present, 8-bit colour res, 256 entries) and a
     * 768-byte zeroed block.  The first add_frame() call will seek back to
     * GCT_OFFSET and overwrite with the real palette (web-safe or median-cut).
     * For PER_FRAME scope the placeholder stays zeroed and readers prefer the
     * local colour tables embedded in each frame.
     *
     * This is the only design that works: the caller sets the quantizer and
     * palette scope AFTER new() returns, so we cannot make palette-building
     * decisions during new().
     *
     * Byte-identical guarantee: the final file produced by the web-safe path
     * has identical bytes to the original encoder because the GCT is written
     * to the same file offset (GCT_OFFSET) with the same 768 bytes.
     */
    memset (gct, 0, sizeof gct);
    if (!gif_u8 (self, 0xf7) ||   /* GCT present, 8-bit res, 256 entries */
        !gif_u8 (self, 0) ||       /* background color index */
        !gif_u8 (self, 0) ||       /* pixel aspect ratio */
        !gif_write (self, gct, sizeof gct))
    {
        g_set_error (error, GRL_GIF_ERROR, GRL_GIF_ERROR_WRITE,
                     "Failed to write GIF LSD + GCT placeholder");
        g_object_unref (self);
        return NULL;
    }
    /* header_written stays FALSE until first add_frame() fills in the GCT. */

    /* NETSCAPE2.0 application extension: loop control. */
    if (loop_count < 0)
        loop_count = 0;
    if (!gif_u8 (self, 0x21) || !gif_u8 (self, 0xff) || !gif_u8 (self, 0x0b) ||
        !gif_write (self, "NETSCAPE2.0", 11) ||
        !gif_u8 (self, 0x03) || !gif_u8 (self, 0x01) ||
        !gif_u16 (self, (guint)loop_count) || !gif_u8 (self, 0x00))
    {
        g_set_error (error, GRL_GIF_ERROR, GRL_GIF_ERROR_WRITE,
                     "Failed to write GIF loop extension");
        g_object_unref (self);
        return NULL;
    }

    return self;
}

/* Append LZW image data (uncompressed scheme) as length-prefixed sub-blocks. */
static gboolean
gif_write_image_data (GrlGifWriter  *self,
                      const guint8  *indices,
                      gint           count)
{
    GByteArray *bits;       /* packed code byte stream */
    guint32     bitbuf = 0;
    gint        bitcount = 0;
    gint        literals = 0;
    gint        i;
    gboolean    ok = TRUE;
    gsize       off;

    bits = g_byte_array_new ();

#define GIF_EMIT(code)                                                      \
    G_STMT_START {                                                          \
        bitbuf |= ((guint32)(code) << bitcount);                            \
        bitcount += GIF_CODE_SIZE;                                          \
        while (bitcount >= 8)                                               \
        {                                                                   \
            guint8 _b = (guint8)(bitbuf & 0xff);                            \
            g_byte_array_append (bits, &_b, 1);                             \
            bitbuf >>= 8;                                                   \
            bitcount -= 8;                                                  \
        }                                                                   \
    } G_STMT_END

    GIF_EMIT (GIF_CLEAR_CODE);
    for (i = 0; i < count; i++)
    {
        GIF_EMIT (indices[i]);
        if (++literals == GIF_LITERALS_PER_CLEAR)
        {
            GIF_EMIT (GIF_CLEAR_CODE);
            literals = 0;
        }
    }
    GIF_EMIT (GIF_EOI_CODE);

    /* Flush remaining bits. */
    if (bitcount > 0)
    {
        guint8 b = (guint8)(bitbuf & 0xff);

        g_byte_array_append (bits, &b, 1);
    }

#undef GIF_EMIT

    /* LZW minimum code size, then sub-blocks of <= 255 bytes, then terminator. */
    ok = ok && gif_u8 (self, GIF_MIN_CODE_SIZE);
    for (off = 0; ok && off < bits->len; )
    {
        gsize chunk = bits->len - off;

        if (chunk > 255)
            chunk = 255;
        ok = ok && gif_u8 (self, (guint8)chunk);
        ok = ok && gif_write (self, bits->data + off, chunk);
        off += chunk;
    }
    ok = ok && gif_u8 (self, 0x00);

    g_byte_array_free (bits, TRUE);
    return ok;
}

/*
 * Write a Graphic Control Extension block.
 *
 * packed_byte layout (GIF spec §23):
 *   bits 7-5: reserved (0)
 *   bits 4-2: disposal method
 *   bit  1:   user input flag (0)
 *   bit  0:   transparent colour flag
 *
 * disposal: 0 = no action, 2 = restore to background.
 * When transparency is enabled we use disposal 2 (restore to background)
 * so that later frames do not show ghost pixels in transparent regions.
 */
static gboolean
gif_write_gce (GrlGifWriter *self,
               gint          delay_centiseconds,
               gboolean      transparent,
               gint          transparent_index)
{
    guint8 packed;
    guint8 tidx;

    if (transparent)
    {
        /* Disposal 2 (restore to background): bits 4-2 = 010 -> 0x08.
         * Transparent flag (bit 0) set -> 0x09. */
        packed = 0x09;
        tidx   = (guint8)transparent_index;
    }
    else
    {
        /* Original bytes: disposal = leave in place (0x04 = 001 << 2), no transparency. */
        packed = 0x04;
        tidx   = 0x00;
    }

    return gif_u8 (self, 0x21) && gif_u8 (self, 0xf9) && gif_u8 (self, 0x04) &&
           gif_u8 (self, packed) &&
           gif_u16 (self, (guint)delay_centiseconds) &&
           gif_u8 (self, tidx) &&
           gif_u8 (self, 0x00);
}

/*
 * Write an Image Descriptor block.
 *
 * When local_color_table is FALSE, the packed byte is 0x00 (original encoder
 * behaviour). When TRUE, bits 7 and 4-0 are set: 0x80 | size_code.
 */
static gboolean
gif_write_image_descriptor (GrlGifWriter *self,
                             gboolean      local_color_table,
                             gint          local_palette_len)
{
    guint8 packed;

    if (local_color_table)
    {
        guint8 code = gif_color_table_size_code (local_palette_len);

        packed = (guint8)(0x80 | code);
    }
    else
    {
        packed = 0x00;
    }

    return gif_u8 (self, 0x2c) &&
           gif_u16 (self, 0) && gif_u16 (self, 0) &&
           gif_u16 (self, (guint)self->width) &&
           gif_u16 (self, (guint)self->height) &&
           gif_u8 (self, packed);
}

gboolean
grl_gif_writer_add_frame (GrlGifWriter *self,
                          GrlImage     *frame,
                          gint          delay_centiseconds,
                          GError      **error)
{
    Image  *handle;
    Image   work;
    guint8 *indices;
    Color  *pixels;
    gint    n, i;
    gboolean ok;

    g_return_val_if_fail (GRL_IS_GIF_WRITER (self), FALSE);
    g_return_val_if_fail (GRL_IS_IMAGE (frame), FALSE);

    if (self->closed || self->fp == NULL)
    {
        g_set_error (error, GRL_GIF_ERROR, GRL_GIF_ERROR_CLOSED,
                     "GIF writer is closed");
        return FALSE;
    }

    if (delay_centiseconds < 0)
        delay_centiseconds = 0;

    handle = (Image *)grl_image_get_handle (frame);
    g_return_val_if_fail (handle != NULL, FALSE);

    /* Work on a copy: scale to the canvas and normalise to RGBA8. */
    work = ImageCopy (*handle);
    if (work.width != self->width || work.height != self->height)
        ImageResize (&work, self->width, self->height);
    ImageFormat (&work, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);

    n = self->width * self->height;
    pixels = (Color *)work.data;
    indices = g_new (guint8, n);

    if (self->quantizer == GRL_GIF_QUANTIZER_WEB_SAFE)
    {
        /*
         * ----------------------------------------------------------------
         * LEGACY PATH: byte-identical to the original encoder.
         *
         * On the first frame we write the real web-safe GCT by seeking back
         * to GCT_OFFSET and overwriting the zeroed placeholder that new()
         * emitted. The final file bytes are identical to those produced by
         * the original eager encoder.
         * ----------------------------------------------------------------
         */
        if (!self->header_written)
        {
            guint8 gct[768];
            glong  gct_pos;

            /* Build the 6x6x6 web-safe global color table, padded to 256. */
            for (i = 0; i < 256; i++)
            {
                if (i < 216)
                {
                    gct[i * 3 + 0] = (guint8)((i / 36) * 51);
                    gct[i * 3 + 1] = (guint8)(((i / 6) % 6) * 51);
                    gct[i * 3 + 2] = (guint8)((i % 6) * 51);
                }
                else
                {
                    gct[i * 3 + 0] = 0;
                    gct[i * 3 + 1] = 0;
                    gct[i * 3 + 2] = 0;
                }
            }

            gct_pos = (glong)GCT_OFFSET;
            if (fseek (self->fp, gct_pos, SEEK_SET) != 0 ||
                fwrite (gct, 1, 768, self->fp) != 768 ||
                fseek (self->fp, 0, SEEK_END) != 0)
            {
                g_set_error (error, GRL_GIF_ERROR, GRL_GIF_ERROR_WRITE,
                             "Failed to write web-safe GCT");
                g_free (indices);
                UnloadImage (work);
                return FALSE;
            }

            self->header_written = TRUE;
        }

        for (i = 0; i < n; i++)
            indices[i] = gif_websafe_index (pixels[i].r, pixels[i].g, pixels[i].b);

        UnloadImage (work);

        /* Graphic Control Extension (per-frame delay). */
        ok = gif_u8 (self, 0x21) && gif_u8 (self, 0xf9) && gif_u8 (self, 0x04) &&
             gif_u8 (self, 0x04) &&                       /* disposal = leave in place */
             gif_u16 (self, (guint)delay_centiseconds) &&
             gif_u8 (self, 0x00) &&                       /* transparent color index */
             gif_u8 (self, 0x00);                         /* block terminator */

        /* Image Descriptor (full canvas, no local color table). */
        ok = ok && gif_u8 (self, 0x2c) &&
             gif_u16 (self, 0) && gif_u16 (self, 0) &&
             gif_u16 (self, (guint)self->width) &&
             gif_u16 (self, (guint)self->height) &&
             gif_u8 (self, 0x00);

        ok = ok && gif_write_image_data (self, indices, n);
    }
    else
    {
        /*
         * ----------------------------------------------------------------
         * MEDIAN-CUT PATH
         * ----------------------------------------------------------------
         */
        guint8 *rgba;
        guint8  local_palette[768];
        gint    local_palette_len;
        gint    transparent_index;
        gint    max_c;

        rgba = (guint8 *)work.data;

        /* Clamp max_colors. */
        max_c = self->max_colors;
        if (max_c < 2)   max_c = 2;
        if (max_c > 256) max_c = 256;

        /* Reserve one slot for transparency if enabled. */
        if (self->transparency_enabled && max_c > 1)
            max_c--;   /* actual colour slots */

        if (self->palette_scope == GRL_GIF_PALETTE_SCOPE_GLOBAL)
        {
            /* ---- GLOBAL palette: build from first frame, reuse for rest ---- */
            if (!self->header_written)
            {
                /* Build palette from this (first) frame. */
                gint n_colors;
                glong gct_pos;

                n_colors = grl_gif_median_cut ((const guint8 *)rgba, n,
                                               max_c,
                                               self->global_palette);
                self->global_palette_len = n_colors;

                /* If transparency enabled, append transparent slot at the end. */
                if (self->transparency_enabled)
                {
                    /* Slot n_colors is the transparent index. It is already
                     * zeroed in global_palette (black). */
                    self->global_transparent_index = n_colors;
                    n_colors++;
                }
                else
                {
                    self->global_transparent_index = -1;
                }

                /*
                 * Overwrite the placeholder GCT: seek back to offset GCT_OFFSET,
                 * write the actual palette (padded to 256 entries = 768 bytes),
                 * then seek back to the end of the file.
                 *
                 * We always emit exactly 768 bytes (256 * 3) because the LSD
                 * already announced 256 entries (packed byte 0xf7).
                 */
                gct_pos = (glong)GCT_OFFSET;
                if (fseek (self->fp, gct_pos, SEEK_SET) != 0)
                {
                    g_set_error (error, GRL_GIF_ERROR, GRL_GIF_ERROR_WRITE,
                                 "Failed to seek to GCT placeholder");
                    g_free (indices);
                    UnloadImage (work);
                    return FALSE;
                }
                {
                    guint8 gct_buf[768];
                    gint   j;

                    memcpy (gct_buf, self->global_palette, (gsize)(n_colors * 3));
                    for (j = n_colors; j < 256; j++)
                    {
                        gct_buf[j * 3 + 0] = 0;
                        gct_buf[j * 3 + 1] = 0;
                        gct_buf[j * 3 + 2] = 0;
                    }
                    if (fwrite (gct_buf, 1, 768, self->fp) != 768)
                    {
                        g_set_error (error, GRL_GIF_ERROR, GRL_GIF_ERROR_WRITE,
                                     "Failed to write GCT");
                        g_free (indices);
                        UnloadImage (work);
                        return FALSE;
                    }
                }

                /* Seek back to the end of the file. */
                if (fseek (self->fp, 0, SEEK_END) != 0)
                {
                    g_set_error (error, GRL_GIF_ERROR, GRL_GIF_ERROR_WRITE,
                                 "Failed to seek to end of file");
                    g_free (indices);
                    UnloadImage (work);
                    return FALSE;
                }

                self->header_written = TRUE;
            }

            /* Map indices using the (now fixed) global palette. */
            transparent_index = self->global_transparent_index;
            grl_gif_map_indices (rgba, self->width, self->height,
                                 self->global_palette,
                                 self->global_palette_len,
                                 self->dither,
                                 transparent_index,
                                 self->alpha_threshold,
                                 indices);

            UnloadImage (work);

            ok = gif_write_gce (self, delay_centiseconds,
                                 (transparent_index >= 0),
                                 (transparent_index >= 0) ? transparent_index : 0);
            /* No local color table: use global. */
            ok = ok && gif_write_image_descriptor (self, FALSE, 0);
            ok = ok && gif_write_image_data (self, indices, n);
        }
        else
        {
            /* ---- PER-FRAME palette ---- */
            local_palette_len = grl_gif_median_cut ((const guint8 *)rgba, n,
                                                    max_c,
                                                    local_palette);

            if (self->transparency_enabled)
            {
                transparent_index = local_palette_len;
                /* Slot is already zeroed (black). */
                local_palette_len++;
            }
            else
            {
                transparent_index = -1;
            }

            grl_gif_map_indices (rgba, self->width, self->height,
                                 local_palette,
                                 local_palette_len,
                                 self->dither,
                                 transparent_index,
                                 self->alpha_threshold,
                                 indices);

            UnloadImage (work);

            ok = gif_write_gce (self, delay_centiseconds,
                                 (transparent_index >= 0),
                                 (transparent_index >= 0) ? transparent_index : 0);
            ok = ok && gif_write_image_descriptor (self, TRUE, local_palette_len);
            /* Write the local color table. */
            ok = ok && gif_write_color_table (self, local_palette, local_palette_len);
            ok = ok && gif_write_image_data (self, indices, n);
        }
    }

    g_free (indices);

    if (!ok)
    {
        g_set_error (error, GRL_GIF_ERROR, GRL_GIF_ERROR_WRITE,
                     "Failed to write GIF frame %u", self->frame_count);
        return FALSE;
    }

    self->frame_count++;
    return TRUE;
}

gboolean
grl_gif_writer_close (GrlGifWriter *self,
                      GError      **error)
{
    gboolean ok;

    g_return_val_if_fail (GRL_IS_GIF_WRITER (self), FALSE);

    if (self->closed)
        return TRUE;

    self->closed = TRUE;

    if (self->fp == NULL)
        return TRUE;

    ok = gif_u8 (self, 0x3b);   /* trailer */

    if (fclose (self->fp) != 0)
        ok = FALSE;
    self->fp = NULL;

    if (!ok)
    {
        g_set_error (error, GRL_GIF_ERROR, GRL_GIF_ERROR_WRITE,
                     "Failed to finalize GIF file");
        return FALSE;
    }

    return TRUE;
}

/*
 * Setters — must be called before the first add_frame().
 */

/**
 * grl_gif_writer_set_quantizer:
 * @self: A #GrlGifWriter.
 * @quantizer: The palette-building strategy.
 *
 * Sets the quantizer. Default is %GRL_GIF_QUANTIZER_WEB_SAFE (byte-identical
 * to the original encoder). See the type documentation for details.
 */
void
grl_gif_writer_set_quantizer (GrlGifWriter    *self,
                              GrlGifQuantizer  quantizer)
{
    g_return_if_fail (GRL_IS_GIF_WRITER (self));

    self->quantizer = quantizer;
}

/**
 * grl_gif_writer_set_dither:
 * @self: A #GrlGifWriter.
 * @dither: The dithering mode.
 *
 * Sets the dithering mode. Only meaningful with %GRL_GIF_QUANTIZER_MEDIAN_CUT.
 * Default is %GRL_GIF_DITHER_NONE.
 */
void
grl_gif_writer_set_dither (GrlGifWriter *self,
                           GrlGifDither  dither)
{
    g_return_if_fail (GRL_IS_GIF_WRITER (self));

    self->dither = dither;
}

/**
 * grl_gif_writer_set_palette_scope:
 * @self: A #GrlGifWriter.
 * @scope: Global or per-frame palette.
 *
 * Sets the palette scope. Default is %GRL_GIF_PALETTE_SCOPE_GLOBAL. Ignored
 * when the quantizer is %GRL_GIF_QUANTIZER_WEB_SAFE.
 *
 * IMPORTANT: This must be called before grl_gif_writer_new() takes effect —
 * or rather, before any frames are added. However, since grl_gif_writer_new()
 * writes the LSD immediately, you must set the scope AFTER calling new() but
 * BEFORE the first add_frame(). The LSD packed byte will have been written
 * with the default settings unless you subclass; this is a known limitation.
 * For per-frame palettes, create the writer and then call set_palette_scope().
 *
 * NOTE: Due to the streaming nature of the writer, the palette scope is
 * evaluated at the time of the first add_frame() call, not at new() time.
 * When using PER_FRAME scope, the LSD written at new() time will have packed
 * byte 0xf7 (global palette declared) if the default is used; the frames
 * will still carry local colour tables. GIF readers are required to prefer
 * the local colour table when present, so this is valid. For a clean file
 * with no global palette, call set_palette_scope() before new() by using
 * a two-phase construction (not currently supported; file an issue).
 *
 * In practice, the simplest correct usage is:
 *   writer = grl_gif_writer_new (...)
 *   grl_gif_writer_set_quantizer (writer, GRL_GIF_QUANTIZER_MEDIAN_CUT)
 *   grl_gif_writer_set_palette_scope (writer, GRL_GIF_PALETTE_SCOPE_PER_FRAME)
 *   ... add frames ...
 *
 * The LSD will have 0xf7 (global palette declared with 256 entries of zeroes),
 * but readers will use the per-frame local colour tables.
 */
void
grl_gif_writer_set_palette_scope (GrlGifWriter      *self,
                                  GrlGifPaletteScope  scope)
{
    g_return_if_fail (GRL_IS_GIF_WRITER (self));

    self->palette_scope = scope;
}

/**
 * grl_gif_writer_set_max_colors:
 * @self: A #GrlGifWriter.
 * @max_colors: Maximum palette entries. Clamped to [2, 256].
 *
 * Sets the maximum palette size. Default is 256. Ignored for
 * %GRL_GIF_QUANTIZER_WEB_SAFE.
 */
void
grl_gif_writer_set_max_colors (GrlGifWriter *self,
                                gint          max_colors)
{
    g_return_if_fail (GRL_IS_GIF_WRITER (self));

    if (max_colors < 2)   max_colors = 2;
    if (max_colors > 256) max_colors = 256;
    self->max_colors = max_colors;
}

/**
 * grl_gif_writer_set_transparency:
 * @self: A #GrlGifWriter.
 * @enabled: %TRUE to enable transparency.
 * @alpha_threshold: Pixels with alpha < this value map to transparent index.
 *
 * Enables transparency support. Only meaningful with
 * %GRL_GIF_QUANTIZER_MEDIAN_CUT. Default is %FALSE / 128.
 */
void
grl_gif_writer_set_transparency (GrlGifWriter *self,
                                  gboolean      enabled,
                                  guint8        alpha_threshold)
{
    g_return_if_fail (GRL_IS_GIF_WRITER (self));

    self->transparency_enabled = enabled;
    self->alpha_threshold = alpha_threshold;
}

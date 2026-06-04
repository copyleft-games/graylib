/* grl-gif-writer.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Streaming animated GIF writer.
 *
 * This is a small, self-contained GIF89a encoder. Frames are quantised to a
 * fixed 6x6x6 web-safe palette (216 colors). Pixel data is encoded with GIF's
 * LZW scheme used in its "uncompressed" form: fixed 9-bit codes with a Clear
 * code emitted often enough that the decoder's code table never has to grow.
 * This trades file size for implementation simplicity and broad compatibility
 * (the output decodes in every conformant GIF reader). A true LZW compressor
 * can be dropped in later without changing the public API.
 */

#include "config.h"
#include "grl-gif-writer.h"
#include "grl-image.h"
#include <raylib.h>
#include <glib/gstdio.h>
#include <stdio.h>
#include <errno.h>

/* GIF LZW constants for an 8-bit (256-entry) code space. */
#define GIF_MIN_CODE_SIZE 8
#define GIF_CLEAR_CODE    256
#define GIF_EOI_CODE      257
#define GIF_CODE_SIZE     (GIF_MIN_CODE_SIZE + 1)   /* fixed 9-bit codes */
/* Clear before the decoder table reaches 512 entries (stays 9-bit). */
#define GIF_LITERALS_PER_CLEAR 253

struct _GrlGifWriter
{
    GObject  parent_instance;

    FILE    *fp;
    gint     width;
    gint     height;
    guint    frame_count;
    gboolean closed;
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
    gint          i;

    g_return_val_if_fail (filename != NULL, NULL);
    g_return_val_if_fail (width > 0 && width <= G_MAXUINT16, NULL);
    g_return_val_if_fail (height > 0 && height <= G_MAXUINT16, NULL);

    self = g_object_new (GRL_TYPE_GIF_WRITER, NULL);
    self->width = width;
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

    /* Build the 6x6x6 web-safe global color table, padded to 256 entries. */
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

    /* Header + Logical Screen Descriptor + Global Color Table. */
    if (!gif_write (self, "GIF89a", 6) ||
        !gif_u16 (self, (guint)width) ||
        !gif_u16 (self, (guint)height) ||
        !gif_u8 (self, 0xf7) ||   /* GCT present, 8-bit res, 256 entries */
        !gif_u8 (self, 0) ||      /* background color index */
        !gif_u8 (self, 0) ||      /* pixel aspect ratio */
        !gif_write (self, gct, sizeof gct))
    {
        g_set_error (error, GRL_GIF_ERROR, GRL_GIF_ERROR_WRITE,
                     "Failed to write GIF header");
        g_object_unref (self);
        return NULL;
    }

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

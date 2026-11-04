/* grl-pep-private.h
 *
 * Copyright 2026 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Private .pep (pixel-art palette) codec used by GrlImage. raylib 6.2 added
 * the format but its loader frees the pixels it returns, so graylib keeps
 * raylib's .pep support off and decodes/encodes with its own copy of
 * raylib's external/pep.h.
 */

#pragma once

#include <glib.h>
#include <raylib.h>

G_BEGIN_DECLS

/* GRL_PEP_MAX_SIZE: largest width/height a .pep stores (12 bits) */
#define GRL_PEP_MAX_SIZE (4096)

/* GRL_PEP_MAX_COLORS: a .pep palette holds at most 256 colours */
#define GRL_PEP_MAX_COLORS (256)

/*
 * grl_pep_is_file_type:
 * @file_type: an extension (".pep") or a file name
 *
 * Returns: whether @file_type names the .pep format (case-insensitive)
 */
gboolean    grl_pep_is_file_type    (const gchar    *file_type);

/*
 * grl_pep_decode:
 * @data: the bytes of a .pep file
 * @size: their size
 * @out: (out): the decoded R8G8B8A8 image (pixels from malloc(), freed by
 *   UnloadImage()); zeroed on failure
 *
 * Decodes losslessly: no colour is made transparent and alpha is not
 * premultiplied.
 *
 * Returns: %TRUE on success
 */
gboolean    grl_pep_decode          (const guint8   *data,
                                     gsize           size,
                                     Image          *out);

/*
 * grl_pep_encode:
 * @image: the image to encode (any uncompressed format)
 * @out_size: (out): size of the returned data
 *
 * Encodes @image as .pep with 8-bit channels. Fails (with a warning)
 * when the image is larger than %GRL_PEP_MAX_SIZE on a side or has more
 * than %GRL_PEP_MAX_COLORS distinct colours, since the format would lose
 * them.
 *
 * Returns: (transfer full) (nullable): g_malloc()ed .pep bytes, or %NULL
 */
guint8 *    grl_pep_encode          (const Image    *image,
                                     gsize          *out_size);

G_END_DECLS

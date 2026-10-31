/* SPDX-License-Identifier: AGPL-3.0-or-later */
#include <graylib.h>
#include <gio/gio.h>
#include <string.h>

/* A 256x128 chain has two more bytes than base_bytes * 4 / 3. Validate
 * independently of any display so the historical DDS truncation cannot recur. */
static void
test_layout (void)
{
	g_autofree guint8 *data = g_malloc0 (174765);
	g_autoptr(GBytes) exact = g_bytes_new (data, 174764);
	g_autoptr(GBytes) short_tail = g_bytes_new (data, 174762);
	g_autoptr(GBytes) extra = g_bytes_new (data, 174765);
	g_autoptr(GBytes) base = g_bytes_new (data, 131072);
	g_autoptr(GBytes) single = g_bytes_new (data, 4);
	g_autoptr(GError) error = NULL;
	g_autoptr(GrlTexture) texture = NULL;

	g_assert_null (grl_texture_new_from_rgba_mipmaps (256, 128, 9, short_tail, &error));
	g_assert_error (error, G_IO_ERROR, G_IO_ERROR_INVALID_DATA);
	g_clear_error (&error);
	g_assert_null (grl_texture_new_from_rgba_mipmaps (256, 128, 9, extra, &error));
	g_assert_error (error, G_IO_ERROR, G_IO_ERROR_INVALID_DATA);
	g_clear_error (&error);
	g_assert_null (grl_texture_new_from_rgba_mipmaps (256, 128, 10, exact, &error));
	g_assert_error (error, G_IO_ERROR, G_IO_ERROR_INVALID_DATA);
	g_clear_error (&error);
	g_assert_null (grl_texture_new_from_rgba_mipmaps (0, 128, 9, exact, &error));
	g_assert_error (error, G_IO_ERROR, G_IO_ERROR_INVALID_DATA);
	g_clear_error (&error);
	texture = grl_texture_new_from_rgba_mipmaps (256, 128, 9, exact, &error);
	g_assert_null (texture);
	g_assert_error (error, G_IO_ERROR, G_IO_ERROR_NOT_INITIALIZED);
	g_clear_error (&error);
	/* Partial chains and a single 1x1 level are valid layouts too. */
	g_assert_null (grl_texture_new_from_rgba_mipmaps (256, 128, 1, base, &error));
	g_assert_error (error, G_IO_ERROR, G_IO_ERROR_NOT_INITIALIZED);
	g_clear_error (&error);
	g_assert_null (grl_texture_new_from_rgba_mipmaps (1, 1, 1, single, &error));
	g_assert_error (error, G_IO_ERROR, G_IO_ERROR_NOT_INITIALIZED);
	g_clear_error (&error);
	g_assert_null (grl_texture_new_from_rgba_mipmaps (1, 1, 2, single, &error));
	g_assert_error (error, G_IO_ERROR, G_IO_ERROR_INVALID_DATA);
	g_clear_error (&error);
	g_assert_null (grl_texture_new_from_rgba_mipmaps (16384, 16384, 1, single, &error));
	g_assert_error (error, G_IO_ERROR, G_IO_ERROR_INVALID_DATA);
}

/* Actual upload/readback runs only on the caller-selected private display.
 * The exact allocation and final 1x1 tail are also checked by ASan in consumers. */
static void
test_upload (void)
{
	g_autoptr(GrlWindow) window = NULL;
	g_autoptr(GrlTexture) texture = NULL;
	g_autoptr(GrlImage) image = NULL;
	g_autoptr(GrlColor) color = NULL;
	g_autoptr(GBytes) pixels = NULL;
	g_autoptr(GError) error = NULL;
	guint8 *data;

	if (g_getenv ("DISPLAY") == NULL && g_getenv ("WAYLAND_DISPLAY") == NULL) {
		g_test_skip ("Graphics context not requested");
		return;
	}
	window = grl_window_new (64, 64, "RGBA mipmap regression");
	data = g_malloc (174764);
	memset (data, 255, 174764);
	pixels = g_bytes_new_take (data, 174764);
	texture = grl_texture_new_from_rgba_mipmaps (256, 128, 9, pixels, &error);
	g_assert_no_error (error);
	g_assert_nonnull (texture);
	g_assert_true (grl_texture_is_valid (texture));
	g_assert_cmpint (grl_texture_get_mipmaps (texture), ==, 9);
	/* Caller bytes can be released immediately after the synchronous upload. */
	g_clear_pointer (&pixels, g_bytes_unref);
	image = grl_texture_to_image (texture);
	g_assert_nonnull (image);
	color = grl_image_get_pixel (image, 255, 127);
	g_assert_nonnull (color);
	g_assert_cmpuint (color->r, ==, 255);
	g_assert_cmpuint (color->a, ==, 255);
}

/* The portrait chain must retain the final 1x2 and 1x1 levels: 43692
 * bytes, not the truncated base_bytes * 4 / 3 estimate of 43690. */
static void
test_layout_64x128 (void)
{
	g_autofree guint8 *data = g_malloc0 (43692);
	g_autoptr(GBytes) exact = g_bytes_new (data, 43692);
	g_autoptr(GBytes) short_tail = g_bytes_new (data, 43690);
	g_autoptr(GError) error = NULL;

	g_assert_null (grl_texture_new_from_rgba_mipmaps (64, 128, 8, short_tail, &error));
	g_assert_error (error, G_IO_ERROR, G_IO_ERROR_INVALID_DATA);
	g_clear_error (&error);
	/* Correct bytes pass layout validation, reaching the missing-context error. */
	g_assert_null (grl_texture_new_from_rgba_mipmaps (64, 128, 8, exact, &error));
	g_assert_error (error, G_IO_ERROR, G_IO_ERROR_NOT_INITIALIZED);
}

/* Exercise the exact portrait allocation and nonuniform base pixels on the
 * private GPU context; releasing caller bytes also checks synchronous ownership. */
static void
test_upload_64x128 (void)
{
	g_autoptr(GrlWindow) window = NULL;
	g_autoptr(GrlTexture) texture = NULL;
	g_autoptr(GrlImage) image = NULL;
	g_autoptr(GBytes) pixels = NULL;
	g_autoptr(GError) error = NULL;
	guint8 *data;
	gint x, y;

	if (g_getenv ("DISPLAY") == NULL && g_getenv ("WAYLAND_DISPLAY") == NULL) {
		g_test_skip ("Graphics context not requested");
		return;
	}
	window = grl_window_new (64, 64, "64x128 RGBA mipmap regression");
	data = g_malloc (43692);
	memset (data, 165, 43692);
	for (y = 0; y < 128; y++) {
		for (x = 0; x < 64; x++) {
			gsize offset = ((gsize)y * 64 + (gsize)x) * 4;

			data[offset] = (guint8)x;
			data[offset + 1] = (guint8)y;
			data[offset + 2] = (guint8)(x ^ y);
			data[offset + 3] = 255;
		}
	}
	pixels = g_bytes_new_take (data, 43692);
	texture = grl_texture_new_from_rgba_mipmaps (64, 128, 8, pixels, &error);
	g_assert_no_error (error);
	g_assert_nonnull (texture);
	g_assert_true (grl_texture_is_valid (texture));
	g_assert_cmpint (grl_texture_get_mipmaps (texture), ==, 8);
	g_clear_pointer (&pixels, g_bytes_unref);
	image = grl_texture_to_image (texture);
	g_assert_nonnull (image);
	g_assert_cmpint (grl_image_get_width (image), ==, 64);
	g_assert_cmpint (grl_image_get_height (image), ==, 128);
	for (y = 0; y < 128; y++) {
		for (x = 0; x < 64; x++) {
			g_autoptr(GrlColor) color = grl_image_get_pixel (image, x, y);

			g_assert_nonnull (color);
			g_assert_cmpuint (color->r, ==, (guint8)x);
			g_assert_cmpuint (color->g, ==, (guint8)y);
			g_assert_cmpuint (color->b, ==, (guint8)(x ^ y));
			g_assert_cmpuint (color->a, ==, 255);
		}
	}
}

int
main (int argc, char **argv)
{
	g_test_init (&argc, &argv, NULL);
	g_test_add_func ("/texture-mipmaps/layout", test_layout);
	g_test_add_func ("/texture-mipmaps/upload", test_upload);
	g_test_add_func ("/texture-mipmaps/layout-64x128", test_layout_64x128);
	g_test_add_func ("/texture-mipmaps/upload-64x128", test_upload_64x128);
	return g_test_run ();
}

/* SPDX-License-Identifier: AGPL-3.0-or-later
 * Verify the generated typelib's public ownership contracts without devices.
 */
#include <girepository/girepository.h>

static GIRepository *repository;

/* Resolve methods from the actual compiled typelib, not header text. */
static GIFunctionInfo *
find_method (const gchar *type_name,
             const gchar *method_name)
{
	g_autoptr(GIBaseInfo) type = NULL;
	GIFunctionInfo *method;

	type = gi_repository_find_by_name (repository, "Graylib", type_name);
	g_assert_true (GI_IS_OBJECT_INFO (type));
	method = gi_object_info_find_method ((GIObjectInfo *)type, method_name);
	g_assert_nonnull (method);
	return method;
}

/* Check both component type and length semantics, including unbounded raw
 * GPU buffers whose size is derived from format/dimensions by the caller. */
static void
assert_array (GITypeInfo *type,
              GITypeTag element_tag,
              gint length_index)
{
	g_autoptr(GITypeInfo) element = NULL;
	guint actual_index = 0;

	g_assert_cmpint (gi_type_info_get_tag (type), ==, GI_TYPE_TAG_ARRAY);
	g_assert_false (gi_type_info_is_zero_terminated (type));
	element = gi_type_info_get_param_type (type, 0);
	g_assert_cmpint (gi_type_info_get_tag (element), ==, element_tag);
	if (length_index < 0)
		g_assert_false (gi_type_info_get_array_length_index (type, &actual_index));
	else {
		g_assert_true (gi_type_info_get_array_length_index (type, &actual_index));
		g_assert_cmpuint (actual_index, ==, (guint)length_index);
	}
}

/* Raw pixels are borrowed, binary (never terminated), and nullable only for
 * allocation. GPU handle creation and attribute APIs must remain callable. */
static void
test_raw_gpu_metadata (void)
{
	static const gchar *names[] = {
		"rlgl_load_texture", "rlgl_update_texture", "rlgl_unload_texture",
		"rlgl_set_vertex_attribute", "rlgl_set_vertex_attribute_divisor"
	};
	guint i;

	for (i = 0; i < G_N_ELEMENTS (names); i++) {
		g_autoptr(GIBaseInfo) info = NULL;

		info = gi_repository_find_by_name (repository, "Graylib", names[i]);
		g_assert_true (GI_IS_FUNCTION_INFO (info));
		if (i < 2) {
			g_autoptr(GIArgInfo) arg = NULL;
			g_autoptr(GITypeInfo) type = NULL;

			arg = gi_callable_info_get_arg ((GICallableInfo *)info, i == 0 ? 0 : 6);
			type = gi_arg_info_get_type_info (arg);
			assert_array (type, GI_TYPE_TAG_UINT8, -1);
			g_assert_cmpint (gi_arg_info_get_ownership_transfer (arg), ==, GI_TRANSFER_NOTHING);
			g_assert_cmpint (gi_arg_info_may_be_null (arg), ==, i == 0);
		}
	}
}

/* A caller owns both the raw resource bytes and the returned length; errors
 * are native GError throws rather than an ordinary pointer output argument. */
static void
test_resource_metadata (void)
{
	g_autoptr(GIFunctionInfo) method = find_method ("ResourcePack", "load_raw");
	g_autoptr(GITypeInfo) result = NULL;
	g_autoptr(GIArgInfo) count = NULL;

	result = gi_callable_info_get_return_type ((GICallableInfo *)method);
	assert_array (result, GI_TYPE_TAG_UINT8, 1);
	g_assert_cmpint (gi_callable_info_get_caller_owns ((GICallableInfo *)method), ==, GI_TRANSFER_EVERYTHING);
	g_assert_true (gi_callable_info_may_return_null ((GICallableInfo *)method));
	g_assert_true (gi_callable_info_can_throw_gerror ((GICallableInfo *)method));
	g_assert_cmpuint (gi_callable_info_get_n_args ((GICallableInfo *)method), ==, 2);
	count = gi_callable_info_get_arg ((GICallableInfo *)method, 1);
	g_assert_cmpint (gi_arg_info_get_direction (count), ==, GI_DIRECTION_OUT);
}

/* Borrowed pixels and material maps must never be freed by bindings. Boxed
 * model transforms, by contrast, are independent allocations owned by callers. */
static void
test_renderer_ownership (void)
{
	g_autoptr(GIFunctionInfo) pixels = find_method ("Image", "get_pixels");
	g_autoptr(GIFunctionInfo) texture = find_method ("Material", "get_texture");
	g_autoptr(GIFunctionInfo) transform = find_method ("Model", "get_transform");
	g_autoptr(GITypeInfo) result = NULL;

	result = gi_callable_info_get_return_type ((GICallableInfo *)pixels);
	assert_array (result, GI_TYPE_TAG_UINT8, 0);
	g_assert_cmpint (gi_callable_info_get_caller_owns ((GICallableInfo *)pixels), ==, GI_TRANSFER_NOTHING);
	g_assert_true (gi_callable_info_may_return_null ((GICallableInfo *)pixels));
	g_assert_cmpint (gi_callable_info_get_caller_owns ((GICallableInfo *)texture), ==, GI_TRANSFER_NOTHING);
	g_assert_true (gi_callable_info_may_return_null ((GICallableInfo *)texture));
	g_assert_cmpint (gi_callable_info_get_caller_owns ((GICallableInfo *)transform), ==, GI_TRANSFER_EVERYTHING);
}

/* Failed PNG parsing is nullable, and the pointer array carries its length. */
static void
test_png_metadata (void)
{
	g_autoptr(GIBaseInfo) info = NULL;
	g_autoptr(GIFunctionInfo) method = NULL;
	g_autoptr(GITypeInfo) result = NULL;

	info = gi_repository_find_by_name (repository, "Graylib", "PngChunk");
	g_assert_true (GI_IS_STRUCT_INFO (info));
	method = gi_struct_info_find_method ((GIStructInfo *)info, "read_all");
	g_assert_nonnull (method);
	result = gi_callable_info_get_return_type ((GICallableInfo *)method);
	assert_array (result, GI_TYPE_TAG_INTERFACE, 1);
	g_assert_true (gi_callable_info_may_return_null ((GICallableInfo *)method));
	g_assert_cmpint (gi_callable_info_get_caller_owns ((GICallableInfo *)method), ==, GI_TRANSFER_EVERYTHING);
}

/* Public format-dependent audio updates remain visible as raw byte arrays.
 * This verifies metadata only: no audio device or playback is started. */
static void
test_audio_buffer_metadata (void)
{
	static const gchar *types[] = { "Sound", "AudioStream" };
	guint i;

	for (i = 0; i < G_N_ELEMENTS (types); i++) {
		g_autoptr(GIFunctionInfo) method = find_method (types[i], "update");
		g_autoptr(GIArgInfo) data = gi_callable_info_get_arg ((GICallableInfo *)method, 0);
		g_autoptr(GITypeInfo) type = gi_arg_info_get_type_info (data);

		assert_array (type, GI_TYPE_TAG_UINT8, -1);
		g_assert_false (gi_arg_info_may_be_null (data));
		g_assert_cmpint (gi_arg_info_get_ownership_transfer (data), ==, GI_TRANSFER_NOTHING);
		g_assert_cmpuint (gi_callable_info_get_n_args ((GICallableInfo *)method), ==, 2);
	}
}

/* Decoded mip bytes are borrowed GBytes, not an unbounded native pointer. */
static void
test_mipmap_metadata (void)
{
	g_autoptr(GIFunctionInfo) method = find_method ("Texture", "new_from_rgba_mipmaps");
	g_autoptr(GIArgInfo) arg = gi_callable_info_get_arg ((GICallableInfo *)method, 3);
	g_autoptr(GITypeInfo) type = gi_arg_info_get_type_info (arg);
	g_autoptr(GIBaseInfo) interface = gi_type_info_get_interface (type);

	g_assert_cmpstr (gi_base_info_get_namespace (interface), ==, "GLib");
	g_assert_cmpstr (gi_base_info_get_name (interface), ==, "Bytes");
	g_assert_cmpint (gi_arg_info_get_ownership_transfer (arg), ==, GI_TRANSFER_NOTHING);
	g_assert_true (gi_callable_info_can_throw_gerror ((GICallableInfo *)method));
	g_assert_true (gi_callable_info_may_return_null ((GICallableInfo *)method));
	g_assert_cmpint (gi_callable_info_get_caller_owns ((GICallableInfo *)method), ==, GI_TRANSFER_EVERYTHING);
}

/* Preserve the upstream native callbacks' notified lifetime, closure and
 * destroy indices when combining them with the public raw-buffer contracts. */
static void
test_native_callback_metadata (void)
{
	g_autoptr(GIFunctionInfo) method = find_method ("AudioStream", "set_callback");
	g_autoptr(GIArgInfo) callback = gi_callable_info_get_arg ((GICallableInfo *)method, 0);
	g_autoptr(GITypeInfo) type = gi_arg_info_get_type_info (callback);
	g_autoptr(GIBaseInfo) interface = gi_type_info_get_interface (type);
	g_autoptr(GIFunctionInfo) background = find_method ("Music", "set_background_streaming");
	guint index = 0;

	g_assert_true (GI_IS_CALLBACK_INFO (interface));
	g_assert_cmpstr (gi_base_info_get_name (interface), ==, "AudioStreamCallback");
	g_assert_true (gi_arg_info_may_be_null (callback));
	g_assert_cmpint (gi_arg_info_get_scope (callback), ==, GI_SCOPE_TYPE_NOTIFIED);
	g_assert_true (gi_arg_info_get_closure_index (callback, &index));
	g_assert_cmpuint (index, ==, 1);
	g_assert_true (gi_arg_info_get_destroy_index (callback, &index));
	g_assert_cmpuint (index, ==, 2);
	g_assert_nonnull (background);
}

/* The independent policy is explicit; the shared-memory constructor remains
 * present, while checked writes carry a real byte length and GError contract. */
static void
test_independent_voice_metadata (void)
{
	g_autoptr(GIFunctionInfo) voice = find_method ("Sound", "new_voice");
	g_autoptr(GIFunctionInfo) alias = find_method ("Sound", "new_alias");
	g_autoptr(GIFunctionInfo) update = find_method ("Sound", "update_checked");
	g_autoptr(GIArgInfo) data = gi_callable_info_get_arg ((GICallableInfo *)update, 0);
	g_autoptr(GITypeInfo) type = gi_arg_info_get_type_info (data);

	g_assert_nonnull (alias);
	g_assert_true (gi_callable_info_can_throw_gerror ((GICallableInfo *)voice));
	g_assert_true (gi_callable_info_may_return_null ((GICallableInfo *)voice));
	g_assert_cmpint (gi_callable_info_get_caller_owns ((GICallableInfo *)voice), ==, GI_TRANSFER_EVERYTHING);
	assert_array (type, GI_TYPE_TAG_UINT8, 1);
	g_assert_true (gi_callable_info_can_throw_gerror ((GICallableInfo *)update));
}

int
main (int argc, char **argv)
{
	g_autoptr(GError) error = NULL;
	g_autoptr(GIRepository) owned_repository = NULL;
	const gchar *directory;

	g_test_init (&argc, &argv, NULL);
	directory = g_getenv ("GRL_TEST_TYPELIB_DIR");
	g_assert_nonnull (directory);
	owned_repository = gi_repository_new ();
	repository = owned_repository;
	gi_repository_prepend_search_path (repository, directory);
	g_assert_nonnull (gi_repository_require (repository, "Graylib", "1", 0, &error));
	g_assert_no_error (error);
	g_test_add_func ("/introspection/raw-gpu", test_raw_gpu_metadata);
	g_test_add_func ("/introspection/resource", test_resource_metadata);
	g_test_add_func ("/introspection/renderer-ownership", test_renderer_ownership);
	g_test_add_func ("/introspection/png", test_png_metadata);
	g_test_add_func ("/introspection/audio-buffer-metadata", test_audio_buffer_metadata);
	g_test_add_func ("/introspection/mipmap-metadata", test_mipmap_metadata);
	g_test_add_func ("/introspection/native-callback-metadata", test_native_callback_metadata);
	g_test_add_func ("/introspection/independent-voice", test_independent_voice_metadata);
	return g_test_run ();
}

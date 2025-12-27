/* grl-rlgl-framebuffer.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Framebuffer operations for rlgl.
 */

#include "config.h"
#include "grl-rlgl.h"
#include <rlgl.h>

/**
 * grl_rlgl_load_framebuffer:
 *
 * Creates a new framebuffer object.
 *
 * Returns: Framebuffer ID, or 0 on failure
 */
guint
grl_rlgl_load_framebuffer (void)
{
    return rlLoadFramebuffer ();
}

/**
 * grl_rlgl_framebuffer_attach:
 * @fbo_id: Framebuffer ID
 * @tex_id: Texture or renderbuffer ID to attach
 * @attach_type: Attachment point (color, depth, stencil)
 * @tex_type: Type of texture attachment
 * @mip_level: Mipmap level to attach
 *
 * Attaches a texture or renderbuffer to a framebuffer.
 */
void
grl_rlgl_framebuffer_attach (guint fbo_id,
                             guint tex_id,
                             GrlRlglFramebufferAttachType attach_type,
                             GrlRlglFramebufferTexType tex_type,
                             gint mip_level)
{
    rlFramebufferAttach (fbo_id, tex_id, (int)attach_type, (int)tex_type, mip_level);
}

/**
 * grl_rlgl_framebuffer_complete:
 * @id: Framebuffer ID to check
 *
 * Checks if a framebuffer is complete and ready for rendering.
 *
 * Returns: %TRUE if the framebuffer is complete
 */
gboolean
grl_rlgl_framebuffer_complete (guint id)
{
    /* Fix bool/gboolean ABI mismatch */
    unsigned char raw = rlFramebufferComplete (id);
    return raw != 0;
}

/**
 * grl_rlgl_unload_framebuffer:
 * @id: Framebuffer ID to unload
 *
 * Deletes a framebuffer object.
 */
void
grl_rlgl_unload_framebuffer (guint id)
{
    rlUnloadFramebuffer (id);
}

/**
 * grl_rlgl_enable_framebuffer:
 * @id: Framebuffer ID to enable
 *
 * Binds a framebuffer for rendering.
 */
void
grl_rlgl_enable_framebuffer (guint id)
{
    rlEnableFramebuffer (id);
}

/**
 * grl_rlgl_disable_framebuffer:
 *
 * Unbinds the current framebuffer (back to default).
 */
void
grl_rlgl_disable_framebuffer (void)
{
    rlDisableFramebuffer ();
}

/**
 * grl_rlgl_get_active_framebuffer:
 *
 * Gets the currently bound framebuffer ID.
 *
 * Returns: Active framebuffer ID
 */
guint
grl_rlgl_get_active_framebuffer (void)
{
    return rlGetActiveFramebuffer ();
}

/**
 * grl_rlgl_active_draw_buffers:
 * @count: Number of draw buffers to activate
 *
 * Activates multiple draw buffers for multiple render targets.
 */
void
grl_rlgl_active_draw_buffers (gint count)
{
    rlActiveDrawBuffers (count);
}

/**
 * grl_rlgl_blit_framebuffer:
 * @src_x: Source X position
 * @src_y: Source Y position
 * @src_width: Source width
 * @src_height: Source height
 * @dst_x: Destination X position
 * @dst_y: Destination Y position
 * @dst_width: Destination width
 * @dst_height: Destination height
 * @buffer_mask: Buffer mask (color, depth, stencil)
 *
 * Copies a region from one framebuffer to another.
 */
void
grl_rlgl_blit_framebuffer (gint src_x,
                           gint src_y,
                           gint src_width,
                           gint src_height,
                           gint dst_x,
                           gint dst_y,
                           gint dst_width,
                           gint dst_height,
                           gint buffer_mask)
{
    rlBlitFramebuffer (src_x, src_y, src_width, src_height,
                       dst_x, dst_y, dst_width, dst_height,
                       buffer_mask);
}

/* grl-input-private.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Private header for GrlInput internals - for use within the core module only.
 */

#pragma once

#include <glib.h>

G_BEGIN_DECLS

/*
 * Delivers queued synthetic (test-injected) input events through the
 * backend callbacks.  Called by grl_window_poll_events() right after the
 * real event pump; not public API.
 */
void    grl_input_drain_injections  (void);

G_END_DECLS

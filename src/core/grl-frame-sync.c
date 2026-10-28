/* grl-frame-sync.c
 *
 * Copyright 2026 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Bounded frames in flight through GL sync objects.
 */

#include "config.h"
#include "grl-frame-sync.h"
#include <raylib.h>
#include <rlgl.h>		/* rlGetProcAddress, rlGetVersion, rlDrawRenderBatchActive */

/**
 * SECTION:grl-frame-sync
 * @title: Frame sync
 * @short_description: Bound how many frames the CPU runs ahead of the GPU
 *
 * A GL driver lets the CPU queue frames ahead of the GPU before it blocks
 * (Mesa about one, other drivers up to three); every queued frame is
 * extra input-to-photon latency, since the input it was built from is that
 * much older when it reaches the screen.  Frame sync bounds the queue
 * itself: before each buffer swap it fences the frame (glFenceSync()) and
 * waits (glClientWaitSync()) until at most @frames - 1 earlier frames are
 * unfinished on the GPU, so with the frame the CPU builds next at most
 * @frames are in flight.
 *
 * - 1: the GPU has finished the frame before its swap, so the next input
 *   poll sees an idle GPU (lowest latency; CPU and GPU work no longer
 *   overlap, so a GPU-bound game loses frame rate).
 * - 2: one frame of CPU/GPU overlap (the usual "max frame latency 1" of
 *   other APIs; what Mesa does on its own).
 * - 0: off, the driver's own limit.
 *
 * The wait sits before the buffer swap, inside the end of the frame, so
 * raylib's frame limiter and its input poll (both run by EndDrawing()
 * after the swap) still come after it: the input the next frame reads is
 * sampled after the wait, not before.  The frame's draws are flushed to
 * GL first, and a fence the GPU already passed is checked without a flush
 * or a block, so on a GPU that keeps up the cost is a fence per frame.
 *
 * It needs sync objects (OpenGL 3.2+, or OpenGL ES 3.0), which raylib's
 * desktop OpenGL 3.3 build always has; elsewhere grl_frame_sync_is_supported()
 * returns %FALSE and every call is a no-op.  It is process-wide, like the
 * raylib window and GL context it acts on.  #GrlWindow:max-frames-in-flight
 * is the usual way to turn it on; code that calls raylib's EndDrawing()
 * itself uses grl_frame_sync_end_drawing() in its place.
 */

/* Minimal GL sync declarations: graylib does not include GL headers, and
   raylib's glad is internal to rlgl, so the three entry points are looked
   up through rlGetProcAddress() (raylib's loader: glfwGetProcAddress on
   desktop) and declared here. */
#if defined(_WIN32)
#define GRL_GLAPIENTRY __stdcall
#else
#define GRL_GLAPIENTRY
#endif

#define GRL_GL_SYNC_GPU_COMMANDS_COMPLETE (0x9117)
#define GRL_GL_SYNC_FLUSH_COMMANDS_BIT    (0x00000001)
#define GRL_GL_ALREADY_SIGNALED           (0x911A)
#define GRL_GL_TIMEOUT_EXPIRED            (0x911B)
#define GRL_GL_CONDITION_SATISFIED        (0x911C)
#define GRL_GL_WAIT_FAILED                (0x911D)

/* A fence wait gives up after this long (ns) so a lost GPU (reset, hung
   driver) cannot freeze the game thread; the frame is then let through. */
#define GRL_FRAME_SYNC_TIMEOUT_NS         (G_GUINT64_CONSTANT (100000000))

typedef struct _GrlGlSync *GrlGlSync;
typedef GrlGlSync (GRL_GLAPIENTRY *GrlGlFenceSyncFn) (guint condition, guint flags);
typedef guint (GRL_GLAPIENTRY *GrlGlClientWaitSyncFn) (GrlGlSync sync, guint flags,
                                                       guint64 timeout);
typedef void (GRL_GLAPIENTRY *GrlGlDeleteSyncFn) (GrlGlSync sync);

/* Support probe states: probed once per GL context (reset re-probes) */
typedef enum
{
    GRL_FRAME_SYNC_UNPROBED = 0,
    GRL_FRAME_SYNC_SUPPORTED,
    GRL_FRAME_SYNC_UNSUPPORTED
} GrlFrameSyncProbe;

/* Process-wide state.  Everything here is touched only from the thread
   whose GL context is current (raylib's main thread). */
static struct
{
    guint                  max_frames;      /* 0 = off */
    GrlFrameSyncProbe      probe;
    GrlGlFenceSyncFn       fence_sync;
    GrlGlClientWaitSyncFn  client_wait_sync;
    GrlGlDeleteSyncFn      delete_sync;

    /* ring of outstanding fences, oldest at head */
    GrlGlSync              fences[GRL_FRAME_SYNC_MAX_FRAMES];
    guint                  head;
    guint                  count;

    gdouble                last_wait;       /* seconds, last before_present */
    guint64                timeouts;
} frame_sync;

/* Looks up the GL sync entry points once a context exists.  raylib's
   desktop build is OpenGL 3.3 core (sync is core since 3.2) and its ES3
   build has sync in core too; GL 1.1/2.1/ES2 and the software renderer
   get none, since an extension check would need the GL headers raylib
   keeps private, and Mesa's GetProcAddress returns stubs for functions
   the context lacks, so a non-NULL pointer alone proves nothing. */
static gboolean
grl_frame_sync_probe (void)
{
    gint version;

    if (frame_sync.probe != GRL_FRAME_SYNC_UNPROBED)
        return frame_sync.probe == GRL_FRAME_SYNC_SUPPORTED;

    /* no context yet: do not cache, a window may open later */
    if (!IsWindowReady ())
        return FALSE;

    frame_sync.probe = GRL_FRAME_SYNC_UNSUPPORTED;
    version = rlGetVersion ();
    if (version != RL_OPENGL_33 && version != RL_OPENGL_43 && version != RL_OPENGL_ES_30)
        return FALSE;

    frame_sync.fence_sync = (GrlGlFenceSyncFn) rlGetProcAddress ("glFenceSync");
    frame_sync.client_wait_sync = (GrlGlClientWaitSyncFn) rlGetProcAddress ("glClientWaitSync");
    frame_sync.delete_sync = (GrlGlDeleteSyncFn) rlGetProcAddress ("glDeleteSync");
    if (frame_sync.fence_sync == NULL ||
        frame_sync.client_wait_sync == NULL ||
        frame_sync.delete_sync == NULL)
        return FALSE;

    frame_sync.probe = GRL_FRAME_SYNC_SUPPORTED;
    return TRUE;
}

/* Removes the oldest fence from the ring (it must have one) and returns it */
static GrlGlSync
grl_frame_sync_pop (void)
{
    GrlGlSync fence;

    fence = frame_sync.fences[frame_sync.head];
    frame_sync.fences[frame_sync.head] = NULL;
    frame_sync.head = (frame_sync.head + 1) % GRL_FRAME_SYNC_MAX_FRAMES;
    frame_sync.count--;
    return fence;
}

/* Deletes every outstanding fence without waiting (legal on unsignaled
   fences: GL frees them once signaled).  Needs the context current. */
static void
grl_frame_sync_drop_all (void)
{
    while (frame_sync.count > 0)
    {
        GrlGlSync fence = grl_frame_sync_pop ();

        if (fence != NULL && frame_sync.delete_sync != NULL && IsWindowReady ())
            frame_sync.delete_sync (fence);
    }
    frame_sync.head = 0;
}

/**
 * grl_frame_sync_set_max_frames_in_flight:
 * @frames: frames the CPU may queue ahead of the GPU; 0 turns frame sync
 *   off (the driver's own limit applies)
 *
 * Sets how many frames may be in flight, counting the one the CPU builds
 * next: at each swap, frame sync waits until at most @frames - 1 earlier
 * frames are unfinished on the GPU.  1 is the lowest latency (the GPU
 * is idle when input is polled; CPU and GPU work stop overlapping); 2
 * keeps one frame of overlap.  Values above %GRL_FRAME_SYNC_MAX_FRAMES are
 * clamped.  Lowering it takes effect at the next present; 0 drops the
 * outstanding fences at once.
 */
void
grl_frame_sync_set_max_frames_in_flight (guint frames)
{
    frames = MIN (frames, (guint) GRL_FRAME_SYNC_MAX_FRAMES);
    frame_sync.max_frames = frames;
    if (frames == 0)
    {
        grl_frame_sync_drop_all ();
        frame_sync.last_wait = 0.0;
    }
}

/**
 * grl_frame_sync_get_max_frames_in_flight:
 *
 * Returns: the bound set with grl_frame_sync_set_max_frames_in_flight()
 *   (0: off)
 */
guint
grl_frame_sync_get_max_frames_in_flight (void)
{
    return frame_sync.max_frames;
}

/**
 * grl_frame_sync_is_supported:
 *
 * Whether the current GL context has sync objects.  %FALSE before a
 * window is open.
 *
 * Returns: %TRUE if frame sync can work on this context
 */
gboolean
grl_frame_sync_is_supported (void)
{
    return grl_frame_sync_probe ();
}

/**
 * grl_frame_sync_is_active:
 *
 * Returns: %TRUE when frame sync is on and supported, so the present
 *   path waits on fences
 */
gboolean
grl_frame_sync_is_active (void)
{
    return frame_sync.max_frames > 0 && grl_frame_sync_probe ();
}

/**
 * grl_frame_sync_before_present:
 *
 * Fences the frame about to be presented and waits until the frames
 * still unfinished on the GPU leave room for the next one: with a bound of
 * 1, until this frame's rendering is done (the GPU is idle when the next
 * input poll runs); with 2, until the previous frame's is (one frame of
 * CPU/GPU overlap).  Call right before the buffer swap, after the frame's
 * draws are flushed to GL (rlDrawRenderBatchActive()).  A no-op when frame
 * sync is off or unsupported.  The time it waited is reported by
 * grl_frame_sync_get_last_wait().
 */
void
grl_frame_sync_before_present (void)
{
    GrlGlSync fence;
    gint64    start;
    guint     tail;

    frame_sync.last_wait = 0.0;
    if (!grl_frame_sync_is_active ())
        return;

    /* 1. Fence this frame's rendering.  It goes in before the swap, so it
       signals when the frame is drawn, without whatever the swap itself
       queues (a compositor blit, a PRIME copy).  A full ring (the bound
       was lowered) retires its oldest first. */
    if (frame_sync.count == GRL_FRAME_SYNC_MAX_FRAMES)
        frame_sync.delete_sync (grl_frame_sync_pop ());
    fence = frame_sync.fence_sync (GRL_GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    if (fence != NULL)
    {
        tail = (frame_sync.head + frame_sync.count) % GRL_FRAME_SYNC_MAX_FRAMES;
        frame_sync.fences[tail] = fence;
        frame_sync.count++;
    }

    /* 2. Wait on (and retire) the oldest fences until max_frames - 1
       remain: those frames may still be on the GPU while the CPU builds
       the next one, which makes max_frames in flight.  The first wait
       flushes the command stream, so the GPU has every queued frame. */
    start = g_get_monotonic_time ();
    while (frame_sync.count > 0 && frame_sync.count >= frame_sync.max_frames)
    {
        guint result;

        fence = grl_frame_sync_pop ();

        /* An already-signaled fence (the usual case when the GPU keeps
           up) is checked without the flush bit and with no timeout:
           that skips a command-buffer submission, which on the GPU-light
           frames of a fast game costs more than the fence itself. */
        result = frame_sync.client_wait_sync (fence, 0, 0);
        if (result != GRL_GL_ALREADY_SIGNALED && result != GRL_GL_CONDITION_SATISFIED &&
            result != GRL_GL_WAIT_FAILED)
            result = frame_sync.client_wait_sync (fence, GRL_GL_SYNC_FLUSH_COMMANDS_BIT,
                                                  GRL_FRAME_SYNC_TIMEOUT_NS);
        frame_sync.delete_sync (fence);

        if (result == GRL_GL_TIMEOUT_EXPIRED)
        {
            frame_sync.timeouts++;
        }
        else if (result == GRL_GL_WAIT_FAILED)
        {
            /* A context that cannot wait on its own fences: turn the
               feature off for this context rather than fail every frame. */
            g_warning ("graylib: glClientWaitSync failed; frame sync disabled");
            grl_frame_sync_drop_all ();
            frame_sync.probe = GRL_FRAME_SYNC_UNSUPPORTED;
            break;
        }
    }
    frame_sync.last_wait = (gdouble) (g_get_monotonic_time () - start) / (gdouble) G_USEC_PER_SEC;
}

/**
 * grl_frame_sync_end_drawing:
 *
 * raylib's EndDrawing() with frame sync around its swap: flushes the
 * render batch, waits (grl_frame_sync_before_present()), then runs
 * EndDrawing() (swap, frame limiter, input poll) and inserts the frame's
 * fence.  With frame sync off it is exactly EndDrawing().  For code that
 * drives raylib directly instead of through grl_window_end_drawing().
 */
void
grl_frame_sync_end_drawing (void)
{
    if (!grl_frame_sync_is_active ())
    {
        EndDrawing ();
        return;
    }

    rlDrawRenderBatchActive ();
    grl_frame_sync_before_present ();
    EndDrawing ();
}

/**
 * grl_frame_sync_get_last_wait:
 *
 * Returns: seconds the last grl_frame_sync_before_present() spent waiting
 *   on the GPU (0 when it did not wait)
 */
gdouble
grl_frame_sync_get_last_wait (void)
{
    return frame_sync.last_wait;
}

/**
 * grl_frame_sync_get_pending:
 *
 * Returns: the number of presented frames whose fence has not been
 *   waited on yet (at most the bound minus one after each present)
 */
guint
grl_frame_sync_get_pending (void)
{
    return frame_sync.count;
}

/**
 * grl_frame_sync_get_timeouts:
 *
 * Returns: how many fence waits gave up after 100 ms since the process
 *   started (a GPU that stopped answering); normally 0
 */
guint64
grl_frame_sync_get_timeouts (void)
{
    return frame_sync.timeouts;
}

/**
 * grl_frame_sync_reset:
 *
 * Deletes the outstanding fences and forgets the support probe.  Call
 * while the GL context is still current, before it is destroyed
 * (#GrlWindow does on dispose); the bound itself is kept for the next
 * window.
 */
void
grl_frame_sync_reset (void)
{
    grl_frame_sync_drop_all ();
    frame_sync.probe = GRL_FRAME_SYNC_UNPROBED;
    frame_sync.fence_sync = NULL;
    frame_sync.client_wait_sync = NULL;
    frame_sync.delete_sync = NULL;
    frame_sync.last_wait = 0.0;
}

/* grl-audio-queue.c
 *
 * Copyright 2026 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * The audio command thread.
 */

#include "config.h"
#include "grl-audio-queue-private.h"

/* ==========================================================================
 * The audio command thread
 *
 * raylib has one audio lock.  Its mixer holds it for every device period
 * (and runs graylib's stream callbacks and processors inside it), and
 * UpdateMusicStream() holds it for a whole sub-buffer decode, so a
 * game-thread PlaySound() or SetSoundVolume() could wait milliseconds.
 * Instead the game thread pushes a command and returns:
 *
 * - The list is a Treiber stack: a push is a compare-and-swap of the head,
 *   the thread takes the whole list with another (no ABA: nothing is ever
 *   popped one by one) and reverses it, so each producer's commands run
 *   in the order it pushed them.  The game thread never takes a lock that
 *   anything holds across a raylib call.
 * - Waking: the thread takes wake_lock, announces it is about to sleep
 *   (sleeping = 1), checks the list once more and waits on wake_cond; a
 *   producer pushes, then wakes it only if it sees sleeping set.  Both
 *   orders are sequentially consistent atomics, so either the producer
 *   sees the flag or the thread sees the command.  The producer never
 *   waits for wake_lock (the thread could be preempted holding it): it
 *   signals under the lock when a trylock gets it (then the thread is in
 *   its wait or has not checked yet: no lost wake-up), and without it
 *   otherwise (the thread is awake, or in the few instructions between
 *   its check and its wait, where the signal can be lost); the thread
 *   never waits longer than IDLE_WAIT_US, which bounds that rare case.
 * - Mirror: a command that changes whether a voice plays carries a serial;
 *   the game thread records the state the command will leave (predicted).
 *   After applying it the thread reads raylib's state back and publishes
 *   it, then the serial.  A getter reports the prediction until the serial
 *   arrives, then the published state, which the thread refreshes every
 *   WATCH_INTERVAL_US for voices that play (that is how the natural end of
 *   a sound or track shows up).  Voices pushed inline (no thread) or in an
 *   earlier generation of the thread are answered by raylib directly.
 * - Background music refills are the refresh of a music voice (the old
 *   grl-music thread is this one now).
 *
 * The thread never drops the last reference of a GObject: commands hold
 * the plain refcounted GrlAudioVoice the object embeds, so a GObject's
 * finalize (and its weak refs, notifies) stays on the thread that dropped
 * it.  Commands are recycled, not freed: the thread drops a command's
 * voice reference and data, then hands the bare command back on a second
 * lock-free list (done_head); a producer whose own cache is empty takes
 * that whole list into its thread-local cache in one exchange and pops
 * from it, so a push neither mallocs in the steady state nor frees a
 * backlog (thousands of commands pile up while the thread waits for a
 * long lock hold; freeing them in one push cost up to 0.4 ms), nor frees
 * across threads (which can take malloc's arena lock).  The caches hold
 * at most the peak number of commands in flight.  Voices are freed
 * wherever their last reference goes, after release, without raylib
 * calls.
 * ========================================================================== */

#define WATCH_INTERVAL_US (5000)
#define IDLE_WAIT_US      (20000)

typedef enum
{
    COMMAND_PLAIN,
    COMMAND_STATE,
    COMMAND_UNWATCH,
    COMMAND_RELEASE,
    COMMAND_FENCE
} CommandKind;

typedef struct _Command Command;

struct _Command
{
    Command           *next;
    CommandKind        kind;
    GrlAudioVoice     *voice;      /* a reference; NULL for a fence */
    GrlAudioVoiceFunc  func;
    gfloat             value;
    gpointer           data;
    GDestroyNotify     data_free;
    guint              serial;     /* COMMAND_STATE */
};

static gpointer   queue_head;          /* Command *, atomic */
static gpointer   done_head;           /* Command *, atomic: applied, to reuse */
static gint       queue_sleeping;      /* atomic: the thread is about to wait */
static gint       queue_quit;          /* atomic */
static gint       queue_running;       /* atomic: pushes are queued */
static guint      queue_generation;    /* game thread; bumped on each start */
static GThread   *queue_thread;
static GMutex     wake_lock;
static GCond      wake_cond;

/* the fences: flush waits for its own to be passed; fence_lock only
 * pairs the condition wait with the broadcast, the ticket is atomic */
static GMutex     fence_lock;
static GCond      fence_cond;
static gint       fence_passed;        /* atomic: the last ticket passed */
static guint      fence_issued;        /* game thread */

/* ---- voices ------------------------------------------------------------ */

void
grl_audio_voice_init (GrlAudioVoice            *voice,
                      const GrlAudioVoiceClass *klass)
{
    voice->klass = klass;
    voice->ref_count = 1;
    voice->requested = 0;
    voice->generation = 0;
    voice->predicted = FALSE;
    voice->resumable = FALSE;
    voice->applied = 0;
    voice->playing = 0;
    voice->watched = FALSE;
    voice->released = FALSE;
}

GrlAudioVoice *
grl_audio_voice_ref (GrlAudioVoice *voice)
{
    g_atomic_int_inc (&voice->ref_count);
    return voice;
}

void
grl_audio_voice_unref (GrlAudioVoice *voice)
{
    if (g_atomic_int_dec_and_test (&voice->ref_count))
        voice->klass->finalize (voice);
}

void
grl_audio_voice_publish_playing (GrlAudioVoice *voice,
                                 gboolean       playing)
{
    g_atomic_int_set (&voice->playing, playing ? 1 : 0);
}

/* ---- the thread's side ------------------------------------------------- */

/* the voices that play, refreshed every WATCH_INTERVAL_US (thread only) */
static GPtrArray *watch_list;

static void
watch_add (GrlAudioVoice *voice)
{
    if (voice->watched || voice->released)
        return;
    voice->watched = TRUE;
    g_ptr_array_add (watch_list, grl_audio_voice_ref (voice));
}

static void
watch_remove (GrlAudioVoice *voice)
{
    if (!voice->watched)
        return;
    voice->watched = FALSE;
    if (watch_list != NULL && g_ptr_array_remove_fast (watch_list, voice))
        grl_audio_voice_unref (voice);
}

/*
 * Runs one command; @threaded: on the thread (else inline, where nothing
 * is watched: getters ask raylib directly).
 */
static void
command_apply (Command  *cmd,
               gboolean  threaded)
{
    GrlAudioVoice *voice = cmd->voice;
    gboolean       plays;

    switch (cmd->kind)
    {
    case COMMAND_PLAIN:
        if (!voice->released)
            cmd->func (voice, cmd->value, cmd->data);
        break;

    case COMMAND_STATE:
        if (!voice->released)
        {
            if (cmd->func != NULL)
                cmd->func (voice, cmd->value, cmd->data);
            plays = voice->klass->refresh (voice);
            if (threaded)
            {
                if (plays)
                    watch_add (voice);
                else
                    watch_remove (voice);
            }
        }
        /* the state is published before the serial that makes it visible */
        g_atomic_int_set ((gint *)&voice->applied, (gint)cmd->serial);
        break;

    case COMMAND_UNWATCH:
        watch_remove (voice);
        break;

    case COMMAND_RELEASE:
        watch_remove (voice);
        if (!voice->released)
        {
            voice->released = TRUE;
            voice->klass->release (voice);
        }
        break;

    case COMMAND_FENCE:
        /* fences run in ticket order: the latest passed is this one */
        g_atomic_int_set (&fence_passed, (gint)cmd->serial);
        g_mutex_lock (&fence_lock);
        g_cond_broadcast (&fence_cond);
        g_mutex_unlock (&fence_lock);
        break;

    default:
        g_assert_not_reached ();
    }
}

static void
command_free (Command *cmd)
{
    if (cmd->data_free != NULL)
        cmd->data_free (cmd->data);
    if (cmd->voice != NULL)
        grl_audio_voice_unref (cmd->voice);
    g_free (cmd);
}

/* takes every queued command, oldest first */
static Command *
queue_take_all (void)
{
    Command *list;
    Command *ordered;

    do
        list = g_atomic_pointer_get (&queue_head);
    while (list != NULL && !g_atomic_pointer_compare_and_exchange (&queue_head, list, NULL));

    /* the stack is newest first */
    ordered = NULL;
    while (list != NULL)
    {
        Command *next = list->next;

        list->next = ordered;
        ordered = list;
        list = next;
    }
    return ordered;
}

/* drops what a command holds, keeping the command itself for reuse */
static void
command_clear (Command *cmd)
{
    if (cmd->data_free != NULL)
        cmd->data_free (cmd->data);
    cmd->data = NULL;
    cmd->data_free = NULL;
    if (cmd->voice != NULL)
        grl_audio_voice_unref (cmd->voice);
    cmd->voice = NULL;
}

/* hands applied (cleared) commands back for reuse */
static void
done_push_list (Command *first,
                Command *last)
{
    gpointer head;

    do
    {
        head = g_atomic_pointer_get (&done_head);
        last->next = head;
    }
    while (!g_atomic_pointer_compare_and_exchange (&done_head, head, first));
}

/* a producer thread's spare commands (a chain through next) */
static void
cache_free (gpointer data)
{
    Command *cmd = data;

    while (cmd != NULL)
    {
        Command *next = cmd->next;

        g_free (cmd);
        cmd = next;
    }
}

static GPrivate command_cache = G_PRIVATE_INIT (cache_free);

/* a command shell: from this thread's cache, refilled from done_head in
 * one exchange, else a new one */
static Command *
command_alloc (void)
{
    Command *cmd;

    cmd = g_private_get (&command_cache);
    if (cmd == NULL && g_atomic_pointer_get (&done_head) != NULL)
    {
        do
            cmd = g_atomic_pointer_get (&done_head);
        while (cmd != NULL && !g_atomic_pointer_compare_and_exchange (&done_head, cmd, NULL));
    }
    if (cmd == NULL)
        return g_new (Command, 1);
    g_private_set (&command_cache, cmd->next);
    return cmd;
}

static void
queue_drain (void)
{
    Command *first;
    Command *cmd;
    Command *last;

    first = queue_take_all ();
    last = NULL;
    for (cmd = first; cmd != NULL; cmd = cmd->next)
    {
        command_apply (cmd, TRUE);
        command_clear (cmd);
        last = cmd;
    }
    if (first != NULL)
        done_push_list (first, last);
}

/* refreshes (refills, for music) every voice that plays; drops the rest */
static void
watch_tick (void)
{
    guint i;

    i = 0;
    while (i < watch_list->len)
    {
        GrlAudioVoice *voice = g_ptr_array_index (watch_list, i);

        if (!voice->released && voice->klass->refresh (voice))
        {
            i++;
            continue;
        }
        voice->watched = FALSE;
        g_ptr_array_remove_index_fast (watch_list, i);
        grl_audio_voice_unref (voice);
    }
}

static gpointer
queue_main (gpointer data)
{
    gint64 next_tick;

    watch_list = g_ptr_array_new ();
    next_tick = g_get_monotonic_time () + WATCH_INTERVAL_US;

    for (;;)
    {
        gint64 now;

        queue_drain ();

        now = g_get_monotonic_time ();
        if (now >= next_tick)
        {
            watch_tick ();
            next_tick = now + WATCH_INTERVAL_US;
        }

        /* quit once everything pushed before the request has run */
        if (g_atomic_int_get (&queue_quit) && g_atomic_pointer_get (&queue_head) == NULL)
            break;

        /* sleep until a push (or the next tick while something plays) */
        g_mutex_lock (&wake_lock);
        g_atomic_int_set (&queue_sleeping, 1);
        if (g_atomic_pointer_get (&queue_head) == NULL && !g_atomic_int_get (&queue_quit))
        {
            gint64 until;

            until = watch_list->len > 0 ? next_tick : g_get_monotonic_time () + IDLE_WAIT_US;
            (void)g_cond_wait_until (&wake_cond, &wake_lock, until);
        }
        g_atomic_int_set (&queue_sleeping, 0);
        g_mutex_unlock (&wake_lock);
    }

    /* the voices stay loaded (their objects own them); only the watch goes */
    while (watch_list->len > 0)
    {
        GrlAudioVoice *voice = g_ptr_array_index (watch_list, watch_list->len - 1);

        g_ptr_array_remove_index_fast (watch_list, watch_list->len - 1);
        voice->watched = FALSE;
        grl_audio_voice_unref (voice);
    }
    g_clear_pointer (&watch_list, g_ptr_array_unref);
    return data;
}

/* ---- the producers' side ----------------------------------------------- */

/* wakes the thread without ever waiting for wake_lock (see above) */
static void
queue_wake (void)
{
    if (g_mutex_trylock (&wake_lock))
    {
        g_cond_signal (&wake_cond);
        g_mutex_unlock (&wake_lock);
    }
    else
    {
        g_cond_signal (&wake_cond);
    }
}

/* queues @cmd, or runs it at once when no thread runs */
static void
queue_push (Command *cmd)
{
    gpointer head;

    if (!g_atomic_int_get (&queue_running))
    {
        command_apply (cmd, FALSE);
        command_free (cmd);
        return;
    }

    do
    {
        head = g_atomic_pointer_get (&queue_head);
        cmd->next = head;
    }
    while (!g_atomic_pointer_compare_and_exchange (&queue_head, head, cmd));

    if (g_atomic_int_get (&queue_sleeping))
        queue_wake ();
}

static Command *
command_new (CommandKind        kind,
             GrlAudioVoice     *voice,
             GrlAudioVoiceFunc  func,
             gfloat             value,
             gpointer           data,
             GDestroyNotify     data_free)
{
    Command *cmd;

    cmd = command_alloc ();
    cmd->next = NULL;
    cmd->kind = kind;
    cmd->voice = voice != NULL ? grl_audio_voice_ref (voice) : NULL;
    cmd->func = func;
    cmd->value = value;
    cmd->data = data;
    cmd->data_free = data_free;
    cmd->serial = 0;
    return cmd;
}

void
grl_audio_voice_push (GrlAudioVoice     *voice,
                      GrlAudioVoiceFunc  func,
                      gfloat             value,
                      gpointer           data,
                      GDestroyNotify     data_free)
{
    g_return_if_fail (voice != NULL);
    g_return_if_fail (func != NULL);

    queue_push (command_new (COMMAND_PLAIN, voice, func, value, data, data_free));
}

void
grl_audio_voice_push_state (GrlAudioVoice     *voice,
                            GrlAudioVoiceFunc  func,
                            gfloat             value,
                            gpointer           data,
                            GDestroyNotify     data_free,
                            gboolean           predicted)
{
    Command *cmd;

    g_return_if_fail (voice != NULL);

    cmd = command_new (COMMAND_STATE, voice, func, value, data, data_free);

    /* the prediction holds until the thread publishes this serial */
    voice->requested++;
    voice->predicted = predicted != FALSE;
    voice->generation = g_atomic_int_get (&queue_running) ? queue_generation : 0;
    cmd->serial = voice->requested;
    queue_push (cmd);
}

void
grl_audio_voice_unwatch (GrlAudioVoice *voice)
{
    g_return_if_fail (voice != NULL);

    queue_push (command_new (COMMAND_UNWATCH, voice, NULL, 0.0f, NULL, NULL));
}

void
grl_audio_voice_release (GrlAudioVoice *voice)
{
    g_return_if_fail (voice != NULL);

    queue_push (command_new (COMMAND_RELEASE, voice, NULL, 0.0f, NULL, NULL));
    /* the command holds its own reference; the caller's goes */
    grl_audio_voice_unref (voice);
}

gboolean
grl_audio_voice_peek_playing (GrlAudioVoice *voice,
                              gboolean      *playing)
{
    g_return_val_if_fail (voice != NULL, FALSE);
    g_return_val_if_fail (playing != NULL, FALSE);

    /* inline, or last steered in an earlier run of the thread: ask raylib */
    if (!g_atomic_int_get (&queue_running) || voice->generation == 0 ||
        voice->generation != queue_generation)
        return FALSE;

    if ((guint)g_atomic_int_get ((gint *)&voice->applied) != voice->requested)
        *playing = voice->predicted;
    else
        *playing = g_atomic_int_get (&voice->playing) != 0;
    return TRUE;
}

gboolean
grl_audio_voice_is_settled (GrlAudioVoice *voice)
{
    g_return_val_if_fail (voice != NULL, TRUE);

    if (!g_atomic_int_get (&queue_running))
        return TRUE;
    return (guint)g_atomic_int_get ((gint *)&voice->applied) == voice->requested;
}

/* ---- lifecycle --------------------------------------------------------- */

void
grl_audio_queue_start (void)
{
    if (g_atomic_int_get (&queue_running))
        return;

    queue_generation++;
    if (queue_generation == 0)
        queue_generation = 1;
    g_atomic_int_set (&queue_quit, 0);
    queue_thread = g_thread_new ("grl-audio", queue_main, NULL);
    g_atomic_int_set (&queue_running, 1);
}

void
grl_audio_queue_stop (void)
{
    GThread *thread;

    if (!g_atomic_int_get (&queue_running))
        return;

    /* pushes from here on run inline; what is queued still runs first */
    g_atomic_int_set (&queue_running, 0);
    g_atomic_int_set (&queue_quit, 1);
    queue_wake ();

    thread = queue_thread;
    queue_thread = NULL;
    g_thread_join (thread);
}

gboolean
grl_audio_queue_is_running (void)
{
    return g_atomic_int_get (&queue_running) != 0;
}

void
grl_audio_queue_flush (void)
{
    Command *cmd;
    guint    ticket;

    if (!g_atomic_int_get (&queue_running))
        return;

    ticket = ++fence_issued;
    cmd = command_new (COMMAND_FENCE, NULL, NULL, 0.0f, NULL, NULL);
    cmd->serial = ticket;
    queue_push (cmd);

    g_mutex_lock (&fence_lock);
    while ((gint)((guint)g_atomic_int_get (&fence_passed) - ticket) < 0)
        g_cond_wait (&fence_cond, &fence_lock);
    g_mutex_unlock (&fence_lock);
}

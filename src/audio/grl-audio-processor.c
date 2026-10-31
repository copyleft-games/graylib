/* grl-audio-processor.c
 *
 * Copyright 2026 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Per-voice audio processors over raylib's AttachAudioStreamProcessor().
 *
 * raylib's stream processor is a bare `void (*)(void *, unsigned int)`:
 * no user data, and the samples it is handed live in a scratch buffer
 * that says nothing about which voice they belong to.  So graylib keeps
 * a fixed table of GRL_AUDIO_PROCESSOR_MAX_SLOTS trampolines, each a
 * distinct static function that knows its own slot index; adding a
 * processor takes a free slot, stores the callback and its data there
 * and attaches that slot's trampoline to the voice's stream.  raylib
 * walks the voice's processor list in order and calls each trampoline,
 * which forwards to the stored callback.
 *
 * Threading: raylib calls the trampolines on miniaudio's thread with its
 * audio lock held; Attach/DetachAudioStreamProcessor take the same
 * lock.  A slot's callback is therefore written before its trampoline
 * is attached and cleared only after it was detached, so the device
 * thread never sees a half-written slot and needs no lock of its own.
 * slot_lock only guards which slots are taken, among callers on other
 * threads, and is never held while raylib is called.
 *
 * A detach raylib cannot take (the device is closed: its lock is gone)
 * leaves the trampoline on the voice's buffer.  When the buffer is about
 * to be freed that does not matter and the slot is reused; otherwise the
 * slot is retired for good (its callback cleared, never handed out
 * again), so a voice that outlives a device restart can never call a
 * processor that was given to another voice since.
 */

#include "config.h"

#include "grl-audio-processor.h"
#include "grl-audio-processor-private.h"

typedef struct
{
    GrlAudioProcessFunc func;      /* NULL: nothing runs */
    gpointer            user_data;
    gboolean            taken;     /* handed out (or retired) */
} ProcessorSlot;

typedef struct
{
    guint               id;
    guint               slot;
    gboolean            attached;  /* the trampoline sits on a raylib stream */
    GrlAudioProcessFunc func;
    gpointer            user_data;
    GDestroyNotify      destroy;
} ProcessorEntry;

struct _GrlAudioProcessorSet
{
    GPtrArray *entries;  /* ProcessorEntry *, in run order */
};

static GMutex        slot_lock;
static ProcessorSlot slots[GRL_AUDIO_PROCESSOR_MAX_SLOTS];
static guint         n_taken;
static guint         next_id = 1;

/* what every trampoline does: the slot's callback on raylib's block */
static void
run_slot (guint         slot,
          void         *buffer,
          unsigned int  frames)
{
    GrlAudioProcessFunc func;

    func = slots[slot].func;
    if (func != NULL && buffer != NULL && frames > 0)
        func ((gfloat *)buffer, (guint)frames, GRL_AUDIO_PROCESSOR_CHANNELS, slots[slot].user_data);
}

/*
 * The trampolines: TRAMPOLINE (hi, lo) is slot hi * 16 + lo, sixteen rows
 * of sixteen make the GRL_AUDIO_PROCESSOR_MAX_SLOTS (256) of them.
 */
#define TRAMPOLINE(hi, lo) \
    static void \
    trampoline_##hi##_##lo (void *buffer, unsigned int frames) \
    { \
        run_slot ((guint)((hi) * 16 + (lo)), buffer, frames); \
    }

#define TRAMPOLINE_ROW(hi) \
    TRAMPOLINE (hi, 0)  TRAMPOLINE (hi, 1)  TRAMPOLINE (hi, 2)  TRAMPOLINE (hi, 3) \
    TRAMPOLINE (hi, 4)  TRAMPOLINE (hi, 5)  TRAMPOLINE (hi, 6)  TRAMPOLINE (hi, 7) \
    TRAMPOLINE (hi, 8)  TRAMPOLINE (hi, 9)  TRAMPOLINE (hi, 10) TRAMPOLINE (hi, 11) \
    TRAMPOLINE (hi, 12) TRAMPOLINE (hi, 13) TRAMPOLINE (hi, 14) TRAMPOLINE (hi, 15)

TRAMPOLINE_ROW (0)  TRAMPOLINE_ROW (1)  TRAMPOLINE_ROW (2)  TRAMPOLINE_ROW (3)
TRAMPOLINE_ROW (4)  TRAMPOLINE_ROW (5)  TRAMPOLINE_ROW (6)  TRAMPOLINE_ROW (7)
TRAMPOLINE_ROW (8)  TRAMPOLINE_ROW (9)  TRAMPOLINE_ROW (10) TRAMPOLINE_ROW (11)
TRAMPOLINE_ROW (12) TRAMPOLINE_ROW (13) TRAMPOLINE_ROW (14) TRAMPOLINE_ROW (15)

#define TRAMPOLINE_REF_ROW(hi) \
    trampoline_##hi##_0,  trampoline_##hi##_1,  trampoline_##hi##_2,  trampoline_##hi##_3, \
    trampoline_##hi##_4,  trampoline_##hi##_5,  trampoline_##hi##_6,  trampoline_##hi##_7, \
    trampoline_##hi##_8,  trampoline_##hi##_9,  trampoline_##hi##_10, trampoline_##hi##_11, \
    trampoline_##hi##_12, trampoline_##hi##_13, trampoline_##hi##_14, trampoline_##hi##_15

static const AudioCallback trampolines[GRL_AUDIO_PROCESSOR_MAX_SLOTS] = {
    TRAMPOLINE_REF_ROW (0),  TRAMPOLINE_REF_ROW (1),  TRAMPOLINE_REF_ROW (2),  TRAMPOLINE_REF_ROW (3),
    TRAMPOLINE_REF_ROW (4),  TRAMPOLINE_REF_ROW (5),  TRAMPOLINE_REF_ROW (6),  TRAMPOLINE_REF_ROW (7),
    TRAMPOLINE_REF_ROW (8),  TRAMPOLINE_REF_ROW (9),  TRAMPOLINE_REF_ROW (10), TRAMPOLINE_REF_ROW (11),
    TRAMPOLINE_REF_ROW (12), TRAMPOLINE_REF_ROW (13), TRAMPOLINE_REF_ROW (14), TRAMPOLINE_REF_ROW (15)
};

/* raylib's bool read through a byte (graylib's bool/gboolean rule) */
static gboolean
device_ready (void)
{
    unsigned char raw;

    raw = IsAudioDeviceReady ();
    return raw != 0;
}

/* a free slot (marked taken) and a fresh id, or FALSE when none is free */
static gboolean
take_slot (guint *out_slot,
           guint *out_id)
{
    guint i;

    g_mutex_lock (&slot_lock);
    for (i = 0; i < GRL_AUDIO_PROCESSOR_MAX_SLOTS; i++)
    {
        if (!slots[i].taken)
        {
            slots[i].taken = TRUE;
            n_taken++;
            *out_slot = i;
            *out_id = next_id++;
            if (next_id == 0)
                next_id = 1;
            g_mutex_unlock (&slot_lock);
            return TRUE;
        }
    }
    g_mutex_unlock (&slot_lock);
    return FALSE;
}

/* gives a slot back (its callback already cleared) */
static void
release_slot (guint slot)
{
    g_mutex_lock (&slot_lock);
    if (slots[slot].taken)
    {
        slots[slot].taken = FALSE;
        n_taken--;
    }
    g_mutex_unlock (&slot_lock);
}

/*
 * Takes one entry off raylib's stream and frees it.  The slot's callback
 * is cleared only once raylib let go of the trampoline (the detach holds
 * the audio lock, so no block is running it afterwards).
 */
static void
entry_drop (ProcessorEntry    *entry,
            const AudioStream *stream,
            gboolean           unloading)
{
    gboolean reusable;

    reusable = TRUE;
    if (entry->attached)
    {
        if (stream != NULL && stream->buffer != NULL && device_ready ())
            DetachAudioStreamProcessor (*stream, trampolines[entry->slot]);
        else if (!unloading)
            reusable = FALSE;  /* the trampoline stays on a live buffer */
        entry->attached = FALSE;
    }

    slots[entry->slot].func = NULL;
    slots[entry->slot].user_data = NULL;
    if (reusable)
        release_slot (entry->slot);

    if (entry->destroy != NULL)
        entry->destroy (entry->user_data);
    g_free (entry);
}

guint
grl_audio_processor_get_n_free_slots (void)
{
    guint n_free;

    g_mutex_lock (&slot_lock);
    n_free = GRL_AUDIO_PROCESSOR_MAX_SLOTS - n_taken;
    g_mutex_unlock (&slot_lock);
    return n_free;
}

GrlAudioProcessorSet *
grl_audio_processor_set_new (void)
{
    GrlAudioProcessorSet *set;

    set = g_new0 (GrlAudioProcessorSet, 1);
    set->entries = g_ptr_array_new ();
    return set;
}

guint
grl_audio_processor_set_add (GrlAudioProcessorSet *set,
                             const AudioStream    *stream,
                             GrlAudioProcessFunc   func,
                             gpointer              user_data,
                             GDestroyNotify        destroy)
{
    ProcessorEntry *entry;
    guint           slot;
    guint           id;

    if (!take_slot (&slot, &id))
    {
        g_warning ("graylib: all %u audio processor slots are taken", (guint)GRL_AUDIO_PROCESSOR_MAX_SLOTS);
        if (destroy != NULL)
            destroy (user_data);
        return 0;
    }

    entry = g_new0 (ProcessorEntry, 1);
    entry->id = id;
    entry->slot = slot;
    entry->func = func;
    entry->user_data = user_data;
    entry->destroy = destroy;

    /* the slot is filled before raylib can call its trampoline */
    slots[slot].user_data = user_data;
    slots[slot].func = func;
    if (stream != NULL && stream->buffer != NULL && device_ready ())
    {
        AttachAudioStreamProcessor (*stream, trampolines[slot]);
        entry->attached = TRUE;
    }

    g_ptr_array_add (set->entries, entry);
    return id;
}

gboolean
grl_audio_processor_set_remove (GrlAudioProcessorSet *set,
                                const AudioStream    *stream,
                                guint                 id)
{
    guint i;

    for (i = 0; i < set->entries->len; i++)
    {
        ProcessorEntry *entry;

        entry = (ProcessorEntry *)g_ptr_array_index (set->entries, i);
        if (entry->id == id)
        {
            /* out of the list first: a destroy notify may look at it */
            g_ptr_array_remove_index (set->entries, i);
            entry_drop (entry, stream, FALSE);
            return TRUE;
        }
    }
    return FALSE;
}

void
grl_audio_processor_set_clear (GrlAudioProcessorSet *set,
                               const AudioStream    *stream,
                               gboolean              unloading)
{
    while (set->entries->len > 0)
    {
        ProcessorEntry *entry;

        entry = (ProcessorEntry *)g_ptr_array_steal_index (set->entries, set->entries->len - 1);
        entry_drop (entry, stream, unloading);
    }
}

void
grl_audio_processor_set_free (GrlAudioProcessorSet *set)
{
    if (set == NULL)
        return;
    /* an owner that forgot to clear still gives its slots back */
    grl_audio_processor_set_clear (set, NULL, FALSE);
    g_ptr_array_unref (set->entries);
    g_free (set);
}

guint
grl_audio_processor_set_get_count (GrlAudioProcessorSet *set)
{
    return set != NULL ? set->entries->len : 0;
}

void
grl_audio_processor_set_run (GrlAudioProcessorSet *set,
                             gfloat               *frames,
                             guint                 n_frames)
{
    guint i;

    if (set == NULL || frames == NULL || n_frames == 0)
        return;
    for (i = 0; i < set->entries->len; i++)
    {
        ProcessorEntry *entry;

        entry = (ProcessorEntry *)g_ptr_array_index (set->entries, i);
        entry->func (frames, n_frames, GRL_AUDIO_PROCESSOR_CHANNELS, entry->user_data);
    }
}

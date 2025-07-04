/*
 * Macros are stored in a dynamically growing list (macro_list).
 * The editor tracks the active macro through the global current_macro pointer.
 * Each macro records key codes which can later be replayed.
 * These routines manage creation, recording and playback of those macros.
 */
#include "macro.h"
#include <stdlib.h>
#include <string.h>

typedef struct {
    Macro **items;
    int count;
    int capacity;
} MacroList;

static MacroList macro_list = {0};
extern Macro *current_macro;
MacroState macro_state = {false, false};

static int ensure_capacity(void) {
    if (macro_list.count >= macro_list.capacity) {
        int newcap = macro_list.capacity ? macro_list.capacity * 2 : 4;
        Macro **tmp = realloc(macro_list.items, sizeof(Macro*) * newcap);
        if (!tmp)
            return -1;
        macro_list.items = tmp;
        macro_list.capacity = newcap;
    }
    return 0;
}

/*
 * Create a new macro with the given name and add it to the global list.
 * If this is the first macro created it becomes the current_macro.
 */
Macro *macro_create(const char *name, int play_key) {
    if (!name)
        return NULL;
    if (ensure_capacity() != 0)
        return NULL;
    Macro *m = calloc(1, sizeof(Macro));
    if (!m)
        return NULL;
    m->name = strdup(name);
    if (!m->name) {
        free(m);
        return NULL;
    }
    m->play_key = play_key;
    m->active = false;
    macro_list.items[macro_list.count++] = m;
    if (!current_macro) {
        current_macro = m;
        m->active = true;
    }
    return m;
}

/*
 * Find a macro by name.  Passing NULL returns the current_macro.
 */
Macro *macro_get(const char *name) {
    if (!name)
        return current_macro;
    for (int i = 0; i < macro_list.count; ++i) {
        if (strcmp(macro_list.items[i]->name, name) == 0)
            return macro_list.items[i];
    }
    return NULL;
}

/*
 * Set the provided macro as the current active macro.
 * Clears the active flag on all other macros.
 */
void macro_set_current(Macro *m) {
    if (!m)
        return;
    current_macro = m;
    for (int i = 0; i < macro_list.count; ++i)
        macro_list.items[i]->active = (macro_list.items[i] == m);
}

/*
 * Remove the macro with the specified name from the list and free it.
 * Updates current_macro if the deleted macro was active.
 */
void macro_delete(const char *name) {
    if (!name)
        return;
    for (int i = 0; i < macro_list.count; ++i) {
        Macro *m = macro_list.items[i];
        if (strcmp(m->name, name) == 0) {
            free(m->name);
            free(m);
            for (int j = i; j < macro_list.count - 1; ++j)
                macro_list.items[j] = macro_list.items[j + 1];
            macro_list.count--;

            /* Update current macro if the deleted one was active */
            if (current_macro == m) {
                Macro *newm = macro_list.count ? macro_list.items[0] : NULL;
                if (newm)
                    macro_set_current(newm);
                else
                    current_macro = NULL;
            }

            /*
             * Shrink the storage array when the list becomes much smaller
             * than its capacity. Free all storage when no macros remain.
             */
            if (macro_list.count == 0) {
                free(macro_list.items);
                macro_list.items = NULL;
                macro_list.capacity = 0;
            } else if (macro_list.capacity > 4 &&
                       macro_list.count < macro_list.capacity / 2) {
                int newcap = macro_list.capacity / 2;
                if (newcap < 4)
                    newcap = 4;
                Macro **tmp = realloc(macro_list.items,
                                      sizeof(Macro*) * newcap);
                if (tmp) {
                    macro_list.items = tmp;
                    macro_list.capacity = newcap;
                }
            }
            return;
        }
    }
}

/* Begin recording into the given macro, resetting its contents. */
void macro_start(Macro *macro) {
    if (!macro)
        return;
    macro->length = 0;
    macro->recording = true;
    macro_state.recording = true;
}

/* Stop recording the given macro. */
void macro_stop(Macro *macro) {
    if (!macro)
        return;
    macro->recording = false;
    macro_state.recording = false;
}

/* Store a key press in a macro while recording. */
void macro_record_key(Macro *macro, wint_t ch) {
    if (!macro || !macro->recording)
        return;
    if (macro->length >= MACRO_MAX_KEYS)
        return;
    macro->keys[macro->length++] = ch;
}

/*
 * Replay the macro 'count' times. This simply feeds each recorded key
 * back through the regular input handler so playback behaves the same as
 * if the user typed the keys manually.
 */
void macro_play_times(Macro *macro, EditorContext *ctx, FileState *fs, int count) {
    if (!macro || macro->recording || count <= 0)
        return;

    macro_state.playing = true;
    if (ctx)
        update_status_bar(ctx, fs);

    for (int n = 0; n < count; ++n) {
        for (int i = 0; i < macro->length; ++i)
            handle_regular_mode(ctx, fs, macro->keys[i]);
    }

    macro_state.playing = false;
    if (ctx)
        update_status_bar(ctx, fs);
}

/*
 * Replay all recorded keys in the macro.  The editor context and
 * file state are used so playback mimics user input.
 */
void macro_play(Macro *macro, EditorContext *ctx, FileState *fs) {
    macro_play_times(macro, ctx, fs, 1);
}

/* Return the number of macros currently stored. */
int macro_count(void) {
    return macro_list.count;
}

/* Internal capacity exposed for testing purposes */
int macro_capacity(void) {
    return macro_list.capacity;
}

/* Retrieve a macro by list index for enumeration. */
Macro *macro_at(int index) {
    if (index < 0 || index >= macro_list.count)
        return NULL;
    return macro_list.items[index];
}

/*
 * Rename the given macro to the provided new name. Memory for the
 * name string is managed here, freeing the old name and duplicating
 * the new one.
 */
void macro_rename(Macro *m, const char *new_name) {
    if (!m || !new_name || new_name[0] == '\0')
        return;
    char *dup = strdup(new_name);
    if (!dup)
        return;
    free(m->name);
    m->name = dup;
}

/*
 * Free all macros and reset the macro list.
 */
void macros_free_all(void) {
    for (int i = 0; i < macro_list.count; ++i) {
        Macro *m = macro_list.items[i];
        if (!m)
            continue;
        free(m->name);
        free(m);
    }
    free(macro_list.items);
    macro_list.items = NULL;
    macro_list.count = 0;
    macro_list.capacity = 0;
    current_macro = NULL;
    macro_state.recording = false;
    macro_state.playing = false;
}

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
Macro *macro_create(const char *name) {
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
    m->play_key = 0;
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
            if (current_macro == m) {
                Macro *newm = macro_list.count ? macro_list.items[0] : NULL;
                if (newm)
                    macro_set_current(newm);
                else
                    current_macro = NULL;
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
 * Replay all recorded keys in the macro.  The editor context and
 * file state are used so playback mimics user input.
 */
void macro_play(Macro *macro, EditorContext *ctx, FileState *fs) {
    if (!macro || macro->recording)
        return;
    macro_state.playing = true;
    if (ctx)
        update_status_bar(ctx, fs);
    for (int i = 0; i < macro->length; ++i) {
        handle_regular_mode(ctx, fs, macro->keys[i]);
    }
    macro_state.playing = false;
    if (ctx)
        update_status_bar(ctx, fs);
}

/* Return the number of macros currently stored. */
int macro_count(void) {
    return macro_list.count;
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

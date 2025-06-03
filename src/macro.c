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
    macro_list.items[macro_list.count++] = m;
    if (!current_macro)
        current_macro = m;
    return m;
}

Macro *macro_get(const char *name) {
    if (!name)
        return current_macro;
    for (int i = 0; i < macro_list.count; ++i) {
        if (strcmp(macro_list.items[i]->name, name) == 0)
            return macro_list.items[i];
    }
    return NULL;
}

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
            if (current_macro == m)
                current_macro = macro_list.count ? macro_list.items[0] : NULL;
            return;
        }
    }
}

void macro_start(Macro *macro) {
    if (!macro)
        return;
    macro->length = 0;
    macro->recording = true;
    macro_state.recording = true;
}

void macro_stop(Macro *macro) {
    if (!macro)
        return;
    macro->recording = false;
    macro_state.recording = false;
}

void macro_record_key(Macro *macro, wint_t ch) {
    if (!macro || !macro->recording)
        return;
    if (macro->length >= MACRO_MAX_KEYS)
        return;
    macro->keys[macro->length++] = ch;
}

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

int macro_count(void) {
    return macro_list.count;
}

Macro *macro_at(int index) {
    if (index < 0 || index >= macro_list.count)
        return NULL;
    return macro_list.items[index];
}

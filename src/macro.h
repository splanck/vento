#ifndef MACRO_H
#define MACRO_H

#include <wchar.h>
#include <stdbool.h>
#include "editor.h"
#include "files.h"
#include <stddef.h>

#define MACRO_MAX_KEYS 1024

typedef struct Macro {
    char *name;
    wint_t keys[MACRO_MAX_KEYS];
    int length;
    int play_key;
    bool recording;
    bool active;
} Macro;

Macro *macro_create(const char *name, int play_key);
void macro_set_current(Macro *m);
Macro *macro_get(const char *name);
void macro_delete(const char *name);
void macro_start(Macro *macro);
void macro_stop(Macro *macro);
void macro_record_key(Macro *macro, wint_t ch);
void macro_play_times(Macro *m, EditorContext *ctx, FileState *fs, int count);
void macro_play(Macro *macro, EditorContext *ctx, FileState *fs);
int macro_count(void);
int macro_capacity(void);
Macro *macro_at(int index);
void macro_rename(Macro *m, const char *new_name);
void macros_free_all(void);

typedef struct {
    bool recording;
    bool playing;
} MacroState;

extern MacroState macro_state;

#endif /* MACRO_H */

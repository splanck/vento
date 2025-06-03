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
    bool recording;
} Macro;

Macro *macro_create(const char *name);
Macro *macro_get(const char *name);
void macro_delete(const char *name);
void macro_start(Macro *macro);
void macro_stop(Macro *macro);
void macro_record_key(Macro *macro, wint_t ch);
void macro_play(Macro *macro, EditorContext *ctx, FileState *fs);
int macro_count(void);
Macro *macro_at(int index);

typedef struct {
    bool recording;
    bool playing;
} MacroState;

extern MacroState macro_state;

#endif /* MACRO_H */

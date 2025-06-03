#ifndef MACRO_H
#define MACRO_H

#include <wchar.h>
#include <stdbool.h>
#include "editor.h"
#include "files.h"

#define MACRO_MAX_KEYS 1024

typedef struct Macro {
    wint_t keys[MACRO_MAX_KEYS];
    int length;
    bool recording;
} Macro;

void macro_start(Macro *macro);
void macro_stop(Macro *macro);
void macro_record_key(Macro *macro, wint_t ch);
void macro_play(Macro *macro, EditorContext *ctx, FileState *fs);

extern Macro macro_state;

#endif /* MACRO_H */

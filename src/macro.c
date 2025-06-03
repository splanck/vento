#include "macro.h"

void macro_start(Macro *macro) {
    if (!macro)
        return;
    macro->length = 0;
    macro->recording = true;
}

void macro_stop(Macro *macro) {
    if (!macro)
        return;
    macro->recording = false;
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
    for (int i = 0; i < macro->length; ++i) {
        handle_regular_mode(ctx, fs, macro->keys[i]);
    }
}

#ifndef EDITOR_STATE_H
#define EDITOR_STATE_H

#include "file_manager.h"
#include "config.h"

struct EditorContext {
    FileManager file_manager;
    AppConfig   config;
    FileState  *active_file;
    WINDOW     *text_win;
    int enable_color;
    int enable_mouse;
};

static inline void sync_editor_context(struct EditorContext *ctx) {
    if (!ctx) return;
    ctx->file_manager = file_manager;
    ctx->active_file = active_file;
    ctx->text_win = text_win;
}

#endif // EDITOR_STATE_H


#ifndef EDITOR_STATE_H
#define EDITOR_STATE_H

#include "file_manager.h"
#include "config.h"

typedef struct EditorContext {
    FileManager file_manager;
    AppConfig   config;
    FileState  *active_file;
    WINDOW     *text_win;
    int enable_color;
    int enable_mouse;
} EditorContext;

#endif // EDITOR_STATE_H


#ifndef FILES_H
#define FILES_H

#include <ncurses.h>
#include "editor.h"

typedef struct {
    char filename[256];
    char **text_buffer;
    int line_count;
    int start_line;
    int cursor_x, cursor_y;
    Node *undo_stack;
    Node *redo_stack;
    bool selection_mode;
    int sel_start_x, sel_start_y;
    int sel_end_x, sel_end_y;
    char *clipboard;
    int syntax_mode;
    WINDOW *text_win;
} FileState;

FileState *initialize_file_state(const char *filename, int max_lines, int max_cols);
void free_file_state(FileState *file_state, int max_lines);
int load_file_into_buffer(FileState *file_state);

#endif
#ifndef FILES_H
#define FILES_H

#include <ncurses.h>

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

#endif
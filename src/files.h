#ifndef FILES_H
#define FILES_H

#include <ncurses.h>
#include "editor.h"

typedef struct FileState {
    char filename[256];
    char **text_buffer;
    int line_count;
    int max_lines;
    int start_line;
    int cursor_x, cursor_y;
    int line_capacity;
    Node *undo_stack;
    Node *redo_stack;
    bool selection_mode;
    int sel_start_x, sel_start_y;
    int sel_end_x, sel_end_y;
    char *clipboard;
    int syntax_mode;
    bool in_multiline_comment;
    /* The last line index scanned for multiline comment state */
    int last_scanned_line;
    /* Comment state after scanning last_scanned_line */
    bool last_comment_state;
    WINDOW *text_win;
} FileState;

FileState *initialize_file_state(const char *filename, int max_lines, int max_cols);
void free_file_state(FileState *file_state, int max_lines);
int load_file_into_buffer(FileState *file_state);
int ensure_line_capacity(FileState *fs, int min_needed);

#endif

#ifndef FILES_H
#define FILES_H

#include <ncurses.h>
#include <stdbool.h>
#include <stdio.h>
#include "editor.h"
#include "line_buffer.h"

typedef struct FileState {
    char filename[256];
    LineBuffer buffer;
    int start_line;
    int scroll_x; /* leftmost visible column */
    int cursor_x, cursor_y;
    int saved_cursor_x, saved_cursor_y;
    int line_capacity;
    Node *undo_stack;
    Node *redo_stack;
    bool selection_mode;
    int sel_start_x, sel_start_y;
    int sel_end_x, sel_end_y;
    /* Coordinates of the most recent search match within the buffer. */
    int match_start_x, match_start_y;
    int match_end_x, match_end_y;
    int syntax_mode;
    bool in_multiline_comment;
    bool in_multiline_string;
    char string_delim;
    /* The last line index scanned for multiline comment state */
    int last_scanned_line;
    /* Comment state after scanning last_scanned_line */
    bool last_comment_state;
    int nested_mode; /* 0=none,1=JS,2=CSS */
    WINDOW *text_win;
    FILE *fp;          /* Open file handle for lazy loading */
    long file_pos;     /* Offset of fp when partially loaded */
    bool file_complete;/* True when the entire file is loaded */
    bool modified;     /* True if the buffer has unsaved changes */
} FileState;

FileState *initialize_file_state(const char *filename, int max_lines, int max_cols);
void free_file_state(FileState *file_state);
int load_file_into_buffer(FileState *file_state);
int ensure_line_capacity(FileState *fs, int min_needed);
int ensure_col_capacity(FileState *fs, int cols);
int load_next_lines(FileState *fs, int count);
void ensure_line_loaded(FileState *fs, int idx);
void load_all_remaining_lines(FileState *fs);

#endif

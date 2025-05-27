#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include <ncurses.h>
#include <stdbool.h>

#define CLIPBOARD_SIZE 4096

#include "files.h"
extern char *clipboard;
extern bool selection_mode;
extern int sel_start_x, sel_start_y;
extern int sel_end_x, sel_end_y;

void start_selection_mode(FileState *fs, int cursor_x, int cursor_y);
void end_selection_mode(FileState *fs);
void copy_selection(FileState *fs);
void paste_clipboard(FileState *fs, int *cursor_x, int *cursor_y);
void handle_selection_mode(FileState *fs, int ch, int *cursor_x, int *cursor_y);

#endif // CLIPBOARD_H

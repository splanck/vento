#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include <ncurses.h>
#include <stdbool.h>

#define CLIPBOARD_SIZE 4096

#include "files.h"

void start_selection_mode(FileState *fs, int cursor_x, int cursor_y);
void end_selection_mode(FileState *fs);
void copy_selection(FileState *fs);
void paste_clipboard(FileState *fs, int *cursor_x, int *cursor_y);
void handle_selection_mode(FileState *fs, int ch, int *cursor_x, int *cursor_y);
void copy_selection_keyboard(FileState *fs);
void cut_selection(FileState *fs);

#endif // CLIPBOARD_H

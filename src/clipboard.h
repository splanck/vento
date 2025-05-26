#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include <ncurses.h>
#include <stdbool.h>

#define CLIPBOARD_SIZE 4096

extern char *clipboard;
extern bool selection_mode;
extern int sel_start_x, sel_start_y;
extern int sel_end_x, sel_end_y;

void start_selection_mode(int cursor_x, int cursor_y);
void end_selection_mode();
void copy_selection();
void paste_clipboard(int *cursor_x, int *cursor_y);
void handle_selection_mode(int ch, int *cursor_x, int *cursor_y);

#endif // CLIPBOARD_H

#ifndef DIALOG_H
#define DIALOG_H

#include <ncurses.h>
#include <stddef.h>

WINDOW *dialog_open(int height, int width, const char *title);
void   dialog_close(WINDOW *win);
int    dialog_prompt(WINDOW *win, int y, int x, char *buf, size_t len);

#endif // DIALOG_H

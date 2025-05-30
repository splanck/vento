#ifndef UI_COMMON_H
#define UI_COMMON_H

#include <ncurses.h>
#include <stddef.h>

WINDOW *create_centered_window(int height, int width, WINDOW *parent);
WINDOW *create_popup_window(int height, int width, WINDOW *parent);
int show_message(const char *msg);
int show_scrollable_window(const char **options, int count, WINDOW *parent);
void str_to_upper(char *dst, const char *src, size_t dst_size);

#endif // UI_COMMON_H

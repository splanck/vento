#include "ui_common.h"
#include "config.h"
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>

WINDOW *create_centered_window(int height, int width, WINDOW *parent) {
    int win_y, win_x;

    if (parent) {
        int py, px, ph, pw;
        getbegyx(parent, py, px);
        getmaxyx(parent, ph, pw);
        win_y = py + (ph - height) / 2;
        win_x = px + (pw - width) / 2;
    } else {
        win_y = (LINES - height) / 2;
        win_x = (COLS - width) / 2;
    }

    WINDOW *win = newwin(height, width, win_y, win_x);
    if (!win) {
        endwin();
        fprintf(stderr, "Failed to create new window\n");
        exit(EXIT_FAILURE);
    }

    box(win, 0, 0);

    return win;
}

WINDOW *create_popup_window(int height, int width, WINDOW *parent) {
    return create_centered_window(height, width, parent);
}

void show_message(const char *msg) {
    curs_set(0);
    int win_height = 3;
    int win_width = (int)strlen(msg) + 4;
    int win_y = (LINES - win_height) / 2;
    int win_x = (COLS - win_width) / 2;

    WINDOW *win = newwin(win_height, win_width, win_y, win_x);
    box(win, 0, 0);
    mvwprintw(win, 1, 2, "%s", msg);
    wrefresh(win);
    wgetch(win);
    wclear(win);
    wrefresh(win);
    delwin(win);
    wrefresh(stdscr);
    curs_set(1);
}

int show_scrollable_window(const char **options, int count, WINDOW *parent) {
    curs_set(0);
    int highlight = 0;
    int ch;
    int start = 0;

    if (count <= 0)
        return -1;

    int own = 0;
    int win_height, win_width;
    WINDOW *win;
    if (parent) {
        int ph, pw;
        getmaxyx(parent, ph, pw);
        win_height = ph - 4;
        win_width = pw - 4;
        win = create_popup_window(win_height, win_width, parent);
        if (!win) {
            curs_set(1);
            return -1;
        }
        own = 1;
    } else {
        win_height = LINES - 4;
        win_width = COLS - 4;
        win = create_popup_window(win_height, win_width, NULL);
        if (!win) {
            curs_set(1);
            return -1;
        }
        own = 1;
    }
    keypad(win, TRUE);

    while (1) {
        werase(win);
        box(win, 0, 0);

        int max_display = win_height - 2; // leave last line for instructions
        if (highlight < start)
            start = highlight;
        if (highlight >= start + max_display)
            start = highlight - max_display + 1;

        for (int i = 0; i < max_display && i + start < count; ++i) {
            int idx = i + start;
            if (idx == highlight)
                wattron(win, A_REVERSE);
            mvwprintw(win, i + 1, 1, "%s", options[idx]);
            wattroff(win, A_REVERSE);
        }

        for (int i = count - start; i < max_display; ++i) {
            mvwprintw(win, i + 1, 1, "%*s", win_width - 2, "");
        }

        mvwprintw(win, win_height - 1, 1, "Arrows: move  Enter: select  ESC: cancel");
        wrefresh(win);

        ch = wgetch(win);
        if (ch == KEY_UP) {
            if (highlight > 0)
                --highlight;
        } else if (ch == KEY_DOWN) {
            if (highlight < count - 1)
                ++highlight;
        } else if (ch == '\n') {
            if (own) {
                werase(win);
                wrefresh(win);
                delwin(win);
                if (parent) {
                    touchwin(parent);
                    wrefresh(parent);
                } else {
                    wrefresh(stdscr);
                }
            }
            curs_set(1);
            return highlight;
        } else if (ch == 27) {
            if (own) {
                werase(win);
                wrefresh(win);
                delwin(win);
                if (parent) {
                    touchwin(parent);
                    wrefresh(parent);
                } else {
                    wrefresh(stdscr);
                }
            }
            curs_set(1);
            return -1;
        } else if (ch == KEY_MOUSE && enable_mouse) {
            MEVENT ev;
            if (getmouse(&ev) == OK &&
                (ev.bstate & (BUTTON1_PRESSED | BUTTON1_CLICKED | BUTTON1_RELEASED))) {
                int wy, wx;
                getbegyx(win, wy, wx);
                int row = ev.y - wy - 1;
                int col = ev.x - wx - 1;
                int max_display = win_height - 2;
                if (row >= 0 && row < max_display &&
                    col >= 0 && col < win_width - 2) {
                    int idx = start + row;
                    if (idx >= 0 && idx < count)
                        highlight = idx;

                    if (ev.bstate & (BUTTON1_RELEASED | BUTTON1_CLICKED)) {
                        if (own) {
                            werase(win);
                            wrefresh(win);
                            delwin(win);
                            if (parent) {
                                touchwin(parent);
                                wrefresh(parent);
                            } else {
                                wrefresh(stdscr);
                            }
                        }
                        return highlight;
                    }
                }
            }
        }
    }

    curs_set(1);
    return -1;
}

void str_to_upper(char *dst, const char *src, size_t dst_size) {
    size_t i;
    for (i = 0; i + 1 < dst_size && src[i]; ++i) {
        dst[i] = toupper((unsigned char)src[i]);
    }
    dst[i] = '\0';
}


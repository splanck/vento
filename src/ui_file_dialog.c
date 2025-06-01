#include "ui_common.h"
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <ncurses.h>
#include <stdio.h>
#include "config.h"
#include "editor_state.h"

void get_dir_contents(const char *dir_path, char ***choices, int *n_choices) {
    DIR *dir;
    struct dirent *entry;
    int count = 0;

    dir = opendir(dir_path);
    if (!dir) {
        *choices = NULL;
        *n_choices = 0;
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        count++;
    }
    rewinddir(dir);

    *choices = (char **)malloc(count * sizeof(char *));
    if (!*choices) {
        closedir(dir);
        *choices = NULL;
        *n_choices = 0;
        return;
    }

    int i = 0;
    while ((entry = readdir(dir)) != NULL) {
        (*choices)[i] = strdup(entry->d_name);
        if (!(*choices)[i]) {
            for (int j = 0; j < i; ++j) {
                free((*choices)[j]);
            }
            free(*choices);
            closedir(dir);
            *choices = NULL;
            *n_choices = 0;
            return;
        }
        i++;
    }
    closedir(dir);

    *n_choices = count;
}

void free_dir_contents(char **choices, int n_choices) {
    for (int i = 0; i < n_choices; ++i) {
        free(choices[i]);
    }
    free(choices);
}

static int file_dialog_loop(char *path, int max_len,
                            int cursor_on_enter, int cursor_on_mouse) {
    curs_set(0);
    int highlight = 0;
    int ch;
    char cwd[1024];
    char input[1024] = "";
    int input_len = 0;
    int start = 0;

    int win_height = LINES - 4;
    int win_width = COLS - 4;
    if (win_width > COLS - 2)
        win_width = COLS - 2;
    if (win_width < 2)
        win_width = 2;
    WINDOW *win = create_popup_window(win_height, win_width, NULL);
    if (!win) {
        curs_set(1);
        show_message("Unable to create window");
        return 0;
    }
    keypad(win, TRUE);

    getcwd(cwd, sizeof(cwd));

    while (1) {
        werase(win);
        box(win, 0, 0);
        mvwprintw(win, 1, 2, "Current Directory: %s", cwd);

        char **choices = NULL;
        int n_choices = 0;
        get_dir_contents(cwd, &choices, &n_choices);

        int max_display = win_height - 5;
        if (highlight < start)
            start = highlight;
        if (highlight >= start + max_display)
            start = highlight - max_display + 1;

        for (int i = 0; i < max_display && i + start < n_choices; ++i) {
            int idx = i + start;
            if (idx == highlight)
                wattron(win, A_REVERSE);
            mvwprintw(win, i + 2, 2, "%s", choices[idx]);
            wattroff(win, A_REVERSE);
        }

        for (int i = n_choices - start; i < max_display; ++i) {
            mvwprintw(win, i + 2, 2, "%*s", win_width - 4, "");
        }

        mvwprintw(win, win_height - 3, 2,
                  "Arrows: move  Enter: select  ESC: cancel");
        mvwprintw(win, win_height - 2, 2, "Path: %s", input);
        wmove(win, win_height - 2, 8 + input_len);
        wrefresh(win);

        ch = wgetch(win);
        if (ch == KEY_UP) {
            if (highlight > 0)
                --highlight;
        } else if (ch == KEY_DOWN) {
            if (highlight < n_choices - 1)
                ++highlight;
        } else if (ch == '\n') {
            if (input_len > 0) {
                char tmp[2048];
                if (input[0] == '/') {
                    strncpy(tmp, input, sizeof(tmp) - 1);
                    tmp[sizeof(tmp) - 1] = '\0';
                } else {
                    snprintf(tmp, sizeof(tmp), "%s/%s", cwd, input);
                }
                strncpy(path, tmp, max_len);
                path[max_len - 1] = '\0';
                free_dir_contents(choices, n_choices);
                wclear(win);
                wrefresh(win);
                delwin(win);
                wrefresh(stdscr);
                if (cursor_on_enter)
                    curs_set(1);
                return 1;
            }

            if (n_choices > 0) {
                struct stat sb;
                char next_path[2048];
                snprintf(next_path, sizeof(next_path), "%s/%s", cwd,
                         choices[highlight]);
                if (stat(next_path, &sb) == 0 && S_ISDIR(sb.st_mode)) {
                    strncpy(cwd, next_path, sizeof(cwd));
                    cwd[sizeof(cwd) - 1] = '\0';
                    highlight = 0;
                    start = 0;
                    input_len = 0;
                    input[0] = '\0';
                } else {
                    strncpy(path, next_path, max_len);
                    path[max_len - 1] = '\0';
                    free_dir_contents(choices, n_choices);
                    wclear(win);
                    wrefresh(win);
                    delwin(win);
                    wrefresh(stdscr);
                    curs_set(1);
                    return 1;
                }
            }
        } else if (ch == KEY_MOUSE && enable_mouse) {
            MEVENT ev;
            if (getmouse(&ev) == OK &&
                (ev.bstate & (BUTTON1_PRESSED | BUTTON1_CLICKED |
                               BUTTON1_RELEASED))) {
                int wy, wx;
                getbegyx(win, wy, wx);
                int row = ev.y - wy - 2;
                int col = ev.x - wx - 2;
                int max_display = win_height - 5;
                if (row >= 0 && row < max_display &&
                    col >= 0 && col < win_width - 4) {
                    int idx = start + row;
                    if (idx >= 0 && idx < n_choices)
                        highlight = idx;

                    if (ev.bstate & (BUTTON1_RELEASED | BUTTON1_CLICKED)) {
                        if (n_choices > 0) {
                            struct stat sb;
                            char next_path[2048];
                            snprintf(next_path, sizeof(next_path), "%s/%s", cwd,
                                     choices[highlight]);
                            if (stat(next_path, &sb) == 0 && S_ISDIR(sb.st_mode)) {
                                strncpy(cwd, next_path, sizeof(cwd));
                                cwd[sizeof(cwd) - 1] = '\0';
                                highlight = 0;
                                start = 0;
                                input_len = 0;
                                input[0] = '\0';
                            } else {
                                strncpy(path, next_path, max_len);
                                path[max_len - 1] = '\0';
                                free_dir_contents(choices, n_choices);
                                wclear(win);
                                wrefresh(win);
                                delwin(win);
                                wrefresh(stdscr);
                                if (cursor_on_mouse)
                                    curs_set(1);
                                return 1;
                            }
                        }
                    }
                }
            }
        } else if (ch == KEY_BACKSPACE || ch == 127) {
            if (input_len > 0) {
                input_len--;
                input[input_len] = '\0';
            }
        } else if (ch == 27) {
            free_dir_contents(choices, n_choices);
            wclear(win);
            wrefresh(win);
            delwin(win);
            wrefresh(stdscr);
            curs_set(1);
            return 0;
        } else if (isprint(ch)) {
            if (input_len < (int)sizeof(input) - 1) {
                input[input_len++] = ch;
                input[input_len] = '\0';
            }
        }

        free_dir_contents(choices, n_choices);
    }

    wclear(win);
    wrefresh(win);
    delwin(win);
    wrefresh(stdscr);
    curs_set(1);
    return 0;
}

int show_open_file_dialog(char *path, int max_len) {
    return file_dialog_loop(path, max_len, 1, 0);
}

int show_save_file_dialog(char *path, int max_len) {
    return file_dialog_loop(path, max_len, 0, 1);
}


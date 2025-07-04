/*
 * Implements the open and save file dialogs.
 *
 * The dialog displays the current directory contents in a scrollable list
 * and an input field for manually entering a path. Navigation works with
 * the arrow keys or the mouse when enabled. Enter selects the highlighted
 * item or the typed path, while Escape cancels the dialog.
 */
#include "ui_common.h"
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <ncurses.h>
#include <stdio.h>
#include "config.h"
#include "editor.h"
#include "syntax.h"
#include "editor_state.h"

/**
 * Read all entries from `dir_path` into an allocated array.
 *
 * dir_path  - directory to scan
 * choices   - receives a pointer to the allocated array of filenames
 * n_choices - receives the number of returned entries
 *
 * Memory for the array and each string is allocated. Call
 * free_dir_contents() to release it.
 */
static int str_casecmp(const void *a, const void *b) {
    const char *as = *(const char * const *)a;
    const char *bs = *(const char * const *)b;
    return strcasecmp(as, bs);
}

void get_dir_contents(const char *dir_path, char ***choices, int *n_choices) {
    DIR *dir = opendir(dir_path);
    if (!dir) {
        *choices = NULL;
        *n_choices = 0;
        return;
    }

    char **list = NULL;
    int count = 0;
    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char **tmp = realloc(list, sizeof(char *) * (count + 1));
        if (!tmp) {
            for (int i = 0; i < count; ++i)
                free(list[i]);
            free(list);
            closedir(dir);
            *choices = NULL;
            *n_choices = 0;
            return;
        }
        list = tmp;
        list[count] = strdup(entry->d_name);
        if (!list[count]) {
            for (int i = 0; i < count; ++i)
                free(list[i]);
            free(list);
            closedir(dir);
            *choices = NULL;
            *n_choices = 0;
            return;
        }
        count++;
    }
    closedir(dir);

    if (count > 0)
        qsort(list, count, sizeof(char *), str_casecmp);

    *choices = list;
    *n_choices = count;
}

/**
 * Free memory allocated by get_dir_contents().
 *
 * choices   - array returned by get_dir_contents
 * n_choices - number of entries in the array
 */
void free_dir_contents(char **choices, int n_choices) {
    for (int i = 0; i < n_choices; ++i) {
        free(choices[i]);
    }
    free(choices);
}

/**
 * Core loop implementing the file selection dialog.
 *
 * ctx             - editor context
 * path            - buffer receiving the chosen path
 * max_len         - size of the path buffer
 * cursor_on_enter - show cursor when selecting with Enter
 * cursor_on_mouse - show cursor when selecting with the mouse
 *
 * Returns 1 when a file path is chosen, or 0 if the dialog is cancelled.
 * Directory listings are allocated via get_dir_contents() and freed before
 * each iteration ends.
 */
static int file_dialog_loop(EditorContext *ctx, char *path, int max_len,
                            int cursor_on_enter, int cursor_on_mouse) {
    curs_set(0);
    int highlight = 0;
    int ch;
    char cwd[PATH_MAX];
    char input[PATH_MAX] = "";
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

    if (!getcwd(cwd, sizeof(cwd))) {
        wclear(win);
        wrefresh(win);
        delwin(win);
        curs_set(1);
        show_message("Unable to get current directory");
        return 0;
    }

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
            char path_buf[PATH_MAX];
            struct stat sb;
            int needed = snprintf(path_buf, sizeof(path_buf), "%s/%s", cwd,
                                 choices[idx]);
            int is_dir = 0;
            if (needed >= 0 && needed < (int)sizeof(path_buf))
                is_dir = stat(path_buf, &sb) == 0 && S_ISDIR(sb.st_mode);
            char disp[1024];
            if (is_dir)
                snprintf(disp, sizeof(disp), "%s/", choices[idx]);
            else {
                strncpy(disp, choices[idx], sizeof(disp));
                disp[sizeof(disp) - 1] = '\0';
            }
            if (idx == highlight)
                wattron(win, A_REVERSE);
            if (is_dir && ctx->enable_color)
                wattron(win, COLOR_PAIR(SYNTAX_KEYWORD) | A_BOLD);
            mvwprintw(win, i + 2, 2, "%s", disp);
            if (is_dir && ctx->enable_color)
                wattroff(win, COLOR_PAIR(SYNTAX_KEYWORD) | A_BOLD);
            wattroff(win, A_REVERSE);
        }

        for (int i = n_choices - start; i < max_display; ++i) {
            mvwprintw(win, i + 2, 2, "%*s", win_width - 4, "");
        }

        mvwprintw(win, win_height - 3, 2,
                  "Arrows: move  Enter: open/select  ESC: cancel");
        mvwprintw(win, win_height - 2, 2, "Path: %s", input);
        wmove(win, win_height - 2, 8 + input_len);
        wrefresh(win);

        ch = wgetch(win);
        if (ch == KEY_RESIZE) {
            resizeterm(0, 0);
            win_height = LINES - 4;
            win_width = COLS - 4;
            if (win_height > LINES - 2)
                win_height = LINES - 2;
            if (win_height < 2)
                win_height = 2;
            if (win_width > COLS - 2)
                win_width = COLS - 2;
            if (win_width < 2)
                win_width = 2;
            wresize(win, win_height, win_width);
            int win_y = (LINES - win_height) / 2;
            int win_x = (COLS - win_width) / 2;
            if (win_y < 0)
                win_y = 0;
            if (win_y > LINES - win_height)
                win_y = LINES - win_height;
            if (win_x < 0)
                win_x = 0;
            if (win_x > COLS - win_width)
                win_x = COLS - win_width;
            mvwin(win, win_y, win_x);
            werase(win);
            touchwin(win);
            wrefresh(stdscr);
            continue;
        } else if (ch == KEY_UP) {
            if (highlight > 0)
                --highlight;
        } else if (ch == KEY_DOWN) {
            if (highlight < n_choices - 1)
                ++highlight;
        } else if (ch == '\n') {
            if (input_len > 0) {
                char tmp[PATH_MAX];
                if (input[0] == '/') {
                    if (strlen(input) >= sizeof(tmp)) {
                        free_dir_contents(choices, n_choices);
                        wclear(win);
                        wrefresh(win);
                        delwin(win);
                        curs_set(1);
                        show_message("Path too long");
                        return 0;
                    }
                    strncpy(tmp, input, sizeof(tmp) - 1);
                    tmp[sizeof(tmp) - 1] = '\0';
                } else {
                    int needed = snprintf(tmp, sizeof(tmp), "%s/%s", cwd, input);
                    if (needed < 0 || needed >= (int)sizeof(tmp)) {
                        free_dir_contents(choices, n_choices);
                        wclear(win);
                        wrefresh(win);
                        delwin(win);
                        curs_set(1);
                        show_message("Path too long");
                        return 0;
                    }
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
                char next_path[PATH_MAX];
                int needed = snprintf(next_path, sizeof(next_path), "%s/%s", cwd,
                                     choices[highlight]);
                if (needed < 0 || needed >= (int)sizeof(next_path)) {
                    free_dir_contents(choices, n_choices);
                    wclear(win);
                    wrefresh(win);
                    delwin(win);
                    curs_set(1);
                    show_message("Path too long");
                    return 0;
                }
                if (stat(next_path, &sb) == 0 && S_ISDIR(sb.st_mode)) {
                    if (strlen(next_path) >= sizeof(cwd)) {
                        free_dir_contents(choices, n_choices);
                        wclear(win);
                        wrefresh(win);
                        delwin(win);
                        curs_set(1);
                        show_message("Path too long");
                        return 0;
                    }
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
        } else if (ch == KEY_MOUSE && ctx->enable_mouse) {
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
                            char next_path[PATH_MAX];
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

/**
 * Present a dialog for choosing a file to open.
 *
 * ctx     - editor context
 * path    - buffer receiving the selected path
 * max_len - size of the path buffer
 *
 * Returns 1 if a path is chosen, 0 if the dialog is cancelled.
 */
int show_open_file_dialog(EditorContext *ctx, char *path, int max_len) {
    return file_dialog_loop(ctx, path, max_len, 1, 0);
}

/**
 * Present a dialog for choosing where to save a file.
 *
 * ctx     - editor context
 * path    - buffer receiving the selected path
 * max_len - size of the path buffer
 *
 * Returns 1 if a path is chosen, 0 if the dialog is cancelled.
 */
int show_save_file_dialog(EditorContext *ctx, char *path, int max_len) {
    return file_dialog_loop(ctx, path, max_len, 0, 1);
}


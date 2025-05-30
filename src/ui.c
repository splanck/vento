#include <ncurses.h>
#include <ctype.h>
#include <string.h>
#include "config.h"
#include "ui.h"
#include "syntax.h"
#include "ui_common.h"

void create_dialog(const char *message, char *output, int max_input_len) {
    curs_set(0);
    /* Use existing color pairs configured by the application */

    int win_y = LINES / 3;
    int win_x = (COLS - strlen(message) - 30) / 2;
    int win_width = strlen(message) + 30;
    int win_height = 7;

    WINDOW *dialog_win = newwin(win_height, win_width, win_y, win_x);
    if (!dialog_win) {
        if (output)
            output[0] = '\0';
        show_message("Unable to create window");
        return;
    }
    keypad(dialog_win, TRUE);
    wbkgd(dialog_win, COLOR_PAIR(SYNTAX_BG));
    wrefresh(stdscr);

    box(dialog_win, 0, 0);

    wattron(dialog_win, A_BOLD);
    mvwprintw(dialog_win, 1, (win_width - strlen(message)) / 2, "%s", message);
    wattroff(dialog_win, A_BOLD);

    int input_x = 2;
    int input_y = 3;
    wattron(dialog_win, COLOR_PAIR(SYNTAX_KEYWORD));
    mvwprintw(dialog_win, input_y, input_x, "Input: ");
    wattroff(dialog_win, COLOR_PAIR(SYNTAX_KEYWORD));
    wmove(dialog_win, input_y, input_x + 7);

    int ch, input_len = 0;
    int cancelled = 0;
    while ((ch = wgetch(dialog_win)) != '\n' && input_len < max_input_len - 1) {
        if (isprint(ch)) {
            mvwprintw(dialog_win, input_y, input_x + 7 + input_len, "%c", ch);
            output[input_len] = ch;
            input_len++;
            wmove(dialog_win, input_y, input_x + 7 + input_len);
            wrefresh(dialog_win);
        } else if (ch == KEY_BACKSPACE || ch == 127) {
            if (input_len > 0) {
                input_len--;
                mvwprintw(dialog_win, input_y, input_x + 7 + input_len, " ");
                wmove(dialog_win, input_y, input_x + 7 + input_len);
                wrefresh(dialog_win);
            }
        } else if (ch == 27) {
            cancelled = 1;
            break;
        }
    }
    if (cancelled)
        output[0] = '\0';
    else
        output[input_len] = '\0';

    wclear(dialog_win);
    wrefresh(dialog_win);
    delwin(dialog_win);
    wrefresh(stdscr);
    curs_set(1);
}

int show_find_dialog(char *output, int max_input_len, const char *preset) {
    curs_set(0);
    /* Use configured color pairs for dialog display */

    int win_y = LINES / 3;
    int win_x = (COLS - 40) / 2;
    int win_width = 40;
    int win_height = 7;

    WINDOW *dialog_win = newwin(win_height, win_width, win_y, win_x);
    if (!dialog_win) {
        if (output)
            output[0] = '\0';
        show_message("Unable to create window");
        curs_set(1);
        return 0;
    }
    keypad(dialog_win, TRUE);
    wbkgd(dialog_win, COLOR_PAIR(SYNTAX_BG));
    wrefresh(stdscr);

    box(dialog_win, 0, 0);

    char *message = "Find: ";
    wattron(dialog_win, A_BOLD);
    mvwprintw(dialog_win, 1, (win_width - strlen(message)) / 2, "%s", message);
    wattroff(dialog_win, A_BOLD);

    int input_x = 7;
    int input_y = 3;
    wattron(dialog_win, COLOR_PAIR(SYNTAX_KEYWORD));
    mvwprintw(dialog_win, input_y, input_x, "Input: ");
    wattroff(dialog_win, COLOR_PAIR(SYNTAX_KEYWORD));
    int input_len = 0;
    if (preset && *preset) {
        strncpy(output, preset, max_input_len - 1);
        output[max_input_len - 1] = '\0';
        mvwprintw(dialog_win, input_y, input_x + 7, "%s", output);
        input_len = (int)strlen(output);
    } else {
        output[0] = '\0';
    }
    wmove(dialog_win, input_y, input_x + 7 + input_len);
    int ch;
    int cancelled = 0;
    while ((ch = wgetch(dialog_win)) != '\n' && input_len < max_input_len - 1) {
        if (dialog_win != NULL) {
            if (isprint(ch)) {
                if (input_len >= max_input_len - 3)
                    continue;

                mvwprintw(dialog_win, input_y, input_x + 7 + input_len, "%c", ch);
                output[input_len] = ch;
                input_len++;
                wmove(dialog_win, input_y, input_x + 7 + input_len);
                wrefresh(dialog_win);
            } else if (ch == KEY_BACKSPACE || ch == 127) {
                if (input_len > 0) {
                    input_len--;
                    mvwprintw(dialog_win, input_y, input_x + 7 + input_len, " ");
                    wmove(dialog_win, input_y, input_x + 7 + input_len);
                    wrefresh(dialog_win);
                }
            } else if (ch == 27) {
                cancelled = 1;
                break;
            }
        } else {
            break;
        }
    }
    if (cancelled)
        output[0] = '\0';
    else
        output[input_len] = '\0';

    wclear(dialog_win);
    wrefresh(dialog_win);
    delwin(dialog_win);
    wrefresh(stdscr);
    curs_set(1);

    return cancelled ? 0 : 1;
}

int show_replace_dialog(char *search, int max_search_len,
                        char *replace, int max_replace_len) {
    curs_set(0);
    if (!show_find_dialog(search, max_search_len, NULL) || search[0] == '\0') {
        curs_set(1);
        return 0;
    }

    create_dialog("Replace:", replace, max_replace_len);
    curs_set(1);
    return 1;
}

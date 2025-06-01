#include <ncurses.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "config.h"
#include "ui.h"
#include "syntax.h"
#include "ui_common.h"
#include "dialog.h"

void create_dialog(const char *message, char *output, int max_input_len) {
    int win_width = (int)strlen(message) + 30;
    WINDOW *dialog_win = dialog_open(7, win_width, message);
    if (!dialog_win) {
        if (output)
            output[0] = '\0';
        return;
    }

    int input_x = 2;
    int input_y = 3;
    if (enable_color)
        wattron(dialog_win, COLOR_PAIR(SYNTAX_KEYWORD));
    mvwprintw(dialog_win, input_y, input_x, "Input: ");
    if (enable_color)
        wattroff(dialog_win, COLOR_PAIR(SYNTAX_KEYWORD));
    wrefresh(dialog_win);

    int ok = dialog_prompt(dialog_win, input_y, input_x + 7, output, max_input_len);
    if (!ok)
        output[0] = '\0';

    dialog_close(dialog_win);
}

int show_find_dialog(char *output, int max_input_len, const char *preset) {
    int win_width = 40;
    WINDOW *dialog_win = dialog_open(7, win_width, "Find:");
    if (!dialog_win) {
        if (output)
            output[0] = '\0';
        return 0;
    }

    int input_x = 7;
    int input_y = 3;
    if (enable_color)
        wattron(dialog_win, COLOR_PAIR(SYNTAX_KEYWORD));
    mvwprintw(dialog_win, input_y, input_x, "Input: ");
    if (enable_color)
        wattroff(dialog_win, COLOR_PAIR(SYNTAX_KEYWORD));
    if (preset && *preset) {
        strncpy(output, preset, max_input_len - 1);
        output[max_input_len - 1] = '\0';
    } else {
        output[0] = '\0';
    }

    int ok = dialog_prompt(dialog_win, input_y, input_x + 7, output, max_input_len);
    if (!ok)
        output[0] = '\0';

    dialog_close(dialog_win);
    return ok;
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

int show_goto_dialog(int *line_number) {
    char buf[32];

    create_dialog("Go To Line:", buf, sizeof(buf));

    if (buf[0] == '\0') {
        return 0;
    }

    *line_number = atoi(buf);
    return 1;
}

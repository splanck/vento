#include "editor.h"
#include "config.h"
#include "ui_common.h"
#include <ncurses.h>
#include <string.h>

void show_help() {
    curs_set(0);
    int win_height = 18;
    int win_width = COLS - 40;
    int win_y = (LINES - win_height) / 2;
    int win_x = (COLS - win_width) / 2;

    WINDOW *help_win = newwin(win_height, win_width, win_y, win_x);
    keypad(help_win, TRUE);
    wbkgd(help_win, COLOR_PAIR(1));
    wrefresh(stdscr);
    box(help_win, 0, 0);

    mvwprintw(help_win, 1, 2, "Help:");
    mvwprintw(help_win, 3, 2, "CTRL-H: Show this help");
    mvwprintw(help_win, 4, 2, "CTRL-A: About");
    mvwprintw(help_win, 5, 2, "CTRL-L: Load a new file (browse files)");
    mvwprintw(help_win, 6, 2, "CTRL-O: Save As (browse dir or name)");
    mvwprintw(help_win, 7, 2, "CTRL-S: Save");
    mvwprintw(help_win, 8, 2, "CTRL-J: Start/Stop selection mode");
    mvwprintw(help_win, 9, 2, "CTRL-V: Paste from clipboard");
    mvwprintw(help_win, 10, 2, "CTRL-N: New file");
    mvwprintw(help_win, 11, 2, "CTRL-Y: Redo");
    mvwprintw(help_win, 12, 2, "CTRL-Z: Undo");
    mvwprintw(help_win, 13, 2, "CTRL-F: Search for text string");
    mvwprintw(help_win, 14, 2, "F3: Find next occurrence");
    mvwprintw(help_win, 15, 2, "CTRL-R: Replace");
    mvwprintw(help_win, 16, 2, "CTRL-G: Go to line");

    mvwprintw(help_win, 3, win_width / 2, "CTRL-C: Copy selection");
    mvwprintw(help_win, 4, win_width / 2, "CTRL-X: Cut selection");
    mvwprintw(help_win, 5, win_width / 2, "CTRL-W: Move forward to next word");
    mvwprintw(help_win, 6, win_width / 2, "CTRL-B: Move backward to previous word");
    mvwprintw(help_win, 7, win_width / 2, "CTRL-D: Delete current line");
    mvwprintw(help_win, 8, win_width / 2, "Arrow Keys: Navigate text");
    mvwprintw(help_win, 9, win_width / 2, "Page Up/Down: Scroll document");
    mvwprintw(help_win, 10, win_width / 2, "Home/CTRL-Left: Move to line start");
    mvwprintw(help_win, 11, win_width / 2, "End/CTRL-Right: Move to line end");
    mvwprintw(help_win, 12, win_width / 2, "CTRL-PgUp: Move to top of doc");
    mvwprintw(help_win, 13, win_width / 2, "CTRL-PgDn: Move to end of doc");
    mvwprintw(help_win, 15, win_width / 2, "F5: Insert blank line");
    mvwprintw(help_win, 16, win_width / 2, "F6: Next file");
    mvwprintw(help_win, 17, win_width / 2, "F7: Previous file");

    mvwprintw(help_win, win_height - 1,
              (win_width - strlen("(Press any key to close)")) / 2,
              "(Press any key to close)");
    wrefresh(help_win);
    wgetch(help_win);

    wclear(help_win);
    wrefresh(help_win);
    delwin(help_win);
    wrefresh(stdscr);
    curs_set(1);
}

void show_about() {
    curs_set(0);
    int win_height = 10;
    int win_width = COLS - 20;
    int win_y = (LINES - win_height) / 2;
    int win_x = (COLS - win_width) / 2;

    WINDOW *about_win = newwin(win_height, win_width, win_y, win_x);
    keypad(about_win, TRUE);
    wbkgd(about_win, COLOR_PAIR(1));
    wrefresh(stdscr);
    box(about_win, 0, 0);

    mvwprintw(about_win, 1, 2, "Vento Text Editor");
    mvwprintw(about_win, 2, 2, "Version: %s", VERSION);
    mvwprintw(about_win, 3, 2, "License: GPL v3");
    mvwprintw(about_win, 4, 2, "Vento is open-source software licensed under the GPL v3.");
    mvwprintw(about_win, win_height - 2, 2, "(Press any key to close)");
    wrefresh(about_win);
    wgetch(about_win);

    wclear(about_win);
    wrefresh(about_win);
    delwin(about_win);
    wrefresh(stdscr);
    curs_set(1);
}

void show_warning_dialog() {
    curs_set(0);
    int win_height = 7;
    int win_width = COLS - 20;
    int win_y = (LINES - win_height) / 2;
    int win_x = (COLS - win_width) / 2;

    WINDOW *warning_win = newwin(win_height, win_width, win_y, win_x);
    keypad(warning_win, TRUE);
    wbkgd(warning_win, COLOR_PAIR(1));
    wrefresh(stdscr);
    box(warning_win, 0, 0);

    char *message1 = "Warning: This is experimental software.";
    char *message2 = "It is under development and not intended for production use.";
    char *message3 = "(Press any key to dismiss)";

    mvwprintw(warning_win, 2, (win_width - strlen(message1)) / 2, "%s", message1);
    mvwprintw(warning_win, 3, (win_width - strlen(message2)) / 2, "%s", message2);
    mvwprintw(warning_win, 5, (win_width - strlen(message3)) / 2, "%s", message3);

    wrefresh(warning_win);
    wgetch(warning_win);

    werase(warning_win);
    wrefresh(warning_win);
    delwin(warning_win);
    wrefresh(stdscr);

    werase(text_win);
    box(text_win, 0, 0);
    draw_text_buffer(active_file, text_win);
    wrefresh(text_win);
    update_status_bar(active_file);
    wrefresh(stdscr);
    curs_set(1);
}

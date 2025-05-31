#include "editor.h"
#include "config.h"
#include "ui_common.h"
#include "syntax.h"
#include <ncurses.h>
#include <string.h>

void show_help() {
    curs_set(0);
    wbkgd(stdscr, enable_color ? COLOR_PAIR(SYNTAX_BG) : A_NORMAL);

    const char *help_lines[] = {
        "CTRL-H: Show this help",
        "CTRL-A: About",
        "CTRL-L: Load a new file (browse files)",
        "CTRL-O: Save As (browse dir or name)",
        "CTRL-S: Save",
        "CTRL-J: Start/Stop selection mode",
        "CTRL-V: Paste from clipboard",
        "CTRL-N: New file",
        "CTRL-Y: Redo",
        "CTRL-Z: Undo",
        "CTRL-F: Search for text string",
        "F3: Find next occurrence",
        "CTRL-R: Replace",
        "CTRL-G: Go to line",
        "CTRL-C: Copy selection",
        "CTRL-X: Cut selection",
        "CTRL-W: Move forward to next word",
        "CTRL-B: Move backward to previous word",
        "CTRL-D: Delete current line",
        "Arrow Keys: Navigate text",
        "Page Up/Down: Scroll document",
        "Home/CTRL-Left: Move to line start",
        "End/CTRL-Right: Move to line end",
        "CTRL-PgUp: Move to top of doc",
        "CTRL-PgDn: Move to end of doc",
        "F5: Insert blank line",
        "F6: Next file",
        "F7: Previous file"
    };

    int help_count = sizeof(help_lines) / sizeof(help_lines[0]);

    int max_len = 0;
    for (int i = 0; i < help_count; ++i) {
        int len = (int)strlen(help_lines[i]);
        if (len > max_len)
            max_len = len;
    }

    int win_width = max_len + 4; // add padding for borders
    if (win_width > COLS - 2)
        win_width = COLS - 2;

    show_scrollable_window(help_lines, help_count, NULL, win_width);
    wrefresh(stdscr);
    curs_set(1);
}

void show_about() {
    curs_set(0);
    int win_height = 10;
    int win_width = COLS - 20;
    if (win_width > COLS - 2 || win_width < 2)
        win_width = COLS - 2;
    int win_y = (LINES - win_height) / 2;
    int win_x = (COLS - win_width) / 2;
    if (win_x < 0)
        win_x = 0;

    WINDOW *about_win = newwin(win_height, win_width, win_y, win_x);
    if (!about_win) {
        show_message("Unable to create window");
        return;
    }
    keypad(about_win, TRUE);
    wbkgd(about_win, enable_color ? COLOR_PAIR(SYNTAX_BG) : A_NORMAL);
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
    if (win_width > COLS - 2 || win_width < 2)
        win_width = COLS - 2;
    int win_y = (LINES - win_height) / 2;
    int win_x = (COLS - win_width) / 2;
    if (win_x < 0)
        win_x = 0;

    WINDOW *warning_win = newwin(win_height, win_width, win_y, win_x);
    if (!warning_win) {
        show_message("Unable to create window");
        return;
    }
    keypad(warning_win, TRUE);
    wbkgd(warning_win, enable_color ? COLOR_PAIR(SYNTAX_BG) : A_NORMAL);
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

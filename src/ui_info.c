#include "editor.h"
#include "config.h"
#include "ui_common.h"
#include "syntax.h"
#include "dialog.h"
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

    if (show_scrollable_window(help_lines, help_count, NULL, win_width) == ERR)
        return;
    wrefresh(stdscr);
    curs_set(1);
}

void show_about() {
    int win_height = 10;
    int win_width = COLS - 20;
    WINDOW *about_win = dialog_open(win_height, win_width, "About");
    if (!about_win)
        return;

    mvwprintw(about_win, 1, 2, "Vento Text Editor");
    mvwprintw(about_win, 2, 2, "Version: %s", VERSION);
    mvwprintw(about_win, 3, 2, "License: GPL v3");
    mvwprintw(about_win, 4, 2, "Vento is open-source software licensed under the GPL v3.");
    mvwprintw(about_win, win_height - 2, 2, "(Press any key to close)");
    wrefresh(about_win);
    wgetch(about_win);

    dialog_close(about_win);
}

void show_warning_dialog() {
    int win_height = 7;
    int win_width = COLS - 20;
    WINDOW *warning_win = dialog_open(win_height, win_width, "Warning");
    if (!warning_win)
        return;

    char *message1 = "Warning: This is experimental software.";
    char *message2 = "It is under development and not intended for production use.";
    char *message3 = "(Press any key to dismiss)";

    mvwprintw(warning_win, 2, (win_width - strlen(message1)) / 2, "%s", message1);
    mvwprintw(warning_win, 3, (win_width - strlen(message2)) / 2, "%s", message2);
    mvwprintw(warning_win, 5, (win_width - strlen(message3)) / 2, "%s", message3);

    wrefresh(warning_win);
    wgetch(warning_win);

    dialog_close(warning_win);

    werase(text_win);
    box(text_win, 0, 0);
    draw_text_buffer(active_file, text_win);
    wrefresh(text_win);
    update_status_bar(active_file);
    wrefresh(stdscr);
}

#include "editor.h"
#include "config.h"
#include "editor_state.h"
#include "ui_common.h"
#include "syntax.h"
#include "dialog.h"
#include "menu.h"
#include <ncurses.h>
#include <string.h>

void show_help(EditorContext *ctx) {
    curs_set(0);
    wbkgd(stdscr, ctx->enable_color ? COLOR_PAIR(SYNTAX_BG) : A_NORMAL);

    const char *help_lines[] = {
        "CTRL-H or F1: Show this help",
        "CTRL-A: About",
        "CTRL-N: New file",
        "CTRL-L: Load a new file (browse files)",
        "CTRL-S: Save",
        "CTRL-O: Save As (browse dir or name)",
        "CTRL-J: Start/Stop selection mode",
        "CTRL-C: Copy selection",
        "CTRL-X: Cut selection",
        "CTRL-V: Paste from clipboard",
        "CTRL-D: Delete current line",
        "F5: Insert blank line",
        "CTRL-Z: Undo",
        "CTRL-Y: Redo",
        "CTRL-F: Search for text string",
        "F3: Find next occurrence",
        "CTRL-R: Replace",
        "CTRL-G: Go to line",
        "CTRL-W: Move forward to next word",
        "CTRL-B: Move backward to previous word",
        "Arrow Keys: Navigate text",
        "Page Up/Down: Scroll document",
        "Home/CTRL-Left: Move to line start",
        "End/CTRL-Right: Move to line end",
        "CTRL-PgUp: Move to top of doc",
        "CTRL-PgDn: Move to end of doc",
        "F6: Next file",
        "F7: Previous file",
        "CTRL-T: Open menus",
        "Left/Right: Switch menus",
        "Up/Down: Choose item",
        "Enter/Esc: Select/close menu",
        "Settings -> Theme: Choose color theme"
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

void show_about(EditorContext *ctx) {
    char version_line[64];
    snprintf(version_line, sizeof(version_line), "Version: %s", VERSION);

    const char *lines[] = {
        "Vento Text Editor",
        version_line,
        "License: GPL v3",
        "Vento is open-source software licensed under the GPL v3."
    };
    const int count = (int)(sizeof(lines) / sizeof(lines[0]));
    const char *footer = "(Press any key to close)";

    int max_len = 0;
    for (int i = 0; i < count; ++i) {
        int len = (int)strlen(lines[i]);
        if (len > max_len)
            max_len = len;
    }
    if ((int)strlen(footer) > max_len)
        max_len = (int)strlen(footer);

    int win_width = max_len + 4;
    if (win_width > COLS - 2)
        win_width = COLS - 2;
    int win_height = count + 4;
    if (win_height > LINES - 2)
        win_height = LINES - 2;

    WINDOW *about_win = dialog_open(win_height, win_width, "About");
    if (!about_win)
        return;

    if (ctx->enable_color)
        wattron(about_win, COLOR_PAIR(SYNTAX_KEYWORD) | A_BOLD);
    mvwprintw(about_win, 1, (win_width - (int)strlen(lines[0])) / 2, "%s", lines[0]);
    if (ctx->enable_color)
        wattroff(about_win, COLOR_PAIR(SYNTAX_KEYWORD) | A_BOLD);

    for (int i = 1; i < count; ++i)
        mvwprintw(about_win, i + 1, (win_width - (int)strlen(lines[i])) / 2, "%s", lines[i]);

    mvwprintw(about_win, win_height - 2,
              (win_width - (int)strlen(footer)) / 2, "%s", footer);

    wrefresh(about_win);
    wgetch(about_win);

    dialog_close(about_win);
}

void show_warning_dialog(EditorContext *ctx) {
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

    werase(ctx->text_win);
    box(ctx->text_win, 0, 0);
    draw_text_buffer(ctx->active_file, ctx->text_win);
    wrefresh(ctx->text_win);
    update_status_bar(ctx, ctx->active_file);
    drawBar();
    wnoutrefresh(stdscr);
    doupdate();
}

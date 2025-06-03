#include "ui.h"
#include "ui_common.h"
#include "config.h"
#include "macro.h"
#include "editor_state.h"
#include <ncurses.h>
#include <string.h>
#include <stdlib.h>

int select_int(EditorContext *ctx, const char *prompt, int current, WINDOW *parent);

/*
 * Manage macros dialog
 * --------------------
 * Presents a scrollable list of defined macros and allows the user to
 * select the active macro, rename or delete existing entries and create
 * new ones.
 */

void show_manage_macros(EditorContext *ctx) {
    curs_set(0);
    int highlight = 0;
    int start = 0;
    int ch;

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
        return;
    }
    keypad(win, TRUE);

    while (1) {
        int count = macro_count();
        if (count == 0) {
            highlight = -1;
        } else {
            if (highlight < 0)
                highlight = 0;
            if (highlight >= count)
                highlight = count - 1;
        }

        werase(win);
        box(win, 0, 0);
        mvwprintw(win, 1, 2, "Macros:");

        int max_display = win_height - 4;
        if (highlight < start)
            start = highlight;
        if (highlight >= start + max_display)
            start = highlight - max_display + 1;

        for (int i = 0; i < max_display && i + start < count; ++i) {
            int idx = i + start;
            Macro *m = macro_at(idx);
            if (!m)
                continue;
            if (idx == highlight)
                wattron(win, A_REVERSE);
            const char *kname = m->play_key ? keyname(m->play_key) : "-";
            mvwprintw(win, i + 2, 2, "%s (%s)%s", m->name, kname,
                      m->active ? " *" : "");
            if (idx == highlight)
                wattroff(win, A_REVERSE);
        }

        for (int i = count - start; i < max_display; ++i)
            mvwprintw(win, i + 2, 2, "%*s", win_width - 4, "");

        mvwprintw(win, win_height - 2, 2,
                  "Arrows:move  Enter:select  N:new  R:rename  K:key  P:play  D:delete  ESC:close");
        wrefresh(win);

        ch = wgetch(win);
        if (ch == KEY_RESIZE) {
            resizeterm(0, 0);
            win_height = LINES - 4;
            win_width = COLS - 4;
            if (win_width > COLS - 2)
                win_width = COLS - 2;
            if (win_width < 2)
                win_width = 2;
            wresize(win, win_height, win_width);
            mvwin(win, (LINES - win_height) / 2,
                  (COLS - win_width) / 2 < 0 ? 0 : (COLS - win_width) / 2);
            continue;
        } else if (ch == KEY_UP) {
            if (highlight > 0)
                --highlight;
        } else if (ch == KEY_DOWN) {
            if (highlight < count - 1)
                ++highlight;
        } else if (ch == '\n') {
            if (highlight >= 0 && highlight < count)
                macro_set_current(macro_at(highlight));
            macros_save(&app_config);
            break;
        } else if (ch == 'n' || ch == 'N') {
            char name[64] = "";
            create_dialog(ctx, "New Macro:", name, sizeof(name));
            if (name[0]) {
                Macro *m = macro_create(name);
                if (m)
                    highlight = macro_count() - 1;
                macros_save(&app_config);
            }
        } else if (ch == 'd' || ch == 'D') {
            if (highlight >= 0 && highlight < count) {
                Macro *m = macro_at(highlight);
                if (m) {
                    char *tmp = strdup(m->name);
                    if (tmp) {
                        macro_delete(tmp);
                        free(tmp);
                        if (highlight >= macro_count())
                            highlight = macro_count() - 1;
                        macros_save(&app_config);
                    }
                }
            }
        } else if (ch == 'k' || ch == 'K') {
            if (highlight >= 0 && highlight < count) {
                Macro *m = macro_at(highlight);
                if (m) {
                    int val = select_int(ctx, "Playback Key", m->play_key, win);
                    m->play_key = val;
                    macros_save(&app_config);
                }
            }
        } else if (ch == 'p' || ch == 'P') {
            if (highlight >= 0 && highlight < count) {
                Macro *m = macro_at(highlight);
                if (m) {
                    int repeat = select_int(ctx, "Repeat Count", 1, win);
                    macro_play_times(m, ctx, ctx->active_file, repeat);
                    touchwin(win);
                    wrefresh(win);
                }
            }
        } else if (ch == 'r' || ch == 'R') {
            if (highlight >= 0 && highlight < count) {
                Macro *m = macro_at(highlight);
                if (m) {
                    char buf[64];
                    strncpy(buf, m->name, sizeof(buf) - 1);
                    buf[sizeof(buf) - 1] = '\0';
                    create_dialog(ctx, "Rename Macro:", buf, sizeof(buf));
                    if (buf[0]) {
                        macro_rename(m, buf);
                        macros_save(&app_config);
                    }
                }
            }
        } else if (ch == 27) {
            break;
        }
    }

    werase(win);
    wrefresh(win);
    delwin(win);
    wrefresh(stdscr);
    curs_set(1);
}


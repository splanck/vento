#include "ui_common.h"
#include "ui.h"
#include "config.h"
#include "editor.h"
#include "editor_state.h"
#include <ncurses.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <stddef.h>
#include <dirent.h>

/*
 * Settings dialog
 * ---------------
 * This file implements the configuration dialog presented to the user.
 * All options are rendered in a scrolling list. Keyboard arrows or the
 * mouse change the highlighted field and pressing Enter invokes editing.
 * Changes are written to the provided AppConfig structure and helper
 * callbacks update runtime state (such as enabling mouse support).
 */

const char *select_color(const char *current, WINDOW *parent);
const char *select_theme(const char *current, WINDOW *parent);
int select_bool(const char *prompt, int current, WINDOW *parent);
int select_int(EditorContext *ctx, const char *prompt, int current,
               WINDOW *parent);

/* Compare two strings case-insensitively for qsort. */
static int str_casecmp(const void *a, const void *b) {
    const char *as = *(const char * const *)a;
    const char *bs = *(const char * const *)b;
    return strcasecmp(as, bs);
}

/* Enable or disable mouse reporting based on the config option. */
static void apply_mouse(AppConfig *cfg) {
    enable_mouse = cfg->enable_mouse;
    if (enable_mouse)
        mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
    else
        mousemask(0, NULL);
}

enum OptionType { OPT_BOOL, OPT_COLOR, OPT_THEME, OPT_INT };

enum { COLOR_LEN = sizeof(((AppConfig *)0)->background_color),
       THEME_LEN = sizeof(((AppConfig *)0)->theme) };

typedef struct {
    const char *label;
    enum OptionType type;
    size_t offset;
    void (*after_change)(AppConfig *cfg);
} Option;

static const Option options[] = {
    {"Enable color", OPT_BOOL, offsetof(AppConfig, enable_color), NULL},
    {"Enable mouse", OPT_BOOL, offsetof(AppConfig, enable_mouse), apply_mouse},
    {"Show line numbers", OPT_BOOL, offsetof(AppConfig, show_line_numbers), NULL},
    {"Show startup warning", OPT_BOOL, offsetof(AppConfig, show_startup_warning), NULL},
    {"Ignore case in search", OPT_BOOL, offsetof(AppConfig, search_ignore_case), NULL},
    {"Tab width", OPT_INT, offsetof(AppConfig, tab_width), NULL},
    {"Theme", OPT_THEME, offsetof(AppConfig, theme), NULL},
    {"Background color", OPT_COLOR, offsetof(AppConfig, background_color), NULL},
    {"Text color", OPT_COLOR, offsetof(AppConfig, text_color), NULL},
    {"Keyword color", OPT_COLOR, offsetof(AppConfig, keyword_color), NULL},
    {"Comment color", OPT_COLOR, offsetof(AppConfig, comment_color), NULL},
    {"String color", OPT_COLOR, offsetof(AppConfig, string_color), NULL},
    {"Type color", OPT_COLOR, offsetof(AppConfig, type_color), NULL},
    {"Symbol color", OPT_COLOR, offsetof(AppConfig, symbol_color), NULL},
    {"Search color", OPT_COLOR, offsetof(AppConfig, search_color), NULL},
    {"Macro record key", OPT_INT, offsetof(AppConfig, macro_record_key), NULL},
    {"Macro play key", OPT_INT, offsetof(AppConfig, macro_play_key), NULL},
};

#define FIELD_COUNT ((int)(sizeof(options) / sizeof(options[0])))

/* Calculate a width wide enough to display all option labels and values. */
static int compute_dialog_width(const AppConfig *cfg) {
    int longest = 0;
    for (int i = 0; i < FIELD_COUNT; ++i) {
        const Option *opt = &options[i];
        const char *val;
        if (opt->type == OPT_BOOL) {
            int v = *(int *)((char *)cfg + opt->offset);
            val = v ? "Enabled" : "Disabled";
        } else {
            val = (const char *)cfg + opt->offset;
        }
        int len = strlen(opt->label) + 2 + (int)strlen(val);
        if (len > longest)
            longest = len;
    }

    int width = longest + 4;
    if (width < 50)
        width = 50;
    if (width > COLS - 2)
        width = COLS - 2;
    if (width < 2)
        width = 2;
    return width;
}

/* Resize and reposition the settings window when option widths change. */
static void update_settings_window(WINDOW *win, int *win_height, int *win_width,
                                   const AppConfig *cfg) {
    int desired_width = compute_dialog_width(cfg);
    if (desired_width != *win_width) {
        *win_width = desired_width;
        wresize(win, *win_height, *win_width);
        int win_y = (LINES - *win_height) / 2;
        int win_x = (COLS - *win_width) / 2;
        if (win_x < 0)
            win_x = 0;
        mvwin(win, win_y, win_x);
    }
}

/* Draw a small preview of theme colors on the last row of a window. */
static void render_theme_sample(const AppConfig *cfg, WINDOW *win, int row) {
    if (!cfg || !win)
        return;

    short bg = get_color_code(cfg->background_color);
    short fg = get_color_code(cfg->text_color);
    short kw = get_color_code(cfg->keyword_color);
    short cm = get_color_code(cfg->comment_color);
    short st = get_color_code(cfg->string_color);
    short ty = get_color_code(cfg->type_color);
    short sy = get_color_code(cfg->symbol_color);
    short se = get_color_code(cfg->search_color);
    short base = COLOR_PAIRS - 7;
    if (base < 1)
        base = 1;
    if (base + 6 >= COLOR_PAIRS)
        return;

    init_pair(base, fg == -1 ? COLOR_WHITE : fg, bg);
    init_pair(base + 1, kw, bg);
    init_pair(base + 2, cm, bg);
    init_pair(base + 3, st, bg);
    init_pair(base + 4, ty, bg);
    init_pair(base + 5, sy, bg);
    init_pair(base + 6, se, bg);

    int width = getmaxx(win) - 2;
    wattron(win, COLOR_PAIR(base));
    mvwhline(win, row, 1, ' ', width);
    int col = 2;

    wattron(win, COLOR_PAIR(base + 1) | A_BOLD);
    mvwprintw(win, row, col, "keyword ");
    col = getcurx(win);

    wattron(win, COLOR_PAIR(base + 2));
    mvwprintw(win, row, col, "comment ");
    col = getcurx(win);

    wattron(win, COLOR_PAIR(base + 3));
    mvwprintw(win, row, col, "string ");
    col = getcurx(win);

    wattron(win, COLOR_PAIR(base + 4));
    mvwprintw(win, row, col, "type ");
    col = getcurx(win);

    wattron(win, COLOR_PAIR(base + 5));
    mvwprintw(win, row, col, "symbol ");
    col = getcurx(win);

    wattron(win, COLOR_PAIR(base + 6));
    mvwprintw(win, row, col, "search");
}

/* Undo any color pairs created by render_theme_sample. */
static void clear_theme_sample_pairs(void) {
    short base = COLOR_PAIRS - 7;
    if (base < 1)
        base = 1;
    if (base + 6 >= COLOR_PAIRS)
        return;
    for (short i = 0; i < 7; ++i)
        init_pair(base + i, -1, -1);
}

/* Invoke the appropriate editor for a single settings entry. */
static void edit_option(EditorContext *ctx, AppConfig *cfg, WINDOW *win,
                        const Option *opt) {
    if (opt->type == OPT_BOOL) {
        int *val = (int *)((char *)cfg + opt->offset);
        *val = select_bool(opt->label, *val, win);
    } else if (opt->type == OPT_COLOR) {
        char *str = (char *)cfg + opt->offset;
        const char *sel = select_color(str, win);
        if (sel) {
            strncpy(str, sel, COLOR_LEN - 1);
            str[COLOR_LEN - 1] = '\0';
        }
    } else if (opt->type == OPT_THEME) {
        char *str = (char *)cfg + opt->offset;
        const char *sel = select_theme(str, win);
        if (sel) {
            strncpy(str, sel, THEME_LEN - 1);
            str[THEME_LEN - 1] = '\0';
            free((char *)sel);
        }
    } else if (opt->type == OPT_INT) {
        int *val = (int *)((char *)cfg + opt->offset);
        *val = select_int(ctx, opt->label, *val, win);
    }
    if (opt->after_change)
        opt->after_change(cfg);
}

/**
 * Display the interactive settings dialog.
 *
 * ctx - editor context used for nested dialogs.
 * cfg - configuration structure to modify.
 *
 * Returns 1 if any option was changed and kept, 0 otherwise.
 */
int show_settings_dialog(EditorContext *ctx, AppConfig *cfg) {
    curs_set(0);
    AppConfig original = *cfg;
    int cancelled = 0;

    mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);

    int highlight = 0;
    int start = 0;
    int ch;
    int done = 0;

    int win_height = FIELD_COUNT + 4;
    int win_width = compute_dialog_width(cfg);

    WINDOW *win = create_popup_window(win_height, win_width, NULL);
    if (!win) {
        curs_set(1);
        show_message("Unable to create window");
        return 0;
    }
    getmaxyx(win, win_height, win_width);
    keypad(win, TRUE);

    while (!done) {
        werase(win);
        box(win, 0, 0);

        int max_display = win_height - 3;
        if (max_display > FIELD_COUNT)
            max_display = FIELD_COUNT;

        if (highlight < start)
            start = highlight;
        if (highlight >= start + max_display)
            start = highlight - max_display + 1;

        for (int i = 0; i < max_display && i + start < FIELD_COUNT; ++i) {
            int idx = i + start;
            const Option *opt = &options[idx];
            if (idx == highlight)
                wattron(win, A_REVERSE);

            const char *val;
            if (opt->type == OPT_BOOL) {
                int v = *(int *)((char *)cfg + opt->offset);
                val = v ? "Enabled" : "Disabled";
            } else {
                val = (char *)cfg + opt->offset;
            }
            mvwprintw(win, i + 1, 2, "%s: %s", opt->label, val);

            if (idx == highlight)
                wattroff(win, A_REVERSE);
        }

        for (int i = FIELD_COUNT - start; i < max_display; ++i)
            mvwprintw(win, i + 1, 2, "%*s", win_width - 4, "");

        mvwprintw(win, win_height - 2, 2,
                  "Arrows: move  Enter: change  ESC: cancel");
        wrefresh(win);

        ch = wgetch(win);
        if (ch == KEY_UP) {
            if (highlight > 0)
                --highlight;
        } else if (ch == KEY_DOWN) {
            if (highlight < FIELD_COUNT - 1)
                ++highlight;
        } else if (ch == '\n') {
            edit_option(ctx, cfg, win, &options[highlight]);
            update_settings_window(win, &win_height, &win_width, cfg);
        } else if (ch == KEY_MOUSE) {
            MEVENT ev;
            if (getmouse(&ev) == OK &&
                (ev.bstate & (BUTTON1_PRESSED | BUTTON1_CLICKED |
                               BUTTON1_RELEASED))) {
                int wy, wx;
                getbegyx(win, wy, wx);
                int row = ev.y - wy - 1;
                int col = ev.x - wx - 2;
                int max_display = win_height - 3;
                if (row >= 0 && row < max_display &&
                    col >= 0 && col < win_width - 4) {
                    int idx = start + row;
                    if (idx >= 0 && idx < FIELD_COUNT)
                        highlight = idx;
                    if (ev.bstate & (BUTTON1_RELEASED | BUTTON1_CLICKED)) {
                        edit_option(ctx, cfg, win, &options[highlight]);
                        update_settings_window(win, &win_height, &win_width, cfg);
                    }
                }
            }
        } else if (ch == 27) {
            cancelled = 1;
            done = 1;
        }
    }

    if (cancelled)
        *cfg = original;

    apply_mouse(cfg);

    wclear(win);
    wrefresh(win);
    delwin(win);
    wrefresh(stdscr);
    curs_set(1);

    return !cancelled && memcmp(&original, cfg, sizeof(AppConfig)) != 0;
}

/**
 * Present a list of color names for selection.
 *
 * current - currently selected color name or NULL.
 * parent  - optional parent window for the popup.
 *
 * Returns the chosen static string, or NULL if cancelled.
 */
const char *select_color(const char *current, WINDOW *parent) {
    curs_set(0);
    static const char *colors[] = {
        "BLACK", "RED", "GREEN", "YELLOW",
        "BLUE", "MAGENTA", "CYAN", "WHITE"
    };
    const int count = sizeof(colors) / sizeof(colors[0]);

    int highlight = 0;
    if (current) {
        for (int i = 0; i < count; ++i) {
            if (strcasecmp(current, colors[i]) == 0) {
                highlight = i;
                break;
            }
        }
    }

    int start = 0;
    int own = 0;
    int win_height, win_width;
    WINDOW *win;
    if (parent) {
        int ph, pw;
        getmaxyx(parent, ph, pw);
        win_height = ph - 4;
        win_width = pw - 4;
        if (win_width > COLS - 2)
            win_width = COLS - 2;
        if (win_width < 2)
            win_width = 2;
        win = create_popup_window(win_height, win_width, parent);
        if (!win) {
            curs_set(1);
            show_message("Unable to create window");
            return NULL;
        }
        own = 1;
    } else {
        win_height = LINES - 4;
        win_width = COLS - 4;
        if (win_width > COLS - 2)
            win_width = COLS - 2;
        if (win_width < 2)
            win_width = 2;
        win = create_popup_window(win_height, win_width, NULL);
        if (!win) {
            curs_set(1);
            show_message("Unable to create window");
            return NULL;
        }
        own = 1;
    }
    keypad(win, TRUE);

    while (1) {
        werase(win);
        box(win, 0, 0);

        int max_display = win_height - 2;
        if (highlight < start)
            start = highlight;
        if (highlight >= start + max_display)
            start = highlight - max_display + 1;

        for (int i = 0; i < max_display && i + start < count; ++i) {
            int idx = i + start;
            if (idx == highlight)
                wattron(win, A_REVERSE);
            mvwprintw(win, i + 1, 1, "%s", colors[idx]);
            wattroff(win, A_REVERSE);
        }

        for (int i = count - start; i < max_display; ++i) {
            mvwprintw(win, i + 1, 1, "%*s", win_width - 2, "");
        }


        mvwprintw(win, win_height - 1, 1, "Arrows: move  Enter: select  ESC: cancel");
        wrefresh(win);

        int ch = wgetch(win);
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
            return colors[highlight];
        } else if (ch == KEY_MOUSE) {
            MEVENT ev;
            if (getmouse(&ev) == OK &&
                (ev.bstate & (BUTTON1_PRESSED | BUTTON1_CLICKED |
                               BUTTON1_RELEASED))) {
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
                        curs_set(1);
                        return colors[highlight];
                    }
                }
            }
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
            return NULL;
        }
    }

    curs_set(1);
    return NULL;
}

/**
 * Show available theme files and let the user choose one.
 *
 * current - name of the currently loaded theme or NULL.
 * parent  - optional parent window for the popup list.
 *
 * Returns a newly allocated string with the selected theme name or
 * NULL if the selection was cancelled.  The caller must free it.
 */
const char *select_theme(const char *current, WINDOW *parent) {
    curs_set(0);
    const char *dirs[3];
    size_t dir_count = 0;

    const char *env_dir = getenv("VENTO_THEME_DIR");
    if (env_dir && *env_dir)
        dirs[dir_count++] = env_dir;

    dirs[dir_count++] = THEME_DIR;

    if (strcmp(THEME_DIR, "themes") != 0)
        dirs[dir_count++] = "themes";

    size_t count = 0;
    struct dirent *ent;
    char **names = NULL;
    DIR *open_dirs[3];
    size_t open_count = 0;

    for (size_t d = 0; d < dir_count; ++d) {
        DIR *dir = opendir(dirs[d]);
        if (!dir)
            continue;
        open_dirs[open_count++] = dir;
        while ((ent = readdir(dir)) != NULL) {
            if (ent->d_name[0] == '.')
                continue;
            const char *dot = strrchr(ent->d_name, '.');
            if (!dot || strcasecmp(dot, ".theme") != 0)
                continue;
            char name[64];
            size_t len = dot - ent->d_name;
            if (len >= sizeof(name))
                len = sizeof(name) - 1;
            strncpy(name, ent->d_name, len);
            name[len] = '\0';
            int exists = 0;
            for (size_t i = 0; i < count; ++i) {
                if (strcasecmp(name, names[i]) == 0) {
                    exists = 1;
                    break;
                }
            }
            if (!exists) {
                char **tmp = realloc(names, sizeof(char *) * (count + 1));
                if (!tmp) {
                    goto dir_fail;
                }
                names = tmp;
                names[count] = strdup(name);
                if (!names[count]) {
                    goto dir_fail;
                }
                ++count;
            }
        }
    }

    for (size_t i = 0; i < open_count; ++i)
        closedir(open_dirs[i]);
    open_count = 0;

    if (count == 0) {
        free(names);
        curs_set(1);
        return NULL;
    }

    goto after_dir_scan;

dir_fail:
    for (size_t i = 0; i < open_count; ++i)
        closedir(open_dirs[i]);
    open_count = 0;
    for (size_t i = 0; i < count; ++i)
        free(names[i]);
    free(names);
    curs_set(1);
    return NULL;

after_dir_scan:

    qsort(names, count, sizeof(char *), str_casecmp);

    int highlight = 0;
    if (current) {
        for (size_t i = 0; i < count; ++i) {
            if (strcasecmp(current, names[i]) == 0) {
                highlight = i;
                break;
            }
        }
    }

    int start = 0;
    int win_height, win_width;
    WINDOW *win;
    if (parent) {
        int ph, pw;
        getmaxyx(parent, ph, pw);
        win_height = ph - 4;
        win_width = pw - 4;
        if (win_width > COLS - 2)
            win_width = COLS - 2;
        if (win_width < 2)
            win_width = 2;
        win = create_popup_window(win_height, win_width, parent);
        if (!win) {
            for (size_t i = 0; i < count; ++i)
                free(names[i]);
            free(names);
            curs_set(1);
            show_message("Unable to create window");
            return NULL;
        }
    } else {
        win_height = LINES - 4;
        win_width = COLS - 4;
        if (win_width > COLS - 2)
            win_width = COLS - 2;
        if (win_width < 2)
            win_width = 2;
        win = create_popup_window(win_height, win_width, NULL);
        if (!win) {
            for (size_t i = 0; i < count; ++i)
                free(names[i]);
            free(names);
            curs_set(1);
            show_message("Unable to create window");
            return NULL;
        }
    }
    /* win is valid here */
    keypad(win, TRUE);

    while (1) {
        werase(win);
        box(win, 0, 0);

        int max_display = win_height - 3;
        if (highlight < start)
            start = highlight;
        if (highlight >= start + max_display)
            start = highlight - max_display + 1;

        for (int i = 0; i < max_display && i + start < (int)count; ++i) {
            int idx = i + start;
            if (idx == highlight)
                wattron(win, A_REVERSE);
            mvwprintw(win, i + 1, 1, "%s", names[idx]);
            wattroff(win, A_REVERSE);
        }

        for (int i = count - start; i < max_display; ++i) {
            mvwprintw(win, i + 1, 1, "%*s", win_width - 2, "");
        }

        AppConfig preview = app_config;
        load_theme(names[highlight], &preview);
        render_theme_sample(&preview, win, win_height - 2);

        mvwprintw(win, win_height - 1, 1,
                  "Arrows: move  Enter: select  ESC: cancel");
        wrefresh(win);

        int ch = wgetch(win);
        if (ch == KEY_UP) {
            if (highlight > 0)
                --highlight;
        } else if (ch == KEY_DOWN) {
            if (highlight < (int)count - 1)
                ++highlight;
        } else if (ch == '\n') {
            const char *result = strdup(names[highlight]);
            for (size_t i = 0; i < count; ++i)
                free(names[i]);
            free(names);
            werase(win);
            wrefresh(win);
            clear_theme_sample_pairs();
            delwin(win);
            if (parent) {
                touchwin(parent);
                wrefresh(parent);
            } else {
                wrefresh(stdscr);
            }
            curs_set(1);
            return result;
        } else if (ch == KEY_MOUSE) {
            MEVENT ev;
            if (getmouse(&ev) == OK &&
                (ev.bstate & (BUTTON1_PRESSED | BUTTON1_CLICKED |
                               BUTTON1_RELEASED))) {
                int wy, wx;
                getbegyx(win, wy, wx);
                int row = ev.y - wy - 1;
                int col = ev.x - wx - 1;
                int max_display = win_height - 3;
                if (row >= 0 && row < max_display &&
                    col >= 0 && col < win_width - 2) {
                    int idx = start + row;
                    if (idx >= 0 && idx < (int)count)
                        highlight = idx;
                    if (ev.bstate & (BUTTON1_RELEASED | BUTTON1_CLICKED)) {
                        const char *result = strdup(names[highlight]);
                        for (size_t i = 0; i < count; ++i)
                            free(names[i]);
                        free(names);
                        werase(win);
                        wrefresh(win);
                        clear_theme_sample_pairs();
                        delwin(win);
                        if (parent) {
                            touchwin(parent);
                            wrefresh(parent);
                        } else {
                            wrefresh(stdscr);
                        }
                        curs_set(1);
                        return result;
                    }
                }
            }
        } else if (ch == 27) {
            for (size_t i = 0; i < count; ++i)
                free(names[i]);
            free(names);
            werase(win);
            wrefresh(win);
            clear_theme_sample_pairs();
            delwin(win);
            if (parent) {
                touchwin(parent);
                wrefresh(parent);
            } else {
                wrefresh(stdscr);
            }
            curs_set(1);
            return NULL;
        }
    }
}

/**
 * Prompt the user to choose between Enabled or Disabled.
 *
 * prompt  - optional text displayed above the options.
 * current - currently selected boolean value.
 * parent  - optional parent window for the popup.
 *
 * Returns the chosen value or the original value if cancelled.
 */
int select_bool(const char *prompt, int current, WINDOW *parent) {
    curs_set(0);
    static const char *options[] = {"Disabled", "Enabled"};
    int highlight = current ? 1 : 0;

    int own = 0;
    int win_height = 6;
    int win_width = 20;
    if (win_width > COLS - 2)
        win_width = COLS - 2;
    if (win_width < 2)
        win_width = 2;
    WINDOW *win;
    if (parent) {
        win = create_popup_window(win_height, win_width, parent);
        if (!win) {
            curs_set(1);
            show_message("Unable to create window");
            return current;
        }
        own = 1;
    } else {
        win = create_popup_window(win_height, win_width, NULL);
        if (!win) {
            curs_set(1);
            show_message("Unable to create window");
            return current;
        }
        own = 1;
    }
    keypad(win, TRUE);

    while (1) {
        werase(win);
        box(win, 0, 0);

        int row = 1;
        if (prompt) {
            mvwprintw(win, row++, 1, "%s", prompt);
        }

        for (int i = 0; i < 2; ++i) {
            if (highlight == i)
                wattron(win, A_REVERSE);
            mvwprintw(win, row + i, 1, "%s", options[i]);
            wattroff(win, A_REVERSE);
        }

        mvwprintw(win, win_height - 1, 1, "Arrows: move  Enter: select  ESC: cancel");
        wrefresh(win);

        int ch = wgetch(win);
        if (ch == KEY_UP || ch == KEY_LEFT) {
            if (highlight > 0)
                --highlight;
        } else if (ch == KEY_DOWN || ch == KEY_RIGHT) {
            if (highlight < 1)
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
            return highlight == 1 ? 1 : 0;
        } else if (ch == KEY_MOUSE) {
            MEVENT ev;
            if (getmouse(&ev) == OK &&
                (ev.bstate & (BUTTON1_PRESSED | BUTTON1_CLICKED |
                               BUTTON1_RELEASED))) {
                int wy, wx;
                getbegyx(win, wy, wx);
                int option_start = prompt ? 2 : 1;
                int row_click = ev.y - wy - option_start;
                int col = ev.x - wx - 1;
                if (row_click >= 0 && row_click < 2 &&
                    col >= 0 && col < win_width - 2) {
                    highlight = row_click;

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
                        curs_set(1);
                        return highlight == 1 ? 1 : 0;
                    }
                }
            }
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
            return current;
        }
    }

    curs_set(1);
    return current;
}

/**
 * Prompt the user for an integer value.
 *
 * ctx     - editor context used for the input dialog.
 * prompt  - text describing the value being edited.
 * current - current integer value.
 * parent  - optional parent window for the popup.
 *
 * Returns the parsed integer or the original value if cancelled or invalid.
 */
int select_int(EditorContext *ctx, const char *prompt, int current,
               WINDOW *parent) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%d", current);
    char msg[64];
    snprintf(msg, sizeof(msg), "%s", prompt ? prompt : "Value");
    create_dialog(ctx, msg, buf, sizeof(buf));
    if (parent) {
        touchwin(parent);
        wrefresh(parent);
    } else {
        wrefresh(stdscr);
    }
    if (buf[0] == '\0')
        return current;

    errno = 0;
    char *end;
    long val = strtol(buf, &end, 10);
    if (errno != 0 || *end != '\0' || val > INT_MAX || val <= 0)
        return current;

    return (int)val;
}

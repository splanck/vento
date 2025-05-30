#include "ui_common.h"
#include "config.h"
#include <ncurses.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>

int show_settings_dialog(AppConfig *cfg) {
    AppConfig original = *cfg;

    enum {
        FIELD_ENABLE_COLOR,
        FIELD_MOUSE,
        FIELD_BACKGROUND,
        FIELD_KEYWORD,
        FIELD_COMMENT,
        FIELD_STRING,
        FIELD_TYPE,
        FIELD_SYMBOL,
        FIELD_COUNT
    };

    const char *labels[FIELD_COUNT] = {
        "Enable color",
        "Enable mouse",
        "Background color",
        "Keyword color",
        "Comment color",
        "String color",
        "Type color",
        "Symbol color"
    };

    int highlight = 0;
    int ch;
    int done = 0;

    int win_height = FIELD_COUNT + 4;
    int longest = 0;
    for (int i = 0; i < FIELD_COUNT; ++i) {
        const char *val = "";
        switch (i) {
        case FIELD_ENABLE_COLOR:
            val = cfg->enable_color ? "Enabled" : "Disabled";
            break;
        case FIELD_MOUSE:
            val = cfg->enable_mouse ? "Enabled" : "Disabled";
            break;
        case FIELD_BACKGROUND:
            val = cfg->background_color;
            break;
        case FIELD_KEYWORD:
            val = cfg->keyword_color;
            break;
        case FIELD_COMMENT:
            val = cfg->comment_color;
            break;
        case FIELD_STRING:
            val = cfg->string_color;
            break;
        case FIELD_TYPE:
            val = cfg->type_color;
            break;
        case FIELD_SYMBOL:
            val = cfg->symbol_color;
            break;
        }
        int len = strlen(labels[i]) + 2 + strlen(val);
        if (len > longest)
            longest = len;
    }
    int win_width = longest + 4;
    if (win_width < 50)
        win_width = 50;

    WINDOW *win = create_popup_window(win_height, win_width, NULL);
    if (!win)
        return 0;
    keypad(win, TRUE);

    while (!done) {
        werase(win);
        box(win, 0, 0);

        for (int i = 0; i < FIELD_COUNT; ++i) {
            if (i == highlight)
                wattron(win, A_REVERSE);

            switch (i) {
            case FIELD_ENABLE_COLOR:
                mvwprintw(win, i + 1, 2, "%s: %s", labels[i],
                          cfg->enable_color ? "Enabled" : "Disabled");
                break;
            case FIELD_MOUSE:
                mvwprintw(win, i + 1, 2, "%s: %s", labels[i],
                          cfg->enable_mouse ? "Enabled" : "Disabled");
                break;
            case FIELD_BACKGROUND:
                mvwprintw(win, i + 1, 2, "%s: %s", labels[i],
                          cfg->background_color);
                break;
            case FIELD_KEYWORD:
                mvwprintw(win, i + 1, 2, "%s: %s", labels[i],
                          cfg->keyword_color);
                break;
            case FIELD_COMMENT:
                mvwprintw(win, i + 1, 2, "%s: %s", labels[i],
                          cfg->comment_color);
                break;
            case FIELD_STRING:
                mvwprintw(win, i + 1, 2, "%s: %s", labels[i],
                          cfg->string_color);
                break;
            case FIELD_TYPE:
                mvwprintw(win, i + 1, 2, "%s: %s", labels[i], cfg->type_color);
                break;
            case FIELD_SYMBOL:
                mvwprintw(win, i + 1, 2, "%s: %s", labels[i],
                          cfg->symbol_color);
                break;
            }

            if (i == highlight)
                wattroff(win, A_REVERSE);
        }

        mvwprintw(win, win_height - 2, 2,
                  "Arrows: move  Enter: change  ESC: done");
        wrefresh(win);

        ch = wgetch(win);
        if (ch == KEY_UP) {
            if (highlight > 0)
                --highlight;
        } else if (ch == KEY_DOWN) {
            if (highlight < FIELD_COUNT - 1)
                ++highlight;
        } else if (ch == '\n') {
            switch (highlight) {
            case FIELD_ENABLE_COLOR:
                cfg->enable_color =
                    select_bool("Enable color", cfg->enable_color, win);
                break;
            case FIELD_MOUSE:
                cfg->enable_mouse =
                    select_bool("Enable mouse", cfg->enable_mouse, win);
                break;
            case FIELD_BACKGROUND: {
                const char *sel = select_color(cfg->background_color, win);
                if (sel) {
                    strncpy(cfg->background_color, sel,
                            sizeof(cfg->background_color) - 1);
                    cfg->background_color[sizeof(cfg->background_color) - 1] =
                        '\0';
                }
                break;
            }
            case FIELD_KEYWORD: {
                const char *sel = select_color(cfg->keyword_color, win);
                if (sel) {
                    strncpy(cfg->keyword_color, sel,
                            sizeof(cfg->keyword_color) - 1);
                    cfg->keyword_color[sizeof(cfg->keyword_color) - 1] = '\0';
                }
                break;
            }
            case FIELD_COMMENT: {
                const char *sel = select_color(cfg->comment_color, win);
                if (sel) {
                    strncpy(cfg->comment_color, sel,
                            sizeof(cfg->comment_color) - 1);
                    cfg->comment_color[sizeof(cfg->comment_color) - 1] = '\0';
                }
                break;
            }
            case FIELD_STRING: {
                const char *sel = select_color(cfg->string_color, win);
                if (sel) {
                    strncpy(cfg->string_color, sel,
                            sizeof(cfg->string_color) - 1);
                    cfg->string_color[sizeof(cfg->string_color) - 1] = '\0';
                }
                break;
            }
            case FIELD_TYPE: {
                const char *sel = select_color(cfg->type_color, win);
                if (sel) {
                    strncpy(cfg->type_color, sel,
                            sizeof(cfg->type_color) - 1);
                    cfg->type_color[sizeof(cfg->type_color) - 1] = '\0';
                }
                break;
            }
            case FIELD_SYMBOL: {
                const char *sel = select_color(cfg->symbol_color, win);
                if (sel) {
                    strncpy(cfg->symbol_color, sel,
                            sizeof(cfg->symbol_color) - 1);
                    cfg->symbol_color[sizeof(cfg->symbol_color) - 1] = '\0';
                }
                break;
            }
            }
        } else if (ch == 27) {
            done = 1;
        }
    }

    wclear(win);
    wrefresh(win);
    delwin(win);
    wrefresh(stdscr);

    return memcmp(&original, cfg, sizeof(AppConfig)) != 0;
}

const char *select_color(const char *current, WINDOW *parent) {
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
        win = create_popup_window(win_height, win_width, parent);
        if (!win)
            return NULL;
        own = 1;
    } else {
        win_height = LINES - 4;
        win_width = COLS - 4;
        win = create_popup_window(win_height, win_width, NULL);
        if (!win)
            return NULL;
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
            return colors[highlight];
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
            return NULL;
        }
    }

    return NULL;
}

int select_bool(const char *prompt, int current, WINDOW *parent) {
    static const char *options[] = {"Disabled", "Enabled"};
    int highlight = current ? 1 : 0;

    int own = 0;
    int win_height = 6;
    int win_width = 20;
    WINDOW *win;
    if (parent) {
        win = create_popup_window(win_height, win_width, parent);
        if (!win)
            return current;
        own = 1;
    } else {
        win = create_popup_window(win_height, win_width, NULL);
        if (!win)
            return current;
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
            return highlight == 1 ? 1 : 0;
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
            return current;
        }
    }

    return current;
}

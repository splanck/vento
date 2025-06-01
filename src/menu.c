#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include "menu.h"
#include "editor.h"
#include "search.h"
#include "file_ops.h"
#include "ui.h"
#include "undo.h"
#include "config.h"

#include "editor_state.h"

#define ARRAY_LEN(arr) ((int)(sizeof(arr) / sizeof((arr)[0])))

/* confirm_quit implemented in vento.c */
bool confirm_quit(void);

static EditorContext *menu_ctx = NULL;


static void menuNewFile_cb(void)    { menuNewFile(menu_ctx); }
static void menuLoadFile_cb(void)   { menuLoadFile(menu_ctx); }
static void menuSaveFile_cb(void)   { menuSaveFile(menu_ctx); }
static void menuSaveAs_cb(void)     { menuSaveAs(menu_ctx); }
static void menuCloseFile_cb(void)  { menuCloseFile(menu_ctx); }
static void menuNextFile_cb(void)   { menuNextFile(menu_ctx); }
static void menuPrevFile_cb(void)   { menuPrevFile(menu_ctx); }
static void menuSettings_cb(void)   { menuSettings(menu_ctx); }
static void menuQuitEditor_cb(void) { menuQuitEditor(menu_ctx); }
static void menuUndo_cb(void)       { menuUndo(menu_ctx); }
static void menuRedo_cb(void)       { menuRedo(menu_ctx); }
static void menuFind_cb(void)       { menuFind(menu_ctx); }
static void menuReplace_cb(void)    { menuReplace(menu_ctx); }
static void menuAbout_cb(void)      { menuAbout(menu_ctx); }
static void menuHelp_cb(void)       { menuHelp(menu_ctx); }

static MenuItem file_items[] = {
    {"New File", menuNewFile_cb, false},
    {"Load File", menuLoadFile_cb, false},
    {"Save File", menuSaveFile_cb, false},
    {"Save As", menuSaveAs_cb, false},
    {"", NULL, true},
    {"Close File", menuCloseFile_cb, false},
    {"Quit", menuQuitEditor_cb, false},
};

static MenuItem edit_items[] = {
    {"Undo", menuUndo_cb, false},
    {"Redo", menuRedo_cb, false},
    {"", NULL, true},
    {"Find", menuFind_cb, false},
    {"Replace", menuReplace_cb, false},
};

static MenuItem nav_items[] = {
    {"Next File", menuNextFile_cb, false},
    {"Previous File", menuPrevFile_cb, false},
};

static MenuItem opt_items[] = {
    {"Settings", menuSettings_cb, false},
};

static MenuItem help_items[] = {
    {"Help Screen", menuHelp_cb, false},
    {"About Vento", menuAbout_cb, false},
};

static Menu menus_static[] = {
    {"File", file_items, ARRAY_LEN(file_items)},
    {"Edit", edit_items, ARRAY_LEN(edit_items)},
    {"Navigate", nav_items, ARRAY_LEN(nav_items)},
    {"Options", opt_items, ARRAY_LEN(opt_items)},
    {"Help", help_items, ARRAY_LEN(help_items)},
};

static int menuPositions_static[ARRAY_LEN(menus_static)] = {0};

Menu *menus = menus_static;
int menuCount = ARRAY_LEN(menus_static);
int *menuPositions = menuPositions_static;

/**
 * Initializes the menus.
 */
void initializeMenus(EditorContext *ctx) {
    menu_ctx = ctx;
    menus = menus_static;
    menuPositions = menuPositions_static;
    menuCount = ARRAY_LEN(menus_static);
}

/**
 * Handles the navigation and interaction with menus.
 * 
 * @param menus The array of menus.
 * @param menuCount The number of menus in the array.
 * @param currentMenu A pointer to the index of the current menu.
 * @param currentItem A pointer to the index of the current item in the menu.
 */
void handleMenuNavigation(Menu *menus, int menuCount, int *currentMenu, int *currentItem) {
    int ch;
    bool inMenu = true;
    int prevMenu = -1;

    curs_set(0);
    /* Keep the existing text content displayed while the menu is open */
    wnoutrefresh(text_win);

    while (inMenu) {
        if (*currentMenu != prevMenu) {
            touchwin(text_win);
            wnoutrefresh(text_win);
            prevMenu = *currentMenu;
        }
        if (*currentMenu < 0 || *currentMenu >= menuCount) {
            inMenu = false;
            continue;
        }

        // Draw menu bar and compute menu positions
        drawMenuBar(menus, menuCount);
        wnoutrefresh(stdscr);
        doupdate();

        if (menus[*currentMenu].items == NULL) {
            inMenu = false;
            continue;
        }

        int startX = menuPositions[*currentMenu];
        int startY = 1;
        int boxWidth = 20;
        int boxHeight = menus[*currentMenu].itemCount + 2;
        if (startX + boxWidth > COLS || startY + boxHeight > LINES) {
            inMenu = false;
            continue;
        }
        if (!drawMenu(&menus[*currentMenu], *currentItem, startX, startY)) {
            inMenu = false;
            continue;
        }
        wnoutrefresh(stdscr);
        doupdate();

        ch = getch();
        switch (ch) {
            case KEY_LEFT:
                if (*currentMenu > 0) (*currentMenu)--; // Move to the previous menu
                *currentItem = 0; // Reset the current item index
                while (*currentItem < menus[*currentMenu].itemCount &&
                       menus[*currentMenu].items[*currentItem].separator)
                    (*currentItem)++; // Skip separators
                break;
            case KEY_RIGHT:
                if (*currentMenu < menuCount - 1) (*currentMenu)++; // Move to the next menu
                *currentItem = 0; // Reset the current item index
                while (*currentItem < menus[*currentMenu].itemCount &&
                       menus[*currentMenu].items[*currentItem].separator)
                    (*currentItem)++; // Skip separators
                break;
            case KEY_UP:
                if (*currentItem > 0) (*currentItem)--; // Move to the previous item
                while (*currentItem > 0 &&
                       menus[*currentMenu].items[*currentItem].separator)
                    (*currentItem)--; // Skip separators
                break;
            case KEY_DOWN:
                if (*currentItem < menus[*currentMenu].itemCount - 1) (*currentItem)++; // Move to the next item
                while (*currentItem < menus[*currentMenu].itemCount - 1 &&
                       menus[*currentMenu].items[*currentItem].separator)
                    (*currentItem)++; // Skip separators
                break;
            case KEY_MOUSE: {
                if (!enable_mouse)
                    break;
                MEVENT ev;
                if (getmouse(&ev) == OK &&
                    (ev.bstate & (BUTTON1_PRESSED |
                                   BUTTON1_RELEASED |
                                   BUTTON1_CLICKED))) {
                    if (ev.y == 0) {
                        int newMenu = -1;
                        for (int i = 0; i < menuCount; ++i) {
                            int start = menuPositions[i];
                            int end = (i == menuCount - 1) ? COLS : menuPositions[i + 1];
                            if (ev.x >= start && ev.x < end) {
                                newMenu = i;
                                break;
                            }
                        }
                        if (newMenu >= 0 && newMenu < menuCount) {
                            *currentMenu = newMenu;
                            *currentItem = 0;
                            while (*currentItem < menus[*currentMenu].itemCount &&
                                   menus[*currentMenu].items[*currentItem].separator)
                                (*currentItem)++; // Skip separators
                            int startX = menuPositions[*currentMenu];
                            int startY = 1;
                            if (!drawMenu(&menus[*currentMenu], *currentItem, startX, startY)) {
                                inMenu = false;
                            }
                            wnoutrefresh(stdscr);
                            doupdate();
                            break;
                        }
                    }

                    if (*currentMenu < 0 || *currentMenu >= menuCount) {
                        inMenu = false;
                        break;
                    }
                    int startX = menuPositions[*currentMenu];
                    int startY = 1;
                    int boxWidth = 20;
                    int boxHeight = menus[*currentMenu].itemCount + 2;
                    if (startX + boxWidth > COLS || startY + boxHeight > LINES) {
                        inMenu = false;
                        break;
                    }

                    if (ev.x >= startX && ev.x < startX + boxWidth &&
                        ev.y >= startY && ev.y < startY + boxHeight) {
                        int row = ev.y - startY - 1;
                        if (row >= 0 && row < menus[*currentMenu].itemCount) {
                            if (!menus[*currentMenu].items[row].separator) {
                                *currentItem = row;
                                if (!drawMenu(&menus[*currentMenu], *currentItem, startX, startY)) {
                                    inMenu = false;
                                    break;
                                }
                                wnoutrefresh(stdscr);
                                doupdate();
                                menus[*currentMenu].items[*currentItem].action();
                                inMenu = false;
                            }
                        }
                    } else {
                        inMenu = false;
                    }
                }
                break;
            }
            case '\n': // Enter key
                if (*currentItem >= 0 && *currentItem < menus[*currentMenu].itemCount &&
                    menus[*currentMenu].items != NULL &&
                    !menus[*currentMenu].items[*currentItem].separator) {
                    menus[*currentMenu].items[*currentItem].action();
                    inMenu = false; // Exit after valid selection
                }
                break;
            case 27: // ESC key
                inMenu = false; // Exit the menu loop
                break;
            default:
                break;
        }
    }
    touchwin(text_win);
    wnoutrefresh(text_win);
    wnoutrefresh(stdscr);
    doupdate();
    curs_set(1);
}


void menuNewFile(EditorContext *ctx) {
    new_file(ctx->active_file);
    ctx->active_file = active_file;
    ctx->text_win = text_win;
}

void menuLoadFile(EditorContext *ctx) {
    load_file(ctx, ctx->active_file, NULL);
    ctx->active_file = active_file;
    ctx->text_win = text_win;
}

void menuSaveFile(EditorContext *ctx) {
    save_file(ctx, ctx->active_file);
}

void menuSaveAs(EditorContext *ctx) {
    save_file_as(ctx, ctx->active_file);
}

void menuCloseFile(EditorContext *ctx) {
    close_current_file(ctx, ctx->active_file, &ctx->active_file->cursor_x,
                      &ctx->active_file->cursor_y);
    ctx->active_file = active_file;
    ctx->text_win = text_win;
}

void menuNextFile(EditorContext *ctx) {
    next_file(ctx, ctx->active_file, &ctx->active_file->cursor_x, &ctx->active_file->cursor_y);
}

void menuPrevFile(EditorContext *ctx) {
    prev_file(ctx, ctx->active_file, &ctx->active_file->cursor_x, &ctx->active_file->cursor_y);
}

void menuSettings(EditorContext *ctx) {
    if (show_settings_dialog(ctx, &ctx->config)) {
        config_save(&ctx->config);
        config_load(&ctx->config);
        ctx->enable_mouse = enable_mouse;
        ctx->enable_color = enable_color;
        if (ctx->enable_mouse)
            mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
        else
            mousemask(0, NULL);
        apply_colors();
        redraw();
        drawBar();
    }
}

void menuQuitEditor(EditorContext *ctx) {
    (void)ctx;
    if (confirm_quit())
        close_editor();
}

void menuUndo(EditorContext *ctx) {
    undo(ctx->active_file);
}

void menuRedo(EditorContext *ctx) {
    redo(ctx->active_file);
}

void menuFind(EditorContext *ctx) {
    find(ctx, ctx->active_file, 1);
}

void menuReplace(EditorContext *ctx) {
    replace(ctx, ctx->active_file);
}

void menuAbout(EditorContext *ctx) {
    show_about(ctx);
}

void menuHelp(EditorContext *ctx) {
    show_help(ctx);
}


/**
 * Frees the memory allocated for all menus and their menu items.
 */
void freeMenus() {
    (void)menus;
    (void)menuPositions;
    menuCount = ARRAY_LEN(menus_static);
    menus = menus_static;
    menuPositions = menuPositions_static;
}

int menu_click_open(int x, int y) {
    if (y == 0) {
        int currentMenu = -1;
        for (int i = 0; i < menuCount; ++i) {
            int start = menuPositions[i];
            int end = (i == menuCount - 1) ? COLS : menuPositions[i + 1];
            if (x >= start && x < end) {
                currentMenu = i;
                break;
            }
        }
        if (currentMenu < 0 || currentMenu >= menuCount) {
            return 0;
        }

        int currentItem = 0;
        handleMenuNavigation(menus, menuCount, &currentMenu, &currentItem);
        mousemask(enable_mouse ? ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION : 0, NULL);
        return 1;
    }

    return 0;
}


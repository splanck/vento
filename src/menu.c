/*
 * Menu system overview
 * --------------------
 * The editor's menu bar is built entirely from statically defined
 * Menu and MenuItem structures.  Each Menu contains an array of
 * MenuItems with an associated callback.  The arrays of items for the
 * "File", "Edit" and other top level menus are declared below and then
 * grouped into the menus_static array.
 *
 * initializeMenus() assigns menus_static and the accompanying
 * menuPositions_static array to the global pointers used throughout the
 * UI.  drawMenuBar() (implemented in menu_draw.c) walks the menus array,
 * draws the menu labels on the top screen row and records the starting
 * x‑coordinate of each label in menuPositions.  Those stored positions
 * are later consulted by handleMenuNavigation() and drawMenu() to place
 * drop‑down windows and to detect mouse clicks on the menu bar.
 *
 * Navigation through the menu bar ultimately leads to the invocation of
 * the callback associated with the highlighted MenuItem.  The callbacks
 * themselves live in this file and update the EditorContext and other
 * global state as appropriate.
 */
#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include "menu.h"
#include "editor.h"
#include "search.h"
#include "file_ops.h"
#include "ui.h"
#include "clipboard.h"
#include "undo.h"
#include "config.h"
#include "macro.h"
#include "ui_common.h"

#include "editor_state.h"

#define ARRAY_LEN(arr) ((int)(sizeof(arr) / sizeof((arr)[0])))

/* confirm_quit implemented in vento.c */
bool confirm_quit(void);

static EditorContext *menu_ctx = NULL;

/* forward declarations for macro menu actions */
static void menuMacroStart(EditorContext *ctx);
static void menuMacroStop(EditorContext *ctx);
static void menuMacroPlay(EditorContext *ctx);
static void menuManageMacros(EditorContext *ctx);


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
static void menuCut_cb(void)        { menuCut(menu_ctx); }
static void menuCopy_cb(void)       { menuCopy(menu_ctx); }
static void menuPaste_cb(void)      { menuPaste(menu_ctx); }
static void menuFind_cb(void)       { menuFind(menu_ctx); }
static void menuReplace_cb(void)    { menuReplace(menu_ctx); }
static void menuAbout_cb(void)      { menuAbout(menu_ctx); }
static void menuHelp_cb(void)       { menuHelp(menu_ctx); }
static void menuMacroStart_cb(void) { menuMacroStart(menu_ctx); }
static void menuMacroStop_cb(void)  { menuMacroStop(menu_ctx); }
static void menuMacroPlay_cb(void)  { menuMacroPlay(menu_ctx); }
static void menuManage_cb(void)     { menuManageMacros(menu_ctx); }

/* Items for the File menu.  Selecting one of these invokes the
 * corresponding menu* callback above. */
static MenuItem file_items[] = {
    {"New File", "Ctrl-N", menuNewFile_cb, false},
    {"Load File", "Ctrl-L", menuLoadFile_cb, false},
    {"Save File", "Ctrl-S", menuSaveFile_cb, false},
    {"Save As", "Ctrl-O", menuSaveAs_cb, false},
    {"", NULL, NULL, true},
    {"Close File", "Ctrl-Q", menuCloseFile_cb, false},
    {"Quit", NULL, menuQuitEditor_cb, false},
};

/* Items for the Edit menu triggering undo/redo and search actions. */
static MenuItem edit_items[] = {
    {"Undo", "Ctrl-Z", menuUndo_cb, false},
    {"Redo", "Ctrl-Y", menuRedo_cb, false},
    {"", NULL, NULL, true},
    {"Cut", "Ctrl-X", menuCut_cb, false},
    {"Copy", "Ctrl-C", menuCopy_cb, false},
    {"Paste", "Ctrl-V", menuPaste_cb, false},
    {"Find", "Ctrl-F", menuFind_cb, false},
    {"Replace", "Ctrl-R", menuReplace_cb, false},
};

/* Items for the Navigate menu for switching buffers. */
static MenuItem nav_items[] = {
    {"Next File", "F6", menuNextFile_cb, false},
    {"Previous File", "F7", menuPrevFile_cb, false},
};

/* Options menu currently only exposes the settings dialog. */
static MenuItem opt_items[] = {
    {"Settings", NULL, menuSettings_cb, false},
};

/* Macros menu controls recording and playback of macros. */
static MenuItem macros_items[] = {
    {"Start Recording", "F2", menuMacroStart_cb, false},
    {"Stop Recording", "F2", menuMacroStop_cb, false},
    {"", NULL, NULL, true},
    {"Play Last Macro", "F4", menuMacroPlay_cb, false},
    {"", NULL, NULL, true},
    {"Manage Macros...", NULL, menuManage_cb, false},
};

/* Help menu containing documentation and about screen. */
static MenuItem help_items[] = {
    {"Help Screen", "Ctrl-H", menuHelp_cb, false},
    {"About Vento", "Ctrl-A", menuAbout_cb, false},
};

static Menu menus_static[] = {
    {"File", file_items, ARRAY_LEN(file_items)},
    {"Edit", edit_items, ARRAY_LEN(edit_items)},
    {"Navigate", nav_items, ARRAY_LEN(nav_items)},
    {"Options", opt_items, ARRAY_LEN(opt_items)},
    {"Macros", macros_items, ARRAY_LEN(macros_items)},
    {"Help", help_items, ARRAY_LEN(help_items)},
};

static int menuPositions_static[ARRAY_LEN(menus_static)] = {0};

Menu *menus = menus_static;
int menuCount = ARRAY_LEN(menus_static);
int *menuPositions = menuPositions_static;

/**
 * Initializes global menu pointers for the UI.
 *
 * @param ctx Editor context whose window and configuration are used by
 *            menu callbacks.
 *
 * This function sets the global arrays used by drawMenuBar() and
 * handleMenuNavigation() to the statically defined menus.  It should be
 * called once during startup before any menu drawing occurs.
 */
void initializeMenus(EditorContext *ctx) {
    menu_ctx = ctx;
    menus = menus_static;
    menuPositions = menuPositions_static;
    menuCount = ARRAY_LEN(menus_static);
}

/**
 * Handles user input while a drop-down menu is open.
 *
 * The function relies on menuPositions, which is populated by
 * drawMenuBar(), to determine where each menu label begins on
 * the top bar.  Those positions are used both for drop-down
 * placement and for hit testing when mouse events occur.
 * Keyboard navigation updates the currentMenu and currentItem
 * indices, and KEY_MOUSE events compare the click coordinates
 * against menuPositions to select or activate items.
 *
 * @param menus The array of menus.
 * @param menuCount The number of menus in the array.
 * @param currentMenu A pointer to the index of the current menu.
 * @param currentItem A pointer to the index of the current item in the menu.
 *
 * currentMenu and currentItem are updated in place to reflect the user's
 * navigation.  When an item is activated the associated callback may modify
 * the global EditorContext and other shared state.
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
        int boxWidth = calcMenuWidth(&menus[*currentMenu]);
        int boxHeight = menus[*currentMenu].itemCount + 2;
        if (startX + boxWidth > COLS)
            startX = COLS - boxWidth;
        if (startX < 0)
            startX = 0;
        if (startY + boxHeight > LINES)
            startY = LINES - boxHeight;
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
            case KEY_RESIZE:
                resizeterm(0, 0);
                drawMenuBar(menus, menuCount);
                wnoutrefresh(stdscr);
                doupdate();
                if (!drawMenu(&menus[*currentMenu], *currentItem,
                               menuPositions[*currentMenu], 1)) {
                    inMenu = false;
                }
                wnoutrefresh(stdscr);
                doupdate();
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
                    int boxWidth = calcMenuWidth(&menus[*currentMenu]);
                    int boxHeight = menus[*currentMenu].itemCount + 2;
                    if (startX + boxWidth > COLS)
                        startX = COLS - boxWidth;
                    if (startX < 0)
                        startX = 0;
                    if (startY + boxHeight > LINES)
                        startY = LINES - boxHeight;

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


/**
 * Callback for File -> "New File".
 *
 * Creates an empty buffer and updates the editor context to point at the
 * newly created file and text window.
 *
 * @param ctx Editor context to update after the buffer is created.
 */
void menuNewFile(EditorContext *ctx) {
    new_file(ctx, ctx->active_file);
    ctx->active_file = active_file;
    ctx->text_win = text_win;
}

/**
 * Callback for File -> "Load File".
 *
 * Opens a file from disk and switches the active buffer to the loaded
 * file.  The editor context's active_file and text_win members are
 * updated to match the globals created by load_file().
 *
 * @param ctx Editor context to synchronize with the loaded file.
 */
void menuLoadFile(EditorContext *ctx) {
    load_file(ctx, ctx->active_file, NULL);
    ctx->active_file = active_file;
    ctx->text_win = text_win;
    sync_editor_context(ctx);
    update_status_bar(ctx, ctx->active_file);
}

/**
 * Callback for File -> "Save File".
 *
 * Writes the active buffer to disk.
 *
 * @param ctx Editor context providing the active file.
 */
void menuSaveFile(EditorContext *ctx) {
    save_file(ctx, ctx->active_file);
}

/**
 * Callback for File -> "Save As".
 *
 * Prompts the user for a new path and saves the active buffer there.
 *
 * @param ctx Editor context providing the active file.
 */
void menuSaveAs(EditorContext *ctx) {
    save_file_as(ctx, ctx->active_file);
}

/**
 * Callback for File -> "Close File".
 *
 * Removes the current buffer from the editor and updates ctx to point to
 * the new active file (if any).
 *
 * @param ctx Editor context to update after closing the file.
 */
void menuCloseFile(EditorContext *ctx) {
    int cx = ctx->active_file ? ctx->active_file->cursor_x : 0;
    int cy = ctx->active_file ? ctx->active_file->cursor_y : 0;
    close_current_file(ctx, ctx->active_file, &cx, &cy);
    if (active_file) {
        active_file->cursor_x = cx;
        active_file->cursor_y = cy;
    }
    ctx->active_file = active_file;
    ctx->text_win = text_win;
}

/**
 * Callback for Navigate -> "Next File".
 *
 * Switches the active buffer to the next file managed by the file
 * manager and updates the editor context accordingly.
 *
 * @param ctx Editor context that tracks the active file.
 */
void menuNextFile(EditorContext *ctx) {
    (void)next_file(ctx);
}

/**
 * Callback for Navigate -> "Previous File".
 *
 * Switches the active buffer to the previous file.
 *
 * @param ctx Editor context that tracks the active file.
 */
void menuPrevFile(EditorContext *ctx) {
    (void)prev_file(ctx);
}

/**
 * Callback for Macros -> "Start Recording".
 *
 * Begins recording keystrokes into the current macro.  The macro state
 * is stored globally and the status bar is refreshed to reflect that
 * recording is active.
 *
 * @param ctx Editor context (unused).
 */
static void menuMacroStart(EditorContext *ctx) {
    (void)ctx;
    if (current_macro)
        macro_start(current_macro);
    update_status_bar(menu_ctx, menu_ctx->active_file);
}

/**
 * Callback for Macros -> "Stop Recording".
 *
 * Stops recording the current macro and persists it to disk.
 *
 * @param ctx Editor context (unused).
 */
static void menuMacroStop(EditorContext *ctx) {
    (void)ctx;
    if (current_macro) {
        macro_stop(current_macro);
        macros_save(&app_config);
    }
    update_status_bar(menu_ctx, menu_ctx->active_file);
}

/**
 * Callback for Macros -> "Play Last Macro".
 *
 * Executes the keystrokes recorded in the most recently captured macro
 * in the context of the active file.
 *
 * @param ctx Editor context providing the active file.
 */
static void menuMacroPlay(EditorContext *ctx) {
    if (current_macro)
        macro_play(current_macro, ctx, ctx->active_file);
    update_status_bar(menu_ctx, menu_ctx->active_file);
}

/**
 * Callback for Macros -> "Manage Macros...".
 *
 * Displays a placeholder message.  Management functionality is not
 * implemented.
 *
 * @param ctx Editor context (unused).
 */
static void menuManageMacros(EditorContext *ctx) {
    show_manage_macros(ctx);
    update_status_bar(menu_ctx, menu_ctx->active_file);
}

/**
 * Callback for Options -> "Settings".
 *
 * Presents the settings dialog and applies any configuration changes
 * chosen by the user.  Updates mouse and color state and redraws the UI.
 *
 * @param ctx Editor context whose configuration is modified.
 */
void menuSettings(EditorContext *ctx) {
    if (show_settings_dialog(ctx, &ctx->config)) {
        config_save(&ctx->config);
        config_load(&ctx->config);
        app_config = ctx->config;
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

/**
 * Callback for File -> "Quit".
 *
 * Prompts the user for confirmation and exits the editor if confirmed.
 *
 * @param ctx Editor context (unused).
 */
void menuQuitEditor(EditorContext *ctx) {
    (void)ctx;
    if (confirm_quit())
        close_editor();
}

/**
 * Callback for Edit -> "Undo".
 *
 * Reverts the last change in the active buffer.
 *
 * @param ctx Editor context providing the active file.
 */
void menuUndo(EditorContext *ctx) {
    undo(ctx->active_file);
}

/**
 * Callback for Edit -> "Redo".
 *
 * Reapplies the last undone change in the active buffer.
 *
 * @param ctx Editor context providing the active file.
 */
void menuRedo(EditorContext *ctx) {
    redo(ctx->active_file);
}

/**
 * Callback for Edit -> "Cut".
 *
 * Removes the selected text and copies it to the clipboard.
 */
void menuCut(EditorContext *ctx) {
    cut_selection(ctx->active_file);
    redraw();
}

/**
 * Callback for Edit -> "Copy".
 *
 * Copies the selected text to the clipboard.
 */
void menuCopy(EditorContext *ctx) {
    copy_selection_keyboard(ctx->active_file);
}

/**
 * Callback for Edit -> "Paste".
 *
 * Inserts clipboard contents at the cursor position.
 */
void menuPaste(EditorContext *ctx) {
    paste_clipboard(ctx->active_file,
                    &ctx->active_file->cursor_x,
                    &ctx->active_file->cursor_y);
    redraw();
}

/**
 * Callback for Edit -> "Find".
 *
 * Prompts for a search string and searches from the cursor position.
 *
 * @param ctx Editor context providing the active file.
 */
void menuFind(EditorContext *ctx) {
    find(ctx, ctx->active_file, 1);
}

/**
 * Callback for Edit -> "Replace".
 *
 * Opens the search/replace dialog operating on the active file.
 *
 * @param ctx Editor context providing the active file.
 */
void menuReplace(EditorContext *ctx) {
    replace(ctx, ctx->active_file);
}

/**
 * Callback for Help -> "About Vento".
 *
 * Displays version and license information.
 *
 * @param ctx Editor context for the display window.
 */
void menuAbout(EditorContext *ctx) {
    show_about(ctx);
}

/**
 * Callback for Help -> "Help Screen".
 *
 * Shows the built-in help text.
 *
 * @param ctx Editor context for the display window.
 */
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


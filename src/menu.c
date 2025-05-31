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

/* confirm_quit implemented in vento.c */
bool confirm_quit(void);

Menu *menus = NULL;
int menuCount = 0;
int *menuPositions = NULL;


/**
 * Initializes the menus.
 */
void initializeMenus() {
    // Set the number of menus
    menuCount = 5;

    // Allocate memory for the menus array
    menus = calloc(menuCount, sizeof(Menu));
    if (menus == NULL) {
        freeMenus();
        allocation_failed("initializeMenus");
    }

    menuPositions = calloc(menuCount, sizeof(int));
    if (!menuPositions) {
        freeMenus();
        allocation_failed("initializeMenus");
    }

    // Create the file menu
    MenuItem *fileMenuItems = malloc(7 * sizeof(MenuItem));
    if (fileMenuItems == NULL) {
        freeMenus();
        allocation_failed("initializeMenus");
    }
    fileMenuItems[0] = (MenuItem){"New File", menuNewFile, false};
    fileMenuItems[1] = (MenuItem){"Load File", menuLoadFile, false};
    fileMenuItems[2] = (MenuItem){"Save File", menuSaveFile, false};
    fileMenuItems[3] = (MenuItem){"Save As", menuSaveAs, false};
    fileMenuItems[4] = (MenuItem){"", NULL, true};
    fileMenuItems[5] = (MenuItem){"Close File", menuCloseFile, false};
    fileMenuItems[6] = (MenuItem){"Quit", menuQuitEditor, false};

    // Initialize and assign the file menu
    Menu fileMenu = {"File", fileMenuItems, 7};
    menus[0] = fileMenu;

    // Create the edit menu
    MenuItem *editMenuItems = malloc(5 * sizeof(MenuItem));
    if (editMenuItems == NULL) {
        freeMenus();
        allocation_failed("initializeMenus");
    }
    editMenuItems[0] = (MenuItem){"Undo", menuUndo, false};
    editMenuItems[1] = (MenuItem){"Redo", menuRedo, false};
    editMenuItems[2] = (MenuItem){"", NULL, true};
    editMenuItems[3] = (MenuItem){"Find", menuFind, false};
    editMenuItems[4] = (MenuItem){"Replace", menuReplace, false};

    // Initialize and assign the edit menu
    Menu editMenu = {"Edit", editMenuItems, 5};
    menus[1] = editMenu;

    // Create the navigate menu
    MenuItem *navMenuItems = malloc(2 * sizeof(MenuItem));
    if (navMenuItems == NULL) {
        freeMenus();
        allocation_failed("initializeMenus");
    }
    navMenuItems[0] = (MenuItem){"Next File", menuNextFile, false};
    navMenuItems[1] = (MenuItem){"Previous File", menuPrevFile, false};
    Menu navMenu = {"Navigate", navMenuItems, 2};
    menus[2] = navMenu;

    // Create the options menu
    MenuItem *optMenuItems = malloc(1 * sizeof(MenuItem));
    if (optMenuItems == NULL) {
        freeMenus();
        allocation_failed("initializeMenus");
    }
    optMenuItems[0] = (MenuItem){"Settings", menuSettings, false};
    Menu optMenu = {"Options", optMenuItems, 1};
    menus[3] = optMenu;

    // Create the help menu
    MenuItem *helpMenuItems = malloc(2 * sizeof(MenuItem));
    if (helpMenuItems == NULL) {
        freeMenus();
        allocation_failed("initializeMenus");
    }
    helpMenuItems[0] = (MenuItem){"Help Screen", menuHelp, false};
    helpMenuItems[1] = (MenuItem){"About Vento", menuAbout, false};
    // Initialize and assign the help menu
    Menu helpMenu = {"Help", helpMenuItems, 2};
    menus[4] = helpMenu;

    drawMenuBar(menus, menuCount);
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

    curs_set(0);

    while (inMenu) {
        if (*currentMenu < 0 || *currentMenu >= menuCount) {
            inMenu = false;
            continue;
        }
        // Redraw the editor content
        werase(text_win);
        box(text_win, 0, 0);
        draw_text_buffer(active_file, text_win);
        update_status_bar(active_file);
        wrefresh(text_win);

        // Draw menu bar and compute menu positions
        drawMenuBar(menus, menuCount);

        if (menus[*currentMenu].items == NULL) {
            inMenu = false;
            refresh();
            continue;
        }

        int startX = menuPositions[*currentMenu];
        int startY = 1;
        int boxWidth = 20;
        int boxHeight = menus[*currentMenu].itemCount + 2;
        if (startX + boxWidth > COLS || startY + boxHeight > LINES) {
            inMenu = false;
            refresh();
            continue;
        }
        if (!drawMenu(&menus[*currentMenu], *currentItem, startX, startY)) {
            inMenu = false;
            refresh();
            continue;
        }
        refresh();

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
                            refresh();
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
                                refresh();
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
    curs_set(1);
}


void menuNewFile() {
    new_file(active_file);
}

void menuLoadFile() {
    load_file(active_file, NULL);
}

void menuSaveFile() {
    save_file(active_file);
}

void menuSaveAs() {
    save_file_as(active_file);
}

void menuCloseFile() {
    close_current_file(active_file, &active_file->cursor_x, &active_file->cursor_y);
}

void menuNextFile() {
    next_file(active_file, &active_file->cursor_x, &active_file->cursor_y);
}

void menuPrevFile() {
    prev_file(active_file, &active_file->cursor_x, &active_file->cursor_y);
}

void menuSettings() {
    if (show_settings_dialog(&app_config)) {
        config_save(&app_config);
        config_load(&app_config);
        if (enable_mouse)
            mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
        else
            mousemask(0, NULL);
        apply_colors();
        redraw();
        drawBar();
    }
}

void menuQuitEditor() {
    if (confirm_quit())
        close_editor();
}

void menuUndo() {
    undo(active_file);
}

void menuRedo() {
    redo(active_file);
}

void menuFind() {
    find(active_file, 1);
}

void menuReplace() {
    replace(active_file);
}

void menuAbout() {
    show_about();
}

void menuHelp() {
    show_help();
}


/**
 * Frees the memory allocated for all menus and their menu items.
 */
void freeMenus() {
    if (menus == NULL) {
        return;
    }

    for (int i = 0; i < menuCount; ++i) {
        if (menus[i].items != NULL) {
            free(menus[i].items);
            menus[i].items = NULL;  // Avoid dangling pointer
        }
    }

    free(menus);
    menus = NULL;
    free(menuPositions);
    menuPositions = NULL;
    menuCount = 0;
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


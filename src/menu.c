#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include "menu.h"
#include "editor.h"
#include "search.h"
#include "file_ops.h"
#include "ui.h"
#include "undo.h"

Menu *menus = NULL;
int menuCount = 0;

void drawMenuBar(Menu *menus, int menuCount) {
    // Clear the top row
    move(0, 0);
    clrtoeol();

    // Draw the menu bar
    for (int i = 0; i < menuCount; ++i) {
        mvprintw(0, i * 10, "%s", menus[i].label);
    }
    refresh();
}

void drawBar() {
    drawMenuBar(menus, menuCount);
}

/**
 * Initializes the menus.
 */
void initializeMenus() {
    // Set the number of menus
    menuCount = 3;

    // Allocate memory for the menus array
    menus = calloc(menuCount, sizeof(Menu));
    if (menus == NULL) {
        fprintf(stderr, "Failed to allocate memory for menus\n");
        exit(EXIT_FAILURE);
    }

    // Create the file menu
    MenuItem *fileMenuItems = malloc(7 * sizeof(MenuItem));
    if (fileMenuItems == NULL) {
        fprintf(stderr, "Failed to allocate memory for file menu items\n");
        freeMenus();
        exit(EXIT_FAILURE);
    }
    fileMenuItems[0] = (MenuItem){"New File", menuNewFile};
    fileMenuItems[1] = (MenuItem){"Load File", menuLoadFile};
    fileMenuItems[2] = (MenuItem){"Save File", menuSaveFile};
    fileMenuItems[3] = (MenuItem){"Close File", menuCloseFile};
    fileMenuItems[4] = (MenuItem){"Next File", menuNextFile};
    fileMenuItems[5] = (MenuItem){"Previous File", menuPrevFile};
    fileMenuItems[6] = (MenuItem){"Quit", menuQuitEditor};

    // Initialize and assign the file menu
    Menu fileMenu = {"File", fileMenuItems, 7};
    menus[0] = fileMenu;

    // Create the edit menu
    MenuItem *editMenuItems = malloc(3 * sizeof(MenuItem));
    if (editMenuItems == NULL) {
        fprintf(stderr, "Failed to allocate memory for edit menu items\n");
        freeMenus();
        exit(EXIT_FAILURE);
    }
    editMenuItems[0] = (MenuItem){"Undo", menuUndo};
    editMenuItems[1] = (MenuItem){"Redo", menuRedo};
    editMenuItems[2] = (MenuItem){"Find", menuFind};

    // Initialize and assign the edit menu
    Menu editMenu = {"Edit", editMenuItems, 3};
    menus[1] = editMenu;

    // Create the help menu
    MenuItem *helpMenuItems = malloc(3 * sizeof(MenuItem));
    if (helpMenuItems == NULL) {
        fprintf(stderr, "Failed to allocate memory for help menu items\n");
        freeMenus();
        exit(EXIT_FAILURE);
    }
    helpMenuItems[0] = (MenuItem){"About Vento", menuAbout};
    helpMenuItems[1] = (MenuItem){"Help Screen", menuHelp};
    helpMenuItems[2] = (MenuItem){"Test Window", menuTestwindow};

    // Initialize and assign the help menu
    Menu helpMenu = {"Help", helpMenuItems, 3};
    menus[2] = helpMenu;

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

    while (inMenu) {
        // Redraw the editor content
        werase(text_win);
        box(text_win, 0, 0);
        draw_text_buffer(active_file, text_win);
        update_status_bar(active_file);
        wrefresh(text_win);

        // Draw menu bar
        for (int i = 0; i < menuCount; ++i) {
            mvprintw(0, i * 10, "%s", menus[i].label);
        }
        drawMenu(&menus[*currentMenu], *currentItem, *currentMenu * 10, 1);
        refresh();

        ch = getch();
        switch (ch) {
            case KEY_LEFT:
                if (*currentMenu > 0) (*currentMenu)--; // Move to the previous menu
                *currentItem = 0; // Reset the current item index
                break;
            case KEY_RIGHT:
                if (*currentMenu < menuCount - 1) (*currentMenu)++; // Move to the next menu
                *currentItem = 0; // Reset the current item index
                break;
            case KEY_UP:
                if (*currentItem > 0) (*currentItem)--; // Move to the previous item in the current menu
                break;
            case KEY_DOWN:
                if (*currentItem < menus[*currentMenu].itemCount - 1) (*currentItem)++; // Move to the next item in the current menu
                break;
            case '\n': // Enter key
                menus[*currentMenu].items[*currentItem].action(); // Execute the action associated with the selected menu item
                inMenu = false; // Exit the menu loop
                break;
            case 27: // ESC key
                inMenu = false; // Exit the menu loop
                break;
            default:
                break;
        }
    }
}

/**
 * Draws the menu on the screen.
 * 
 * @param menu The menu to be drawn.
 * @param currentItem The index of the currently selected item in the menu.
 * @param startX The starting x-coordinate of the menu window.
 * @param startY The starting y-coordinate of the menu window.
 */
void drawMenu(Menu *menu, int currentItem, int startX, int startY) {
    int boxWidth = 20;
    int boxHeight = menu->itemCount + 2;

    // Create a new window for the menu
    WINDOW *menuWin = newwin(boxHeight, boxWidth, startY, startX);
    box(menuWin, 0, 0);

    // Draw each menu item
    for (int i = 0; i < menu->itemCount; ++i) {
        if (i == currentItem) {
            wattron(menuWin, A_REVERSE); // Highlight the currently selected item
        }
        mvwprintw(menuWin, 1 + i, 1, "%s", menu->items[i].label); // Print the label of the menu item
        if (i == currentItem) {
            wattroff(menuWin, A_REVERSE); // Turn off the highlight for the currently selected item
        }
    }

    wrefresh(menuWin); // Refresh the menu window
    delwin(menuWin); // Delete the menu window
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

void menuCloseFile() {
    close_current_file(active_file, &active_file->cursor_x, &active_file->cursor_y);
}

void menuNextFile() {
    next_file(active_file, &active_file->cursor_x, &active_file->cursor_y);
}

void menuPrevFile() {
    prev_file(active_file, &active_file->cursor_x, &active_file->cursor_y);
}

void menuQuitEditor() {
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

void menuAbout() {
    show_about();
}

void menuHelp() {
    show_help();
}

void menuTestwindow() {
    char selected_path[100] = "";
    int result = show_open_file_dialog(selected_path, sizeof(selected_path));
    if (result) {
        mvprintw(LINES - 2, 2, "Selected: %s", selected_path);
        refresh();
        getch();
        mvprintw(LINES - 2, 2, "                             ");
        refresh();
    }
    redraw();
    drawBar();
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
    menuCount = 0;
}


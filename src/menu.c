#include <ncurses.h>
#include <stdlib.h>
#include "menu.h"
#include "editor.h"
#include "ui.h"

Menu *menus = NULL;
int menuCount = 0;

void drawMenuBar(Menu *menus, int menuCount) {
    // Clear the top row
    move(0, 0);
    clrtoeol();

    // Draw the menu bar
    for (int i = 0; i < menuCount; ++i) {
        mvprintw(0, i * 10, menus[i].label);
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
    menus = malloc(menuCount * sizeof(Menu));

    // Create the file menu
    MenuItem *fileMenuItems = malloc(4 * sizeof(MenuItem));
    fileMenuItems[0] = (MenuItem){"New File", menuNewFile};
    fileMenuItems[1] = (MenuItem){"Load File", menuLoadFile};
    fileMenuItems[2] = (MenuItem){"Save File", menuSaveFile};
    fileMenuItems[3] = (MenuItem){"Quit", menuQuitEditor};

    // Initialize the file menu
    Menu fileMenu = {"File", fileMenuItems, 4};

    // Create the edit menu
    MenuItem *editMenuItems = malloc(3 * sizeof(MenuItem));
    editMenuItems[0] = (MenuItem){"Undo", menuUndo};
    editMenuItems[1] = (MenuItem){"Redo", menuRedo};
    editMenuItems[2] = (MenuItem){"Find", menuFind};

    // Initialize the edit menu
    Menu editMenu = {"Edit", editMenuItems, 3};

    // Create the help menu
    MenuItem *helpMenuItems = malloc(3 * sizeof(MenuItem));
    helpMenuItems[0] = (MenuItem){"About Vento", menuAbout};
    helpMenuItems[1] = (MenuItem){"Help Screen", menuHelp};
    helpMenuItems[2] = (MenuItem){"Test Window", menuTestwindow};

    // Initialize the help menu
    Menu helpMenu = {"Help", helpMenuItems, 3};

    // Assign the menus to the menus array
    menus[0] = fileMenu;
    menus[1] = editMenu;
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
        draw_text_buffer(text_win);
        update_status_bar(1, 1);
        wrefresh(text_win);

        // Draw menu bar
        for (int i = 0; i < menuCount; ++i) {
            mvprintw(0, i * 10, menus[i].label);
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
        mvwprintw(menuWin, 1 + i, 1, menu->items[i].label); // Print the label of the menu item
        if (i == currentItem) {
            wattroff(menuWin, A_REVERSE); // Turn off the highlight for the currently selected item
        }
    }

    wrefresh(menuWin); // Refresh the menu window
    delwin(menuWin); // Delete the menu window
}

void menuNewFile() {
    new_file();
}

void menuLoadFile() {
    load_file(NULL);
}

void menuSaveFile() {
    save_file();
}

void menuQuitEditor() {
    close_editor();
}

void menuUndo() {
    undo();
}

void menuRedo() {
    redo();
}

void menuFind() {
    find(1);
}

void menuAbout() {
    show_about();
}

void menuHelp() {
    show_help();
}

void menuTestwindow() {
    show_select_file("/home/", 100);
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


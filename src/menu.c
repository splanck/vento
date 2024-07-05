#include <ncurses.h>
#include <stdlib.h>
#include "menu.h"
#include "editor.h"

Menu *menus = NULL;
int menuCount = 0;

void initializeMenus() {
    menuCount = 2;
    menus = malloc(menuCount * sizeof(Menu));

    MenuItem *fileMenuItems = malloc(4 * sizeof(MenuItem));
    fileMenuItems[0] = (MenuItem){"New File", newFile};
    fileMenuItems[1] = (MenuItem){"Load File", loadFile};
    fileMenuItems[2] = (MenuItem){"Save File", saveFile};
    fileMenuItems[3] = (MenuItem){"Quit", quitEditor};

    Menu fileMenu = {"File", fileMenuItems, 4};

    MenuItem *editMenuItems = malloc(2 * sizeof(MenuItem));
    editMenuItems[0] = (MenuItem){"Undo", undo};
    editMenuItems[1] = (MenuItem){"Redo", redo};

    Menu editMenu = {"Edit", editMenuItems, 2};

    menus[0] = fileMenu;
    menus[1] = editMenu;
}

void handleMenuNavigation(Menu *menus, int menuCount, int *currentMenu, int *currentItem) {
    int ch;
    bool inMenu = true;

    while (inMenu) {
        clear();
        for (int i = 0; i < menuCount; ++i) {
            mvprintw(0, i * 10, menus[i].label);
        }
        drawMenu(&menus[*currentMenu], *currentItem);
        refresh();

        ch = getch();
        switch (ch) {
            case KEY_LEFT:
                if (*currentMenu > 0) (*currentMenu)--;
                break;
            case KEY_RIGHT:
                if (*currentMenu < menuCount - 1) (*currentMenu)++;
                break;
            case KEY_UP:
                if (*currentItem > 0) (*currentItem)--;
                break;
            case KEY_DOWN:
                if (*currentItem < menus[*currentMenu].itemCount - 1) (*currentItem)++;
                break;
            case '\n': // Enter key
                menus[*currentMenu].items[*currentItem].action();
                inMenu = false;
                break;
            case 27: // ESC key
                inMenu = false;
                break;
            default:
                break;
        }
    }
}

void drawMenu(Menu *menu, int currentItem) {
    for (int i = 0; i < menu->itemCount; ++i) {
        if (i == currentItem) {
            attron(A_REVERSE);
        }
        mvprintw(1 + i, 0, menu->items[i].label);
        if (i == currentItem) {
            attroff(A_REVERSE);
        }
    }
}

void newFile() {
    // Implementation of newFile
}

void loadFile() {
    // Implementation of loadFile
}

void saveFile() {
    // Implementation of saveFile
}

void quitEditor() {
    // Implementation of quitEditor
}

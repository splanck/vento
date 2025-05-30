#include <ncurses.h>
#include <stdio.h>
#include "menu.h"

/* Draws the top menu bar using the global menu list */
void drawMenuBar(Menu *menus, int menuCount) {
    move(0, 0);
    clrtoeol();
    for (int i = 0; i < menuCount; ++i) {
        mvprintw(0, i * 10, "%s", menus[i].label);
    }
    refresh();
}

/* Convenience wrapper used by various modules */
void drawBar(void) {
    drawMenuBar(menus, menuCount);
}

/* Draws the drop-down menu window */
bool drawMenu(Menu *menu, int currentItem, int startX, int startY) {
    int boxWidth = 20;
    int boxHeight = menu->itemCount + 2;

    if (startX < 0 || startY < 0 || startX + boxWidth > COLS ||
        startY + boxHeight > LINES) {
        return false;
    }

    WINDOW *menuWin = newwin(boxHeight, boxWidth, startY, startX);
    if (menuWin == NULL) {
        fprintf(stderr, "Failed to create menu window\n");
        return false;
    }
    box(menuWin, 0, 0);

    for (int i = 0; i < menu->itemCount; ++i) {
        if (i == currentItem) {
            wattron(menuWin, A_REVERSE);
        }
        mvwprintw(menuWin, 1 + i, 1, "%s", menu->items[i].label);
        if (i == currentItem) {
            wattroff(menuWin, A_REVERSE);
        }
    }

    wrefresh(menuWin);
    delwin(menuWin);
    return true;
}

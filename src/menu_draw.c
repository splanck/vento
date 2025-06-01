#include <ncurses.h>
#include <stdio.h>
#include <string.h>
#include "menu.h"
#include "config.h"
#include "syntax.h"
#include "editor_state.h"

/* Draws the top menu bar using the global menu list */
void drawMenuBar(Menu *menus, int menuCount) {
    move(0, 0);
    clrtoeol();

    wbkgdset(stdscr, enable_color ? COLOR_PAIR(SYNTAX_BG) : A_NORMAL);
    if (enable_color)
        wattron(stdscr, COLOR_PAIR(SYNTAX_KEYWORD));

    int pos = 0;
    for (int i = 0; i < menuCount; ++i) {
        if (menuPositions)
            menuPositions[i] = pos;
        mvprintw(0, pos, "%s", menus[i].label);
        pos += (int)strlen(menus[i].label) + 2; // add padding
    }

    if (enable_color)
        wattroff(stdscr, COLOR_PAIR(SYNTAX_KEYWORD));

    wnoutrefresh(stdscr);
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
        MenuItem *item = &menu->items[i];
        if (item->separator) {
            mvwhline(menuWin, 1 + i, 1, ACS_HLINE, boxWidth - 2);
            continue;
        }
        if (i == currentItem) {
            wattron(menuWin, A_REVERSE);
        }
        mvwprintw(menuWin, 1 + i, 1, "%s", item->label);
        if (i == currentItem) {
            wattroff(menuWin, A_REVERSE);
        }
    }

    wrefresh(menuWin);
    delwin(menuWin);
    return true;
}

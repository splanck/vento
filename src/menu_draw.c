/*
 * menu_draw.c
 * -----------
 * Rendering helpers for the menu system.  The routines here draw the
 * persistent menu bar across the top of the screen as well as the
 * temporary drop‑down menus used during navigation.
 */

#include <ncurses.h>
#include <stdio.h>
#include <string.h>
#include "menu.h"
#include "config.h"
#include "syntax.h"
#include "editor_state.h"

int calcMenuWidth(Menu *menu) {
    int longest = 0;
    for (int i = 0; i < menu->itemCount; ++i) {
        MenuItem *item = &menu->items[i];
        if (item->separator)
            continue;
        int len = (int)strlen(item->label);
        if (item->shortcut)
            len += (int)strlen(item->shortcut);
        if (len > longest)
            longest = len;
    }
    return longest + 3; // account for padding and borders
}

/*
 * Draw the top menu bar.
 *
 * menus     - array of Menu structures to display
 * menuCount - number of elements in the array
 *
 * The starting column of each label is stored into the global
 * `menuPositions` array so other modules know where to position
 * drop-downs and how to perform mouse hit testing.
 */
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

/*
 * Convenience wrapper used by various modules to redraw the menu bar
 * using the globally initialized menu list.
 */
void drawBar(void) {
    drawMenuBar(menus, menuCount);
}

/*
 * Display a drop-down menu window.
 *
 * menu        - Menu to display
 * currentItem - index of the item to highlight
 * startX,Y    - screen coordinates for the pop-up window
 *
 * Returns true on success.  The coordinates are typically derived
 * from the global `menuPositions` array populated by drawMenuBar().
 */
bool drawMenu(Menu *menu, int currentItem, int startX, int startY) {
    int boxWidth = calcMenuWidth(menu);
    int boxHeight = menu->itemCount + 2;

    if (startX + boxWidth > COLS)
        startX = COLS - boxWidth;
    if (startX < 0)
        startX = 0;
    if (startY + boxHeight > LINES)
        startY = LINES - boxHeight;
    if (startY < 0)
        startY = 0;

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
        if (item->shortcut) {
            int col = boxWidth - (int)strlen(item->shortcut) - 2;
            if (col > 1)
                mvwprintw(menuWin, 1 + i, col, "%s", item->shortcut);
        }
        if (i == currentItem) {
            wattroff(menuWin, A_REVERSE);
        }
    }

    wrefresh(menuWin);
    delwin(menuWin);
    return true;
}

#ifndef MENU_H
#define MENU_H
#include <stdbool.h>
typedef struct MenuItem {
    const char *label;
    void (*action)(void);
    bool separator;
} MenuItem;

typedef struct Menu {
    const char *label;
    MenuItem *items;
    int itemCount;
} Menu;

void initializeMenus(void);
void handleMenuNavigation(Menu *menus, int menuCount, int *currentMenu, int *currentItem);
bool drawMenu(Menu *menu, int currentItem, int startX, int startY);
void drawMenuBar(Menu *menus, int menuCount);
void menuNewFile(void);
void menuLoadFile(void);
void menuSaveFile(void);
void menuSaveAs(void);
void menuCloseFile(void);
void menuNextFile(void);
void menuPrevFile(void);
void menuSettings(void);
void menuQuitEditor(void);
void menuUndo(void);
void menuRedo(void);
void menuFind(void);
void menuReplace(void);
void menuAbout(void);
void menuHelp(void);
void drawBar(void);
/**
 * Frees the memory allocated for the menus and menu items.
 */
void freeMenus(void);

int menu_click_open(int x, int y);

extern Menu *menus;
extern int menuCount;
extern int *menuPositions;

#endif // MENU_H

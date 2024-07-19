#ifndef MENU_H
#define MENU_H


typedef struct MenuItem {
    const char *label;
    void (*action)();
} MenuItem;

typedef struct Menu {
    const char *label;
    MenuItem *items;
    int itemCount;
} Menu;

void initializeMenus();
void handleMenuNavigation(Menu *menus, int menuCount, int *currentMenu, int *currentItem);
void drawMenu(Menu *menu, int currentItem, int startX, int startY);
void newFile();
void loadFile();
void saveFile();
void quitEditor();

extern Menu *menus;
extern int menuCount;

#endif // MENU_H

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
void menuNewFile();
void menuLoadFile();
void menuSaveFile();
void menuQuitEditor();
void menuUndo();
void menuRedo();
void menuAbout();
void menuHelp();
void menuTestwindow();

extern Menu *menus;
extern int menuCount;

#endif // MENU_H

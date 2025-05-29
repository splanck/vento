#ifndef MENU_H
#define MENU_H
typedef struct MenuItem {
    const char *label;
    void (*action)(void);
} MenuItem;

typedef struct Menu {
    const char *label;
    MenuItem *items;
    int itemCount;
} Menu;

void initializeMenus(void);
void handleMenuNavigation(Menu *menus, int menuCount, int *currentMenu, int *currentItem);
void drawMenu(Menu *menu, int currentItem, int startX, int startY);
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
void menuAbout(void);
void menuHelp(void);
void menuTestwindow(void);
void drawBar(void);
/**
 * Frees the memory allocated for the menus and menu items.
 */
void freeMenus(void);

extern Menu *menus;
extern int menuCount;

#endif // MENU_H

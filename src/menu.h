#ifndef MENU_H
#define MENU_H
#include <stdbool.h>
#include "editor.h"
#include "editor_state.h"
typedef struct MenuItem {
    const char *label;
    const char *shortcut;
    void (*action)(void);
    bool separator;
} MenuItem;

typedef struct Menu {
    const char *label;
    MenuItem *items;
    int itemCount;
} Menu;

void initializeMenus(EditorContext *ctx);
void handleMenuNavigation(Menu *menus, int menuCount, int *currentMenu, int *currentItem);
bool drawMenu(Menu *menu, int currentItem, int startX, int startY);
void drawMenuBar(Menu *menus, int menuCount);
int calcMenuWidth(Menu *menu);
void menuNewFile(EditorContext *ctx);
void menuLoadFile(EditorContext *ctx);
void menuSaveFile(EditorContext *ctx);
void menuSaveAs(EditorContext *ctx);
void menuCloseFile(EditorContext *ctx);
void menuNextFile(EditorContext *ctx);
void menuPrevFile(EditorContext *ctx);
void menuSettings(EditorContext *ctx);
void menuQuitEditor(EditorContext *ctx);
void menuUndo(EditorContext *ctx);
void menuRedo(EditorContext *ctx);
void menuFind(EditorContext *ctx);
void menuReplace(EditorContext *ctx);
void menuAbout(EditorContext *ctx);
void menuHelp(EditorContext *ctx);
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

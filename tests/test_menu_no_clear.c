#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ncurses.h>
#include "menu.h"
#include "config.h"
#include "files.h"

#undef move
#undef clrtoeol
#undef mvprintw
#undef wattron
#undef wattroff
#undef wnoutrefresh
#undef wrefresh
#undef getch
#undef newwin
#undef box
#undef mvwprintw
#undef mvwhline
#undef delwin
#undef bkgd
#undef wbkgd
#undef refresh
#undef curs_set

/* simple WINDOW stub */
typedef struct { int dummy; } SIMPLE_WIN;
static SIMPLE_WIN dummy_win;
WINDOW *stdscr = (WINDOW*)&dummy_win;
WINDOW *text_win = (WINDOW*)&dummy_win;

int COLS = 80;
int LINES = 24;
int enable_color = 0;
int enable_mouse = 0;
AppConfig app_config;
FileState *active_file = NULL;

/* counters for unwanted calls */
static int bkgd_called = 0;
static int wbkgd_called = 0;
static int refresh_called = 0;

int move(int y,int x){(void)y;(void)x;return 0;}
int clrtoeol(void){return 0;}
int mvprintw(int y,int x,const char*fmt,...){(void)y;(void)x;(void)fmt;return 0;}
int wattron(WINDOW*w,int a){(void)w;(void)a;return 0;}
int wattroff(WINDOW*w,int a){(void)w;(void)a;return 0;}
int wnoutrefresh(WINDOW*w){(void)w;return 0;}
int wrefresh(WINDOW*w){(void)w;return 0;}
int getch(void){return 27;} /* ESC */
WINDOW* newwin(int nlines,int ncols,int y,int x){(void)nlines;(void)ncols;(void)y;(void)x;return (WINDOW*)&dummy_win;}
int box(WINDOW*w,chtype a,chtype b){(void)w;(void)a;(void)b;return 0;}
int mvwprintw(WINDOW*w,int y,int x,const char*fmt,...){(void)w;(void)y;(void)x;(void)fmt;return 0;}
int mvwhline(WINDOW*w,int y,int x,chtype ch,int n){(void)w;(void)y;(void)x;(void)ch;(void)n;return 0;}
int delwin(WINDOW*w){(void)w;return 0;}
int bkgd(chtype ch){(void)ch;bkgd_called++;return 0;}
int wbkgd(WINDOW*w,chtype ch){(void)w;(void)ch;wbkgd_called++;return 0;}
int refresh(void){refresh_called++;return 0;}
int curs_set(int c){(void)c;return 0;}

/* stubs for external editor functions referenced in menu.c */
void new_file(FileState*fs){(void)fs;}
void load_file(FileState*fs,const char*fn){(void)fs;(void)fn;}
void save_file(FileState*fs){(void)fs;}
void save_file_as(FileState*fs){(void)fs;}
void close_current_file(FileState*fs,int*cx,int*cy){(void)fs;(void)cx;(void)cy;}
void next_file(FileState*fs,int*cx,int*cy){(void)fs;(void)cx;(void)cy;}
void prev_file(FileState*fs,int*cx,int*cy){(void)fs;(void)cx;(void)cy;}
int show_settings_dialog(AppConfig*cfg){(void)cfg;return 0;}
void config_save(const AppConfig*cfg){(void)cfg;}
void config_load(AppConfig*cfg){(void)cfg;}
mmask_t mousemask(mmask_t newmask, mmask_t *old){(void)newmask;if(old)*old=0;return 0;}
void apply_colors(void){}
void redraw(void){}
bool confirm_quit(void){return false;}
void close_editor(void){}
void undo(FileState*fs){(void)fs;}
void redo(FileState*fs){(void)fs;}
void find(FileState*fs,int n){(void)fs;(void)n;}
void replace(FileState*fs){(void)fs;}
void show_about(void){}
void show_help(void){}
void allocation_failed(const char*msg){(void)msg;abort();}

#include "../src/menu_draw.c"
#include "../src/menu.c"

int main(void){
    MenuItem items[1] = {{"Item", NULL, false}};
    Menu menu = {"File", items, 1};
    int currentMenu = 0;
    int currentItem = 0;
    int positions[1] = {0};
    menuPositions = positions;

    handleMenuNavigation(&menu, 1, &currentMenu, &currentItem);

    assert(bkgd_called == 0);
    assert(wbkgd_called == 0);
    assert(refresh_called == 0);
    return 0;
}

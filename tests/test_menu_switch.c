#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <ncurses.h>
#include "menu.h"
#include "config.h"
#include "files.h"
#include "editor_state.h"

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
#undef touchwin

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

static int touch_calls = 0;
static int wnout_calls = 0;

int move(int y,int x){(void)y;(void)x;return 0;}
int clrtoeol(void){return 0;}
int mvprintw(int y,int x,const char*fmt,...){(void)y;(void)x;(void)fmt;return 0;}
int wattron(WINDOW*w,int a){(void)w;(void)a;return 0;}
int wattroff(WINDOW*w,int a){(void)w;(void)a;return 0;}
int wnoutrefresh(WINDOW*w){if(w==text_win)wnout_calls++;return 0;}
int wrefresh(WINDOW*w){(void)w;return 0;}
int getch(void){static int c=0;return c++==0?KEY_RIGHT:27;}
WINDOW* newwin(int nlines,int ncols,int y,int x){(void)nlines;(void)ncols;(void)y;(void)x;return (WINDOW*)&dummy_win;}
int box(WINDOW*w,chtype a,chtype b){(void)w;(void)a;(void)b;return 0;}
int mvwprintw(WINDOW*w,int y,int x,const char*fmt,...){(void)w;(void)y;(void)x;(void)fmt;return 0;}
int mvwhline(WINDOW*w,int y,int x,chtype ch,int n){(void)w;(void)y;(void)x;(void)ch;(void)n;return 0;}
int delwin(WINDOW*w){(void)w;return 0;}
int bkgd(chtype ch){(void)ch;return 0;}
int wbkgd(WINDOW*w,chtype ch){(void)w;(void)ch;return 0;}
int refresh(void){return 0;}
int curs_set(int c){(void)c;return 0;}
int touchwin(WINDOW*w){if(w==text_win)touch_calls++;return 0;}

/* stubs for external editor functions referenced in menu.c */
void new_file(FileState*fs){(void)fs;}
void load_file(EditorContext*ctx,FileState*fs,const char*fn){(void)ctx;(void)fs;(void)fn;}
void save_file(EditorContext*ctx,FileState*fs){(void)ctx;(void)fs;}
void save_file_as(EditorContext*ctx,FileState*fs){(void)ctx;(void)fs;}
void close_current_file(EditorContext*ctx,FileState*fs,int*cx,int*cy){(void)ctx;(void)fs;(void)cx;(void)cy;}
void next_file(EditorContext *ctx, FileState*fs,int*cx,int*cy){(void)fs;(void)cx;(void)cy;}
void prev_file(EditorContext *ctx, FileState*fs,int*cx,int*cy){(void)fs;(void)cx;(void)cy;}
int show_settings_dialog(EditorContext*ctx,AppConfig*cfg){(void)ctx;(void)cfg;return 0;}
void config_save(const AppConfig*cfg){(void)cfg;}
void config_load(AppConfig*cfg){(void)cfg;}
mmask_t mousemask(mmask_t newmask, mmask_t *old){(void)newmask;if(old)*old=0;return 0;}
void apply_colors(void){}
void redraw(void){}
bool confirm_quit(void){return false;}
void close_editor(void){}
void undo(FileState*fs){(void)fs;}
void redo(FileState*fs){(void)fs;}
void find(EditorContext*ctx,FileState*fs,int n){(void)ctx;(void)fs;(void)n;}
void replace(EditorContext*ctx,FileState*fs){(void)ctx;(void)fs;}
void show_about(EditorContext*ctx){(void)ctx;}
void show_help(EditorContext*ctx){(void)ctx;}
void allocation_failed(const char*msg){(void)msg;abort();}

#include "../src/menu_draw.c"
#include "../src/menu.c"

int main(void){
    MenuItem items1[1] = {{"One", NULL, false}};
    MenuItem items2[1] = {{"Two", NULL, false}};
    Menu menus_arr[2] = {{"A", items1, 1}, {"B", items2, 1}};
    int currentMenu = 0;
    int currentItem = 0;
    int positions[2] = {0,4};
    menuPositions = positions;

    handleMenuNavigation(menus_arr, 2, &currentMenu, &currentItem);

    assert(touch_calls >= 2);
    assert(wnout_calls >= touch_calls);
    return 0;
}

#include <assert.h>
#include <stdbool.h>
#include <ncurses.h>
#include "ui.h"
#include "dialog.h"
#include "files.h"
#include "config.h"
#include "syntax.h"
#include "editor_state.h"

#undef curs_set
#undef newwin
#undef delwin
#undef keypad
#undef wbkgd
#undef wrefresh
#undef box
#undef wattron
#undef mvwprintw
#undef wattroff
#undef wattr_on
#undef wattr_off
#undef wmove
#undef wgetch
#undef wclear
#undef werase
#undef wborder
#undef getbegyx
#undef getmaxyx

/* minimal WINDOW stub */
typedef struct { int dummy; } SIMPLE_WIN;
static SIMPLE_WIN dummy_win;
WINDOW *stdscr = (WINDOW*)&dummy_win;
int COLS = 80;
int LINES = 24;
int enable_color = 0; /* disable color globally */
int enable_mouse = 0;

int wbkgd_attr_last = -2;
int wattron_color_calls = 0;
int wattroff_color_calls = 0;

int curs_set(int c){ (void)c; return 0; }
WINDOW *newwin(int nlines,int ncols,int y,int x){ (void)nlines;(void)ncols;(void)y;(void)x; return (WINDOW*)&dummy_win; }
int delwin(WINDOW*w){ (void)w; return 0; }
int keypad(WINDOW*w, bool b){ (void)w; (void)b; return 0; }
int wbkgd(WINDOW*w, chtype ch){ (void)w; wbkgd_attr_last = ch; return 0; }
int wrefresh(WINDOW*w){ (void)w; return 0; }
int box(WINDOW*w, chtype v, chtype h){ (void)w; (void)v; (void)h; return 0; }
int wattron(WINDOW*w, int a){
    (void)w;
    if(a == COLOR_PAIR(SYNTAX_KEYWORD))
        wattron_color_calls++;
    return 0;
}
int wattroff(WINDOW*w, int a){
    (void)w;
    if(a == COLOR_PAIR(SYNTAX_KEYWORD))
        wattroff_color_calls++;
    return 0;
}
int wattr_on(WINDOW*w, attr_t a, void*opts){
    (void)w; (void)opts;
    if(a == COLOR_PAIR(SYNTAX_KEYWORD))
        wattron_color_calls++;
    return 0;
}
int wattr_off(WINDOW*w, attr_t a, void*opts){
    (void)w; (void)opts;
    if(a == COLOR_PAIR(SYNTAX_KEYWORD))
        wattroff_color_calls++;
    return 0;
}
int mvwprintw(WINDOW*w,int y,int x,const char*fmt,...){ (void)w;(void)y;(void)x;(void)fmt; return 0; }
int wmove(WINDOW*w,int y,int x){ (void)w;(void)y;(void)x; return 0; }
int wgetch(WINDOW*w){ (void)w; return '\n'; }
int wclear(WINDOW*w){ (void)w; return 0; }
int werase(WINDOW*w){ (void)w; return 0; }
int wborder(WINDOW*w,chtype ls,chtype rs,chtype ts,chtype bs,chtype tl,chtype tr,chtype bl,chtype br){(void)w;(void)ls;(void)rs;(void)ts;(void)bs;(void)tl;(void)tr;(void)bl;(void)br; return 0; }
#define getbegyx(win,y,x) do{ (void)(win); y=0; x=0; }while(0)
#define getmaxyx(win,y,x) do{ (void)(win); y=LINES; x=COLS; }while(0)
int wnoutrefresh(WINDOW*w){(void)w;return 0;}
int doupdate(void){return 0;}

/* stubs for other functions */
void draw_text_buffer(FileState*fs, WINDOW*w){ (void)fs; (void)w; }
void update_status_bar(EditorContext *ctx, FileState*fs){ (void)fs; }
int show_message(const char*msg){ (void)msg; return 0; }
void drawBar(void){}

int main(void){
    /* color disabled */
    enable_color = 0;
    wattron_color_calls = 0;
    wattroff_color_calls = 0;
    printf("dialog_open\n");
    WINDOW *win = dialog_open(7, 20, "Test");
    dialog_close(win);
    assert(wbkgd_attr_last == A_NORMAL);
    assert(wattron_color_calls == 0);
    assert(wattroff_color_calls == 0);

    EditorContext ctx = {0};
    printf("show_help\n");
    ctx.enable_color = enable_color;
    show_help(&ctx);
    assert(wbkgd_attr_last == A_NORMAL);

    printf("show_about\n");
    ctx.enable_color = enable_color;
    show_about(&ctx);
    assert(wbkgd_attr_last == A_NORMAL);

    printf("show_warning\n");
    ctx.enable_color = enable_color;
    show_warning_dialog(&ctx);
    assert(wbkgd_attr_last == A_NORMAL);
    
    /* color enabled */
    enable_color = 1;
    wattron_color_calls = 0;
    wattroff_color_calls = 0;
    printf("dialog_open color\n");
    win = dialog_open(7, 20, "Test");
    dialog_close(win);
    assert(wbkgd_attr_last == COLOR_PAIR(SYNTAX_BG));
    assert(wattron_color_calls == 0);
    assert(wattroff_color_calls == 0);

    printf("show_help color\n");
    ctx.enable_color = enable_color;
    show_help(&ctx);
    assert(wbkgd_attr_last == COLOR_PAIR(SYNTAX_BG));

    printf("show_about color\n");
    ctx.enable_color = enable_color;
    show_about(&ctx);
    assert(wbkgd_attr_last == COLOR_PAIR(SYNTAX_BG));

    printf("show_warning color\n");
    ctx.enable_color = enable_color;
    show_warning_dialog(&ctx);
    assert(wbkgd_attr_last == COLOR_PAIR(SYNTAX_BG));

    return 0;
}

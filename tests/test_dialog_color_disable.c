#include <assert.h>
#include <stdbool.h>
#include <ncurses.h>
#include "ui.h"
#include "files.h"
#include "config.h"
#include "syntax.h"

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
#undef wmove
#undef wgetch
#undef wclear
#undef werase
#undef wborder

/* minimal WINDOW stub */
typedef struct { int dummy; } SIMPLE_WIN;
static SIMPLE_WIN dummy_win;
WINDOW *stdscr = (WINDOW*)&dummy_win;
WINDOW *text_win = (WINDOW*)&dummy_win;
FileState *active_file = NULL;
int COLS = 80;
int LINES = 24;
int enable_color = 0; /* disable color globally */

int wbkgd_attr_last = -2;

int curs_set(int c){ (void)c; return 0; }
WINDOW *newwin(int nlines,int ncols,int y,int x){ (void)nlines;(void)ncols;(void)y;(void)x; return (WINDOW*)&dummy_win; }
int delwin(WINDOW*w){ (void)w; return 0; }
int keypad(WINDOW*w, bool b){ (void)w; (void)b; return 0; }
int wbkgd(WINDOW*w, chtype ch){ (void)w; wbkgd_attr_last = ch; return 0; }
int wrefresh(WINDOW*w){ (void)w; return 0; }
int box(WINDOW*w, chtype v, chtype h){ (void)w; (void)v; (void)h; return 0; }
int wattron(WINDOW*w, int a){ (void)w; (void)a; return 0; }
int wattroff(WINDOW*w, int a){ (void)w; (void)a; return 0; }
int mvwprintw(WINDOW*w,int y,int x,const char*fmt,...){ (void)w;(void)y;(void)x;(void)fmt; return 0; }
int wmove(WINDOW*w,int y,int x){ (void)w;(void)y;(void)x; return 0; }
int wgetch(WINDOW*w){ (void)w; return '\n'; }
int wclear(WINDOW*w){ (void)w; return 0; }
int werase(WINDOW*w){ (void)w; return 0; }
int wborder(WINDOW*w,chtype ls,chtype rs,chtype ts,chtype bs,chtype tl,chtype tr,chtype bl,chtype br){(void)w;(void)ls;(void)rs;(void)ts;(void)bs;(void)tl;(void)tr;(void)bl;(void)br; return 0; }

/* stubs for other functions */
void draw_text_buffer(FileState*fs, WINDOW*w){ (void)fs; (void)w; }
void update_status_bar(FileState*fs){ (void)fs; }
int show_message(const char*msg){ (void)msg; return 0; }

int main(void){
    char buf[32];
    /* color disabled */
    enable_color = 0;
    printf("create_dialog\n");
    create_dialog("Test", buf, sizeof(buf));
    assert(wbkgd_attr_last == A_NORMAL);

    printf("show_help\n");
    show_help();
    assert(wbkgd_attr_last == A_NORMAL);

    printf("show_about\n");
    show_about();
    assert(wbkgd_attr_last == A_NORMAL);

    printf("show_warning\n");
    show_warning_dialog();
    assert(wbkgd_attr_last == A_NORMAL);

    /* color enabled */
    enable_color = 1;
    printf("create_dialog color\n");
    create_dialog("Test", buf, sizeof(buf));
    assert(wbkgd_attr_last == COLOR_PAIR(SYNTAX_BG));

    printf("show_help color\n");
    show_help();
    assert(wbkgd_attr_last == COLOR_PAIR(SYNTAX_BG));

    printf("show_about color\n");
    show_about();
    assert(wbkgd_attr_last == COLOR_PAIR(SYNTAX_BG));

    printf("show_warning color\n");
    show_warning_dialog();
    assert(wbkgd_attr_last == COLOR_PAIR(SYNTAX_BG));

    return 0;
}

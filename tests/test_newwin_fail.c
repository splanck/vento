#include <assert.h>
#include <setjmp.h>
#include <ncurses.h>
#include "ui_common.h"
#include "config.h"

#undef newwin
#undef box
#undef mvwprintw
#undef wrefresh
#undef wgetch
#undef wclear
#undef delwin
#undef curs_set

int COLS = 80;
int LINES = 24;
int enable_color = 0;
int enable_mouse = 0;
int show_line_numbers = 0;
AppConfig app_config;
WINDOW *stdscr = NULL;

WINDOW *newwin(int nlines,int ncols,int y,int x){(void)nlines;(void)ncols;(void)y;(void)x;return NULL;}
int box(WINDOW*w,chtype a,chtype b){(void)w;(void)a;(void)b;return 0;}
int mvwprintw(WINDOW*w,int y,int x,const char*fmt,...){(void)w;(void)y;(void)x;(void)fmt;return 0;}
int wrefresh(WINDOW*w){(void)w;return 0;}
int wgetch(WINDOW*w){(void)w;return 0;}
int wclear(WINDOW*w){(void)w;return 0;}
int delwin(WINDOW*w){(void)w;return 0;}
int curs_set(int c){(void)c;return 0;}
int endwin(void){return 0;}

static jmp_buf jb;
void exit(int status){longjmp(jb,1);} // catch unexpected exit

int main(void){
    if(setjmp(jb)!=0){
        assert(!"program called exit");
    }
    WINDOW *w = create_popup_window(5, 10, NULL);
    assert(w == NULL);
    return 0;
}

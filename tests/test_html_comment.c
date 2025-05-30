#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <ncurses.h>
#undef wattron
#undef wattroff
#undef wattrset
#include "syntax.h"

/* minimal WINDOW stub */
typedef struct { int dummy; } SIMPLE_WIN;
static int first_x = -1;

WINDOW *newwin(int nlines, int ncols, int y, int x){(void)nlines;(void)ncols;(void)y;(void)x;return (WINDOW*)calloc(1,sizeof(SIMPLE_WIN));}
int delwin(WINDOW *w){free(w);return 0;}
int mvwprintw(WINDOW*w,int y,int x,const char *fmt,...){
    (void)w; (void)y; (void)fmt;
    if(first_x==-1) first_x = x;
    return 0;
}
int wattron(WINDOW*w,int a){(void)w;(void)a;return 0;}
int wattroff(WINDOW*w,int a){(void)w;(void)a;return 0;}
int wattrset(WINDOW*w,int a){(void)w;(void)a;return 0;}

int main(void){
    WINDOW *w = newwin(1,1,0,0);
    const char *line = "<!--";
    highlight_html_syntax(w, line, 0);
    assert(first_x == 1);
    delwin(w);
    return 0;
}

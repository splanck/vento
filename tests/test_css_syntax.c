#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ncurses.h>
#undef wattron
#undef wattroff
#undef wattrset
#undef mvwprintw
#include "syntax.h"
#include "files.h"

/* simple WINDOW stub */
typedef struct { int dummy; } SIMPLE_WIN;
static int current_attr;
static int call_index;
static char printed[10][64];
static int attrs[10];

WINDOW *newwin(int nlines, int ncols, int y, int x){(void)nlines;(void)ncols;(void)y;(void)x;return (WINDOW*)calloc(1,sizeof(SIMPLE_WIN));}
int delwin(WINDOW*w){free(w);return 0;}
int wattron(WINDOW*w,int a){(void)w; current_attr |= a; return 0;}
int wattroff(WINDOW*w,int a){(void)w; current_attr &= ~a; return 0;}
int wattrset(WINDOW*w,int a){(void)w; current_attr = a; return 0;}
int wattr_on(WINDOW*w, attr_t a, void*opts){(void)w;(void)opts; current_attr|=a; return 0;}
int wattr_off(WINDOW*w, attr_t a, void*opts){(void)w;(void)opts; current_attr&=~a; return 0;}
int mvwprintw(WINDOW*w,int y,int x,const char *fmt,...){(void)w;(void)y;(void)x;va_list ap;va_start(ap,fmt);vsnprintf(printed[call_index],sizeof(printed[call_index]),fmt,ap);va_end(ap);attrs[call_index]=current_attr;call_index++;return 0;}

int main(void){
    FileState fs = {0};
    WINDOW *w = newwin(1,1,0,0);

    const char *line = "margin: 10px;";
    const SyntaxDef *def = syntax_get(CSS_SYNTAX);
    highlight_by_patterns(&fs, w, line, 0, def);
    int kw = 0, num = 0;
    for(int i=0;i<call_index;i++){
        if(strcmp(printed[i], "margin") == 0){
            if(attrs[i] & COLOR_PAIR(SYNTAX_KEYWORD)) kw = 1;
        }
        if(strcmp(printed[i], "10px") == 0 || strcmp(printed[i], "10") == 0){
            if(attrs[i] & COLOR_PAIR(SYNTAX_TYPE)) num = 1;
        }
    }
    assert(kw);
    assert(num);

    delwin(w);
    return 0;
}

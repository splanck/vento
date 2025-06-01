#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <ncurses.h>
#undef wattron
#undef wattroff
#undef wattrset
#include "syntax.h"
#include "files.h"

/* minimal WINDOW stub to satisfy functions */
typedef struct { int dummy; } SIMPLE_WIN;

WINDOW *newwin(int nlines, int ncols, int y, int x){(void)nlines;(void)ncols;(void)y;(void)x;return (WINDOW*)calloc(1,sizeof(SIMPLE_WIN));}
int delwin(WINDOW *w){free(w);return 0;}
int mvwprintw(WINDOW*w,int y,int x,const char *fmt,...){(void)w;(void)y;(void)x;(void)fmt;return 0;}
int wattron(WINDOW*w,int a){(void)w;(void)a;return 0;}
int wattroff(WINDOW*w,int a){(void)w;(void)a;return 0;}
int wattrset(WINDOW*w,int a){(void)w;(void)a;return 0;}

int main(void){
    FileState fs = {0};
    WINDOW *w = newwin(1,1,0,0);

    char line[600];
    memset(line, 'a', sizeof(line)-2);
    line[sizeof(line)-2] = ' ';
    line[sizeof(line)-1] = '\0';

    const SyntaxDef *cdef = syntax_get(C_SYNTAX);
    highlight_by_patterns(&fs, w, line, 0, cdef);
    const SyntaxDef *csdef = syntax_get(CSHARP_SYNTAX);
    highlight_by_patterns(&fs, w, line, 0, csdef);

    delwin(w);
    return 0;
}

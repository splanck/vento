#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <ncurses.h>
#undef mvprintw
#undef wmove
#undef wrefresh
#undef werase
#undef box

#include "files.h"
#include "search.h"
#include "editor.h"

/* minimal WINDOW stub */
typedef struct { int dummy; } SIMPLE_WIN;
WINDOW *newwin(int nlines,int ncols,int y,int x){(void)nlines;(void)ncols;(void)y;(void)x;return (WINDOW*)calloc(1,sizeof(SIMPLE_WIN));}
int delwin(WINDOW*w){free(w);return 0;}
int werase(WINDOW*w){(void)w;return 0;}
int box(WINDOW*w,chtype a,chtype b){(void)w;(void)a;(void)b;return 0;}
int wmove(WINDOW*w,int y,int x){(void)w;(void)y;(void)x;return 0;}
int wrefresh(WINDOW*w){(void)w;return 0;}
int mvprintw(int y,int x,const char*fmt,...){(void)y;(void)x;(void)fmt;return 0;}
int wborder(WINDOW*w,chtype ls,chtype rs,chtype ts,chtype bs,chtype tl,chtype tr,chtype bl,chtype br){(void)w;(void)ls;(void)rs;(void)ts;(void)bs;(void)tl;(void)tr;(void)bl;(void)br;return 0;}

/* required globals and stubs */
WINDOW *text_win = NULL;
FileState *active_file = NULL;
int COLS = 80;
int LINES = 24;
char search_text[256];
int show_find_dialog(char*out,int sz,const char*def){(void)out;(void)sz;(void)def;return 0;}
int show_replace_dialog(char*s,int ss,char*r,int rs){(void)s;(void)ss;(void)r;(void)rs;return 0;}

void draw_text_buffer(FileState*fs, WINDOW*w){(void)fs;(void)w;}
void push(Node **stack, Change change){(void)stack; free(change.old_text); free(change.new_text);} 
void mark_comment_state_dirty(FileState*fs){(void)fs;}
int get_line_number_offset(FileState*fs){(void)fs;return 0;}

int main(void){
    FileState fs = {0};
    fs.line_capacity = 64;
    fs.max_lines = 2;
    fs.text_buffer = calloc(fs.max_lines, sizeof(char*));
    for(int i=0;i<fs.max_lines;i++) fs.text_buffer[i]=calloc(fs.line_capacity, sizeof(char));
    strcpy(fs.text_buffer[0], "foo bar");
    fs.line_count = 1;
    fs.cursor_x = 0; fs.cursor_y = 0;
    fs.start_line = 0;

    text_win = newwin(5,5,0,0);
    active_file = &fs;

    fs.modified = false;
    replace_next_occurrence(&fs, "foo", "baz");

    assert(strcmp(fs.text_buffer[0], "baz bar") == 0);
    assert(fs.modified);
    assert(fs.match_start_x == -1);
    assert(fs.match_end_x == -1);
    assert(fs.match_start_y == -1);
    assert(fs.match_end_y == -1);

    delwin(text_win);
    for(int i=0;i<fs.max_lines;i++) free(fs.text_buffer[i]);
    free(fs.text_buffer);
    return 0;
}

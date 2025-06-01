#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <ncurses.h>
#undef mvprintw
#undef wmove
#undef wrefresh
#undef clrtoeol
#undef refresh

#include "files.h"
#include "search.h"
#include "config.h"

/* minimal stubs */
int mvprintw(int y,int x,const char*fmt,...){(void)y;(void)x;(void)fmt;return 0;}
int wmove(WINDOW*w,int y,int x){(void)w;(void)y;(void)x;return 0;}
int wrefresh(WINDOW*w){(void)w;return 0;}
int clrtoeol(void){return 0;}
int refresh(void){return 0;}
int get_line_number_offset(FileState*fs){(void)fs;return 0;}
void draw_text_buffer(FileState*fs,WINDOW*w){(void)fs;(void)w;}
void push(Node **stack, Change change){(void)stack; free(change.old_text); free(change.new_text);}
void mark_comment_state_dirty(FileState*fs){(void)fs;}
int show_find_dialog(EditorContext*ctx,char*out,int sz,const char*def){(void)ctx;(void)out;(void)sz;(void)def;return 0;}
int show_replace_dialog(EditorContext*ctx,char*s,int ss,char*r,int rs){(void)ctx;(void)s;(void)ss;(void)r;(void)rs;return 0;}
void allocation_failed(const char*msg){(void)msg; abort();}

WINDOW *text_win=NULL;
int COLS=80, LINES=24;
FileState *active_file=NULL;
AppConfig app_config;
char search_text[256];

int main(void){
    FileState fs = {0};
    fs.line_capacity = 32;
    fs.max_lines = 2;
    fs.text_buffer = calloc(fs.max_lines, sizeof(char*));
    for(int i=0;i<fs.max_lines;i++) fs.text_buffer[i]=calloc(fs.line_capacity, sizeof(char));
    strcpy(fs.text_buffer[0], "Hello");
    fs.line_count = 1;
    fs.cursor_x = 0; fs.cursor_y = 0;
    fs.start_line = 0;

    /* case sensitive search should fail */
    app_config.search_ignore_case = 0;
    find_next_occurrence(&fs, "hello");
    assert(fs.match_start_y == -1);

    /* case insensitive search should succeed */
    app_config.search_ignore_case = 1;
    find_next_occurrence(&fs, "hello");
    assert(fs.match_start_y == 0);
    assert(fs.match_start_x == 0);

    for(int i=0;i<fs.max_lines;i++) free(fs.text_buffer[i]);
    free(fs.text_buffer);
    return 0;
}

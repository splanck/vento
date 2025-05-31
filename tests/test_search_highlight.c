#include <assert.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ncurses.h>
#undef mvwprintw
#undef mvwchgat
#undef mvprintw
#undef wmove
#undef wrefresh
#undef werase
#undef box
#undef wnoutrefresh
#undef doupdate
#undef wattrset
#undef wattron
#undef wattroff

#include "files.h"
#include "search.h"
#include "menu.h"
#include "file_manager.h"
#include "menu.h"

/* minimal WINDOW stub */
typedef struct { int dummy; } SIMPLE_WIN;
static int chgat_called = 0;
static int chgat_y, chgat_x, chgat_len;
static attr_t chgat_attr;
static int current_attr;

WINDOW *newwin(int nlines,int ncols,int y,int x){(void)nlines;(void)ncols;(void)y;(void)x;return (WINDOW*)calloc(1,sizeof(SIMPLE_WIN));}
int delwin(WINDOW*w){free(w);return 0;}
int werase(WINDOW*w){(void)w;return 0;}
int box(WINDOW*w,chtype a,chtype b){(void)w;(void)a;(void)b;return 0;}
int wattron(WINDOW*w,int a){(void)w;current_attr|=a;return 0;}
int wattroff(WINDOW*w,int a){(void)w;current_attr&=~a;return 0;}
int wattrset(WINDOW*w,int a){(void)w;current_attr=a;return 0;}
int mvwprintw(WINDOW*w,int y,int x,const char *fmt,...){(void)w;(void)y;(void)x;(void)fmt;return 0;}
int mvwchgat(WINDOW*w,int y,int x,int n,attr_t attr,short c,const void*opts){(void)w;(void)c;(void)opts;chgat_called=1;chgat_y=y;chgat_x=x;chgat_len=n;chgat_attr=attr;return 0;}
int mvprintw(int y,int x,const char *fmt,...){(void)y;(void)x;(void)fmt;return 0;}
int wmove(WINDOW*w,int y,int x){(void)w;(void)y;(void)x;return 0;}
int wrefresh(WINDOW*w){(void)w;return 0;}

/* globals used by search.c */
WINDOW *text_win = NULL;
int COLS = 80;
int LINES = 24;

void apply_syntax_highlighting(FileState *fs, WINDOW *win, const char *line, int y){
    (void)fs; mvwprintw(win,y,1,"%s",line);
}
void sync_multiline_comment(FileState*fs,int line){(void)fs;(void)line;}
void ensure_line_loaded(FileState*fs,int idx){(void)fs;(void)idx;}
int doupdate(void){return 0;}
int wnoutrefresh(WINDOW*w){(void)w;return 0;}
void update_status_bar(FileState*fs){(void)fs;}
static int drawBar_called=0;
void drawBar(void){drawBar_called=1;}
int ensure_col_capacity(FileState*fs,int cols){(void)fs;(void)cols;return 0;}
int show_find_dialog(char*out,int sz,const char*def){(void)out;(void)sz;(void)def;return 0;}
int show_replace_dialog(char*s,int ss,char*r,int rs){(void)s;(void)ss;(void)r;(void)rs;return 0;}
void push(Node **stack, Change change){(void)stack;(void)change;}
void mark_comment_state_dirty(FileState *fs){(void)fs;}
void handle_key_up(FileState*fs){(void)fs;}
void handle_key_down(FileState*fs){(void)fs;}
void handle_key_left(FileState*fs){(void)fs;}
void handle_key_right(FileState*fs){(void)fs;}
void handle_key_backspace(FileState*fs){(void)fs;}
void handle_key_delete(FileState*fs){(void)fs;}
void handle_key_enter(FileState*fs){(void)fs;}
void handle_key_page_up(FileState*fs){(void)fs;}
void handle_key_page_down(FileState*fs){(void)fs;}
void handle_ctrl_key_left(FileState*fs){(void)fs;}
void handle_ctrl_key_right(FileState*fs){(void)fs;}
void handle_ctrl_key_pgup(FileState*fs){(void)fs;}
void handle_ctrl_key_pgdn(FileState*fs){(void)fs;}
void handle_ctrl_key_up(FileState*fs){(void)fs;}
void handle_ctrl_key_down(FileState*fs){(void)fs;}
void handle_key_home(FileState*fs){(void)fs;}
void handle_key_end(FileState*fs){(void)fs;}
void handle_default_key(FileState*fs,int ch){(void)fs;(void)ch;}
void handle_mouse_event(FileState*fs,MEVENT*ev){(void)fs;(void)ev;}
void start_selection_mode(FileState*fs,int x,int y){(void)fs;(void)x;(void)y;}
void update_selection_mouse(FileState*fs,int x,int y){(void)fs;(void)x;(void)y;}
void end_selection_mode(FileState*fs){(void)fs;}
void paste_clipboard(FileState*fs,int*x,int*y){(void)fs;(void)x;(void)y;}
void delete_current_line(FileState*fs){(void)fs;}
void insert_new_line(FileState*fs){(void)fs;}
void next_file(FileState*fs,int*x,int*y){(void)fs;(void)x;(void)y;}
void prev_file(FileState*fs,int*x,int*y){(void)fs;(void)x;(void)y;}
void handle_redo_wrapper(FileState*fs,int*x,int*y){(void)fs;(void)x;(void)y;}
void handle_undo_wrapper(FileState*fs,int*x,int*y){(void)fs;(void)x;(void)y;}
void move_forward_to_next_word(FileState*fs){(void)fs;}
void move_backward_to_previous_word(FileState*fs){(void)fs;}
void show_about(void){}
void show_help(void){}
void handleMenuNavigation(Menu*m,int mc,int*cm,int*ci){(void)m;(void)mc;(void)cm;(void)ci;}
void handle_selection_mode(FileState*fs,int ch,int*cx,int*cy){(void)fs;(void)ch;(void)cx;(void)cy;}
void save_file(FileState*fs){(void)fs;}
void save_file_as(FileState*fs){(void)fs;}
void load_file(FileState*fs,const char*fn){(void)fs;(void)fn;}
void close_current_file(FileState*fs,int*cx,int*cy){(void)fs;(void)cx;(void)cy;}
int menu_click_open(int x,int y){(void)x;(void)y;return 0;}
void menuNewFile(void){}
void menuLoadFile(void){}
void menuSaveFile(void){}
void menuSaveAs(void){}
void menuCloseFile(void){}
void menuNextFile(void){}
void menuPrevFile(void){}
void menuSettings(void){}
void menuQuitEditor(void){}
void menuUndo(void){}
void menuRedo(void){}
void menuFind(void){}
void menuReplace(void){}
void menuAbout(void){}
void menuHelp(void){}
int show_goto_dialog(int*line){(void)line;return 0;}
void go_to_line(FileState*fs,int line){(void)fs;(void)line;}
int enable_mouse = 0;
Menu *menus = NULL; int menuCount = 0;
WINDOW *stdscr = NULL;
FileManager file_manager = {0};
FileState *active_file;
char search_text[256];

#include "../src/editor.c"

int main(void){
    FileState fs = {0};
    fs.line_capacity = 32;
    fs.max_lines = 3;
    fs.text_buffer = calloc(fs.max_lines,sizeof(char*));
    for(int i=0;i<fs.max_lines;i++) fs.text_buffer[i]=calloc(fs.line_capacity,sizeof(char));
    strcpy(fs.text_buffer[0],"hello");
    strcpy(fs.text_buffer[1],"foo bar");
    fs.line_count = 2;
    fs.cursor_x = 0; fs.cursor_y = 0;
    fs.start_line = 0;

    text_win = newwin(10,10,0,0);
    find_next_occurrence(&fs,"foo");
    draw_text_buffer(&fs,text_win);

    assert(chgat_called);
    assert(chgat_y==2);
    assert(chgat_x==1);
    assert(chgat_len==3);
    assert(chgat_attr==A_REVERSE);

    delwin(text_win);
    for(int i=0;i<fs.max_lines;i++) free(fs.text_buffer[i]);
    free(fs.text_buffer);
    return 0;
}

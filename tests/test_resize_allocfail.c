#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <ncurses.h>
#undef refresh
#undef mvwchgat
#undef mvprintw
#undef wmove
#undef wrefresh
#undef werase
#undef wnoutrefresh
#undef doupdate
#undef wattrset
#undef wattron
#undef wattroff
#undef mvwprintw
#undef clear
#undef box
#include "files.h"
#include "file_manager.h"
#include "editor.h"
#include "menu.h"

/* stub ncurses globals */
int COLS = 80;
int LINES = 24;

/* simple WINDOW stub */
typedef struct { int h,w,y,x; } SIMPLE_WIN;
WINDOW *newwin(int nlines,int ncols,int y,int x){
    SIMPLE_WIN *w = calloc(1,sizeof(SIMPLE_WIN));
    w->h=nlines; w->w=ncols; w->y=y; w->x=x;
    return (WINDOW*)w;
}
int delwin(WINDOW*w){free(w);return 0;}
int wresize(WINDOW*w,int h,int c){(void)w;(void)h;(void)c;return 0;}
int mvwin(WINDOW*w,int y,int x){(void)w;(void)y;(void)x;return 0;}
int werase(WINDOW*w){(void)w;return 0;}
int box(WINDOW*w,chtype a,chtype b){(void)w;(void)a;(void)b;return 0;}
int wrefresh(WINDOW*w){(void)w;return 0;}
int wmove(WINDOW*w,int y,int x){(void)w;(void)y;(void)x;return 0;}
int endwin(void){return 0;}
int refresh(void){return 0;}
int clear(void){return 0;}
int resizeterm(int r,int c){(void)r;(void)c;return 0;}
int wnoutrefresh(WINDOW*w){(void)w;return 0;}
int doupdate(void){return 0;}
void wtimeout(WINDOW *w,int t){(void)w;(void)t;}
WINDOW* derwin(WINDOW*w,int nlines,int ncols,int y,int x){(void)w;return newwin(nlines,ncols,y,x);}
int getmaxy(const WINDOW*w){return ((SIMPLE_WIN*)w)->h;}
int getmaxx(const WINDOW*w){return ((SIMPLE_WIN*)w)->w;}
int mvwprintw(WINDOW*w,int y,int x,const char*fmt,...){(void)w;(void)y;(void)x;(void)fmt;return 0;}
int mvwchgat(WINDOW*w,int y,int x,int n,attr_t attr,short c,const void*opts){(void)w;(void)y;(void)x;(void)n;(void)attr;(void)c;(void)opts;return 0;}

/* editor dependency stubs */
void update_status_bar(FileState*fs){(void)fs;}
void drawBar(void){}
void handle_key_up(EditorContext*ctx,FileState*fs){(void)ctx;(void)fs;}
void handle_key_down(EditorContext*ctx,FileState*fs){(void)ctx;(void)fs;}
void handle_key_left(EditorContext*ctx,FileState*fs){(void)ctx;(void)fs;}
void handle_key_right(EditorContext*ctx,FileState*fs){(void)ctx;(void)fs;}
void handle_key_backspace(EditorContext*ctx,FileState*fs){(void)ctx;(void)fs;}
void handle_key_delete(EditorContext*ctx,FileState*fs){(void)ctx;(void)fs;}
void handle_key_enter(EditorContext*ctx,FileState*fs){(void)ctx;(void)fs;}
void handle_key_page_up(EditorContext*ctx,FileState*fs){(void)ctx;(void)fs;}
void handle_key_page_down(EditorContext*ctx,FileState*fs){(void)ctx;(void)fs;}
void handle_ctrl_key_left(EditorContext*ctx,FileState*fs){(void)ctx;(void)fs;}
void handle_ctrl_key_right(EditorContext*ctx,FileState*fs){(void)ctx;(void)fs;}
void handle_ctrl_key_pgup(EditorContext*ctx,FileState*fs){(void)ctx;(void)fs;}
void handle_ctrl_key_pgdn(EditorContext*ctx,FileState*fs){(void)ctx;(void)fs;}
void handle_ctrl_key_up(EditorContext*ctx,FileState*fs){(void)ctx;(void)fs;}
void handle_ctrl_key_down(EditorContext*ctx,FileState*fs){(void)ctx;(void)fs;}
void handle_key_home(EditorContext*ctx,FileState*fs){(void)ctx;(void)fs;}
void handle_key_end(EditorContext*ctx,FileState*fs){(void)ctx;(void)fs;}
void handle_default_key(EditorContext*ctx,FileState*fs,int ch){(void)ctx;(void)fs;(void)ch;}
void handle_mouse_event(EditorContext*ctx,FileState*fs,MEVENT*ev){(void)ctx;(void)fs;(void)ev;}
void start_selection_mode(FileState*fs,int x,int y){(void)fs;(void)x;(void)y;}
void update_selection_mouse(EditorContext*ctx,FileState*fs,int x,int y){(void)ctx;(void)fs;(void)x;(void)y;}
void end_selection_mode(FileState*fs){(void)fs;}
void paste_clipboard(FileState*fs,int*x,int*y){(void)fs;(void)x;(void)y;}
void delete_current_line(FileState*fs){(void)fs;}
void insert_new_line(FileState*fs){(void)fs;}
void next_file(FileState*fs,int*x,int*y){(void)fs;(void)x;(void)y;}
void prev_file(FileState*fs,int*x,int*y){(void)fs;(void)x;(void)y;}
void handle_redo_wrapper(FileState*fs,int*x,int*y){(void)fs;(void)x;(void)y;}
void handle_undo_wrapper(FileState*fs,int*x,int*y){(void)fs;(void)x;(void)y;}
void move_forward_to_next_word(EditorContext*ctx,FileState*fs){(void)ctx;(void)fs;}
void move_backward_to_previous_word(EditorContext*ctx,FileState*fs){(void)ctx;(void)fs;}
void sync_multiline_comment(FileState*fs,int line){(void)fs;(void)line;}
void apply_syntax_highlighting(FileState*fs,WINDOW*win,const char*line,int y){(void)fs;(void)win;(void)line;(void)y;}
void show_about(void){}
void show_help(void){}
void find(FileState*fs,int new_search){(void)fs;(void)new_search;}
void handleMenuNavigation(Menu*m,int mc,int*cm,int*ci){(void)m;(void)mc;(void)cm;(void)ci;}
void handle_selection_mode(FileState*fs,int ch,int*cx,int*cy){(void)fs;(void)ch;(void)cx;(void)cy;}
void replace(FileState*fs){(void)fs;}
void save_file(FileState*fs){(void)fs;}
void save_file_as(FileState*fs){(void)fs;}
void load_file(FileState*fs,const char*fn){(void)fs;(void)fn;}
void close_current_file(FileState*fs,int*cx,int*cy){(void)fs;(void)cx;(void)cy;}
int menu_click_open(int x,int y){(void)x;(void)y;return 0;}
void menuNewFile(EditorContext*ctx){(void)ctx;}
void menuLoadFile(EditorContext*ctx){(void)ctx;}
void menuSaveFile(EditorContext*ctx){(void)ctx;}
void menuSaveAs(EditorContext*ctx){(void)ctx;}
void menuCloseFile(EditorContext*ctx){(void)ctx;}
void menuNextFile(EditorContext*ctx){(void)ctx;}
void menuPrevFile(EditorContext*ctx){(void)ctx;}
void menuSettings(EditorContext*ctx){(void)ctx;}
void menuQuitEditor(EditorContext*ctx){(void)ctx;}
void menuUndo(EditorContext*ctx){(void)ctx;}
void menuRedo(EditorContext*ctx){(void)ctx;}
void menuFind(EditorContext*ctx){(void)ctx;}
void menuReplace(EditorContext*ctx){(void)ctx;}
void menuAbout(EditorContext*ctx){(void)ctx;}
void menuHelp(EditorContext*ctx){(void)ctx;}
int show_goto_dialog(int*line){(void)line;return 0;}
void go_to_line(FileState*fs,int line){(void)fs;(void)line;}
void ensure_line_loaded(FileState*fs,int idx){(void)fs;(void)idx;}

/* globals */
int enable_mouse = 0;
Menu *menus = NULL; int menuCount = 0;
WINDOW *stdscr = NULL;
FileManager file_manager = {0};

/* stub to force failure */
int ensure_col_capacity(FileState*fs,int cols){(void)fs;(void)cols;return -1;}

static jmp_buf jb;

#include "../src/editor.c"

void exit(int status){longjmp(jb,status);}

int main(void){
    FileState fs = {0};
    fs.line_capacity = 20;
    fs.max_lines = 2;
    fs.text_buffer = calloc(fs.max_lines,sizeof(char*));
    for(int i=0;i<fs.max_lines;i++)
        fs.text_buffer[i] = calloc(fs.line_capacity,1);
    fs.line_count = 1;
    fs.text_win = newwin(LINES-2,COLS,1,0);
    active_file = &fs;

    file_manager.files = calloc(1,sizeof(FileState*));
    file_manager.files[0] = &fs;
    file_manager.count = 1;

    COLS = 40;

    if(setjmp(jb) == 0){
        perform_resize();
        return 1;
    }

    for(int i=0;i<fs.max_lines;i++)
        free(fs.text_buffer[i]);
    free(fs.text_buffer);
    free(file_manager.files);
    delwin(fs.text_win);
    return 0;
}

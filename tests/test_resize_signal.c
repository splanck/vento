#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <ncurses.h>
#undef refresh
#undef clear
#undef box
#include "files.h"
#include "file_manager.h"
#include "editor.h"
#include "editor_state.h"
#include "menu.h"

int COLS = 80;
int LINES = 24;

typedef struct { int h,w,y,x; } SIMPLE_WIN;

static WINDOW *resized_win;

WINDOW *newwin(int nlines, int ncols, int y, int x){
    SIMPLE_WIN *w = calloc(1,sizeof(SIMPLE_WIN));
    w->h=nlines; w->w=ncols; w->y=y; w->x=x;
    return (WINDOW*)w;
}
int delwin(WINDOW*w){free(w);return 0;}
int wresize(WINDOW*w,int h,int c){resized_win=w; ((SIMPLE_WIN*)w)->h=h; ((SIMPLE_WIN*)w)->w=c; return 0;}
int mvwin(WINDOW*w,int y,int x){((SIMPLE_WIN*)w)->y=y; ((SIMPLE_WIN*)w)->x=x; return 0;}
int werase(WINDOW*w){(void)w;return 0;}
int box(WINDOW*w,chtype a,chtype b){(void)w;(void)a;(void)b;return 0;}
int wrefresh(WINDOW*w){(void)w;return 0;}
int wmove(WINDOW*w,int y,int x){(void)w;(void)y;(void)x;return 0;}
int keypad(WINDOW*w,bool b){(void)w;(void)b;return 0;}
int meta(WINDOW*w,bool b){(void)w;(void)b;return 0;}
int endwin(void){return 0;}
int refresh(void){return 0;}
int clear(void){return 0;}
int resizeterm(int r,int c){(void)r;(void)c;return 0;}
int wnoutrefresh(WINDOW*w){(void)w;return 0;}
int doupdate(void){return 0;}

static int gcalls=0;
int wgetch(WINDOW*w){(void)w;gcalls++; if(gcalls==2) exiting=1; return ERR;}

void update_status_bar(EditorContext *ctx, FileState*fs){(void)fs;}
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
void start_selection_mode(FileState*fs,int x,int y){(void)fs;(void)x;(void)y;}
void update_selection_mouse(EditorContext*ctx,FileState*fs,int x,int y){(void)ctx;(void)fs;(void)x;(void)y;}
void end_selection_mode(FileState*fs){(void)fs;}
void handle_selection_mode(FileState*fs,int ch,int*cx,int*cy){(void)fs;(void)ch;(void)cx;(void)cy;}
void handleMenuNavigation(Menu*m,int mc,int*cm,int*ci){(void)m;(void)mc;(void)cm;(void)ci;}
int menu_click_open(int x,int y){(void)x;(void)y;return 0;}
void handle_mouse_event(EditorContext*ctx,FileState*fs,MEVENT*ev){(void)ctx;(void)fs;(void)ev;}
void paste_clipboard(FileState*fs,int*x,int*y){(void)fs;(void)x;(void)y;}
void delete_current_line(EditorContext *ctx, FileState*fs){(void)fs;}
void insert_new_line(EditorContext *ctx, FileState*fs){(void)fs;}
void next_file(EditorContext *ctx, FileState*fs,int*x,int*y){(void)fs;(void)x;(void)y;}
void prev_file(EditorContext *ctx, FileState*fs,int*x,int*y){(void)fs;(void)x;(void)y;}
void handle_redo_wrapper(FileState*fs,int*x,int*y){(void)fs;(void)x;(void)y;}
void handle_undo_wrapper(FileState*fs,int*x,int*y){(void)fs;(void)x;(void)y;}
void move_forward_to_next_word(EditorContext*ctx,FileState*fs){(void)ctx;(void)fs;}
void move_backward_to_previous_word(EditorContext*ctx,FileState*fs){(void)ctx;(void)fs;}
void sync_multiline_comment(FileState*fs,int line){(void)fs;(void)line;}
void apply_syntax_highlighting(FileState*fs,WINDOW*win,const char*line,int y){(void)fs;(void)win;(void)line;(void)y;}
void show_about(EditorContext*ctx){(void)ctx;}
void show_help(EditorContext*ctx){(void)ctx;}
void find(EditorContext*ctx,FileState*fs,int n){(void)ctx;(void)fs;(void)n;}
void replace(EditorContext*ctx,FileState*fs){(void)ctx;(void)fs;}
void save_file(EditorContext*ctx,FileState*fs){(void)ctx;(void)fs;}
void save_file_as(EditorContext*ctx,FileState*fs){(void)ctx;(void)fs;}
void load_file(EditorContext*ctx,FileState*fs,const char*fn){(void)ctx;(void)fs;(void)fn;}
void close_current_file(EditorContext*ctx,FileState*fs,int*cx,int*cy){(void)ctx;(void)fs;(void)cx;(void)cy;}
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
int show_goto_dialog(EditorContext*ctx,int*line){(void)ctx;(void)line;return 0;}
void go_to_line(EditorContext *ctx, FileState*fs,int line){(void)fs;(void)line;}

int enable_mouse=0;
Menu*menus=NULL;int menuCount=0;
WINDOW *stdscr=NULL;

#include "../src/editor.c"

int main(void){
    fm_init(&file_manager);
    LINES=20; COLS=50;
    FileState *fs = initialize_file_state("x",2,COLS);
    assert(fs);
    fm_add(&file_manager,fs);
    active_file=fs;

    resize_pending = 1;
    EditorContext ctx = {0};
    ctx.active_file = fs;
    ctx.text_win = fs->text_win;
    ctx.enable_mouse = enable_mouse;
    ctx.enable_color = enable_color;
    run_editor(&ctx);

    assert(resized_win==fs->text_win);

    free_file_state(fs);
    return 0;
}

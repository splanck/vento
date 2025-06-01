#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ncurses.h>
#undef refresh
#undef clear
#undef box
#include "files.h"
#include "file_manager.h"
#include "editor.h"
#include "menu.h"

/* stub ncurses global vars */
int COLS = 80;
int LINES = 24;

/* simple WINDOW implementation used for stubs */
typedef struct {
    int h, w, y, x;
} SIMPLE_WIN;

/* track last resize call */
static WINDOW *last_resize_win;
static int last_resize_h, last_resize_w;

WINDOW *newwin(int nlines, int ncols, int y, int x) {
    SIMPLE_WIN *w = calloc(1, sizeof(SIMPLE_WIN));
    w->h = nlines; w->w = ncols; w->y = y; w->x = x;
    return (WINDOW*)w;
}
int delwin(WINDOW *w){free(w);return 0;}
int wresize(WINDOW *w, int h, int c){last_resize_win=w; last_resize_h=h; last_resize_w=c; ((SIMPLE_WIN*)w)->h=h; ((SIMPLE_WIN*)w)->w=c; return 0;}
int mvwin(WINDOW *w, int y, int x){((SIMPLE_WIN*)w)->y=y; ((SIMPLE_WIN*)w)->x=x; return 0;}
int werase(WINDOW *w){(void)w; return 0;}
int box(WINDOW *w, chtype a, chtype b){(void)w;(void)a;(void)b;return 0;}
int wrefresh(WINDOW *w){(void)w; return 0;}
int wmove(WINDOW *w,int y,int x){(void)w;(void)y;(void)x;return 0;}
int endwin(void){return 0;}
int refresh(void){return 0;}
int clear(void){return 0;}
int resizeterm(int r,int c){(void)r;(void)c;return 0;}
int wnoutrefresh(WINDOW *w){(void)w; return 0;}
int doupdate(void){return 0;}

/* stubs for editor dependencies */
void update_status_bar(FileState *fs){(void)fs;}
static int drawBar_called = 0;
void drawBar(void){drawBar_called=1;}

/* stubs for unused functions referenced in editor.o */
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
void handle_mouse_event(FileState*fs, MEVENT *ev){(void)fs;(void)ev;}
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
void sync_multiline_comment(FileState*fs,int line){(void)fs;(void)line;}
void apply_syntax_highlighting(FileState*fs, WINDOW*win,const char*line,int y){(void)fs;(void)win;(void)line;(void)y;}
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
int menu_click_open(int x,int y){(void)x;(void)y; return 0;}
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
int show_goto_dialog(int *line){(void)line;return 0;}
void go_to_line(FileState *fs,int line){(void)fs;(void)line;}

/* minimal global vars */
int enable_mouse = 0;
Menu *menus = NULL; int menuCount = 0;
WINDOW *stdscr = NULL;

/* Include editor.c for perform_resize implementation */
#include "../src/editor.c"

int main(void){
    fm_init(&file_manager);

    /* initial terminal size */
    LINES = 20; COLS = 50;
    int initial_cols = COLS;
    FileState *fs = initialize_file_state("x", 2, COLS);
    assert(fs);
    fm_add(&file_manager, fs);
    active_file = fs;

    /* put content and cursor beyond the new bounds */
    memset(fs->text_buffer[0], 'A', 40);
    fs->text_buffer[0][40] = '\0';
    fs->cursor_x = 45;
    fs->cursor_y = 15;

    /* resize to smaller dimensions */
    LINES = 10; COLS = 30;
    drawBar_called = 0;
    perform_resize();

    assert(last_resize_win == fs->text_win);
    assert(last_resize_h == LINES - 2);
    assert(last_resize_w == COLS);
    assert(fs->line_capacity == initial_cols);
    assert(fs->cursor_x == COLS - 1);
    assert(fs->cursor_y == LINES - BOTTOM_MARGIN);
    assert(strlen(fs->text_buffer[0]) == 40);
    for(int i=0;i<40;i++)
        assert(fs->text_buffer[0][i] == 'A');

    assert(drawBar_called);

    /* grow terminal and verify capacity expands */
    int prev_capacity = fs->line_capacity;
    LINES = 25; COLS = 70;
    drawBar_called = 0;
    perform_resize();

    assert(last_resize_win == fs->text_win);
    assert(last_resize_h == LINES - 2);
    assert(last_resize_w == COLS);
    assert(fs->line_capacity > prev_capacity);
    assert(strlen(fs->text_buffer[0]) == 40);
    for(int i=0;i<40;i++)
        assert(fs->text_buffer[0][i] == 'A');
    assert(drawBar_called);

    free_file_state(fs);
    return 0;
}

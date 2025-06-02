#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#undef mvprintw
#undef wmove
#undef wrefresh
#undef clrtoeol
#undef refresh
#undef getch
#undef box
#undef timeout
#undef werase
#undef wnoutrefresh
#undef napms
#include "file_manager.h"
#include "file_ops.h"
#include "editor.h"
#include "editor_state.h"

int COLS = 80;
int LINES = 24;
WINDOW *text_win = NULL;
WINDOW *stdscr = NULL;
FileState *active_file = NULL;
FileManager file_manager;
int start_line = 0;
int enable_mouse = 0;
int enable_color = 0;

/* stubs for UI and other dependencies */
int mvprintw(int y,int x,const char*fmt,...){(void)y;(void)x;(void)fmt;return 0;}
int clrtoeol(void){return 0;}
int refresh(void){return 0;}
int getch(void){return 0;}
int box(WINDOW*w,chtype a,chtype b){(void)w;(void)a;(void)b;return 0;}
int wmove(WINDOW*w,int y,int x){(void)w;(void)y;(void)x;return 0;}
int wrefresh(WINDOW*w){(void)w;return 0;}
void timeout(int t){(void)t;}
int werase(WINDOW*w){(void)w;return 0;}
int wnoutrefresh(WINDOW*w){(void)w;return 0;}
int napms(int n){(void)n;return 0;}

void draw_text_buffer(FileState *fs, WINDOW *w){(void)fs;(void)w;}
void redraw(void){}
void clamp_scroll_x(FileState *fs){(void)fs;}
void mark_comment_state_dirty(FileState *fs){(void)fs;}
int ensure_line_capacity(FileState *fs,int n){(void)fs;(void)n;return 0;}
void push(Node **stack, Change ch){(void)stack; free(ch.old_text); free(ch.new_text);}
void redo(FileState *fs){(void)fs;}
void undo(FileState *fs){(void)fs;}
int show_message(const char *msg){(void)msg;return 'y';}
int show_open_file_dialog(EditorContext *ctx,char*p,int m){(void)ctx;(void)p;(void)m;return 0;}
int show_save_file_dialog(EditorContext *ctx,char*p,int m){(void)ctx;(void)p;(void)m;return 0;}
int get_line_number_offset(FileState *fs){(void)fs;return 0;}
void allocation_failed(const char *msg){(void)msg;abort();}
int load_next_lines(FileState *fs,int c){(void)fs;(void)c;return 0;}
void load_all_remaining_lines(FileState *fs){(void)fs;}

FileState* initialize_file_state(const char *filename,int max_lines,int max_cols){
    (void)max_lines; (void)max_cols;
    FileState *fs = calloc(1,sizeof(FileState));
    assert(fs);
    fs->text_win = (WINDOW*)calloc(1,sizeof(int));
    strncpy(fs->filename, filename, sizeof(fs->filename)-1);
    fs->cursor_x = fs->cursor_y = 1;
    return fs;
}

void free_file_state(FileState *fs){
    free(fs->text_win);
    free(fs);
}

int main(void){
    fm_init(&file_manager);
    FileState *fs1 = initialize_file_state("one",0,0);
    fm_add(&file_manager, fs1);
    FileState *fs2 = initialize_file_state("two",0,0);
    fm_add(&file_manager, fs2);
    fm_switch(&file_manager, 0); /* make fs1 active */
    active_file = fs1;
    text_win = fs1->text_win;

    EditorContext ctx = {0};
    ctx.file_manager = file_manager;
    ctx.active_file = active_file;
    ctx.text_win = text_win;

    /* close first file, should switch to fs2 */
    close_current_file(&ctx, active_file, &fs1->cursor_x, &fs1->cursor_y);
    active_file = ctx.active_file;
    assert(file_manager.count == 1);
    assert(active_file == fs2);
    assert(ctx.text_win == fs2->text_win);

    /* ensure further operations use updated context */
    int cx = 0, cy = 0;
    next_file(&ctx, active_file, &cx, &cy);
    assert(ctx.active_file == fs2);
    assert(ctx.text_win == fs2->text_win);

    /* close remaining file, triggers new_file */
    close_current_file(&ctx, active_file, &cx, &cy);
    active_file = ctx.active_file;
    assert(file_manager.count == 1);
    assert(active_file == file_manager.files[file_manager.active_index]);
    assert(ctx.text_win == active_file->text_win);

    next_file(&ctx, active_file, &cx, &cy);
    assert(ctx.active_file == active_file);
    assert(ctx.text_win == active_file->text_win);

    free_file_state(file_manager.files[0]);
    free(file_manager.files);
    return 0;
}

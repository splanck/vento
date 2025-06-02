#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ncurses.h>
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

/* minimal stubs for functions used by file_ops and editor_actions */
int mvprintw(int y,int x,const char*fmt,...){(void)y;(void)x;(void)fmt;return 0;}
int clrtoeol(void){return 0;}
int refresh(void){return 0;}
int getch(void){return 0;}
void box(WINDOW*w,int a,int b){(void)w;(void)a;(void)b;}
void wmove(WINDOW*w,int y,int x){(void)w;(void)y;(void)x;}
void wrefresh(WINDOW*w){(void)w;}
int timeout(int t){(void)t;return 0;}
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
bool any_file_modified(FileManager *fm){(void)fm;return false;}
int show_message(const char *msg){(void)msg;return 'y';}
int show_open_file_dialog(EditorContext *ctx,char*p,int m){(void)ctx;(void)p;(void)m;return 0;}
int show_save_file_dialog(EditorContext *ctx,char*p,int m){(void)ctx;(void)p;(void)m;return 0;}
void update_status_bar(EditorContext *ctx, FileState *fs){(void)ctx;(void)fs;}
int get_line_number_offset(FileState *fs){(void)fs;return 0;}
void allocation_failed(const char *msg){(void)msg;abort();}
int load_next_lines(FileState *fs,int c){(void)fs;(void)c;return 0;}
void load_all_remaining_lines(FileState *fs){(void)fs;}

FileState* initialize_file_state(const char *filename,int max_lines,int max_cols){
    (void)max_lines;(void)max_cols;
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
    const char *f1 = "tmp1.txt";
    const char *f2 = "tmp2.txt";
    FILE *fp = fopen(f1,"w"); fclose(fp);
    fp = fopen(f2,"w"); fclose(fp);

    fm_init(&file_manager);
    FileState *fs1 = initialize_file_state(f1,0,0);
    fm_add(&file_manager, fs1);
    active_file = fs1;
    text_win = fs1->text_win;

    EditorContext ctx = {0};
    ctx.file_manager = file_manager;
    ctx.active_file = active_file;
    ctx.text_win = text_win;

    load_file(&ctx, active_file, f2);

    assert(ctx.active_file == active_file);
    assert(ctx.text_win == text_win);
    assert(ctx.file_manager.count == 2);

    unlink(f1);
    unlink(f2);
    free_file_state(file_manager.files[0]);
    free_file_state(file_manager.files[1]);
    free(file_manager.files);
    return 0;
}

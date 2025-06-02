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
#include "menu.h"
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
AppConfig app_config;

/* minimal WINDOW allocation stubs */
WINDOW *newwin(int nlines,int ncols,int y,int x){(void)nlines;(void)ncols;(void)y;(void)x;return malloc(1);} 
int delwin(WINDOW*w){free(w);return 0;}
int wborder(WINDOW*w,chtype ls,chtype rs,chtype ts,chtype bs,chtype tl,chtype tr,chtype bl,chtype br){(void)w;(void)ls;(void)rs;(void)ts;(void)bs;(void)tl;(void)tr;(void)bl;(void)br;return 0;}

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

/* additional stubs required by menu.c */
int show_settings_dialog(EditorContext*ctx,AppConfig*cfg){(void)ctx;(void)cfg;return 0;}
void config_save(const AppConfig*cfg){(void)cfg;}
void config_load(AppConfig*cfg){(void)cfg;}
mmask_t mousemask(mmask_t newmask, mmask_t *old){(void)newmask; if(old)*old=0; return 0;}
void apply_colors(void){}
bool confirm_quit(void){return false;}
void close_editor(void){}
void find(EditorContext*ctx,FileState*fs,int n){(void)ctx;(void)fs;(void)n;}
void replace(EditorContext*ctx,FileState*fs){(void)ctx;(void)fs;}
void show_about(EditorContext*ctx){(void)ctx;}
void show_help(EditorContext*ctx){(void)ctx;}

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

#include "../src/menu_draw.c"
#include "../src/menu.c"

int main(void){
    fm_init(&file_manager);
    FileState *fs1 = initialize_file_state("one",0,0);
    fm_add(&file_manager, fs1);
    FileState *fs2 = initialize_file_state("two",0,0);
    fm_add(&file_manager, fs2);
    fm_switch(&file_manager, 0);
    active_file = fs1;
    text_win = fs1->text_win;

    EditorContext ctx = {0};
    ctx.file_manager = file_manager;
    ctx.active_file = active_file;
    ctx.text_win = text_win;

    menuCloseFile(&ctx);
    active_file = ctx.active_file;
    assert(file_manager.count == 1);
    assert(active_file == fs2);
    assert(ctx.text_win == fs2->text_win);

    int cx = 0, cy = 0;
    next_file(&ctx, active_file, &cx, &cy);
    assert(ctx.active_file == fs2);
    assert(ctx.text_win == fs2->text_win);

    menuCloseFile(&ctx);
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

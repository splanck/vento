#include <assert.h>
#include <string.h>
#include <ncurses.h>
#include "file_manager.h"
#include "file_ops.h"
#include "editor.h"
#include "editor_state.h"
#include "ui.h"
#include "ui_common.h"
#include "config.h"

int COLS = 80;
int LINES = 24;
WINDOW *text_win = NULL;
WINDOW *stdscr = NULL;
FileState *active_file = NULL;
FileManager file_manager;
AppConfig app_config;
int start_line = 0;
int enable_mouse = 0;
int enable_color = 0;

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
void drawBar(void){}
void show_warning_dialog(EditorContext*ctx){(void)ctx;}
void run_editor(EditorContext *ctx){(void)ctx;}
int endwin(void){return 0;}
void cleanup_on_exit(FileManager *fm){(void)fm;}
void initialize(EditorContext *ctx){(void)ctx;}

#define main vento_main
#include "../src/vento.c"
#undef main

int main(void){
    char *argv[] = {"vento", "missing1.txt", "missing2.txt"};
    vento_main(3, argv);
    assert(file_manager.count == 1);
    assert(file_manager.files[0]->filename[0] == '\0');
    free_file_state(file_manager.files[0]);
    free(file_manager.files);
    return 0;
}

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/resource.h>
#include <dirent.h>
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
int doupdate(void){return 0;}
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

static int count_fds(void){
    DIR *d = opendir("/proc/self/fd");
    int c=0; struct dirent *e;
    while((e=readdir(d))) c++;
    closedir(d);
    return c-2; /* skip . and .. */
}

#define main vento_main
#include "../src/vento.c"
#undef main

int main(void){
    struct rlimit rl;
    getrlimit(RLIMIT_NOFILE, &rl);
    rl.rlim_cur = 32;
    setrlimit(RLIMIT_NOFILE, &rl);

    char files[20][64];
    char *argv[21];
    argv[0] = "vento";
    for(int i=0;i<20;i++){
        snprintf(files[i], sizeof(files[i]), "big_%d.txt", i);
        FILE *fp=fopen(files[i],"w");
        for(int j=0;j<2000;j++) fprintf(fp,"line %d\n",j);
        fclose(fp);
        argv[i+1] = files[i];
    }
    int argc = 21;

    vento_main(argc, argv);

    assert(file_manager.count == 20);
    assert(count_fds() < 32);

    for(int i=0;i<file_manager.count;i++){
        unlink(file_manager.files[i]->filename);
        free_file_state(file_manager.files[i]);
    }
    free(file_manager.files);
    return 0;
}

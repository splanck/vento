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

int COLS = 80;
int LINES = 24;
WINDOW *text_win = NULL;
WINDOW *stdscr = NULL;
FileState *active_file = NULL;
FileManager file_manager;
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

static int count_fds(void){
    DIR *d = opendir("/proc/self/fd");
    int c=0; struct dirent *e;
    while((e=readdir(d))) c++;
    closedir(d);
    return c-2; /* skip . and .. */
}

int main(void){
    struct rlimit rl;
    getrlimit(RLIMIT_NOFILE, &rl);
    rl.rlim_cur = 32;
    setrlimit(RLIMIT_NOFILE, &rl);

    fm_init(&file_manager);
    EditorContext ctx = {0};
    ctx.file_manager = file_manager;
    ctx.active_file = NULL;
    ctx.text_win = NULL;

    char name[64];
    int cx=0, cy=0;
    for(int i=0;i<20;i++){
        snprintf(name,sizeof(name),"big_%d.txt",i);
        FILE *fp=fopen(name,"w");
        for(int j=0;j<2000;j++) fprintf(fp,"line %d\n",j);
        fclose(fp);
        load_file(&ctx, active_file, name);
        if(i>0){
            next_file(&ctx, active_file, &cx, &cy);
        }
        assert(count_fds() < 32);
    }

    /* cleanup */
    for(int i=0;i<file_manager.count;i++){
        unlink(file_manager.files[i]->filename);
        free_file_state(file_manager.files[i]);
    }
    free(file_manager.files);
    return 0;
}

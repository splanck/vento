#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ncurses.h>
#include "file_manager.h"
#include "files.h"
#include "editor.h"
#include "editor_state.h"
#include "ui.h"
#include "ui_common.h"
#include "config.h"

/* globals normally provided elsewhere */
FileState *active_file = NULL;
WINDOW *text_win = NULL;
int COLS = 80;
int LINES = 24;
FileManager file_manager = {0};
AppConfig app_config;
int enable_mouse = 0;
int enable_color = 0;

void fm_init(FileManager *fm){fm->files=NULL;fm->count=0;fm->active_index=-1;}
FileState* fm_current(FileManager *fm){if(!fm||fm->active_index<0||fm->active_index>=fm->count)return NULL;return fm->files[fm->active_index];}
int fm_add(FileManager *fm,FileState *fs){FileState **tmp=realloc(fm->files,sizeof(FileState*)*(fm->count+1));if(!tmp)abort();fm->files=tmp;fm->files[fm->count]=fs;fm->active_index=fm->count;return fm->count++;}
int fm_switch(FileManager *fm,int idx){if(!fm||idx<0||idx>=fm->count)return -1;fm->active_index=idx;return idx;}

/* stubs for unused functionality */
bool any_file_modified(FileManager *fm){(void)fm;return false;}
int show_message(const char*msg){(void)msg;return 0;}
void initialize(EditorContext *ctx){(void)ctx;}
void show_warning_dialog(void){}
void run_editor(EditorContext *ctx){(void)ctx;}
int endwin(void){return 0;}
void cleanup_on_exit(FileManager *fm){(void)fm;}

/* simplified file loader used by main */
void load_file(FileState *fs_unused,const char *filename){
    (void)fs_unused;
    FileState *fs = calloc(1, sizeof(FileState));
    assert(fs);
    strncpy(fs->filename, filename, sizeof(fs->filename)-1);
    int idx = fm_add(&file_manager, fs);
    fm_switch(&file_manager, idx);
    active_file = fm_current(&file_manager);
}

void new_file(FileState *fs_unused){
    (void)fs_unused;
    FileState *fs = calloc(1,sizeof(FileState));
    assert(fs);
    int idx = fm_add(&file_manager, fs);
    fm_switch(&file_manager, idx);
    active_file = fm_current(&file_manager);
}

#define main vento_main
#include "../src/vento.c"
#undef main

int main(void){
    /* create two temp files */
    char t1[] = "file1XXXXXX";
    char t2[] = "file2XXXXXX";
    int fd1 = mkstemp(t1);
    int fd2 = mkstemp(t2);
    assert(fd1 >= 0 && fd2 >= 0);
    close(fd1);
    close(fd2);

    char *argv[] = {"vento", t1, t2};
    vento_main(3, argv);

    assert(file_manager.count == 2);
    assert(strcmp(file_manager.files[0]->filename, t1) == 0);
    assert(strcmp(file_manager.files[1]->filename, t2) == 0);

    unlink(t1);
    unlink(t2);
    return 0;
}

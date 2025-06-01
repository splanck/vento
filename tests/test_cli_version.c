#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ncurses.h>
#include "file_manager.h"
#include "files.h"
#include "editor.h"
#include "editor_state.h"
#include "ui.h"
#include "ui_common.h"
#include "config.h"

FileManager file_manager;
FileState *active_file = NULL;
WINDOW *text_win = NULL;
int COLS = 80;
int LINES = 24;
AppConfig app_config;
int enable_mouse = 0;
int enable_color = 0;
int start_line = 0;

bool any_file_modified(FileManager *fm){(void)fm;return false;}
int show_message(const char*msg){(void)msg;return 0;}
void initialize(EditorContext *ctx){(void)ctx;}
void fm_init(FileManager *fm){(void)fm;}
void load_file(EditorContext *ctx,FileState *fs,const char*fn){(void)ctx;(void)fs;(void)fn;}
void new_file(FileState *fs){(void)fs;}
FileState* fm_current(FileManager *fm){(void)fm;return NULL;}
int fm_switch(FileManager *fm,int idx){(void)fm;(void)idx;return 0;}
void show_warning_dialog(EditorContext*ctx){(void)ctx;}
void run_editor(EditorContext *ctx){(void)ctx;}
void cleanup_on_exit(FileManager *fm){(void)fm;}
void drawBar(void){}
void update_status_bar(EditorContext *ctx, FileState *fs){(void)ctx;(void)fs;}

#define main vento_main
#include "../src/vento.c"
#undef main

int main(void){
    char *argv[] = {"vento", "--version"};
    FILE *tmp = tmpfile();
    assert(tmp);
    int fd = fileno(tmp);
    int saved = dup(1);
    dup2(fd,1);
    vento_main(2, argv);
    fflush(stdout);
    dup2(saved,1);
    lseek(fd,0,SEEK_SET);
    char buf[64];
    size_t n = fread(buf,1,sizeof(buf)-1,tmp);
    buf[n] = '\0';
    assert(strstr(buf, VERSION) != NULL);
    fclose(tmp);
    close(saved);
    return 0;
}

#include <assert.h>
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
int enable_mouse = 0;

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
int endwin(void){return 0;}
void apply_colors(void){}

#define main vento_main
#include "../src/vento.c"
#undef main

int main(void){
    setenv("VENTO_THEME_DIR","./themes",1);
    char *argv[] = {"vento", "--theme=default"};
    vento_main(2, argv);
    assert(strcmp(app_config.theme, "default") == 0);
    assert(strcmp(app_config.keyword_color, "CYAN") == 0);
    assert(strcmp(app_config.comment_color, "GREEN") == 0);
    return 0;
}

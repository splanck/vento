#include <assert.h>
#include <string.h>
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
int enable_color = 0;

bool any_file_modified(FileManager *fm){(void)fm;return false;}
int show_message(const char*msg){(void)msg;return 0;}
void initialize(EditorContext *ctx){(void)ctx;}
void fm_init(FileManager *fm){(void)fm;}
void new_file(FileState *fs){(void)fs;}
FileState* fm_current(FileManager *fm){(void)fm;return active_file;}
int fm_switch(FileManager *fm,int idx){(void)fm;(void)idx;return 0;}
void show_warning_dialog(EditorContext*ctx){(void)ctx;}
static int checked = 0;
void run_editor(EditorContext *ctx){(void)ctx; assert(active_file->cursor_y == 5); checked = 1;}
void cleanup_on_exit(FileManager *fm){(void)fm;}
int endwin(void){return 0;}
void update_status_bar(EditorContext *ctx, FileState *fs){(void)ctx;(void)fs;}
void apply_colors(void){}

static FileState dummy = {0};
void go_to_line(EditorContext *ctx, FileState *fs, int line){(void)ctx;fs->cursor_y=line;}
void load_file(EditorContext *ctx, FileState *unused, const char *filename){
    (void)ctx;(void)unused;
    strncpy(dummy.filename, filename, sizeof(dummy.filename)-1);
    dummy.buffer.count = 10;
    dummy.cursor_x = 1;
    dummy.cursor_y = 1;
    active_file = &dummy;
    if(start_line>0) go_to_line(ctx, active_file, start_line);
}

#define main vento_main
#include "../src/vento.c"
#undef main

int main(void){
    char *argv[] = {"vento", "--line=5", "file.txt"};
    vento_main(3, argv);
    assert(checked);
    return 0;
}

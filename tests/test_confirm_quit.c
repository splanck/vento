#include <assert.h>
#include <stdlib.h>
#include <ncurses.h>
#include "file_manager.h"
#include "menu.h"
#include "editor.h"
#include "editor_state.h"
#include "config.h"
#include "ui_common.h"

#undef refresh
#undef getch
#undef box
#undef mousemask
#undef getmouse
#undef wrefresh
#undef wclear
#undef mvwprintw
#undef werase
#undef curs_set

/* stub ncurses functions */
int COLS = 80;
int LINES = 24;
WINDOW *text_win = NULL;
WINDOW *stdscr = NULL;
int curs_set(int c){(void)c;return 0;}
int werase(WINDOW*w){(void)w;return 0;}
int box(WINDOW*w,chtype a,chtype b){(void)w;(void)a;(void)b;return 0;}
int wrefresh(WINDOW*w){(void)w;return 0;}
int refresh(void){return 0;}
int getch(void){return 0;}
int getmouse(MEVENT*ev){(void)ev;return 0;}
mmask_t mousemask(mmask_t newmask, mmask_t *oldmask){(void)newmask;(void)oldmask;return 0;}
int wclear(WINDOW*w){(void)w;return 0;}
int mvwprintw(WINDOW*w,int y,int x,const char*fmt,...){(void)w;(void)y;(void)x;(void)fmt;return 0;}

/* globals used by menu.c */
FileState *active_file = NULL;
int enable_mouse = 0;
AppConfig app_config;
int start_line = 0;

/* stubs for external editor functions */
void draw_text_buffer(FileState*fs,WINDOW*w){(void)fs;(void)w;}
void update_status_bar(EditorContext *ctx, FileState*fs){(void)fs;}
bool drawMenu(Menu*menu,int ci,int sx,int sy){(void)menu;(void)ci;(void)sx;(void)sy;return true;}
void drawMenuBar(Menu*m,int mc){(void)m;(void)mc;}
void new_file(EditorContext *ctx, FileState*fs){(void)ctx;(void)fs;}
void load_file(EditorContext*ctx,FileState*fs,const char*fn){(void)ctx;(void)fs;(void)fn;}
void save_file(EditorContext*ctx,FileState*fs){(void)ctx;(void)fs;}
void save_file_as(EditorContext*ctx,FileState*fs){(void)ctx;(void)fs;}
void close_current_file(EditorContext*ctx,FileState*fs,int*cx,int*cy){(void)ctx;(void)fs;(void)cx;(void)cy;}
void next_file(EditorContext *ctx, FileState*fs,int*cx,int*cy){(void)fs;(void)cx;(void)cy;}
void prev_file(EditorContext *ctx, FileState*fs,int*cx,int*cy){(void)fs;(void)cx;(void)cy;}
int show_settings_dialog(EditorContext*ctx,AppConfig*cfg){(void)ctx;(void)cfg;return 0;}
void config_save(const AppConfig*cfg){(void)cfg;}
void config_load(AppConfig*cfg){(void)cfg;}
void apply_colors(void){}
void redraw(void){}
void drawBar(void){}
void undo(FileState*fs){(void)fs;}
void redo(FileState*fs){(void)fs;}
void find(EditorContext*ctx,FileState*fs,int n){(void)ctx;(void)fs;(void)n;}
void replace(EditorContext*ctx,FileState*fs){(void)ctx;(void)fs;}
void show_about(EditorContext*ctx){(void)ctx;}
void show_help(EditorContext*ctx){(void)ctx;}
void allocation_failed(const char*msg){(void)msg;abort();}
void initialize(EditorContext *ctx){(void)ctx;}
void show_warning_dialog(EditorContext*ctx){(void)ctx;}
void run_editor(EditorContext *ctx){(void)ctx;}
void cleanup_on_exit(FileManager*fm){(void)fm;}

/* intercept close_editor */
static int close_called = 0;
void close_editor(void){close_called++;}

/* stub show_message returning 'n' */
int show_message(const char*msg){(void)msg;return 'n';}

/* include implementation */
#define main vento_main
#include "../src/vento.c"
#undef main
#include "../src/menu.c"

int main(void){
    fm_init(&file_manager);
    FileState fs = {0};
    fs.modified = true;
    file_manager.files = malloc(sizeof(FileState*));
    file_manager.files[0] = &fs;
    file_manager.count = 1;
    file_manager.active_index = 0;
    active_file = &fs;

    close_called = 0;
    EditorContext ctx = {0};
    menuQuitEditor(&ctx);
    assert(close_called == 0);

    free(file_manager.files);
    return 0;
}

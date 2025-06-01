#include <assert.h>
#include <ncurses.h>
#include <signal.h>
#include "config.h"
#include "file_manager.h"
#include "files.h"
#include "editor.h"

#undef initscr
#undef cbreak
#undef noecho
#undef keypad
#undef meta
#undef mousemask
#undef timeout
#undef bkgd
#undef wbkgd
#undef refresh
#undef sigaction
#undef define_key
#undef addstr

/* global state required by editor_init.c */
WINDOW *stdscr = (WINDOW*)1;
FileManager file_manager = {0};
WINDOW *text_win = NULL;
FileState *active_file = NULL;
int COLS = 80;
int LINES = 24;
int exiting = 0;
int enable_color = 0;
int enable_mouse = 0;
int show_line_numbers = 0;
AppConfig app_config;

static const char *printed = NULL;

/* stub ncurses functions */
WINDOW *initscr(void){ return (WINDOW*)1; }
int cbreak(void){ return 0; }
int noecho(void){ return 0; }
int keypad(WINDOW*w,bool b){ (void)w; (void)b; return 0; }
int meta(WINDOW*w,bool b){ (void)w; (void)b; return 0; }
mmask_t mousemask(mmask_t newmask, mmask_t *old){ if(old) *old = 0; (void)newmask; return 0; }
void timeout(int t){ (void)t; }
int bkgd(chtype ch){ (void)ch; return 0; }
int wbkgd(WINDOW*w, chtype ch){ (void)w; (void)ch; return 0; }
int refresh(void){ return 0; }
int sigaction(int s,const struct sigaction*a, struct sigaction*o){ (void)s;(void)a;(void)o; return 0; }
int define_key(const char*s,int k){ (void)s; (void)k; return 0; }
int addstr(const char*s){ printed = s; return 0; }

void initialize_key_mappings(void){}
void initializeMenus(void){}
void update_status_bar(FileState*fs){ (void)fs; }
void freeMenus(void){}
void syntax_cleanup(void){}
void free_stack(Node*stack){ (void)stack; }
void free_file_state(FileState*fs){ (void)fs; }
void on_sigwinch(int sig){ (void)sig; }

/* minimal config_load stub */
void config_load(AppConfig *cfg){ (void)cfg; }

#include "../src/editor_init.c"

int main(void){
    EditorContext ctx = {0};
    ctx.enable_mouse = 0;
    ctx.enable_color = 0;
    initialize(&ctx);
    addstr("UTF-8: \xCF\x80");
    assert(printed != NULL);
    return 0;
}

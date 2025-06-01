#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ncurses.h>
#include <signal.h>
#include <pwd.h>
#include "config.h"
#include "file_manager.h"
#include "files.h"
#include "editor.h"

#undef start_color
#undef use_default_colors
#undef init_pair
#undef bkgd
#undef wbkgd
#undef has_colors

/* stub ncurses functions and tracking variables */
int start_color_called = 0;
int init_pair_called = 0;
int bkgd_attr = -2;
int wbkgd_attr = -2;

int start_color(void){ start_color_called++; return 0; }
int use_default_colors(void){ return 0; }
int init_pair(short p, short f, short b){ (void)p; (void)f; (void)b; init_pair_called++; return 0; }
int bkgd(chtype ch){ bkgd_attr = ch; return 0; }
int wbkgd(WINDOW *w, chtype ch){ (void)w; wbkgd_attr = ch; return 0; }
bool has_colors(void){ return false; }

/* stub other functions referenced in editor_init.c */
void on_sigwinch(int sig){ (void)sig; }
int define_key(const char *s, int k){ (void)s; (void)k; return 0; }
void initialize_key_mappings(void){}
void initializeMenus(EditorContext *ctx){(void)ctx;}
void update_status_bar(FileState *fs){ (void)fs; }
void freeMenus(void){}
void syntax_cleanup(void){}
void free_stack(Node *stack){ (void)stack; }
void free_file_state(FileState *fs){ (void)fs; }
mmask_t mousemask(mmask_t newmask, mmask_t *old){ (void)newmask; if(old) *old = 0; return 0; }
int cbreak(void){ return 0; }
int noecho(void){ return 0; }
int keypad(WINDOW *w, bool b){ (void)w; (void)b; return 0; }
int meta(WINDOW *w, bool b){ (void)w; (void)b; return 0; }
void wtimeout(WINDOW *w, int t){ (void)w; (void)t; }
int wrefresh(WINDOW *w){ (void)w; return 0; }
WINDOW *initscr(void){ return (WINDOW*)1; }
int sigaction(int s, const struct sigaction *a, struct sigaction *o){ (void)s;(void)a;(void)o; return 0; }

/* stub getpwuid so HOME is used */
struct passwd *getpwuid(uid_t uid){ (void)uid; return NULL; }

/* required globals */
WINDOW *stdscr = (WINDOW*)1;
FileManager file_manager = {0};
WINDOW *text_win = NULL;
FileState *active_file = NULL;
int COLS = 80;
int LINES = 24;
int exiting = 0;

/* include source files under test */
#include "../src/config.c"
#include "../src/editor_init.c"

int main(void){
    /* set HOME to temp dir */
    const char *tmpdir = "./tmp_home_test";
    mkdir(tmpdir, 0700);
    setenv("HOME", tmpdir, 1);

    config_load(&app_config);

    /* has_colors stub forces disable */
    assert(enable_color == 0);
    /* prepare dummy file and window */
    FileState fs = {0};
    fs.text_win = (WINDOW*)2;
    FileState *arr[1];
    arr[0] = &fs;
    file_manager.files = arr;
    file_manager.count = 1;

    apply_colors();

    assert(start_color_called == 0);
    assert(init_pair_called == 0);
    assert(bkgd_attr == A_NORMAL);
    assert(wbkgd_attr == A_NORMAL);

    /* cleanup */
    unlink("./tmp_home_test/.ventorc");
    rmdir(tmpdir);
    return 0;
}

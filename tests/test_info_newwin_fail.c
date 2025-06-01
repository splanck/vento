#include <assert.h>
#include <ncurses.h>
#include "ui.h"
#include "dialog.h"
#include "editor.h"
#include "files.h"
#include "config.h"

#undef newwin
#undef keypad
#undef wbkgd
#undef wrefresh
#undef box
#undef mvwprintw
#undef wgetch
#undef wclear
#undef delwin
#undef curs_set
#undef werase
#undef getbegyx
#undef getmaxyx

/* simple WINDOW stub */
typedef struct { int dummy; } SIMPLE_WIN;
static SIMPLE_WIN dummy_win;

WINDOW *stdscr = (WINDOW*)&dummy_win;
WINDOW *text_win = (WINDOW*)&dummy_win;
FileState *active_file = NULL;
int COLS = 80;
int LINES = 24;
int enable_color = 0;
int enable_mouse = 0;
int show_line_numbers = 0;
AppConfig app_config;

static int show_message_called = 0;
int show_message(const char *msg){ (void)msg; show_message_called++; return 0; }

WINDOW *newwin(int nlines,int ncols,int y,int x){(void)nlines;(void)ncols;(void)y;(void)x;return NULL;}
int keypad(WINDOW*w, bool b){(void)w;(void)b;return 0;}
int wbkgd(WINDOW*w, chtype ch){(void)w;(void)ch;return 0;}
int wrefresh(WINDOW*w){(void)w;return 0;}
int box(WINDOW*w,chtype a,chtype b){(void)w;(void)a;(void)b;return 0;}
int mvwprintw(WINDOW*w,int y,int x,const char*fmt,...){(void)w;(void)y;(void)x;(void)fmt;return 0;}
int wgetch(WINDOW*w){(void)w;return 0;}
int wclear(WINDOW*w){(void)w;return 0;}
int delwin(WINDOW*w){(void)w;return 0;}
int curs_set(int c){(void)c;return 0;}
int werase(WINDOW*w){(void)w;return 0;}
#define getbegyx(win,y,x) do{ (void)(win); y=0; x=0; }while(0)
#define getmaxyx(win,y,x) do{ (void)(win); y=LINES; x=COLS; }while(0)

void draw_text_buffer(FileState*fs, WINDOW*w){(void)fs;(void)w;}
void update_status_bar(FileState*fs){(void)fs;}

int main(void){
    show_message_called = 0;
    WINDOW *w = dialog_open(5, 20, "Test");
    assert(w == NULL);
    assert(show_message_called == 1);

    show_message_called = 0;
    show_help();
    assert(show_message_called == 1);

    show_message_called = 0;
    show_about();
    assert(show_message_called == 1);

    show_message_called = 0;
    show_warning_dialog();
    assert(show_message_called == 1);

    return 0;
}

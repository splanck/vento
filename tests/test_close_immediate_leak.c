#include <assert.h>
#include <stdlib.h>
#include <ncurses.h>
#include "files.h"
#include "file_manager.h"

int LINES = 24;
int COLS = 80;

/* allocate window memory so leaks are detected */
WINDOW *newwin(int nlines, int ncols, int y, int x){
    (void)nlines; (void)ncols; (void)y; (void)x;
    return malloc(1);
}
int delwin(WINDOW *w){ free(w); return 0; }
int keypad(WINDOW *w, bool b){ (void)w; (void)b; return 0; }
int meta(WINDOW *w, bool b){ (void)w; (void)b; return 0; }
int wbkgd(WINDOW *w, chtype ch){ (void)w; (void)ch; return 0; }

/* enable_color and helpers */
#include "editor.h"
int enable_color = 1;
void wtimeout(WINDOW *w, int t){ (void)w; (void)t; }
void free_stack(Node *stack){ while (stack){ Node *n=stack->next; free(stack->change.old_text); free(stack->change.new_text); free(stack); stack=n; } }

int main(void){
    FileManager fm;
    fm_init(&fm);

    FileState *fs = initialize_file_state("dummy", 5, 10);
    assert(fs);

    fm_add(&fm, fs);
    fm_close(&fm, 0);

    assert(fm.count == 0);
    assert(fm.files == NULL);
    return 0;
}

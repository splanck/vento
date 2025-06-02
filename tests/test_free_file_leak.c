#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include "files.h"
#include "file_manager.h"
#include "editor.h"
#include "undo.h"

/* minimal ncurses stubs allocating memory so leaks are detectable */
int LINES = 24;
int COLS = 80;
WINDOW *newwin(int nlines, int ncols, int y, int x){
    (void)nlines; (void)ncols; (void)y; (void)x;
    return malloc(1);
}
int delwin(WINDOW *w){ free(w); return 0; }
int keypad(WINDOW *w, bool b){ (void)w; (void)b; return 0; }
int meta(WINDOW *w, bool b){ (void)w; (void)b; return 0; }
int wbkgd(WINDOW *w, chtype ch){ (void)w; (void)ch; return 0; }

int main(void){
    FileManager fm; 
    fm_init(&fm);

    /* manually allocate FileState with undo and redo history */
    FileState *fs = calloc(1, sizeof(FileState));
    assert(fs);
    fs->text_win = newwin(1,1,0,0);

    Node *u = malloc(sizeof(Node));
    u->change.old_text = strdup("old");
    u->change.new_text = NULL;
    u->next = NULL;
    fs->undo_stack = u;

    Node *r = malloc(sizeof(Node));
    r->change.old_text = NULL;
    r->change.new_text = strdup("new");
    r->next = NULL;
    fs->redo_stack = r;

    fm_add(&fm, fs);
    fm_close(&fm, 0);

    assert(fm.count == 0);
    assert(fm.files == NULL);
    return 0;
}

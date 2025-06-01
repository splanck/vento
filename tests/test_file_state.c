#include <assert.h>
#include <string.h>
#include <ncurses.h>
#include "files.h"
#include "file_manager.h"

/* stub minimal window functions to avoid full ncurses setup */
WINDOW *newwin(int nlines, int ncols, int y, int x) {
    (void)nlines; (void)ncols; (void)y; (void)x;
    return (WINDOW *)1;
}
int delwin(WINDOW *w) { (void)w; return 0; }

int main(void) {
    FileManager fm;
    fm_init(&fm);

    FileState *fs1 = initialize_file_state("a.txt", 5, 20);
    assert(fs1 && fs1->buffer.count == 1);
    strcpy(fs1->buffer.lines[0], "one");
    int idx1 = fm_add(&fm, fs1);
    assert(idx1 >= 0);

    FileState *fs2 = initialize_file_state("b.txt", 5, 20);
    assert(fs2 && fs2->buffer.count == 1);
    strcpy(fs2->buffer.lines[0], "two");
    int idx2 = fm_add(&fm, fs2);
    assert(idx2 >= 0);

    char *p1 = fs1->buffer.lines[0];
    char *p2 = fs2->buffer.lines[0];

    fm_switch(&fm, idx2);
    fm_switch(&fm, idx1);

    assert(fs1->buffer.lines[0] == p1);
    assert(strcmp(fs1->buffer.lines[0], "one") == 0);
    fm_switch(&fm, idx2);
    assert(fs2->buffer.lines[0] == p2);
    assert(strcmp(fs2->buffer.lines[0], "two") == 0);

    free_file_state(fs1);
    free_file_state(fs2);
    return 0;
}

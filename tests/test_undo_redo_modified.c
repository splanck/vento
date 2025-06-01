#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <ncurses.h>
#undef wrefresh
#undef werase
#undef box

#include "undo.h"
#include "files.h"

/* stub simple WINDOW functions */
int wrefresh(WINDOW *w){ (void)w; return 0; }
int werase(WINDOW *w){ (void)w; return 0; }
int box(WINDOW *w, chtype a, chtype b){ (void)w; (void)a; (void)b; return 0; }

void draw_text_buffer(FileState *fs, WINDOW *win){ (void)fs; (void)win; }
void allocation_failed(const char *msg){ (void)msg; abort(); }
int ensure_line_capacity(FileState *fs, int min_needed){ (void)fs; (void)min_needed; return 0; }

/* globals referenced by undo.c */
WINDOW *text_win = NULL;
FileState *active_file = NULL;

int main(void){
    FileState fs = {0};
    fs.line_capacity = 32;
    fs.buffer.capacity = 1;
    fs.buffer.lines = calloc(fs.buffer.capacity, sizeof(char*));
    fs.buffer.lines[0] = calloc(fs.line_capacity, sizeof(char));
    strcpy(fs.buffer.lines[0], "new");
    fs.buffer.count = 1;

    active_file = &fs;

    /* prepare undo stack with edit change */
    Change ch = {0, strdup("old"), strdup("new")};
    push(&fs.undo_stack, ch);

    fs.modified = false;
    undo(&fs);
    assert(strcmp(fs.buffer.lines[0], "old") == 0);
    assert(fs.modified);

    fs.modified = false;
    redo(&fs);
    assert(strcmp(fs.buffer.lines[0], "new") == 0);
    assert(fs.modified);

    free_stack(fs.undo_stack);
    free_stack(fs.redo_stack);
    free(fs.buffer.lines[0]);
    free(fs.buffer.lines);
    return 0;
}

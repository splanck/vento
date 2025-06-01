#include <assert.h>
#include <stdlib.h>
#include <ncurses.h>
#include <string.h>
#include "file_manager.h"
#include "editor.h"
#include "editor_state.h"
#include "file_ops.h"

int COLS = 80;
int LINES = 24;
WINDOW *text_win = NULL;
WINDOW *stdscr = NULL;
FileState *active_file = NULL;

/* stubs required by editor_actions.c and file_manager.c */
void allocation_failed(const char *msg){ (void)msg; abort(); }
void draw_text_buffer(FileState *fs, WINDOW *w){ (void)fs; (void)w; }
void mark_comment_state_dirty(FileState *fs){ (void)fs; }
int ensure_line_capacity(FileState *fs, int n){ (void)fs; (void)n; return 0; }
void push(Node **stack, Change ch){ (void)stack; free(ch.old_text); free(ch.new_text); }
void redo(FileState *fs){ (void)fs; }
void undo(FileState *fs){ (void)fs; }
void redraw(void){}
void clamp_scroll_x(FileState *fs){ (void)fs; }
void free_file_state(FileState *fs){ (void)fs; }

static int confirm_calls = 0;
bool confirm_switch(void){ confirm_calls++; return false; }

int main(void){
    fm_init(&file_manager);
    FileState fs1 = {0};
    FileState fs2 = {0};
    file_manager.files = malloc(2 * sizeof(FileState*));
    file_manager.files[0] = &fs1;
    file_manager.files[1] = &fs2;
    file_manager.count = 2;
    file_manager.active_index = 0;
    active_file = &fs1;
    fs1.modified = true;

    EditorContext ctx = {0};
    ctx.file_manager = file_manager;
    ctx.active_file = active_file;
    ctx.text_win = text_win;

    int cx = 0, cy = 0;
    next_file(&ctx, active_file, &cx, &cy);
    active_file = ctx.active_file;

    assert(confirm_calls == 1);
    assert(file_manager.active_index == 0);
    assert(active_file == &fs1);

    free(file_manager.files);
    return 0;
}

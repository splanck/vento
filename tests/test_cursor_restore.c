#include <assert.h>
#include <stdlib.h>
#include <ncurses.h>
#include "file_manager.h"
#include "editor.h"
#include "editor_state.h"
#include "file_ops.h"

int COLS = 80;
int LINES = 24;
WINDOW *text_win = NULL;
WINDOW *stdscr = NULL;
FileState *active_file = NULL;

/* stubs required by editor_actions.c */
void allocation_failed(const char *msg){(void)msg; abort();}
void draw_text_buffer(FileState *fs, WINDOW *w){(void)fs;(void)w;}
void mark_comment_state_dirty(FileState *fs){(void)fs;}
int ensure_line_capacity(FileState *fs, int n){(void)fs;(void)n;return 0;}
void push(Node **stack, Change ch){(void)stack; free(ch.old_text); free(ch.new_text);}
void redo(FileState *fs){(void)fs;}
void undo(FileState *fs){(void)fs;}
void redraw(void){}
void clamp_scroll_x(FileState *fs){(void)fs;}
void free_file_state(FileState *fs){(void)fs;}
bool confirm_switch(void){return true;}

int main(void){
    fm_init(&file_manager);
    FileState fs1 = {0};
    FileState fs2 = {0};
    fs1.cursor_x = fs1.saved_cursor_x = 5;
    fs1.cursor_y = fs1.saved_cursor_y = 6;
    fs2.cursor_x = fs2.saved_cursor_x = 2;
    fs2.cursor_y = fs2.saved_cursor_y = 3;
    file_manager.files = malloc(2 * sizeof(FileState*));
    file_manager.files[0] = &fs1;
    file_manager.files[1] = &fs2;
    file_manager.count = 2;
    file_manager.active_index = 0;
    active_file = &fs1;

    EditorContext ctx = {0};
    ctx.file_manager = file_manager;
    ctx.active_file = active_file;
    ctx.text_win = text_win;

    CursorPos pos = next_file(&ctx);
    assert(fs1.saved_cursor_x == 5 && fs1.saved_cursor_y == 6);
    active_file = ctx.active_file;
    assert(active_file == &fs2);
    assert(pos.x == 2 && pos.y == 3);
    assert(fs1.cursor_x == 5 && fs1.cursor_y == 6);

    fs2.cursor_x = 10; fs2.cursor_y = 11;
    pos = prev_file(&ctx);
    active_file = ctx.active_file;
    assert(fs2.saved_cursor_x == 10 && fs2.saved_cursor_y == 11);
    assert(active_file == &fs1);
    assert(pos.x == 5 && pos.y == 6);
    assert(fs1.cursor_x == 5 && fs1.cursor_y == 6);
    assert(fs2.cursor_x == 10 && fs2.cursor_y == 11);

    free(file_manager.files);
    return 0;
}

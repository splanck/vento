#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <ncurses.h>
#include "files.h"
#include "line_buffer.h"
#include "clipboard.h"
#include "editor.h"

WINDOW *text_win = NULL;
FileState *active_file = NULL;
void draw_text_buffer(FileState *fs, WINDOW *win) { (void)fs; (void)win; }
void allocation_failed(const char *msg) { (void)msg; abort(); }

/* Stub of insert_new_line without ncurses dependencies */
void insert_new_line(EditorContext *ctx, FileState *fs) {
    if (ensure_line_capacity(fs, fs->buffer.count + 1) < 0)
        abort();
    for (int i = fs->buffer.count; i > fs->cursor_y + fs->start_line - 1; --i) {
        strcpy(fs->buffer.lines[i], fs->buffer.lines[i - 1]);
    }
    fs->buffer.count++;
    fs->buffer.lines[fs->cursor_y + fs->start_line - 1][0] = '\0';
}

int main(void) {
    FileState fs = {0};
    fs.line_capacity = 128;
    lb_init(&fs.buffer, 10);
    lb_insert(&fs.buffer, 0, "hello");
    char *tmp = realloc(fs.buffer.lines[0], fs.line_capacity);
    if (!tmp) abort();
    fs.buffer.lines[0] = tmp;
    fs.text_win = NULL;
    strcpy(global_clipboard, "world\nfoo");
    active_file = &fs;
    text_win = NULL;

    int cx = 6;
    int cy = 1;
    paste_clipboard(&fs, &cx, &cy);

    assert(fs.buffer.count == 2);
    assert(strcmp(fs.buffer.lines[0], "helloworld") == 0);
    assert(strcmp(fs.buffer.lines[1], "foo") == 0);
    assert(cx == 4);
    assert(cy == 2);

    lb_free(&fs.buffer);

    return 0;
}

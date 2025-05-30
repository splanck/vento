#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <ncurses.h>
#include "files.h"
#include "clipboard.h"
#include "editor.h"

WINDOW *text_win = NULL;
FileState *active_file = NULL;
void draw_text_buffer(FileState *fs, WINDOW *win) { (void)fs; (void)win; }
void allocation_failed(const char *msg) { (void)msg; abort(); }

/* Stub of insert_new_line without ncurses dependencies */
void insert_new_line(FileState *fs) {
    if (ensure_line_capacity(fs, fs->line_count + 1) < 0)
        abort();
    for (int i = fs->line_count; i > fs->cursor_y + fs->start_line - 1; --i) {
        strcpy(fs->text_buffer[i], fs->text_buffer[i - 1]);
    }
    fs->line_count++;
    fs->text_buffer[fs->cursor_y + fs->start_line - 1][0] = '\0';
}

int main(void) {
    FileState fs = {0};
    fs.line_capacity = 128;
    fs.max_lines = 10;
    fs.text_buffer = calloc(fs.max_lines, sizeof(char*));
    for (int i = 0; i < fs.max_lines; ++i) {
        fs.text_buffer[i] = calloc(fs.line_capacity, sizeof(char));
    }
    fs.line_count = 1;
    strcpy(fs.text_buffer[0], "hello");
    fs.text_win = NULL;
    strcpy(global_clipboard, "world\nfoo");
    active_file = &fs;
    text_win = NULL;

    int cx = 6;
    int cy = 1;
    paste_clipboard(&fs, &cx, &cy);

    assert(fs.line_count == 2);
    assert(strcmp(fs.text_buffer[0], "helloworld") == 0);
    assert(strcmp(fs.text_buffer[1], "foo") == 0);
    assert(cx == 4);
    assert(cy == 2);

    for (int i = 0; i < fs.max_lines; ++i)
        free(fs.text_buffer[i]);
    free(fs.text_buffer);

    return 0;
}

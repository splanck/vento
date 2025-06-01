#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <ncurses.h>
#include "files.h"
#include "input.h"
#include "editor.h"

int COLS = 10;
int LINES = 24;
int show_line_numbers = 0;
WINDOW *text_win = NULL;
FileState *active_file = NULL;

void draw_text_buffer(FileState *fs, WINDOW *w) { (void)fs; (void)w; }
int get_line_number_offset(FileState *fs) { (void)fs; return 0; }
void mark_comment_state_dirty(FileState *fs){ (void)fs; }
void allocation_failed(const char *msg){ (void)msg; abort(); }
int ensure_line_capacity(FileState *fs, int n){ (void)fs; (void)n; return 0; }
void push(Node **stack, Change ch){ (void)stack; free(ch.old_text); free(ch.new_text); }
void ensure_line_loaded(FileState *fs, int idx){ (void)fs; (void)idx; }
void load_all_remaining_lines(FileState *fs){ (void)fs; }

int main(void) {
    const int len = 2050; /* > 2000 */
    FileState fs = {0};
    fs.line_capacity = len + 10;
    fs.buffer.capacity = 1;
    fs.buffer.lines = calloc(1, sizeof(char*));
    fs.buffer.lines[0] = calloc(fs.line_capacity, 1);
    memset(fs.buffer.lines[0], 'a', len - 1);
    fs.buffer.lines[0][len - 1] = 'Z';
    fs.buffer.lines[0][len] = '\0';
    fs.buffer.count = 1;
    fs.cursor_x = 1;
    fs.cursor_y = 1;
    fs.start_line = 0;
    fs.scroll_x = 0;
    EditorContext ctx = {0};
    ctx.active_file = &fs;
    active_file = &fs;

    for (int i = 0; i < len; i++)
        handle_key_right(&ctx, &fs);
    int visible = COLS - 2; /* no line numbers */
    assert(fs.cursor_x == len + 1);
    assert(fs.scroll_x == fs.cursor_x - visible);
    assert(fs.scroll_x + visible >= len);

    char *tmp = malloc(visible + 1);
    strncpy(tmp, fs.buffer.lines[0] + fs.scroll_x, visible);
    tmp[visible] = '\0';
    size_t tmplen = strlen(tmp);
    assert(tmplen > 0 && tmp[tmplen - 1] == 'Z');
    free(tmp);

    for (int i = len; i > 0; i--)
        handle_key_left(&ctx, &fs);
    assert(fs.cursor_x == 1);
    assert(fs.scroll_x == 0);

    free(fs.buffer.lines[0]);
    free(fs.buffer.lines);
    return 0;
}

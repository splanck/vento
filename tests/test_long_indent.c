#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <ncurses.h>
#include "files.h"
#include "editor.h"
#include "input.h"

WINDOW *text_win = NULL;
FileState *active_file = NULL;

void draw_text_buffer(FileState *fs, WINDOW *w){ (void)fs; (void)w; }
void mark_comment_state_dirty(FileState *fs){ (void)fs; }
void allocation_failed(const char *msg){ (void)msg; abort(); }
int ensure_line_capacity(FileState *fs, int min_needed){ (void)fs; (void)min_needed; return 0; }
void push(Node **stack, Change ch){ (void)stack; free(ch.old_text); free(ch.new_text); }
void ensure_line_loaded(FileState *fs, int idx){ (void)fs; (void)idx; }
void load_all_remaining_lines(FileState *fs){ (void)fs; }

int main(void){
    FileState fs = {0};
    fs.line_capacity = 2000;
    fs.max_lines = 5;
    fs.text_buffer = calloc(fs.max_lines, sizeof(char*));
    for(int i=0;i<fs.max_lines;i++)
        fs.text_buffer[i] = calloc(fs.line_capacity, 1);
    fs.line_count = 1;

    memset(fs.text_buffer[0], ' ', 1100);
    fs.text_buffer[0][1100] = 'x';
    fs.text_buffer[0][1101] = '\0';
    fs.cursor_x = 1101; // position after spaces
    fs.cursor_y = 1;
    fs.start_line = 0;

    active_file = &fs;

    handle_key_enter(&fs);

    assert(fs.line_count == 2);
    assert(strlen(fs.text_buffer[0]) == 1100);
    for(int i=0;i<1100;i++)
        assert(fs.text_buffer[0][i] == ' ');
    for(int i=0;i<1100;i++)
        assert(fs.text_buffer[1][i] == ' ');
    assert(fs.text_buffer[1][1100] == 'x');

    for(int i=0;i<fs.max_lines;i++)
        free(fs.text_buffer[i]);
    free(fs.text_buffer);
    return 0;
}

#include <assert.h>
#include <stdlib.h>
#include <ncurses.h>
#include "file_manager.h"
#include "editor.h"
#include "editor_state.h"
#include "file_ops.h"
#include "files.h"
#include "ui.h"

int COLS = 80;
int LINES = 24;
WINDOW *text_win = NULL;
WINDOW *stdscr = NULL;
FileState *active_file = NULL;
FileManager file_manager = {0};
int start_line = 0;

/* stubs for functions referenced in file_ops.c */
void draw_text_buffer(FileState *fs, WINDOW *win){ (void)fs; (void)win; }
void allocation_failed(const char *msg){ (void)msg; }
void load_all_remaining_lines(FileState *fs){ (void)fs; }
int load_next_lines(FileState *fs,int c){ (void)fs; (void)c; return 0; }
FileState *initialize_file_state(const char *f,int m,int c){ (void)f; (void)m; (void)c; return NULL; }
void free_file_state(FileState *f){ (void)f; }
int fm_add(FileManager *fm, FileState *fs){ (void)fm; (void)fs; return 0; }
void fm_close(FileManager *fm,int i){ (void)fm; (void)i; }
FileState *fm_current(FileManager *fm){ (void)fm; return NULL; }
int fm_switch(FileManager *fm,int i){ (void)fm; (void)i; return 0; }
int show_open_file_dialog(EditorContext *ctx,char*p,int m){ (void)ctx; (void)p; (void)m; return 0; }
int show_save_file_dialog(EditorContext *ctx,char*p,int m){ (void)ctx; (void)p; (void)m; return 0; }
void update_status_bar(EditorContext *ctx, FileState *fs){ (void)ctx; (void)fs; }
void redraw(void){}

/* custom implementations for testing */
bool any_file_modified(FileManager *fm){
    for(int i=0;i<fm->count;i++)
        if(fm->files[i] && fm->files[i]->modified)
            return true;
    return false;
}

static int return_char = 'y';
static int show_calls = 0;
int show_message(const char *msg){ (void)msg; show_calls++; return return_char; }

int main(void){
    FileState fs = {0};
    active_file = &fs;
    file_manager.files = malloc(sizeof(FileState*));
    file_manager.files[0] = &fs;
    file_manager.count = 1;
    file_manager.active_index = 0;
    fs.modified = true;

    return_char = 'y';
    show_calls = 0;
    assert(confirm_switch());
    assert(show_calls == 1);

    return_char = 'Y';
    show_calls = 0;
    assert(confirm_switch());
    assert(show_calls == 1);

    return_char = 'n';
    show_calls = 0;
    assert(!confirm_switch());
    assert(show_calls == 1);

    return_char = 'x';
    show_calls = 0;
    assert(!confirm_switch());
    assert(show_calls == 1);

    fs.modified = false;
    show_calls = 0;
    return_char = 'n';
    assert(confirm_switch());
    assert(show_calls == 0);

    free(file_manager.files);
    return 0;
}


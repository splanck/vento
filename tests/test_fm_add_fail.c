#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ncurses.h>
#undef mvprintw
#undef wmove
#undef wrefresh
#undef clrtoeol
#undef refresh
#undef getch
#undef box
#undef timeout
#undef werase
#undef wnoutrefresh
#undef napms
#include "file_manager.h"
#include "file_ops.h"
#include "editor.h"
#include "editor_state.h"

int COLS = 80;
int LINES = 24;
WINDOW *text_win = NULL;
WINDOW *stdscr = NULL;
FileState *active_file = NULL;
FileManager file_manager;
int start_line = 0;
int enable_mouse = 0;
int enable_color = 0;

/* stub UI helpers */
int mvprintw(int y,int x,const char*fmt,...){(void)y;(void)x;(void)fmt;return 0;}
int clrtoeol(void){return 0;}
int refresh(void){return 0;}
int getch(void){return 0;}
int box(WINDOW*w,chtype a,chtype b){(void)w;(void)a;(void)b;return 0;}
int wmove(WINDOW*w,int y,int x){(void)w;(void)y;(void)x;return 0;}
int wrefresh(WINDOW*w){(void)w;return 0;}
void timeout(int t){(void)t;}
int werase(WINDOW*w){(void)w;return 0;}
int wnoutrefresh(WINDOW*w){(void)w;return 0;}
int napms(int n){(void)n;return 0;}

void draw_text_buffer(FileState *fs, WINDOW *w){(void)fs;(void)w;}
void redraw(void){}
void clamp_scroll_x(FileState *fs){(void)fs;}
void mark_comment_state_dirty(FileState *fs){(void)fs;}
int ensure_line_capacity(FileState *fs,int n){(void)fs;(void)n;return 0;}
void push(Node **stack, Change ch){(void)stack; free(ch.old_text); free(ch.new_text);} 
void redo(FileState *fs){(void)fs;}
void undo(FileState *fs){(void)fs;}
void fm_close(FileManager *fm,int idx){(void)fm;(void)idx;}
bool any_file_modified(FileManager *fm){(void)fm;return false;}
int show_message(const char *msg){(void)msg;return 'y';}
int show_open_file_dialog(EditorContext *ctx,char*p,int m){(void)ctx;(void)p;(void)m;return 0;}
int show_save_file_dialog(EditorContext *ctx,char*p,int m){(void)ctx;(void)p;(void)m;return 0;}
void update_status_bar(EditorContext *ctx, FileState *fs){(void)ctx;(void)fs;}
int get_line_number_offset(FileState *fs){(void)fs;return 0;}
void allocation_failed(const char *msg){(void)msg;abort();}
int load_next_lines(FileState *fs,int c){(void)fs;(void)c;return 0;}
void load_all_remaining_lines(FileState *fs){(void)fs;}

/* simple file manager stubs */
void fm_init(FileManager *fm){fm->files=NULL;fm->count=0;fm->active_index=-1;}
FileState* fm_current(FileManager *fm){if(!fm||fm->active_index<0||fm->active_index>=fm->count) return NULL; return fm->files[fm->active_index];}
int fm_add(FileManager *fm, FileState *fs){(void)fm;(void)fs;return -1;}
int fm_switch(FileManager *fm,int idx){(void)fm;(void)idx;return -1;}

/* track created and freed FileState */
static FileState *created_fs = NULL;
static FileState *freed_fs = NULL;

FileState* initialize_file_state(const char *filename,int max_lines,int max_cols){
    (void)max_lines; (void)max_cols;
    FileState *fs = calloc(1,sizeof(FileState));
    assert(fs);
    fs->text_win = (WINDOW*)calloc(1,sizeof(int));
    strncpy(fs->filename, filename, sizeof(fs->filename)-1);
    fs->cursor_x = fs->cursor_y = 1;
    created_fs = fs;
    return fs;
}

void free_file_state(FileState *fs){
    freed_fs = fs;
    free(fs->text_win);
    free(fs);
}


int main(void){
    const char *existing = "base.txt";
    const char *newfile = "new.txt";
    FILE *fp = fopen(existing,"w"); fclose(fp);
    fp = fopen(newfile,"w"); fclose(fp);

    fm_init(&file_manager);
    FileState *fs1 = initialize_file_state(existing,0,0);
    file_manager.files = malloc(sizeof(FileState*));
    file_manager.files[0] = fs1;
    file_manager.count = 1;
    file_manager.active_index = 0;
    active_file = fs1;
    text_win = fs1->text_win;

    EditorContext ctx = {0};
    ctx.file_manager = file_manager;
    ctx.active_file = active_file;
    ctx.text_win = text_win;

    load_file(&ctx, active_file, newfile);

    assert(ctx.active_file == fs1);
    assert(ctx.text_win == fs1->text_win);
    assert(ctx.file_manager.count == 1);
    assert(freed_fs == created_fs);

    unlink(existing);
    unlink(newfile);
    free_file_state(fs1);
    free(file_manager.files);
    return 0;
}

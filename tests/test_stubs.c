#include "editor.h"
#include "file_manager.h"
#include <stdbool.h>
#include <stdio.h>

FileManager file_manager;
FileState *active_file = NULL;
WINDOW *text_win = NULL;

bool confirm_switch(void) { return true; }
void clamp_scroll_x(FileState *fs) { (void)fs; }
void redraw(void) {}
bool confirm_quit(void) { return true; }
void allocation_failed(const char *msg) { fprintf(stderr, "alloc fail: %s\n", msg ? msg : ""); }
void draw_text_buffer(FileState *fs, WINDOW *win) { (void)fs; (void)win; }
void __wrap_update_status_bar(EditorContext *ctx, FileState *fs) { fprintf(stderr, "update_status_bar called\n"); (void)ctx; (void)fs; }

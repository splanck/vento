#include "editor.h"
#include "file_manager.h"
#include "editor_state.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>

char *__real_strdup(const char *);
WINDOW *__real_create_popup_window(int, int, WINDOW *);

__attribute__((weak)) FileManager file_manager;

int fm_switch_fail = 0;
int fm_add_fail = 0;

int strdup_fail_on = 0;
int strdup_call_count = 0;

int create_popup_fail = 0;
int last_curs_set = -2;


bool __wrap_confirm_switch(void) { return true; }
void __wrap_clamp_scroll_x(FileState *fs) { (void)fs; }
void __wrap_redraw(void) {}
bool confirm_quit(void) { return true; }
void __wrap_allocation_failed(const char *msg) { fprintf(stderr, "alloc fail: %s\n", msg ? msg : ""); }
void __wrap_draw_text_buffer(FileState *fs, WINDOW *win) { (void)fs; (void)win; }
int last_status_count = -1;
void __wrap_update_status_bar(EditorContext *ctx, FileState *fs) {
    last_status_count = ctx ? ctx->file_manager.count : -1;
    fprintf(stderr, "update_status_bar called\n");
    (void)fs;
}

int __wrap_fm_switch(FileManager *fm, int index) {
    if (fm_switch_fail)
        return -1;
    if (!fm || index < 0 || index >= fm->count)
        return -1;
    fm->active_index = index;
    return fm->active_index;
}

int __wrap_fm_add(FileManager *fm, FileState *fs) {
    if (fm_add_fail)
        return -1;
    if (!fm || !fs)
        return -1;
    FileState **tmp = realloc(fm->files, sizeof(FileState*) * (fm->count + 1));
    if (!tmp)
        return -1;
    fm->files = tmp;
    fm->files[fm->count] = fs;
    fm->active_index = fm->count;
    fm->capacity = fm->count + 1;
    fm->count++;
    return fm->active_index;
}

char *__wrap_strdup(const char *s) {
    strdup_call_count++;
    if (strdup_fail_on > 0 && strdup_call_count == strdup_fail_on)
        return NULL;
    return __real_strdup(s);
}

WINDOW *__wrap_create_popup_window(int h, int w, WINDOW *p) {
    if (create_popup_fail) {
        create_popup_fail = 0;
        return NULL;
    }
    return __real_create_popup_window(h, w, p);
}

int __wrap_curs_set(int vis) {
    last_curs_set = vis;
    return 0; /* avoid calling real function */
}

int __wrap_wgetch(WINDOW *win) {
    (void)win;
    return 27; /* ESC to exit immediately */
}

int last_mvprintw_y = -1;
int last_mvprintw_x = -1;
int __wrap_mvprintw(int y, int x, const char *fmt, ...) {
    last_mvprintw_y = y;
    last_mvprintw_x = x;
    (void)fmt;
    return 0; /* suppress real output */
}

#include "minunit.h"
#include "input.h"
#include "files.h"
#include "editor.h"
#include "editor_state.h"
#include "clipboard.h"
#ifndef BUTTON1_DRAGGED
#define BUTTON1_DRAGGED (BUTTON1_PRESSED | REPORT_MOUSE_POSITION)
#endif
#include <ncurses.h>
#include <string.h>

int tests_run = 0;

static char *test_drag_clamp_bottom_right() {
    initscr();
    FileState *fs = initialize_file_state("", 5, 8);
    mu_assert("fs allocated", fs != NULL);
    active_file = fs;
    text_win = fs->text_win;

    fs->buffer.count = 3;
    fs->cursor_x = 1;
    fs->cursor_y = 1;
    start_selection_mode(fs, fs->cursor_x, fs->cursor_y);

    MEVENT ev = {0};
    ev.x = COLS + 10;
    ev.y = LINES + 10;
    ev.bstate = BUTTON1_DRAGGED;

    handle_mouse_event(NULL, fs, &ev);

    mu_assert("sel_end_x clamped", fs->sel_end_x == fs->line_capacity);
    mu_assert("sel_end_y clamped", fs->sel_end_y == fs->buffer.count);
    mu_assert("cursor_x clamped", fs->cursor_x == fs->line_capacity);
    mu_assert("cursor_y clamped", fs->cursor_y == fs->buffer.count);

    free_file_state(fs);
    endwin();
    return 0;
}

static char *test_drag_clamp_top_left() {
    initscr();
    FileState *fs = initialize_file_state("", 5, 8);
    mu_assert("fs allocated", fs != NULL);
    active_file = fs;
    text_win = fs->text_win;

    fs->buffer.count = 3;
    fs->cursor_x = 3;
    fs->cursor_y = 2;
    start_selection_mode(fs, fs->cursor_x, fs->cursor_y);

    MEVENT ev = {0};
    ev.x = -5;
    ev.y = -5;
    ev.bstate = BUTTON1_DRAGGED;

    handle_mouse_event(NULL, fs, &ev);

    mu_assert("sel_end_x min", fs->sel_end_x == 1);
    mu_assert("sel_end_y min", fs->sel_end_y == 1);
    mu_assert("cursor_x min", fs->cursor_x == 1);
    mu_assert("cursor_y min", fs->cursor_y == 1);

    free_file_state(fs);
    endwin();
    return 0;
}

static char *all_tests() {
    mu_run_test(test_drag_clamp_bottom_right);
    mu_run_test(test_drag_clamp_top_left);
    return 0;
}

int main(void) {
    char *result = all_tests();
    if (result) {
        printf("%s\n", result);
    } else {
        printf("ALL TESTS PASSED\n");
    }
    printf("Tests run: %d\n", tests_run);
    return result != 0;
}

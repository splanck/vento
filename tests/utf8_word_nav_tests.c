#include "minunit.h"
#include "input.h"
#include "files.h"
#include "editor.h"
#include "editor_state.h"
#include <locale.h>
#include <ncurses.h>
#include <string.h>

int tests_run = 0;

static char *test_forward_utf8() {
    setlocale(LC_ALL, "");
    initscr();
    FileState *fs = initialize_file_state("", 5, 32);
    mu_assert("fs allocated", fs != NULL);
    active_file = fs;
    text_win = fs->text_win;

    strcpy(fs->buffer.lines[0], "héllö 世界 bar");
    fs->buffer.count = 1;
    fs->cursor_y = 1;
    fs->cursor_x = 1;

    move_forward_to_next_word(NULL, fs);

    mu_assert("moved to second word", fs->cursor_x == 9);

    free_file_state(fs);
    endwin();
    return 0;
}

static char *test_backward_utf8() {
    setlocale(LC_ALL, "");
    initscr();
    FileState *fs = initialize_file_state("", 5, 32);
    mu_assert("fs allocated", fs != NULL);
    active_file = fs;
    text_win = fs->text_win;

    strcpy(fs->buffer.lines[0], "héllö 世界 bar");
    fs->buffer.count = 1;
    fs->cursor_y = 1;
    fs->cursor_x = 16;

    move_backward_to_previous_word(NULL, fs);
    mu_assert("step to space after second", fs->cursor_x == 15);
    move_backward_to_previous_word(NULL, fs);
    mu_assert("step to space after first", fs->cursor_x == 8);
    move_backward_to_previous_word(NULL, fs);
    mu_assert("step to line start", fs->cursor_x == 1);

    free_file_state(fs);
    endwin();
    return 0;
}

static char *test_backward_no_underflow_start_of_line() {
    setlocale(LC_ALL, "");
    initscr();
    FileState *fs = initialize_file_state("", 5, 32);
    mu_assert("fs allocated", fs != NULL);
    active_file = fs;
    text_win = fs->text_win;

    strcpy(fs->buffer.lines[0], "foo");
    strcpy(fs->buffer.lines[1], "bar");
    fs->buffer.count = 2;
    fs->cursor_y = 2;
    fs->cursor_x = 1;

    move_backward_to_previous_word(NULL, fs);
    mu_assert("moved to end of prev line", fs->cursor_y == 1 && fs->cursor_x == 4);
    move_backward_to_previous_word(NULL, fs);
    mu_assert("stopped at file start", fs->cursor_y == 1 && fs->cursor_x == 1);

    free_file_state(fs);
    endwin();
    return 0;
}

static char *test_backward_at_file_start() {
    setlocale(LC_ALL, "");
    initscr();
    FileState *fs = initialize_file_state("", 5, 32);
    mu_assert("fs allocated", fs != NULL);
    active_file = fs;
    text_win = fs->text_win;

    strcpy(fs->buffer.lines[0], "abc");
    fs->buffer.count = 1;
    fs->cursor_y = 1;
    fs->cursor_x = 1;

    move_backward_to_previous_word(NULL, fs);
    mu_assert("remain at file start", fs->cursor_y == 1 && fs->cursor_x == 1);

    free_file_state(fs);
    endwin();
    return 0;
}

static char *all_tests() {
    mu_run_test(test_forward_utf8);
    mu_run_test(test_backward_utf8);
    mu_run_test(test_backward_no_underflow_start_of_line);
    mu_run_test(test_backward_at_file_start);
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

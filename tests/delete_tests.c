#include "minunit.h"
#include "input.h"
#include "files.h"
#include "editor.h"
#include "editor_state.h"
#include <ncurses.h>
#include <string.h>

int tests_run = 0;

static char *test_delete_join_truncate() {
    initscr();
    FileState *fs = initialize_file_state("", 5, 8);
    mu_assert("fs allocated", fs != NULL);
    active_file = fs;
    text_win = fs->text_win;

    strcpy(fs->buffer.lines[0], "hello");
    strcpy(fs->buffer.lines[1], "world");
    fs->buffer.count = 2;
    fs->cursor_y = 1;
    fs->cursor_x = strlen(fs->buffer.lines[0]) + 1;

    handle_key_delete(NULL, fs);

    mu_assert("line truncated", strcmp(fs->buffer.lines[0], "hellowo") == 0);
    mu_assert("line count", fs->buffer.count == 1);

    free_file_state(fs);
    endwin();
    return 0;
}

static char *test_delete_join_exact_capacity() {
    initscr();
    FileState *fs = initialize_file_state("", 5, 8);
    mu_assert("fs allocated", fs != NULL);
    active_file = fs;
    text_win = fs->text_win;

    strcpy(fs->buffer.lines[0], "abc");
    strcpy(fs->buffer.lines[1], "defg");
    fs->buffer.count = 2;
    fs->cursor_y = 1;
    fs->cursor_x = strlen(fs->buffer.lines[0]) + 1;

    handle_key_delete(NULL, fs);

    mu_assert("line joined", strcmp(fs->buffer.lines[0], "abcdefg") == 0);
    mu_assert("line count", fs->buffer.count == 1);

    free_file_state(fs);
    endwin();
    return 0;
}

static char *all_tests() {
    mu_run_test(test_delete_join_truncate);
    mu_run_test(test_delete_join_exact_capacity);
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

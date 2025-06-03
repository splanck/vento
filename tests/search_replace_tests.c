#include "minunit.h"
#include "files.h"
#include "editor.h"
#include "search.h"
#include <ncurses.h>
#include <string.h>

int tests_run = 0;

static char *test_replace_next_long() {
    initscr();
    FileState *fs = initialize_file_state("", 5, 16);
    mu_assert("fs allocated", fs != NULL);
    active_file = fs;
    text_win = fs->text_win;

    strcpy(fs->buffer.lines[0], "abc");
    fs->buffer.count = 1;
    fs->cursor_x = 1;
    fs->cursor_y = 1;

    replace_next_occurrence(fs, "a", "12345678901234567890");

    mu_assert("line truncated", strlen(fs->buffer.lines[0]) == fs->line_capacity - 1);
    mu_assert("cursor capped", fs->cursor_x == fs->line_capacity - 1);

    free_file_state(fs);
    endwin();
    return 0;
}

static char *test_replace_all_long() {
    initscr();
    FileState *fs = initialize_file_state("", 5, 16);
    mu_assert("fs allocated", fs != NULL);
    active_file = fs;
    text_win = fs->text_win;

    strcpy(fs->buffer.lines[0], "aa");
    fs->buffer.count = 1;

    replace_all_occurrences(fs, "a", "12345678901234567890");

    mu_assert("line truncated all", strlen(fs->buffer.lines[0]) == fs->line_capacity - 1);

    free_file_state(fs);
    endwin();
    return 0;
}

static char *all_tests() {
    mu_run_test(test_replace_next_long);
    mu_run_test(test_replace_all_long);
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

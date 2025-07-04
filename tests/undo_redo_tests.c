#include "minunit.h"
#include "files.h"
#include "undo.h"
#include "editor.h"
#include "editor_state.h"
#include <ncurses.h>
#include <string.h>

int tests_run = 0;
extern int strdup_fail_on;
extern int strdup_call_count;
extern int allocation_fail_count;

static char *test_undo_strdup_failure() {
    initscr();
    FileState *fs = initialize_file_state("", 10, 80);
    mu_assert("fs allocated", fs != NULL);
    active_file = fs;
    text_win = fs->text_win;

    strcpy(fs->buffer.lines[0], "abc");
    fs->buffer.count = 1;
    char *new_text = strdup("abc");
    mu_assert("allocated", new_text != NULL);
    push(&fs->undo_stack, (Change){0, NULL, new_text});

    strdup_call_count = 0;
    strdup_fail_on = 1;
    allocation_fail_count = 0;
    undo(fs);
    strdup_fail_on = 0;

    mu_assert("allocation_failed called", allocation_fail_count == 1);
    mu_assert("redo stack empty", fs->redo_stack == NULL);

    free_file_state(fs);
    endwin();
    return 0;
}

static char *test_redo_strdup_failure() {
    initscr();
    FileState *fs = initialize_file_state("", 10, 80);
    mu_assert("fs allocated", fs != NULL);
    active_file = fs;
    text_win = fs->text_win;

    strcpy(fs->buffer.lines[0], "abc");
    fs->buffer.count = 1;
    char *old_text = strdup("abc");
    mu_assert("allocated", old_text != NULL);
    push(&fs->redo_stack, (Change){0, old_text, NULL});

    strdup_call_count = 0;
    strdup_fail_on = 1;
    allocation_fail_count = 0;
    redo(fs);
    strdup_fail_on = 0;

    mu_assert("allocation_failed called", allocation_fail_count == 1);
    mu_assert("undo stack empty", fs->undo_stack == NULL);

    free_file_state(fs);
    endwin();
    return 0;
}

static char *test_clear_text_buffer_frees_stacks() {
    initscr();
    FileState *fs = initialize_file_state("", 10, 80);
    mu_assert("fs allocated", fs != NULL);
    active_file = fs;
    text_win = fs->text_win;

    char *undo_text = strdup("old");
    char *redo_text = strdup("new");
    mu_assert("allocated", undo_text && redo_text);

    push(&fs->undo_stack, (Change){0, undo_text, NULL});
    push(&fs->redo_stack, (Change){0, NULL, redo_text});

    clear_text_buffer();

    mu_assert("undo stack cleared", fs->undo_stack == NULL);
    mu_assert("redo stack cleared", fs->redo_stack == NULL);

    free_file_state(fs);
    endwin();
    return 0;
}

static char *all_tests() {
    mu_run_test(test_undo_strdup_failure);
    mu_run_test(test_redo_strdup_failure);
    mu_run_test(test_clear_text_buffer_frees_stacks);
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

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

static char *test_backspace_undo_redo_character() {
    initscr();
    FileState *fs = initialize_file_state("", 5, 80);
    mu_assert("fs allocated", fs != NULL);
    active_file = fs;
    text_win = fs->text_win;

    strcpy(fs->buffer.lines[0], "abcde");
    fs->buffer.count = 1;
    fs->cursor_y = 1;
    fs->cursor_x = 4;

    handle_key_backspace(NULL, fs);

    mu_assert("char removed", strcmp(fs->buffer.lines[0], "abde") == 0);

    undo(fs);
    mu_assert("undo restored", strcmp(fs->buffer.lines[0], "abcde") == 0);
    redo(fs);
    mu_assert("redo applied", strcmp(fs->buffer.lines[0], "abde") == 0);

    free_file_state(fs);
    endwin();
    return 0;
}

static char *test_delete_undo_redo_character() {
    initscr();
    FileState *fs = initialize_file_state("", 5, 80);
    mu_assert("fs allocated", fs != NULL);
    active_file = fs;
    text_win = fs->text_win;

    strcpy(fs->buffer.lines[0], "abcde");
    fs->buffer.count = 1;
    fs->cursor_y = 1;
    fs->cursor_x = 3;

    handle_key_delete(NULL, fs);

    mu_assert("char deleted", strcmp(fs->buffer.lines[0], "abde") == 0);

    undo(fs);
    mu_assert("undo restored", strcmp(fs->buffer.lines[0], "abcde") == 0);
    redo(fs);
    mu_assert("redo applied", strcmp(fs->buffer.lines[0], "abde") == 0);

    free_file_state(fs);
    endwin();
    return 0;
}

static char *all_tests() {
    mu_run_test(test_undo_strdup_failure);
    mu_run_test(test_redo_strdup_failure);
    mu_run_test(test_clear_text_buffer_frees_stacks);
    mu_run_test(test_backspace_undo_redo_character);
    mu_run_test(test_delete_undo_redo_character);
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

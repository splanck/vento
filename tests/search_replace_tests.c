#include "minunit.h"
#include "search.h"
#include "files.h"
#include "editor.h"
#include "editor_state.h"
#include <ncurses.h>
#include <string.h>

int tests_run = 0;

static int stack_size(Node *head) {
    int count = 0;
    for (Node *n = head; n; n = n->next)
        count++;
    return count;
}

static char *test_replace_long_near_end() {
    initscr();
    FileState *fs = initialize_file_state("", 5, 10);
    mu_assert("fs allocated", fs != NULL);
    active_file = fs;
    text_win = fs->text_win;

    strcpy(fs->buffer.lines[0], "abcdefgh");
    fs->buffer.count = 1;

    replace_next_occurrence(fs, "gh", "0123456789ABCDE");

    int line_len = strlen(lb_get(&fs->buffer, 0));
    mu_assert("truncated line", strcmp(lb_get(&fs->buffer, 0), "abcdef012") == 0);
    mu_assert("cursor clamped", fs->cursor_x == line_len + 1);

    free_file_state(fs);
    endwin();
    return 0;
}

static char *test_replace_all_simple() {
    initscr();
    FileState *fs = initialize_file_state("", 5, 40);
    mu_assert("fs allocated", fs != NULL);
    active_file = fs;
    text_win = fs->text_win;

    strcpy(fs->buffer.lines[0], "foo bar foo");
    strcpy(fs->buffer.lines[1], "foo");
    strcpy(fs->buffer.lines[2], "no match");
    fs->buffer.count = 3;

    replace_all_occurrences(fs, "foo", "baz");

    mu_assert("line0 replaced", strcmp(lb_get(&fs->buffer, 0), "baz bar baz") == 0);
    mu_assert("line1 replaced", strcmp(lb_get(&fs->buffer, 1), "baz") == 0);
    mu_assert("line2 unchanged", strcmp(lb_get(&fs->buffer, 2), "no match") == 0);
    mu_assert("modified set", fs->modified == true);
    mu_assert("undo entries", stack_size(fs->undo_stack) == 2);

    free_file_state(fs);
    endwin();
    return 0;
}

static char *all_tests() {
    mu_run_test(test_replace_long_near_end);
    mu_run_test(test_replace_all_simple);
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

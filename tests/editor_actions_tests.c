#include "minunit.h"
#include "files.h"
#include "editor.h"
#include "editor_state.h"
#include <ncurses.h>
#include <string.h>

int tests_run = 0;

static char *test_insert_new_line_cursor_stays() {
    initscr();
    FileState *fs = initialize_file_state("", 10, 80);
    mu_assert("fs allocated", fs != NULL);
    active_file = fs;
    text_win = fs->text_win;

    EditorContext ctx = {0};
    sync_editor_context(&ctx);

    insert_new_line(&ctx, fs);

    mu_assert("cursor on new line", fs->cursor_y == 2);
    mu_assert("start_line unchanged", fs->start_line == 0);
    mu_assert("new line empty", strcmp(lb_get(&fs->buffer, 1), "") == 0);

    free_file_state(fs);
    endwin();
    return 0;
}

static char *all_tests() {
    mu_run_test(test_insert_new_line_cursor_stays);
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

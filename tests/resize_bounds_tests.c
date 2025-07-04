#include "minunit.h"
#include "input.h"
#include "editor_state.h"
#include <ncurses.h>

int tests_run = 0;

static char *test_narrow_resize_bounds() {
    show_line_numbers = 1;
    initscr();
    resizeterm(2, 3);
    FileState *fs = initialize_file_state("", 5, 8);
    mu_assert("fs allocated", fs != NULL);
    active_file = fs;
    text_win = fs->text_win;
    perform_resize();
    mu_assert("cursor_x >= 1", fs->cursor_x >= 1);
    mu_assert("scroll_x >= 0", fs->scroll_x >= 0);
    fs->cursor_x = fs->line_capacity;
    EditorContext ctx = {0};
    handle_key_right(&ctx, fs);
    mu_assert("scroll_x still >= 0", fs->scroll_x >= 0);
    endwin();
    free_file_state(fs);
    return 0;
}

static char *all_tests() {
    mu_run_test(test_narrow_resize_bounds);
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


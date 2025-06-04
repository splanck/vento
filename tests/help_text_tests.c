#include "minunit.h"
#include "editor.h"
#include "editor_state.h"
#include <ncurses.h>

extern int last_mvprintw_x;

int tests_run = 0;

static char *test_help_position_non_negative() {
    initscr();
    int lines = LINES;
    resizeterm(lines, 10);
    EditorContext ctx = {0};
    file_manager.count = 1;
    file_manager.active_index = 0;
    sync_editor_context(&ctx);
    last_mvprintw_x = -1;
    update_status_bar(&ctx, NULL);
    endwin();
    mu_assert("help col non-negative", last_mvprintw_x >= 0);
    mu_assert("help col zero when small", last_mvprintw_x == 0);
    return 0;
}

static char *all_tests() {
    mu_run_test(test_help_position_non_negative);
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

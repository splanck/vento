#include "minunit.h"
#include "ui.h"
#include "editor_state.h"
#include <ncurses.h>

extern int last_mvwin_y;
extern int last_mvwin_x;
extern int last_wresize_h;
extern int last_wresize_w;
void set_wgetch_sequence(const int *keys, int count);

int tests_run = 0;

static char *test_dialog_resize_clamped() {
    initscr();
    resizeterm(3, 7);
    const int keys[] = { KEY_RESIZE, 27 };
    set_wgetch_sequence(keys, 2);
    EditorContext ctx = {0};
    char path[1];
    show_open_file_dialog(&ctx, path, sizeof(path));
    mu_assert("x non-negative", last_mvwin_x >= 0);
    mu_assert("y non-negative", last_mvwin_y >= 0);
    mu_assert("x within", last_mvwin_x <= COLS - last_wresize_w);
    mu_assert("y within", last_mvwin_y <= LINES - last_wresize_h);
    endwin();
    return 0;
}

static char *all_tests() {
    mu_run_test(test_dialog_resize_clamped);
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

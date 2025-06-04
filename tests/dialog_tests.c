#include "minunit.h"
#include "dialog.h"
#include <ncurses.h>

int tests_run = 0;
extern int create_popup_fail;
extern int last_curs_set;

static char *test_dialog_open_failure() {
    initscr();
    last_curs_set = -2;
    create_popup_fail = 1;

    WINDOW *win = dialog_open(5, 10, "Fail");

    mu_assert("window null", win == NULL);
    mu_assert("cursor restored", last_curs_set == 1);

    endwin();
    return 0;
}

static char *all_tests() {
    mu_run_test(test_dialog_open_failure);
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

#include "minunit.h"
#include "dialog.h"
#include "ui.h"
#include "editor_state.h"
#include <ncurses.h>

void set_wgetch_sequence(const int *keys, int count);

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

static char *test_prompt_cancel_clears_buffer() {
    initscr();
    WINDOW *win = dialog_open(5, 20, "Prompt");
    const int keys[] = { 'a', 27 };
    set_wgetch_sequence(keys, 2);
    char buf[16] = "";
    int ok = dialog_prompt(win, 2, 2, buf, sizeof(buf));
    mu_assert("cancelled", ok == 0);
    mu_assert("buf empty", buf[0] == '\0');
    dialog_close(win);
    endwin();
    return 0;
}

static char *test_create_dialog_cancel_clears_buffer() {
    initscr();
    const int keys[] = {27};
    set_wgetch_sequence(keys, 1);
    EditorContext ctx = {0};
    char buf[16] = "abc";
    create_dialog(&ctx, "Test", buf, sizeof(buf));
    mu_assert("buf empty", buf[0] == '\0');
    endwin();
    return 0;
}

static char *test_show_find_dialog_cancel_clears_buffer() {
    initscr();
    const int keys[] = {27};
    set_wgetch_sequence(keys, 1);
    EditorContext ctx = {0};
    char buf[16] = "abc";
    int ok = show_find_dialog(&ctx, buf, sizeof(buf), NULL);
    mu_assert("cancelled", ok == 0);
    mu_assert("buf empty", buf[0] == '\0');
    endwin();
    return 0;
}

static char *all_tests() {
    mu_run_test(test_dialog_open_failure);
    mu_run_test(test_prompt_cancel_clears_buffer);
    mu_run_test(test_create_dialog_cancel_clears_buffer);
    mu_run_test(test_show_find_dialog_cancel_clears_buffer);
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

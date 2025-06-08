#include "minunit.h"
#include "editor.h"
#include "file_manager.h"
#include "file_ops.h"
#include "editor_state.h"
#include <ncurses.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

extern int last_status_count;
extern char last_mvprintw_buf[];
extern int fm_add_fail;

int tests_run = 0;

static char *test_menu_load_and_navigation() {
    initscr();
    fm_init(&file_manager);
    EditorContext ctx = {0};

    int res = load_file(NULL, NULL, "../README.md");
    mu_assert("first load", res == 0);
    sync_editor_context(&ctx);
    update_status_bar(&ctx, active_file);
    mu_assert("status after first", last_status_count == 1);
    FileState *first = active_file;

    res = load_file(NULL, NULL, "../LICENSE");
    mu_assert("second load", res == 0);
    sync_editor_context(&ctx);
    update_status_bar(&ctx, active_file);
    mu_assert("status after second", last_status_count == 2);
    FileState *second = active_file;

    mu_assert("two files", file_manager.count == 2);

    next_file(&ctx);
    mu_assert("next index", file_manager.active_index == 0);
    mu_assert("next pointer", active_file == first);
    mu_assert("status after next", last_status_count == 2);

    prev_file(&ctx);
    mu_assert("prev index", file_manager.active_index == 1);
    mu_assert("prev pointer", active_file == second);
    mu_assert("status after prev", last_status_count == 2);

    for (int i = file_manager.count - 1; i >= 0; i--) {
        fm_close(&file_manager, i);
    }
    endwin();
    return 0;
}

static char *test_load_error_message() {
    initscr();
    fm_init(&file_manager);
    last_mvprintw_buf[0] = '\0';
    int res = load_file(NULL, NULL, "/no/such/file");
    mu_assert("load failed", res < 0);
    mu_assert("error text", strstr(last_mvprintw_buf, strerror(ENOENT)) != NULL);
    if (file_manager.count > 0) {
        for (int i = file_manager.count - 1; i >= 0; i--)
            fm_close(&file_manager, i);
    }
    endwin();
    return 0;
}

static char *test_load_add_error_message() {
    initscr();
    fm_init(&file_manager);
    last_mvprintw_buf[0] = '\0';
    fm_add_fail = 1;
    errno = ENOMEM;
    int res = load_file(NULL, NULL, "../README.md");
    fm_add_fail = 0;
    mu_assert("load add failed", res < 0);
    mu_assert("error text add", strstr(last_mvprintw_buf, strerror(ENOMEM)) != NULL);
    if (file_manager.count > 0) {
        for (int i = file_manager.count - 1; i >= 0; i--)
            fm_close(&file_manager, i);
    }
    endwin();
    return 0;
}

static char *all_tests() {
    mu_run_test(test_menu_load_and_navigation);
    mu_run_test(test_load_error_message);
    mu_run_test(test_load_add_error_message);
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

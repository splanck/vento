#include "minunit.h"
#include "files.h"
#include "clipboard.h"
#include "editor.h"
#include "editor_state.h"
#include <ncurses.h>
#include <string.h>

int tests_run = 0;
extern int strdup_fail_on;
extern int strdup_call_count;

static char *test_paste_cursor_clamped() {
    initscr();
    FileState *fs = initialize_file_state("", 10, 80);
    mu_assert("fs allocated", fs != NULL);
    active_file = fs;
    text_win = fs->text_win;

    strcpy(fs->buffer.lines[0], "hello");
    strncpy(global_clipboard, "world", sizeof(global_clipboard) - 1);
    global_clipboard[sizeof(global_clipboard) - 1] = '\0';

    int cx = 10; /* beyond end of 'hello' */
    int cy = 1;
    paste_clipboard(fs, &cx, &cy);

    mu_assert("pasted at end", strcmp(fs->buffer.lines[0], "helloworld") == 0);
    mu_assert("cursor at end", cx == (int)strlen("helloworld") + 1);
    mu_assert("y unchanged", cy == 1);

    free_file_state(fs);
    endwin();
    return 0;
}

static char *test_strdup_failure_old_text() {
    initscr();
    FileState *fs = initialize_file_state("", 10, 80);
    mu_assert("fs allocated", fs != NULL);
    active_file = fs;
    text_win = fs->text_win;

    strcpy(fs->buffer.lines[0], "hello");
    strncpy(global_clipboard, "abc", sizeof(global_clipboard) - 1);
    global_clipboard[sizeof(global_clipboard) - 1] = '\0';

    int cx = 6;
    int cy = 1;
    strdup_call_count = 0;
    strdup_fail_on = 1; /* fail first strdup */
    paste_clipboard(fs, &cx, &cy);
    mu_assert("still at line", cy == 1);
    strdup_fail_on = 0;

    free_file_state(fs);
    endwin();
    return 0;
}

static char *test_strdup_failure_new_text() {
    initscr();
    FileState *fs = initialize_file_state("", 10, 80);
    mu_assert("fs allocated", fs != NULL);
    active_file = fs;
    text_win = fs->text_win;

    strcpy(fs->buffer.lines[0], "hello");
    strncpy(global_clipboard, "abc", sizeof(global_clipboard) - 1);
    global_clipboard[sizeof(global_clipboard) - 1] = '\0';

    int cx = 6;
    int cy = 1;
    strdup_call_count = 0;
    strdup_fail_on = 2; /* fail second strdup */
    paste_clipboard(fs, &cx, &cy);
    mu_assert("still at line", cy == 1);
    strdup_fail_on = 0;

    free_file_state(fs);
    endwin();
    return 0;
}

static char *all_tests() {
    mu_run_test(test_paste_cursor_clamped);
    mu_run_test(test_strdup_failure_old_text);
    mu_run_test(test_strdup_failure_new_text);
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

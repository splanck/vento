#include "minunit.h"
#include "files.h"
#include "file_manager.h"
#include "undo.h"
#include "editor.h"
#include <ncurses.h>
#include <string.h>

int tests_run = 0;

static char *test_cleanup_frees_stacks() {
    initscr();
    FileManager fm;
    fm_init(&fm);
    FileState *fs = initialize_file_state("", 2, 8);
    mu_assert("fs allocated", fs != NULL);

    char *u = strdup("u");
    char *r = strdup("r");
    mu_assert("allocated", u && r);
    push(&fs->undo_stack, (Change){0, u, NULL});
    push(&fs->redo_stack, (Change){0, NULL, r});

    fm_add(&fm, fs);

    cleanup_on_exit(&fm);

    mu_assert("fm cleared", fm.files == NULL && fm.count == 0 && fm.active_index == -1);

    endwin();
    return 0;
}

static char *all_tests() {
    mu_run_test(test_cleanup_frees_stacks);
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

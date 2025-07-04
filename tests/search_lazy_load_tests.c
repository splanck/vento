#include "minunit.h"
#include "files.h"
#include "search.h"
#include "editor.h"
#include "editor_state.h"
#include <ncurses.h>
#include <stdio.h>
#include <string.h>

int tests_run = 0;

static char *test_find_beyond_initial_load() {
    const char *path = "lazy_search.txt";
    FILE *f = fopen(path, "w");
    mu_assert("file created", f != NULL);
    for (int i = 0; i < 150; ++i) {
        if (i == 120)
            fprintf(f, "target\n");
        else
            fprintf(f, "line %d\n", i);
    }
    fclose(f);

    initscr();
    FileState *fs = initialize_file_state(path, 10, 80);
    mu_assert("fs allocated", fs != NULL);
    active_file = fs;
    text_win = fs->text_win;

    fs->fp = fopen(path, "r");
    mu_assert("fp open", fs->fp != NULL);
    fs->file_pos = 0;
    fs->file_complete = false;
    fs->buffer.count = 0;

    load_next_lines(fs, 50); /* load only part of the file */

    find_next_occurrence(fs, "target");

    mu_assert("found line", fs->match_start_y == 120);
    mu_assert("lines loaded", fs->buffer.count > 120);

    free_file_state(fs);
    endwin();
    remove(path);
    return 0;
}

static char *all_tests() {
    mu_run_test(test_find_beyond_initial_load);
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

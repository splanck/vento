#include "minunit.h"
#include "search.h"
#include "files.h"
#include "editor.h"
#include "editor_state.h"
#include <ncurses.h>
#include <string.h>

int tests_run = 0;

static char *test_replace_long_near_end() {
    initscr();
    FileState *fs = initialize_file_state("", 5, 10);
    mu_assert("fs allocated", fs != NULL);
    active_file = fs;
    text_win = fs->text_win;

    strcpy(fs->buffer.lines[0], "abcdefgh");
    fs->buffer.count = 1;

    replace_next_occurrence(fs, "gh", "0123456789ABCDE");

    int line_len = strlen(lb_get(&fs->buffer, 0));
    mu_assert("truncated line", strcmp(lb_get(&fs->buffer, 0), "abcdef012") == 0);
    mu_assert("cursor clamped", fs->cursor_x == line_len + 1);

    free_file_state(fs);
    endwin();
    return 0;
}

static char *test_replace_next_beyond_load() {
    const char *path = "lazy_replace_next.txt";
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

    load_next_lines(fs, 50);

    replace_next_occurrence(fs, "target", "found");

    mu_assert("replaced", strcmp(lb_get(&fs->buffer, 120), "found") == 0);
    mu_assert("lines loaded", fs->buffer.count > 120);

    free_file_state(fs);
    endwin();
    remove(path);
    return 0;
}

static char *test_replace_all_beyond_load() {
    const char *path = "lazy_replace_all.txt";
    FILE *f = fopen(path, "w");
    mu_assert("file created", f != NULL);
    for (int i = 0; i < 150; ++i) {
        if (i == 120 || i == 140)
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

    load_next_lines(fs, 50);

    replace_all_occurrences(fs, "target", "found");

    mu_assert("first replaced", strcmp(lb_get(&fs->buffer, 120), "found") == 0);
    mu_assert("second replaced", strcmp(lb_get(&fs->buffer, 140), "found") == 0);
    mu_assert("file loaded", fs->file_complete);

    free_file_state(fs);
    endwin();
    remove(path);
    return 0;
}

static char *all_tests() {
    mu_run_test(test_replace_long_near_end);
    mu_run_test(test_replace_next_beyond_load);
    mu_run_test(test_replace_all_beyond_load);
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

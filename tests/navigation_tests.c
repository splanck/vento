#include "minunit.h"
#include "editor.h"
#include "file_manager.h"
#include <stdio.h>

int tests_run = 0;

int __wrap_fm_switch(FileManager *fm, int index) {
    (void)fm; (void)index; fprintf(stderr, "fm_switch called\n"); return -1; }

static char *test_next_file_switch_failure() {
    fprintf(stderr, "test start\n");
    fm_init(&file_manager);
    FileState a = {0};
    FileState b = {0};
    a.cursor_x = 5; a.cursor_y = 6;
    fm_add(&file_manager, &a);
    fm_add(&file_manager, &b);
    file_manager.active_index = 0;
    active_file = &a;

    CursorPos pos = next_file(NULL);
    fprintf(stderr, "after next_file\n");
    mu_assert("index unchanged", file_manager.active_index == 0);
    mu_assert("pointer unchanged", active_file == &a);
    mu_assert("pos.x unchanged", pos.x == a.cursor_x);
    mu_assert("pos.y unchanged", pos.y == a.cursor_y);
    return 0;
}

static char *test_two_file_cycle() {
    fm_init(&file_manager);
    FileState a = {0};
    FileState b = {0};
    fm_add(&file_manager, &a);
    fm_add(&file_manager, &b);
    file_manager.active_index = 0;
    mu_assert("two files loaded", file_manager.count == 2);
    fm_switch(&file_manager, (file_manager.active_index + 1) % file_manager.count);
    mu_assert("switch to second", file_manager.active_index == 1);
    fm_switch(&file_manager, (file_manager.active_index + 1) % file_manager.count);
    mu_assert("cycle back to first", file_manager.active_index == 0);
    return 0;
}

static char * all_tests() {
    mu_run_test(test_next_file_switch_failure);
    mu_run_test(test_two_file_cycle);
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

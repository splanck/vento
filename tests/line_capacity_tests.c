#include "minunit.h"
#include "files.h"
#include <ncurses.h>

extern int calloc_fail_on;
extern int calloc_call_count;
extern int realloc_fail_on;
extern int realloc_call_count;

int tests_run = 0;

static char *test_allocation_failure_cleanup() {
    initscr();
    FileState *fs = initialize_file_state("", 2, 8);
    mu_assert("fs allocated", fs != NULL);

    calloc_call_count = 0;
    realloc_call_count = 0;
    calloc_fail_on = 2; /* fail second new line */
    realloc_fail_on = 2; /* fail shrink */

    int res = ensure_line_capacity(fs, 5);
    calloc_fail_on = 0;
    realloc_fail_on = 0;

    mu_assert("failure returned", res == -1);
    mu_assert("capacity unchanged", fs->buffer.capacity == 2);
    mu_assert("lines intact", fs->buffer.lines[0] != NULL && fs->buffer.lines[1] != NULL);

    res = ensure_line_capacity(fs, 4);
    mu_assert("second success", res == 0);
    mu_assert("capacity grown", fs->buffer.capacity >= 4);
    for (int i = 0; i < fs->buffer.capacity; ++i)
        mu_assert("line allocated", fs->buffer.lines[i] != NULL);

    free_file_state(fs);
    endwin();
    return 0;
}

static char *all_tests() {
    mu_run_test(test_allocation_failure_cleanup);
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

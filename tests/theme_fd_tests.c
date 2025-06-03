#include "minunit.h"
#include <ncurses.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

extern const char *select_theme(const char *current, WINDOW *parent);

static int count_fds(void) {
    DIR *d = opendir("/proc/self/fd");
    if (!d) return -1;
    int count = 0;
    struct dirent *ent;
    while ((ent = readdir(d)) != NULL) {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
            continue;
        count++;
    }
    closedir(d);
    return count;
}

int tests_run = 0;

static char *test_select_theme_fd_leak() {
    setenv("VENTO_THEME_DIR", "../themes", 1);
    initscr();
    int before = count_fds();
    const char *res = select_theme(NULL, NULL);
    int after = count_fds();
    if (res)
        free((void *)res);
    endwin();
    mu_assert("fd count same", before == after);
    return 0;
}

static char *all_tests() {
    mu_run_test(test_select_theme_fd_leak);
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

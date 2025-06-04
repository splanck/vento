#include "minunit.h"
#include "ui.h"
#include <limits.h>
#include <string.h>
#include <stdio.h>

static const char *dialog_input = NULL;

void __wrap_create_dialog(EditorContext *ctx, const char *message, char *output,
                          int max_input_len) {
    (void)ctx; (void)message;
    if (!output)
        return;
    if (dialog_input) {
        strncpy(output, dialog_input, max_input_len - 1);
        output[max_input_len - 1] = '\0';
    } else {
        output[0] = '\0';
    }
}

int tests_run = 0;

static char *test_reject_int_overflow() {
    initscr();
    EditorContext ctx = {0};
    char buf[64];
    long long big = (long long)INT_MAX + 1LL;
    snprintf(buf, sizeof(buf), "%lld", big);
    dialog_input = buf;
    int line = 0;
    int res = show_goto_dialog(&ctx, &line);
    dialog_input = NULL;
    endwin();
    mu_assert("overflow rejected", res == 0);
    return 0;
}

static char *all_tests() {
    mu_run_test(test_reject_int_overflow);
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

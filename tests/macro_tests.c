#include "minunit.h"
#include "file_manager.h"
#include "files.h"
#include "macro.h"
#include "editor_state.h"
#include <ncurses.h>
#include <string.h>

int tests_run = 0;

static char *test_simple_record_play() {
    initscr();
    fm_init(&file_manager);

    FileState *fs = initialize_file_state("", 10, 80);
    mu_assert("fs allocated", fs != NULL);
    fm_add(&file_manager, fs);
    active_file = fs;
    text_win = fs->text_win;

    EditorContext ctx = {0};
    sync_editor_context(&ctx);

    Macro m = {0};
    macro_start(&m);
    macro_record_key(&m, L'a');
    macro_record_key(&m, L'b');
    macro_record_key(&m, L'c');
    macro_stop(&m);

    macro_play(&m, &ctx, fs);

    mu_assert("buffer contains abc", strcmp(lb_get(&fs->buffer, 0), "abc") == 0);

    free_file_state(fs);
    endwin();
    return 0;
}

static char *all_tests() {
    mu_run_test(test_simple_record_play);
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

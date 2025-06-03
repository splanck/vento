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

    Macro *m1 = macro_create("one");
    mu_assert("m1", m1 != NULL);
    macro_start(m1);
    macro_record_key(m1, L'a');
    macro_record_key(m1, L'b');
    macro_record_key(m1, L'c');
    macro_stop(m1);

    Macro *m2 = macro_create("two");
    mu_assert("m2", m2 != NULL);
    macro_start(m2);
    macro_record_key(m2, L'1');
    macro_record_key(m2, L'2');
    macro_record_key(m2, L'3');
    macro_stop(m2);

    macro_play_times(m1, &ctx, fs, 1);
    macro_play_times(m2, &ctx, fs, 1);

    mu_assert("buffer contains abc123", strcmp(lb_get(&fs->buffer, 0), "abc123") == 0);

    macro_delete("one");
    macro_delete("two");

    free_file_state(fs);
    endwin();
    return 0;
}

static char *test_create_delete_api() {
    int before = macro_count();
    Macro *m = macro_create("temp");
    mu_assert("macro created", m != NULL);
    mu_assert("count increased", macro_count() == before + 1);
    macro_delete("temp");
    mu_assert("count restored", macro_count() == before);
    return 0;
}

static char *all_tests() {
    mu_run_test(test_simple_record_play);
    mu_run_test(test_create_delete_api);
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

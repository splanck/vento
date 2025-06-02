#include "minunit.h"
#include "editor.h"
#include "file_manager.h"
#include "file_ops.h"
#include "editor_state.h"
#include <ncurses.h>
#include <stdio.h>
#include <string.h>

int tests_run = 0;

extern int fm_switch_fail;
extern int fm_add_fail;
extern int last_status_count;

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

    fm_switch_fail = 1;

    CursorPos pos = next_file(NULL);
    fm_switch_fail = 0;
    fprintf(stderr, "after next_file\n");
    mu_assert("index unchanged", file_manager.active_index == 0);
    mu_assert("pointer unchanged", active_file == &a);
    mu_assert("pos.x unchanged", pos.x == a.cursor_x);
    mu_assert("pos.y unchanged", pos.y == a.cursor_y);
    return 0;
}

static char *test_prev_file_switch_failure() {
    fm_init(&file_manager);
    FileState a = {0};
    FileState b = {0};
    b.cursor_x = 3; b.cursor_y = 4;
    fm_add(&file_manager, &a);
    fm_add(&file_manager, &b);
    file_manager.active_index = 1;
    active_file = &b;

    fm_switch_fail = 1;
    CursorPos pos = prev_file(NULL);
    fm_switch_fail = 0;

    mu_assert("index unchanged", file_manager.active_index == 1);
    mu_assert("pointer unchanged", active_file == &b);
    mu_assert("pos.x unchanged", pos.x == b.cursor_x);
    mu_assert("pos.y unchanged", pos.y == b.cursor_y);
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

static char *test_duplicate_open_same_file() {
    initscr();
    fm_init(&file_manager);
    int res = load_file(NULL, NULL, "../README.md");
    mu_assert("first load", res == 0);
    mu_assert("one file after first", file_manager.count == 1);
    FileState *first = file_manager.files[0];
    res = load_file(NULL, NULL, ".././README.md");
    mu_assert("still one file", file_manager.count == 1);
    mu_assert("same pointer", file_manager.files[0] == first);
    fm_close(&file_manager, 0);
    endwin();
    return 0;
}

static char *test_new_file_add_failure() {
    initscr();
    fm_init(&file_manager);
    FileState prev = {0};
    prev.text_win = newwin(1,1,0,0);
    fm_add(&file_manager, &prev);
    file_manager.active_index = 0;
    active_file = &prev;
    text_win = prev.text_win;
    EditorContext ctx = {0};
    sync_editor_context(&ctx);

    fm_add_fail = 1;
    new_file(&ctx, NULL);
    fm_add_fail = 0;

    mu_assert("add failure keeps active index", file_manager.active_index == 0);
    mu_assert("add failure keeps pointer", active_file == &prev);
    mu_assert("ctx updated", ctx.active_file == active_file);
    mu_assert("ctx index", ctx.file_manager.active_index == file_manager.active_index);
    delwin(prev.text_win);
    endwin();
    return 0;
}

static char *test_new_file_switch_failure() {
    initscr();
    fm_init(&file_manager);
    FileState prev = {0};
    prev.text_win = newwin(1,1,0,0);
    fm_add(&file_manager, &prev);
    file_manager.active_index = 0;
    active_file = &prev;
    text_win = prev.text_win;
    EditorContext ctx = {0};
    sync_editor_context(&ctx);

    fm_switch_fail = 1;
    new_file(&ctx, NULL);
    fm_switch_fail = 0;

    mu_assert("switch failure keeps index", file_manager.active_index == 0);
    mu_assert("switch failure keeps pointer", active_file == &prev);
    mu_assert("ctx updated after switch fail", ctx.active_file == active_file);
    mu_assert("ctx index after switch fail", ctx.file_manager.active_index == file_manager.active_index);
    delwin(prev.text_win);
    endwin();
    return 0;
}

static char *test_untitled_ignored_in_cycle() {
    fm_init(&file_manager);
    FileState untitled = {0};
    FileState first = {0};
    FileState second = {0};
    strcpy(first.filename, "file1");
    strcpy(second.filename, "file2");
    fm_add(&file_manager, &untitled);
    fm_add(&file_manager, &first);
    fm_add(&file_manager, &second);
    file_manager.active_index = 1;
    active_file = &first;

    next_file(NULL);
    mu_assert("next skips untitled", file_manager.active_index == 2 && active_file == &second);
    next_file(NULL);
    mu_assert("cycle only real files", file_manager.active_index == 1 && active_file == &first);

    file_manager.active_index = 0;
    active_file = &untitled;
    next_file(NULL);
    mu_assert("next from untitled", file_manager.active_index == 1 && active_file == &first);

    file_manager.active_index = 1;
    active_file = &first;
    prev_file(NULL);
    mu_assert("prev skips untitled", file_manager.active_index == 2 && active_file == &second);
    prev_file(NULL);
    mu_assert("cycle back", file_manager.active_index == 1 && active_file == &first);

    file_manager.active_index = 0;
    active_file = &untitled;
    prev_file(NULL);
    mu_assert("prev from untitled", file_manager.active_index == 2 && active_file == &second);

    return 0;
}

static char *test_status_bar_sync_on_load() {
    initscr();
    fm_init(&file_manager);
    EditorContext ctx = {0};
    sync_editor_context(&ctx);

    int res = load_file(&ctx, NULL, "../README.md");
    mu_assert("first load", res == 0);
    mu_assert("status count after first", last_status_count == 1);

    res = load_file(&ctx, NULL, "../LICENSE");
    mu_assert("second load", res == 0);
    mu_assert("status count after second", last_status_count == 2);

    for (int i = file_manager.count - 1; i >= 0; i--) {
        fm_close(&file_manager, i);
    }
    endwin();
    return 0;
}

static char * all_tests() {
    mu_run_test(test_next_file_switch_failure);
    mu_run_test(test_prev_file_switch_failure);
    mu_run_test(test_two_file_cycle);
    mu_run_test(test_duplicate_open_same_file);
    mu_run_test(test_new_file_add_failure);
    mu_run_test(test_new_file_switch_failure);
    mu_run_test(test_untitled_ignored_in_cycle);
    mu_run_test(test_status_bar_sync_on_load);
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

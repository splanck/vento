#include "minunit.h"
#include "files.h"
#include "syntax.h"
#include "editor.h"
#include "editor_state.h"
#include <ncurses.h>
#include <string.h>

int tests_run = 0;

static char *test_json_highlighting_runs() {
    initscr();
    FileState *fs = initialize_file_state("tests/sample.json", 10, 80);
    mu_assert("fs allocated", fs != NULL);
    active_file = fs;
    text_win = fs->text_win;
    fs->syntax_mode = JSON_SYNTAX;

    /* Load sample file lines */
    const char *lines[] = {
        "{",
        "  \"num\": 123,",
        "  \"flag\": true,",
        "  \"vals\": [null, false]",
        "}"
    };
    int line_count = 5;
    for (int i = 0; i < line_count; i++) {
        strcpy(fs->buffer.lines[i], lines[i]);
    }
    fs->buffer.count = line_count;

    for (int i = 0; i < fs->buffer.count; i++) {
        apply_syntax_highlighting(fs, fs->text_win, fs->buffer.lines[i], i + 1);
    }

    const SyntaxDef *def = syntax_get(JSON_SYNTAX);
    mu_assert("patterns compiled", def && def->patterns[0].compiled);

    free_file_state(fs);
    endwin();
    return 0;
}

static char *all_tests() {
    mu_run_test(test_json_highlighting_runs);
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

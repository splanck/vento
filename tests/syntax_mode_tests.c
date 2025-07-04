#include "minunit.h"
#include "file_ops.h"
#include "syntax.h"

int tests_run = 0;

static char *test_uppercase_extensions() {
    mu_assert(".PY", set_syntax_mode("foo.PY") == PYTHON_SYNTAX);
    mu_assert(".HTML", set_syntax_mode("foo.HTML") == HTML_SYNTAX);
    mu_assert(".HTM", set_syntax_mode("foo.HTM") == HTML_SYNTAX);
    mu_assert(".C", set_syntax_mode("foo.C") == C_SYNTAX);
    mu_assert(".H", set_syntax_mode("foo.H") == C_SYNTAX);
    mu_assert(".JS", set_syntax_mode("foo.JS") == JS_SYNTAX);
    mu_assert(".CSS", set_syntax_mode("foo.CSS") == CSS_SYNTAX);
    mu_assert(".CS", set_syntax_mode("foo.CS") == CSHARP_SYNTAX);
    mu_assert(".JSON", set_syntax_mode("foo.JSON") == JSON_SYNTAX);
    return 0;
}

static char *all_tests() {
    mu_run_test(test_uppercase_extensions);
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

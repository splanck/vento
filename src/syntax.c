#include <ncurses.h>
#include "syntax.h"
#include "editor.h"
#include "files.h"

void apply_syntax_highlighting(FileState *fs, WINDOW *win, const char *line, int y) {
    switch (fs->syntax_mode) {
        case C_SYNTAX:
            highlight_c_syntax(fs, win, line, y);
            break;
        case HTML_SYNTAX:
            highlight_html_syntax(fs, win, line, y);
            break;
        case PYTHON_SYNTAX:
            highlight_python_syntax(fs, win, line, y);
            break;
        case CSHARP_SYNTAX:
            highlight_csharp_syntax(fs, win, line, y);
            break;
        case JS_SYNTAX:
            highlight_js_syntax(fs, win, line, y);
            break;
        case CSS_SYNTAX:
            highlight_css_syntax(fs, win, line, y);
            break;
        case SHELL_SYNTAX:
            highlight_shell_syntax(fs, win, line, y);
            break;
        default:
            highlight_no_syntax(win, line, y);
            break;
    }
}

/* Cleanup compiled regular expressions for all syntax modules */
void syntax_cleanup(void) {
    /* C syntax */
    extern SyntaxRegex C_PATTERNS[];
    extern const int C_PATTERNS_COUNT;
    extern int c_regex_compiled;
    if (c_regex_compiled) {
        free_regex_set(C_PATTERNS, C_PATTERNS_COUNT);
    }

    /* C# syntax */
    extern SyntaxRegex CSHARP_PATTERNS[];
    extern const int CSHARP_PATTERNS_COUNT;
    extern int csharp_regex_compiled;
    if (csharp_regex_compiled) {
        free_regex_set(CSHARP_PATTERNS, CSHARP_PATTERNS_COUNT);
    }

    /* JavaScript syntax */
    extern SyntaxRegex JS_PATTERNS[];
    extern const int JS_PATTERNS_COUNT;
    extern int js_regex_compiled;
    if (js_regex_compiled) {
        free_regex_set(JS_PATTERNS, JS_PATTERNS_COUNT);
    }

    /* CSS syntax */
    extern SyntaxRegex CSS_PATTERNS[];
    extern const int CSS_PATTERNS_COUNT;
    extern int css_regex_compiled;
    if (css_regex_compiled) {
        free_regex_set(CSS_PATTERNS, CSS_PATTERNS_COUNT);
    }

    /* Python syntax */
    extern SyntaxRegex PYTHON_PATTERNS[];
    extern const int PYTHON_PATTERNS_COUNT;
    extern int python_regex_compiled;
    if (python_regex_compiled) {
        free_regex_set(PYTHON_PATTERNS, PYTHON_PATTERNS_COUNT);
    }

    /* Shell syntax */
    extern SyntaxRegex SHELL_PATTERNS[];
    extern const int SHELL_PATTERNS_COUNT;
    extern int shell_regex_compiled;
    if (shell_regex_compiled) {
        free_regex_set(SHELL_PATTERNS, SHELL_PATTERNS_COUNT);
    }
}

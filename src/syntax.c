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
            highlight_html_syntax(win, line, y);
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

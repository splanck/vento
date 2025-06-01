#include <ncurses.h>
#include "syntax.h"
#include "editor.h"
#include "files.h"

void apply_syntax_highlighting(FileState *fs, WINDOW *win, const char *line, int y) {
    const SyntaxDef *def = syntax_get(fs->syntax_mode);
    if (def) {
        highlight_by_patterns(fs, win, line, y, def);
    } else if (fs->syntax_mode == HTML_SYNTAX) {
        highlight_html_syntax(fs, win, line, y);
    } else {
        highlight_no_syntax(win, line, y);
    }
}

/* Cleanup compiled regular expressions for all registered syntax modules */
void syntax_cleanup(void) {
    for (int mode = 0; mode < 8; mode++) {
        const SyntaxDef *def = syntax_get(mode);
        if (def)
            free_regex_set(def->patterns, def->count);
    }
}

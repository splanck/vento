/*
 * syntax.c
 * --------
 * Dispatches to the registered syntax highlighter for the active file
 * and provides cleanup helpers to free any compiled regular expressions
 * when the application exits.
 */
#include <ncurses.h>
#include "syntax.h"
#include "editor.h"
#include "files.h"

/*
 * apply_syntax_highlighting(fs, win, line, y)
 * ------------------------------------------
 * fs   - active FileState describing the file being rendered
 * win  - ncurses window where output should be drawn
 * line - contents of the line to highlight
 * y    - vertical screen position for drawing
 *
 * Chooses the appropriate highlighting strategy by checking the
 * registered SyntaxDef for the file's mode. If no definition exists
 * but HTML mode is selected, the HTML highlighter is used; otherwise
 * the line is printed as plain text.
 */
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

/*
 * syntax_cleanup()
 * ---------------
 * Iterates over each registered syntax mode and frees any compiled
 * regular expressions associated with its patterns.  This should be
 * called during application shutdown to release regex resources.
 */
void syntax_cleanup(void) {
    for (int mode = 0; mode < 8; mode++) {
        const SyntaxDef *def = syntax_get(mode);
        if (def)
            free_regex_set(def->patterns, def->count);
    }
}

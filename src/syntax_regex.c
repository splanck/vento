/*
 * Regex based syntax highlighting utilities
 * -----------------------------------------
 * SyntaxRegex holds a pattern and an attribute along with storage for a
 * compiled regex_t.  compile_regex_set() walks an array of these structures,
 * calling regcomp() on each pattern and marking entries as compiled.
 * highlight_regex_line() then scans a line of text, running regexec() for each
 * compiled pattern at the current offset and printing matches using the
 * pattern's attribute.
 */
#include "syntax.h"
#include <string.h>

/*
 * compile_regex_set
 * -----------------
 * set   - array of SyntaxRegex entries to compile
 * count - number of entries in the array
 *
 * Each entry with a pattern string is passed to regcomp() using REG_EXTENDED.
 * On success the compiled field is set to 1; otherwise it is set to 0.
 * Returns the number of patterns successfully compiled.
 */
int compile_regex_set(SyntaxRegex *set, int count) {
    int ok = 0;
    for (int i = 0; i < count; i++) {
        if (!set[i].pattern) {
            set[i].compiled = 0;
            continue;
        }
        if (regcomp(&set[i].regex, set[i].pattern, REG_EXTENDED)) {
            set[i].compiled = 0;
        } else {
            set[i].compiled = 1;
            ok++;
        }
    }
    return ok;
}

/*
 * free_regex_set
 * --------------
 * set   - array of SyntaxRegex entries previously compiled
 * count - number of entries in the array
 *
 * Calls regfree() on each compiled entry and clears the compiled flag.
 * Pattern strings themselves are left untouched.
 */
void free_regex_set(SyntaxRegex *set, int count) {
    for (int i = 0; i < count; i++) {
        if (set[i].compiled) {
            regfree(&set[i].regex);
            set[i].compiled = 0;
        }
    }
}

/*
 * highlight_regex_line
 * --------------------
 * win      - window to draw into
 * y        - row position in the window
 * x_start  - starting column
 * line     - text to highlight
 * patterns - compiled SyntaxRegex array
 * count    - number of patterns in the array
 *
 * Scans the line from left to right and at each position tries each compiled
 * regular expression. When a pattern matches at the current offset, the
 * matching text is printed with its attribute. Otherwise the next character is
 * printed with default background colors.
 */
void highlight_regex_line(WINDOW *win, int y, int x_start, const char *line,
                          SyntaxRegex *patterns, int count) {
    int x = x_start;
    wattrset(win, COLOR_PAIR(SYNTAX_BG));
    const char *p = line;
    while (*p) {
        int matched = 0;
        for (int i = 0; i < count; i++) {
            if (!patterns[i].compiled)
                continue;
            regmatch_t m;
            if (regexec(&patterns[i].regex, p, 1, &m, 0) == 0 && m.rm_so == 0) {
                wattron(win, patterns[i].attr);
                mvwprintw(win, y, x, "%.*s", (int)m.rm_eo, p);
                wattroff(win, patterns[i].attr);
                x += m.rm_eo;
                p += m.rm_eo;
                matched = 1;
                break;
            }
        }
        if (!matched) {
            mvwprintw(win, y, x++, "%c", *p);
            p++;
        }
    }
}


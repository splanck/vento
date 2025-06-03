/*
 * HTML syntax highlighting routines.
 *
 * Lines are scanned character by character to colorize tags, attributes and
 * comments. When <script> or <style> tags are encountered, the highlighter
 * enters a nested mode by setting FileState->nested_mode so that subsequent
 * lines are delegated to the JavaScript or CSS highlighter until the
 * matching closing tag is processed.
 */
#include <ncurses.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "syntax.h"
#include "files.h"

enum { NESTED_NONE = 0, NESTED_JS = 1, NESTED_CSS = 2 };

/*
 * Print a single character with the given attributes at position (*x, y).
 * The column index pointed to by x is incremented after printing.
 * This helper does not modify FileState->nested_mode.
 */
void print_char_with_attr(WINDOW *win, int y, int *x, char c, int attr) {
    wattron(win, attr);
    mvwprintw(win, y, (*x)++, "%c", c);
    wattroff(win, attr);
}

/*
 * Highlight an HTML comment starting at index *i.
 *
 * win  - window used for output.
 * line - current line of text.
 * i    - pointer to the position in line, updated past the closing -->.
 * y    - screen row.
 * x    - pointer to column index updated as characters are printed.
 *
 * Does not modify FileState->nested_mode.
 */
void handle_html_comment(WINDOW *win, const char *line, int *i, int y, int *x) {
    int len = strlen(line);
    print_char_with_attr(win, y, x, line[(*i)++], COLOR_PAIR(SYNTAX_TYPE) | A_BOLD);
    while (*i < len && !(*i + 1 < len && *i + 2 < len &&
                         line[*i] == '-' && line[*i + 1] == '-' && line[*i + 2] == '>')) {
        print_char_with_attr(win, y, x, line[(*i)++], COLOR_PAIR(SYNTAX_TYPE) | A_BOLD);
    }
    if (*i < len) {
        print_char_with_attr(win, y, x, line[(*i)++], COLOR_PAIR(SYNTAX_TYPE) | A_BOLD);
        (*i) += 2;
    }
}

/*
 * Highlight an HTML tag and its attributes.
 *
 * win  - window used for output.
 * line - current line being processed.
 * i    - pointer to the index of the <, updated past the matching >.
 * y    - screen row.
 * x    - pointer to column index updated as characters are printed.
 *
 * This function does not alter FileState->nested_mode; the caller
 * decides whether entering or leaving a tag changes nesting.
 */

void handle_html_tag(WINDOW *win, const char *line, int *i, int y, int *x) {
    int len = strlen(line);
    bool inAttribute = false;
    while (*i < len && line[*i] != '>') {
        if (line[*i] == ' ' && !inAttribute) {
            inAttribute = true;
            print_char_with_attr(win, y, x, line[(*i)++], COLOR_PAIR(SYNTAX_KEYWORD) | A_BOLD);
        } else if (line[*i] == '=') {
            print_char_with_attr(win, y, x, line[(*i)++], COLOR_PAIR(SYNTAX_COMMENT) | A_BOLD);
            if (line[*i] == '"' || line[*i] == '\'') {
                char quoteType = line[(*i)++];
                print_char_with_attr(win, y, x, quoteType, COLOR_PAIR(SYNTAX_STRING) | A_BOLD);
                while (*i < len && line[*i] != quoteType) {
                    print_char_with_attr(win, y, x, line[(*i)++], COLOR_PAIR(SYNTAX_STRING) | A_BOLD);
                }
                if (*i < len) {
                    print_char_with_attr(win, y, x, line[(*i)++], COLOR_PAIR(SYNTAX_STRING) | A_BOLD);
                }
            }
        } else {
            int color = inAttribute ? COLOR_PAIR(SYNTAX_COMMENT) | A_BOLD : COLOR_PAIR(SYNTAX_KEYWORD) | A_BOLD;
            print_char_with_attr(win, y, x, line[(*i)++], color);
        }
    }
    if (*i < len) {
        print_char_with_attr(win, y, x, line[(*i)++], COLOR_PAIR(SYNTAX_KEYWORD) | A_BOLD);
    }
}

/*
 * Apply HTML highlighting to a single line.
 *
 * fs   - FileState containing nested_mode used for nested <script> and <style> blocks.
 * win  - window to render into.
 * line - text of the line.
 * y    - screen row for output.
 *
 * Sets fs->nested_mode to NESTED_JS or NESTED_CSS when opening tags are
 * encountered and resets it back to NESTED_NONE on their closing tags.
 */

void highlight_html_syntax(FileState *fs, WINDOW *win, const char *line, int y) {
    if (!win || !line) return;

    /* If inside a nested script or style block, delegate to the proper
       highlighter unless we encounter the closing tag */
    const char *trim = line;
    while (isspace((unsigned char)*trim)) trim++;

    if (fs->nested_mode == NESTED_JS) {
        if (strncmp(trim, "</script>", 9) == 0) {
            int x = 1, i = 0;
            wattrset(win, COLOR_PAIR(SYNTAX_BG));
            while (i < (trim - line))
                mvwprintw(win, y, x++, "%c", line[i++]);
            handle_html_tag(win, line, &i, y, &x);
            while (line[i]) mvwprintw(win, y, x++, "%c", line[i++]);
            fs->nested_mode = NESTED_NONE;
        } else {
            const SyntaxDef *def = syntax_get(JS_SYNTAX);
            highlight_by_patterns(fs, win, line, y, def);
        }
        return;
    } else if (fs->nested_mode == NESTED_CSS) {
        if (strncmp(trim, "</style>", 8) == 0) {
            int x = 1, i = 0;
            wattrset(win, COLOR_PAIR(SYNTAX_BG));
            while (i < (trim - line))
                mvwprintw(win, y, x++, "%c", line[i++]);
            handle_html_tag(win, line, &i, y, &x);
            while (line[i]) mvwprintw(win, y, x++, "%c", line[i++]);
            fs->nested_mode = NESTED_NONE;
        } else {
            const SyntaxDef *def = syntax_get(CSS_SYNTAX);
            highlight_by_patterns(fs, win, line, y, def);
        }
        return;
    }

    wattrset(win, COLOR_PAIR(SYNTAX_BG));
    int x = 1;
    int i = 0;
    int len = strlen(line);
    while (i < len) {
        if (isspace((unsigned char)line[i])) {
            mvwprintw(win, y, x++, "%c", line[i++]);
        } else if (line[i] == '<') {
            if (strncmp(&line[i], "<!--", 4) == 0) {
                handle_html_comment(win, line, &i, y, &x);
            } else {
                bool is_script = strncmp(&line[i], "<script", 7) == 0;
                bool is_style = strncmp(&line[i], "<style", 6) == 0;
                handle_html_tag(win, line, &i, y, &x);
                if (is_script)
                    fs->nested_mode = NESTED_JS;
                else if (is_style)
                    fs->nested_mode = NESTED_CSS;
            }
        } else {
            mvwprintw(win, y, x++, "%c", line[i++]);
        }
    }
}

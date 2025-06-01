#include <ncurses.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "syntax.h"
#include "files.h"

enum { NESTED_NONE = 0, NESTED_JS = 1, NESTED_CSS = 2 };

void print_char_with_attr(WINDOW *win, int y, int *x, char c, int attr) {
    wattron(win, attr);
    mvwprintw(win, y, (*x)++, "%c", c);
    wattroff(win, attr);
}

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

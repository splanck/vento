#include <ncurses.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "syntax.h"

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

void highlight_html_syntax(WINDOW *win, const char *line, int y) {
    if (!win || !line) return;
    wattrset(win, COLOR_PAIR(SYNTAX_BG));
    int x = 0;
    int i = 0;
    int len = strlen(line);
    while (i < len) {
        if (isspace((unsigned char)line[i])) {
            mvwprintw(win, y, x++, "%c", line[i++]);
        } else if (line[i] == '<') {
            if (strncmp(&line[i], "<!--", 4) == 0) {
                handle_html_comment(win, line, &i, y, &x);
            } else {
                handle_html_tag(win, line, &i, y, &x);
            }
        } else {
            mvwprintw(win, y, x++, "%c", line[i++]);
        }
    }
}

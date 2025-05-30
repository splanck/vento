#include <ncurses.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "syntax.h"
#include "files.h"

static const char *SHELL_KEYWORDS[] = {
    "if", "then", "else", "fi", "for", "in", "do", "done",
    "while", "case", "esac", "function"
};
static const int SHELL_KEYWORDS_COUNT = sizeof(SHELL_KEYWORDS) / sizeof(SHELL_KEYWORDS[0]);

void highlight_shell_syntax(FileState *fs, WINDOW *win, const char *line, int y) {
    (void)fs;
    wattrset(win, COLOR_PAIR(SYNTAX_BG));
    int len = strlen(line);
    int i = 0;
    int x = 1;
    char word[256];

    while (i < len) {
        if (line[i] == '#') {
            wattron(win, COLOR_PAIR(SYNTAX_COMMENT) | A_BOLD);
            mvwprintw(win, y, x, "%s", &line[i]);
            wattroff(win, COLOR_PAIR(SYNTAX_COMMENT) | A_BOLD);
            break;
        } else if (line[i] == '"' || line[i] == '\'') {
            bool closed;
            int l = scan_string(line, i, line[i], &closed);
            wattron(win, COLOR_PAIR(SYNTAX_STRING) | A_BOLD);
            mvwprintw(win, y, x, "%.*s", l, &line[i]);
            wattroff(win, COLOR_PAIR(SYNTAX_STRING) | A_BOLD);
            x += l;
            i += l;
        } else if (isdigit((unsigned char)line[i])) {
            int l = scan_number(line, i);
            wattron(win, COLOR_PAIR(SYNTAX_TYPE) | A_BOLD);
            mvwprintw(win, y, x, "%.*s", l, &line[i]);
            wattroff(win, COLOR_PAIR(SYNTAX_TYPE) | A_BOLD);
            x += l;
            i += l;
        } else if (isalpha((unsigned char)line[i]) || line[i] == '_') {
            int wlen = scan_identifier(line, i);
            if (wlen >= (int)sizeof(word))
                wlen = (int)sizeof(word) - 1;
            strncpy(word, &line[i], wlen);
            word[wlen] = '\0';
            int is_kw = 0;
            for (int k = 0; k < SHELL_KEYWORDS_COUNT; k++) {
                if (strcmp(word, SHELL_KEYWORDS[k]) == 0) {
                    wattron(win, COLOR_PAIR(SYNTAX_KEYWORD) | A_BOLD);
                    mvwprintw(win, y, x, "%s", word);
                    wattroff(win, COLOR_PAIR(SYNTAX_KEYWORD) | A_BOLD);
                    is_kw = 1;
                    break;
                }
            }
            if (!is_kw) {
                mvwprintw(win, y, x, "%s", word);
            }
            x += wlen;
            i += wlen;
        } else {
            mvwprintw(win, y, x++, "%c", line[i++]);
        }
    }
}

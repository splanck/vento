#include <ncurses.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "syntax.h"
#include "files.h"

#define PYTHON_KEYWORDS_COUNT 35
static const char *PYTHON_KEYWORDS[PYTHON_KEYWORDS_COUNT] = {
    "False", "None", "True", "and", "as", "assert", "async", "await", "break",
    "class", "continue", "def", "del", "elif", "else", "except", "finally",
    "for", "from", "global", "if", "import", "in", "is", "lambda", "nonlocal",
    "not", "or", "pass", "raise", "return", "try", "while", "with", "yield"
};

void highlight_python_syntax(FileState *fs, WINDOW *win, const char *line, int y) {
    wattrset(win, COLOR_PAIR(SYNTAX_BG));
    int length = strlen(line);
    int i = 0;
    int x = 1;

    while (i < length) {
        if (fs->in_multiline_string) {
            bool closed;
            int len = scan_multiline_string(line, i, fs->string_delim, &closed);
            wattron(win, COLOR_PAIR(SYNTAX_STRING) | A_BOLD);
            mvwprintw(win, y, x, "%.*s", len, &line[i]);
            wattroff(win, COLOR_PAIR(SYNTAX_STRING) | A_BOLD);
            fs->in_multiline_string = !closed;
            x += len;
            i += len;
            continue;
        }

        if (isalpha((unsigned char)line[i]) || line[i] == '_') {
            int len = scan_identifier(line, i);
            char word[256];
            if (len >= (int)sizeof(word))
                len = (int)sizeof(word) - 1;
            strncpy(word, &line[i], len);
            word[len] = '\0';

            int is_keyword = 0;
            for (int j = 0; j < PYTHON_KEYWORDS_COUNT; j++) {
                if (strcmp(word, PYTHON_KEYWORDS[j]) == 0) {
                    wattron(win, COLOR_PAIR(SYNTAX_KEYWORD) | A_BOLD);
                    mvwprintw(win, y, x, "%s", word);
                    wattroff(win, COLOR_PAIR(SYNTAX_KEYWORD) | A_BOLD);
                    is_keyword = 1;
                    break;
                }
            }
            if (!is_keyword) {
                mvwprintw(win, y, x, "%s", word);
            }
            x += len;
            i += len;
        } else if (isdigit((unsigned char)line[i])) {
            int len = scan_number(line, i);
            wattron(win, COLOR_PAIR(SYNTAX_TYPE) | A_BOLD);
            mvwprintw(win, y, x, "%.*s", len, &line[i]);
            wattroff(win, COLOR_PAIR(SYNTAX_TYPE) | A_BOLD);
            x += len;
            i += len;
        } else if (line[i] == '#') {
            wattron(win, COLOR_PAIR(SYNTAX_COMMENT) | A_BOLD);
            mvwprintw(win, y, x, "%s", &line[i]);
            wattroff(win, COLOR_PAIR(SYNTAX_COMMENT) | A_BOLD);
            break;
        } else if ((line[i] == '"' || line[i] == '\'' ) &&
                   i + 2 < length && line[i + 1] == line[i] && line[i + 2] == line[i]) {
            fs->in_multiline_string = true;
            fs->string_delim = line[i];
            bool closed;
            int len = scan_multiline_string(line, i, fs->string_delim, &closed);
            wattron(win, COLOR_PAIR(SYNTAX_STRING) | A_BOLD);
            mvwprintw(win, y, x, "%.*s", len, &line[i]);
            wattroff(win, COLOR_PAIR(SYNTAX_STRING) | A_BOLD);
            fs->in_multiline_string = !closed;
            x += len;
            i += len;
        } else if (line[i] == '"' || line[i] == '\'') {
            bool closed;
            int len = scan_string(line, i, line[i], &closed);
            wattron(win, COLOR_PAIR(SYNTAX_STRING) | A_BOLD);
            mvwprintw(win, y, x, "%.*s", len, &line[i]);
            wattroff(win, COLOR_PAIR(SYNTAX_STRING) | A_BOLD);
            x += len;
            i += len;
        } else {
            mvwprintw(win, y, x++, "%c", line[i++]);
        }
    }
}

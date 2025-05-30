#include <ncurses.h>
#include <string.h>
#include <ctype.h>
#include "syntax.h"

#define PYTHON_KEYWORDS_COUNT 35
static const char *PYTHON_KEYWORDS[PYTHON_KEYWORDS_COUNT] = {
    "False", "None", "True", "and", "as", "assert", "async", "await", "break",
    "class", "continue", "def", "del", "elif", "else", "except", "finally",
    "for", "from", "global", "if", "import", "in", "is", "lambda", "nonlocal",
    "not", "or", "pass", "raise", "return", "try", "while", "with", "yield"
};

void highlight_python_syntax(WINDOW *win, const char *line, int y) {
    wattrset(win, COLOR_PAIR(SYNTAX_BG));
    int length = strlen(line);
    int start = 0;
    int i = 0;
    int x = 1;

    while (i < length) {
        if (isalpha((unsigned char)line[i]) || line[i] == '_') {
            start = i;
            while (isalnum((unsigned char)line[i]) || line[i] == '_') i++;
            int word_length = i - start;
            char word[word_length + 1];
            strncpy(word, &line[start], word_length);
            word[word_length] = '\0';

            int is_keyword = 0;
            for (int j = 0; j < PYTHON_KEYWORDS_COUNT; j++) {
                if (strcmp(word, PYTHON_KEYWORDS[j]) == 0) {
                    wattron(win, COLOR_PAIR(SYNTAX_KEYWORD) | A_BOLD);
                    mvwprintw(win, y, x, "%s", word);
                    wattroff(win, COLOR_PAIR(SYNTAX_KEYWORD) | A_BOLD);
                    x += word_length;
                    is_keyword = 1;
                    break;
                }
            }
            if (!is_keyword) {
                mvwprintw(win, y, x, "%s", word);
                x += word_length;
            }
        } else if (line[i] == '#') {
            wattron(win, COLOR_PAIR(SYNTAX_COMMENT) | A_BOLD);
            mvwprintw(win, y, x, "%s", &line[i]);
            wattroff(win, COLOR_PAIR(SYNTAX_COMMENT) | A_BOLD);
            break;
        } else if (line[i] == '"' || line[i] == '\'') {
            char quote = line[i];
            start = i;
            i++;
            while (i < length && line[i] != quote) i++;
            if (i < length) i++;
            wattron(win, COLOR_PAIR(SYNTAX_STRING) | A_BOLD);
            mvwprintw(win, y, x, "%.*s", i - start, &line[start]);
            wattroff(win, COLOR_PAIR(SYNTAX_STRING) | A_BOLD);
            x += i - start;
        } else {
            mvwprintw(win, y, x, "%c", line[i]);
            x++;
            i++;
        }
    }
}

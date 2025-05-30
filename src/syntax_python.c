#include <ncurses.h>
#include <string.h>
#include <ctype.h>
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
    int start = 0;
    int i = 0;
    int x = 1;

    while (i < length) {
        if (fs->in_multiline_string) {
            start = i;
            while (i < length) {
                if (line[i] == '\\' && i + 1 < length) {
                    i += 2;
                    continue;
                }
                if (line[i] == fs->string_delim && i + 2 < length &&
                    line[i + 1] == fs->string_delim && line[i + 2] == fs->string_delim) {
                    i += 3;
                    fs->in_multiline_string = false;
                    break;
                }
                i++;
            }
            wattron(win, COLOR_PAIR(SYNTAX_STRING) | A_BOLD);
            mvwprintw(win, y, x, "%.*s", i - start, &line[start]);
            wattroff(win, COLOR_PAIR(SYNTAX_STRING) | A_BOLD);
            x += i - start;
        } else if (isalpha((unsigned char)line[i]) || line[i] == '_') {
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
        } else if (isdigit((unsigned char)line[i])) {
            start = i++;
            if (line[start] == '0' && i < length && strchr("xXbBoO", line[i])) {
                i++;
                while (i < length && isalnum((unsigned char)line[i])) i++;
            } else {
                while (i < length && (isalnum((unsigned char)line[i]) || line[i] == '.' || line[i] == '_')) i++;
            }
            wattron(win, COLOR_PAIR(SYNTAX_TYPE) | A_BOLD);
            mvwprintw(win, y, x, "%.*s", i - start, &line[start]);
            wattroff(win, COLOR_PAIR(SYNTAX_TYPE) | A_BOLD);
            x += i - start;
        } else if (line[i] == '#') {
            wattron(win, COLOR_PAIR(SYNTAX_COMMENT) | A_BOLD);
            mvwprintw(win, y, x, "%s", &line[i]);
            wattroff(win, COLOR_PAIR(SYNTAX_COMMENT) | A_BOLD);
            break;
        } else if ((line[i] == '"' || line[i] == '\'' ) && i + 2 < length &&
                   line[i + 1] == line[i] && line[i + 2] == line[i]) {
            fs->in_multiline_string = true;
            fs->string_delim = line[i];
            start = i;
            i += 3;
            while (i < length) {
                if (line[i] == '\\' && i + 1 < length) {
                    i += 2;
                    continue;
                }
                if (line[i] == fs->string_delim && i + 2 < length &&
                    line[i + 1] == fs->string_delim && line[i + 2] == fs->string_delim) {
                    i += 3;
                    fs->in_multiline_string = false;
                    break;
                }
                i++;
            }
            wattron(win, COLOR_PAIR(SYNTAX_STRING) | A_BOLD);
            mvwprintw(win, y, x, "%.*s", i - start, &line[start]);
            wattroff(win, COLOR_PAIR(SYNTAX_STRING) | A_BOLD);
            x += i - start;
        } else if (line[i] == '"' || line[i] == '\'') {
            char quote = line[i];
            start = i++;
            while (i < length) {
                if (line[i] == '\\' && i + 1 < length) {
                    i += 2;
                    continue;
                }
                if (line[i] == quote) {
                    i++;
                    break;
                }
                i++;
            }
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

#include <ncurses.h>
#include <string.h>
#include <ctype.h>
#include "syntax.h"
#include "files.h"

static const char *CSHARP_KEYWORDS[] = {
    "abstract", "as", "base", "bool", "break", "byte", "case", "catch", "char",
    "checked", "class", "const", "continue",
    "decimal", "default", "delegate", "do", "double", "else", "enum", "event", "explicit", "extern", "false", "finally",
    "fixed", "float", "for", "foreach", "goto", "if", "implicit", "in", "int", "interface", "internal", "is", "lock",
    "long", "namespace", "new", "null", "object", "operator", "out", "override",
    "params", "private", "protected",
    "public", "readonly", "ref", "return", "sbyte", "sealed", "short", "sizeof",
    "stackalloc", "static", "string",
    "struct", "switch", "this", "throw", "true", "try", "typeof", "uint", "ulong",
    "unchecked", "unsafe", "ushort",
    "using", "virtual", "void", "volatile", "while"
};
static const int CSHARP_KEYWORDS_COUNT = sizeof(CSHARP_KEYWORDS) / sizeof(CSHARP_KEYWORDS[0]);

void highlight_csharp_syntax(FileState *fs, WINDOW *win, const char *line, int y) {
    wattrset(win, COLOR_PAIR(SYNTAX_BG));
    int x = 1;
    int i = 0;
    int len = strlen(line);
    char word[256];
    int word_len = 0;

    while (i < len) {
        if (fs->in_multiline_comment) {
            wattron(win, COLOR_PAIR(SYNTAX_COMMENT) | A_BOLD);
            while (i < len) {
                mvwprintw(win, y, x++, "%c", line[i]);
                if (line[i] == '*' && i + 1 < len && line[i + 1] == '/') {
                    mvwprintw(win, y, x++, "%c", line[i + 1]);
                    i += 2;
                    fs->in_multiline_comment = false;
                    break;
                }
                i++;
            }
            wattroff(win, COLOR_PAIR(SYNTAX_COMMENT) | A_BOLD);
            continue;
        }

        if (isspace((unsigned char)line[i])) {
            mvwprintw(win, y, x++, "%c", line[i]);
            i++;
        } else if (isdigit((unsigned char)line[i])) {
            wattron(win, COLOR_PAIR(SYNTAX_TYPE) | A_BOLD);
            while (i < len && (isdigit((unsigned char)line[i]) || line[i] == '.')) {
                mvwprintw(win, y, x++, "%c", line[i]);
                i++;
            }
            wattroff(win, COLOR_PAIR(SYNTAX_TYPE) | A_BOLD);
        } else if (line[i] == '/' && i + 1 < len && (line[i + 1] == '/' || line[i + 1] == '*')) {
            wattron(win, COLOR_PAIR(SYNTAX_COMMENT) | A_BOLD);
            if (line[i + 1] == '/') {
                while (i < len) {
                    mvwprintw(win, y, x++, "%c", line[i]);
                    i++;
                }
            } else {
                mvwprintw(win, y, x++, "%c", line[i++]);
                mvwprintw(win, y, x++, "%c", line[i++]);
                fs->in_multiline_comment = true;
                while (i < len) {
                    mvwprintw(win, y, x++, "%c", line[i]);
                    if (line[i] == '*' && i + 1 < len && line[i + 1] == '/') {
                        mvwprintw(win, y, x++, "%c", line[i + 1]);
                        i += 2;
                        fs->in_multiline_comment = false;
                        break;
                    }
                    i++;
                }
            }
            wattroff(win, COLOR_PAIR(SYNTAX_COMMENT) | A_BOLD);
        } else if (line[i] == '"' || line[i] == '\'') {
            char quote = line[i];
            wattron(win, COLOR_PAIR(SYNTAX_STRING) | A_BOLD);
            mvwprintw(win, y, x++, "%c", line[i++]);
            while (i < len && line[i] != quote) {
                if (line[i] == '\\' && i + 1 < len) {
                    mvwprintw(win, y, x++, "%c", line[i++]);
                }
                mvwprintw(win, y, x++, "%c", line[i++]);
            }
            if (i < len) {
                mvwprintw(win, y, x++, "%c", line[i++]);
            }
            wattroff(win, COLOR_PAIR(SYNTAX_STRING) | A_BOLD);
        } else if (isalpha((unsigned char)line[i]) || line[i] == '_') {
            word_len = 0;
            while (i < len && (isalnum((unsigned char)line[i]) || line[i] == '_')) {
                word[word_len++] = line[i++];
            }
            word[word_len] = '\0';

            int is_keyword = 0;
            for (int j = 0; j < CSHARP_KEYWORDS_COUNT; j++) {
                if (strcmp(word, CSHARP_KEYWORDS[j]) == 0) {
                    wattron(win, COLOR_PAIR(SYNTAX_KEYWORD) | A_BOLD);
                    mvwprintw(win, y, x, "%s", word);
                    wattroff(win, COLOR_PAIR(SYNTAX_KEYWORD) | A_BOLD);
                    x += word_len;
                    is_keyword = 1;
                    break;
                }
            }
            if (!is_keyword) {
                mvwprintw(win, y, x, "%s", word);
                x += word_len;
            }
        } else if (strchr("(){}[]<>", line[i])) {
            wattron(win, COLOR_PAIR(SYNTAX_SYMBOL) | A_BOLD);
            mvwprintw(win, y, x++, "%c", line[i++]);
            wattroff(win, COLOR_PAIR(SYNTAX_SYMBOL) | A_BOLD);
        } else if (strchr("+-*/%=!&|^~", line[i])) {
            wattron(win, COLOR_PAIR(SYNTAX_SYMBOL) | A_BOLD);
            mvwprintw(win, y, x++, "%c", line[i++]);
            wattroff(win, COLOR_PAIR(SYNTAX_SYMBOL) | A_BOLD);
        } else {
            mvwprintw(win, y, x++, "%c", line[i++]);
        }
    }
}

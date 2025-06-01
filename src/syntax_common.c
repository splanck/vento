#include <ncurses.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "syntax.h"
#include "editor.h"
#include "files.h"

/* Helper token scanning functions shared across language highlighters */
int scan_identifier(const char *line, int start) {
    int i = start;
    if (!isalpha((unsigned char)line[i]) && line[i] != '_')
        return 0;
    i++;
    while (isalnum((unsigned char)line[i]) || line[i] == '_')
        i++;
    return i - start;
}

int scan_number(const char *line, int start) {
    int i = start;
    if (line[i] == '0' && line[i + 1] && strchr("xXbBoO", line[i + 1])) {
        i += 2;
        while (isalnum((unsigned char)line[i]))
            i++;
    } else {
        while (isalnum((unsigned char)line[i]) || line[i] == '.' || line[i] == '_')
            i++;
    }
    return i - start;
}

int scan_string(const char *line, int start, char quote, bool *closed) {
    int i = start + 1;
    *closed = false;
    while (line[i]) {
        if (line[i] == '\\' && line[i + 1]) {
            i += 2;
            continue;
        }
        if (line[i] == quote) {
            i++;
            *closed = true;
            break;
        }
        i++;
    }
    return i - start;
}

int scan_multiline_string(const char *line, int start, char quote, bool *closed) {
    int i = start + 3; /* skip opening quotes */
    *closed = false;
    while (line[i]) {
        if (line[i] == '\\' && line[i + 1]) {
            i += 2;
            continue;
        }
        if (line[i] == quote && line[i + 1] == quote && line[i + 2] == quote) {
            i += 3;
            *closed = true;
            break;
        }
        i++;
    }
    return i - start;
}

// Synchronize the in_multiline_comment flag up to the specified line
void sync_multiline_comment(FileState *fs, int line) {
    bool in_comment;
    bool in_string = false;
    char quote = '\0';

    int start;

    if (line < fs->last_scanned_line) {
        start = 0;
        in_comment = false;
    } else {
        start = fs->last_scanned_line;
        in_comment = fs->last_comment_state;
    }

    int max = line < fs->buffer.count ? line : fs->buffer.count;
    for (int l = start; l < max; l++) {
        char *p = fs->buffer.lines[l];
        for (int i = 0; p[i] != '\0'; i++) {
            char c = p[i];

            if (in_string) {
                if (c == '\\' && p[i + 1] != '\0') {
                    i++; // Skip escaped char
                    continue;
                } else if (c == quote) {
                    in_string = false;
                }
                continue;
            }

            if (!in_comment) {
                if (c == '"' || c == '\'' ) {
                    in_string = true;
                    quote = c;
                } else if (c == '/' && p[i + 1] == '*') {
                    in_comment = true;
                    i++; // skip '*'
                }
            } else {
                if (c == '*' && p[i + 1] == '/') {
                    in_comment = false;
                    i++; // skip '/'
                }
            }
        }
    }

    fs->in_multiline_comment = in_comment;
    fs->last_scanned_line = max;
    fs->last_comment_state = in_comment;
}

void mark_comment_state_dirty(FileState *fs) {
    fs->last_scanned_line = 0;
    fs->last_comment_state = false;
}

void highlight_no_syntax(WINDOW *win, const char *line, int y) {
    // Ensure background color is applied to unhighlighted text
    wattrset(win, COLOR_PAIR(SYNTAX_BG));
    mvwprintw(win, y, 1, "%s", line);
}

// Generic keyword based highlighter used by multiple languages
void highlight_with_keywords(FileState *fs, WINDOW *win, const char *line, int y,
                             const char **keywords, int keyword_count) {
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
            int tok_len = scan_number(line, i);
            wattron(win, COLOR_PAIR(SYNTAX_TYPE) | A_BOLD);
            mvwprintw(win, y, x, "%.*s", tok_len, &line[i]);
            wattroff(win, COLOR_PAIR(SYNTAX_TYPE) | A_BOLD);
            x += tok_len;
            i += tok_len;
        } else if (line[i] == '/' && i + 1 < len && (line[i + 1] == '/' || line[i + 1] == '*')) {
            wattron(win, COLOR_PAIR(SYNTAX_COMMENT) | A_BOLD);
            if (line[i + 1] == '/') {
                mvwprintw(win, y, x, "%s", &line[i]);
                x += len - i;
                i = len;
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
            bool closed;
            int tok_len = scan_string(line, i, line[i], &closed);
            wattron(win, COLOR_PAIR(SYNTAX_STRING) | A_BOLD);
            mvwprintw(win, y, x, "%.*s", tok_len, &line[i]);
            wattroff(win, COLOR_PAIR(SYNTAX_STRING) | A_BOLD);
            x += tok_len;
            i += tok_len;
        } else if (isalpha((unsigned char)line[i]) || line[i] == '_') {
            word_len = scan_identifier(line, i);
            if (word_len >= (int)sizeof(word))
                word_len = (int)sizeof(word) - 1;
            strncpy(word, &line[i], word_len);
            word[word_len] = '\0';
            i += word_len;
            
            int is_keyword = 0;
            for (int j = 0; j < keyword_count; j++) {
                if (strcmp(word, keywords[j]) == 0) {
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

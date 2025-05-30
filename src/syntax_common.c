#include <ncurses.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "syntax.h"
#include "editor.h"
#include "files.h"

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

    int max = line < fs->line_count ? line : fs->line_count;
    for (int l = start; l < max; l++) {
        char *p = fs->text_buffer[l];
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
            while (i < len && (isalnum((unsigned char)line[i]) || line[i] == '_') &&
                   word_len < (int)sizeof(word) - 1) {
                word[word_len++] = line[i++];
            }
            word[word_len] = '\0';

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

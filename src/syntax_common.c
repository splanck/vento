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

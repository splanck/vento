#include <ncurses.h>
#include <string.h>
#include <ctype.h>
#include "syntax.h"
#include "editor.h"

int current_syntax_mode = NO_SYNTAX;

void set_syntax_highlighting(int mode) {
    current_syntax_mode = mode;
}

void apply_syntax_highlighting(WINDOW *win, const char *line, int y) {
    switch (current_syntax_mode) {
        case C_SYNTAX:
            highlight_c_syntax(win, line, y);
            break;
        case HTML_SYNTAX:
            highlight_html_syntax(win, line, y);
            break;
        case NO_SYNTAX:
        default:
            highlight_no_syntax(win, line, y);
            break;
    }
}

void highlight_c_syntax(WINDOW *win, const char *line, int y) {
    int x = 1;
    int i = 0;
    int len = strlen(line);
    char word[256];
    int word_len = 0;

    while (i < len) {
        if (isspace(line[i])) {
            // Print spaces as is
            mvwprintw(win, y, x++, "%c", line[i]);
            i++;
        } else if (isdigit(line[i])) {
            // Highlight numbers
            wattron(win, COLOR_PAIR(5));
            while (i < len && isdigit(line[i])) {
                mvwprintw(win, y, x++, "%c", line[i]);
                i++;
            }
            wattroff(win, COLOR_PAIR(5));
        } else if (line[i] == '/' && line[i + 1] == '/') {
            // Highlight comments
            wattron(win, COLOR_PAIR(3));
            while (i < len) {
                mvwprintw(win, y, x++, "%c", line[i]);
                i++;
            }
            wattroff(win, COLOR_PAIR(3));
        } else if (line[i] == '"' || line[i] == '\'') {
            // Highlight strings
            char quote = line[i];
            wattron(win, COLOR_PAIR(4));
            mvwprintw(win, y, x++, "%c", line[i++]);
            while (i < len && line[i] != quote) {
                mvwprintw(win, y, x++, "%c", line[i++]);
            }
            if (i < len) {
                mvwprintw(win, y, x++, "%c", line[i++]);
            }
            wattroff(win, COLOR_PAIR(4));
        } else if (isalpha(line[i]) || line[i] == '_') {
            // Collect keywords and identifiers
            word_len = 0;
            while (i < len && (isalnum(line[i]) || line[i] == '_')) {
                word[word_len++] = line[i++];
            }
            word[word_len] = '\0';

            // Check if the word is a keyword
            if (strcmp(word, "int") == 0 || strcmp(word, "char") == 0 || strcmp(word, "return") == 0 || 
                strcmp(word, "if") == 0 || strcmp(word, "else") == 0 || strcmp(word, "while") == 0 || 
                strcmp(word, "for") == 0 || strcmp(word, "void") == 0 || strcmp(word, "static") == 0 || 
                strcmp(word, "const") == 0 || strcmp(word, "struct") == 0) {
                wattron(win, COLOR_PAIR(2));
                mvwprintw(win, y, x, "%s", word);
                wattroff(win, COLOR_PAIR(2));
                x += word_len;
            } else {
                mvwprintw(win, y, x, "%s", word);
                x += word_len;
            }
        } else if (strchr("(){}", line[i])) {
            // Highlight parentheses and braces
            wattron(win, COLOR_PAIR(6));
            mvwprintw(win, y, x++, "%c", line[i++]);
            wattroff(win, COLOR_PAIR(6));
        } else {
            // Print any other characters as is
            mvwprintw(win, y, x++, "%c", line[i++]);
        }
    }
}

void highlight_html_syntax(WINDOW *win, const char *line, int y) {
    // Example: HTML syntax highlighting
    // Highlight tags, attributes, etc.
    int color_set = 0;
    if (strstr(line, "<") && strstr(line, ">")) {
        wattron(win, COLOR_PAIR(3));
        color_set = 1;
    }
    mvwprintw(win, y, 1, "%s", line);
    if (color_set) {
        wattroff(win, COLOR_PAIR(3));
    }
}

void highlight_no_syntax(WINDOW *win, const char *line, int y) {
    mvwprintw(win, y, 1, "%s", line);
}

void apply_syntax_highlighting_old(WINDOW *win, const char *line, int y) {
    int x = 1;
    while (*line) {
        if (strncmp(line, "//", 2) == 0) {
            wattron(win, COLOR_PAIR(3)); // Use yellow for comments
            mvwprintw(win, y, x, "%s", line);
            wattroff(win, COLOR_PAIR(3));
            break;
        } else if (*line == '"') {
            wattron(win, COLOR_PAIR(2)); // Use white for strings
            mvwaddch(win, y, x, *line);
            x++;
            line++;
            while (*line && *line != '"') {
                mvwaddch(win, y, x, *line);
                x++;
                line++;
            }
            if (*line == '"') {
                mvwaddch(win, y, x, *line);
                x++;
                line++;
            }
            wattroff(win, COLOR_PAIR(2));
        } else if (isalpha(*line) || *line == '_') {
            char word[64];
            int i = 0;
            while ((isalpha(*line) || isdigit(*line) || *line == '_') && i < 63) {
                word[i++] = *line;
                x++;
                line++;
            }
            word[i] = '\0';
            if (strcmp(word, "int") == 0 || strcmp(word, "return") == 0 || strcmp(word, "if") == 0) {
                wattron(win, COLOR_PAIR(1)); // Use blue for keywords
                mvwprintw(win, y, x - i, "%s", word);
                wattroff(win, COLOR_PAIR(1));
            } else {
                mvwprintw(win, y, x - i, "%s", word);
            }
        } else {
            mvwaddch(win, y, x, *line);
            x++;
            line++;
        }
    }
}

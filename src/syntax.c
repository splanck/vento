#include <ncurses.h>
#include <string.h>
#include <ctype.h>
#include "syntax.h"
#include "editor.h"

void apply_syntax_highlighting(WINDOW *win, const char *line, int y) {
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

#include <ncurses.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "syntax.h"
#include "editor.h"

// Define color pairs as constants for readability
#define COLOR_COMMENT 5
#define COLOR_TAG 2
#define COLOR_ATTRIBUTE 3
#define COLOR_VALUE 4
#define COLOR_NORMAL 1

#define PYTHON_KEYWORDS_COUNT 35

// List of Python keywords
const char *PYTHON_KEYWORDS[PYTHON_KEYWORDS_COUNT] = {
    "False", "None", "True", "and", "as", "assert", "async", "await", "break",
    "class", "continue", "def", "del", "elif", "else", "except", "finally",
    "for", "from", "global", "if", "import", "in", "is", "lambda", "nonlocal",
    "not", "or", "pass", "raise", "return", "try", "while", "with", "yield"
};

// List of C keywords
const char *C_KEYWORDS[] = {
    "auto", "break", "case", "char", "const", "continue", "default", "do", "double", "else", "enum", "extern", "float", "for", "goto", "if", "inline", "int", "long", "register", "restrict", "return", "short", "signed", "sizeof", "static", "struct", "switch", "typedef", "union", "unsigned", "void", "volatile", "while", "_Alignas", "_Alignof", "_Atomic", "_Bool", "_Complex", "_Generic", "_Imaginary", "_Noreturn", "_Static_assert", "_Thread_local"
};
const int C_KEYWORDS_COUNT = sizeof(C_KEYWORDS) / sizeof(C_KEYWORDS[0]);

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
        case PYTHON_SYNTAX:
            highlight_python_syntax(win, line, y);
            break;
        default:
            highlight_no_syntax(win, line, y);
            break;
    }
}

void highlight_no_syntax(WINDOW *win, const char *line, int y) {
    mvwprintw(win, y, 1, "%s", line);
}

void highlight_python_syntax(WINDOW *win, const char *line, int y) {
    int length = strlen(line);
    int start = 0;
    int i = 0;
    int x = 1; // Start at 1 to match the original display position

    while (i < length) {
        if (isalpha(line[i]) || line[i] == '_') {
            start = i;
            while (isalnum(line[i]) || line[i] == '_') i++;
            int word_length = i - start;
            char word[word_length + 1];
            strncpy(word, &line[start], word_length);
            word[word_length] = '\0';

            int is_keyword = 0;
            for (int j = 0; j < PYTHON_KEYWORDS_COUNT; j++) {
                if (strcmp(word, PYTHON_KEYWORDS[j]) == 0) {
                    wattron(win, COLOR_PAIR(2) | A_BOLD);
                    mvwprintw(win, y, x, "%s", word);
                    wattroff(win, COLOR_PAIR(2) | A_BOLD);
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
            wattron(win, COLOR_PAIR(3) | A_BOLD);
            mvwprintw(win, y, x, "%s", &line[i]);
            wattroff(win, COLOR_PAIR(3) | A_BOLD);
            break;
        } else if (line[i] == '"' || line[i] == '\'') {
            char quote = line[i];
            start = i;
            i++;
            while (i < length && line[i] != quote) i++;
            if (i < length) i++;
            wattron(win, COLOR_PAIR(4) | A_BOLD);
            mvwprintw(win, y, x, "%.*s", i - start, &line[start]);
            wattroff(win, COLOR_PAIR(4) | A_BOLD);
            x += i - start;
        } else {
            mvwprintw(win, y, x, "%c", line[i]);
            x++;
            i++;
        }
    }
    wrefresh(win);
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
            wattron(win, COLOR_PAIR(5) | A_BOLD);
            while (i < len && (isdigit(line[i]) || line[i] == '.')) {
                mvwprintw(win, y, x++, "%c", line[i]);
                i++;
            }
            wattroff(win, COLOR_PAIR(5) | A_BOLD);
        } else if (line[i] == '/' && (line[i + 1] == '/' || line[i + 1] == '*')) {
            // Highlight comments
            wattron(win, COLOR_PAIR(3) | A_BOLD);
            if (line[i + 1] == '/') {
                while (i < len) {
                    mvwprintw(win, y, x++, "%c", line[i]);
                    i++;
                }
            } else {
                mvwprintw(win, y, x++, "%c", line[i++]);
                mvwprintw(win, y, x++, "%c", line[i++]);
                while (i < len && !(line[i] == '*' && line[i + 1] == '/')) {
                    mvwprintw(win, y, x++, "%c", line[i]);
                    i++;
                }
                if (i < len) {
                    mvwprintw(win, y, x++, "%c", line[i++]);
                    mvwprintw(win, y, x++, "%c", line[i++]);
                }
            }
            wattroff(win, COLOR_PAIR(3) | A_BOLD);
        } else if (line[i] == '"' || line[i] == '\'') {
            // Highlight strings and character literals
            char quote = line[i];
            wattron(win, COLOR_PAIR(4) | A_BOLD);
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
            wattroff(win, COLOR_PAIR(4) | A_BOLD);
        } else if (isalpha(line[i]) || line[i] == '_') {
            // Collect keywords and identifiers
            word_len = 0;
            while (i < len && (isalnum(line[i]) || line[i] == '_')) {
                word[word_len++] = line[i++];
            }
            word[word_len] = '\0';

            // Check if the word is a keyword
            int is_keyword = 0;
            for (int j = 0; j < C_KEYWORDS_COUNT; j++) {
                if (strcmp(word, C_KEYWORDS[j]) == 0) {
                    wattron(win, COLOR_PAIR(2) | A_BOLD);
                    mvwprintw(win, y, x, "%s", word);
                    wattroff(win, COLOR_PAIR(2) | A_BOLD);
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
            // Highlight parentheses, braces, brackets, and angle brackets
            wattron(win, COLOR_PAIR(6) | A_BOLD);
            mvwprintw(win, y, x++, "%c", line[i++]);
            wattroff(win, COLOR_PAIR(6) | A_BOLD);
        } else if (strchr("+-*/%=!&|^~", line[i])) {
            // Highlight operators
            wattron(win, COLOR_PAIR(6) | A_BOLD);
            mvwprintw(win, y, x++, "%c", line[i++]);
            wattroff(win, COLOR_PAIR(6) | A_BOLD);
        } else {
            // Print any other characters as is
            mvwprintw(win, y, x++, "%c", line[i++]);
        }
    }
}

// Helper function to print a character with specific attributes
void print_char_with_attr(WINDOW *win, int y, int *x, char c, int attr) {
    wattron(win, attr);
    mvwprintw(win, y, (*x)++, "%c", c);
    wattroff(win, attr);
}

// Function to handle HTML comments
void handle_html_comment(WINDOW *win, const char *line, int *i, int y, int *x) {
    int len = strlen(line);
    print_char_with_attr(win, y, x, line[(*i)++], COLOR_PAIR(COLOR_COMMENT) | A_BOLD); // Print the initial '<'
    while (*i < len && !(line[*i] == '-' && line[*i + 1] == '-' && line[*i + 2] == '>')) {
        print_char_with_attr(win, y, x, line[(*i)++], COLOR_PAIR(COLOR_COMMENT) | A_BOLD);
    }
    if (*i < len) { // Also print the closing '>'
        print_char_with_attr(win, y, x, line[(*i)++], COLOR_PAIR(COLOR_COMMENT) | A_BOLD);
        (*i) += 2; // Skip over '-->'
    }
}

// Function to handle HTML tags (opening, closing, and self-closing)
void handle_html_tag(WINDOW *win, const char *line, int *i, int y, int *x) {
    int len = strlen(line);
    bool inAttribute = false;
    while (*i < len && line[*i] != '>') {
        if (line[*i] == ' ' && !inAttribute) {
            // Space outside of attributes, might be the start of an attribute
            inAttribute = true;
            print_char_with_attr(win, y, x, line[(*i)++], COLOR_PAIR(COLOR_TAG) | A_BOLD);
        } else if (line[*i] == '=') {
            // Equal sign, attribute value follows
            print_char_with_attr(win, y, x, line[(*i)++], COLOR_PAIR(COLOR_ATTRIBUTE) | A_BOLD);
            if (line[*i] == '"' || line[*i] == '\'') { // Attribute value enclosed in quotes
                char quoteType = line[(*i)++];
                print_char_with_attr(win, y, x, quoteType, COLOR_PAIR(COLOR_VALUE) | A_BOLD);
                while (*i < len && line[*i] != quoteType) {
                    print_char_with_attr(win, y, x, line[(*i)++], COLOR_PAIR(COLOR_VALUE) | A_BOLD);
                }
                if (*i < len) {
                    print_char_with_attr(win, y, x, line[(*i)++], COLOR_PAIR(COLOR_VALUE) | A_BOLD); // Print the closing quote
                }
            }
        } else {
            // Regular character within the tag
            int color = inAttribute ? COLOR_PAIR(COLOR_ATTRIBUTE) | A_BOLD : COLOR_PAIR(COLOR_TAG) | A_BOLD;
            print_char_with_attr(win, y, x, line[(*i)++], color);
        }
    }
    if (*i < len) { // Print the closing '>'
        print_char_with_attr(win, y, x, line[(*i)++], COLOR_PAIR(COLOR_TAG) | A_BOLD);
    }
}

// Main function to highlight HTML syntax
void highlight_html_syntax(WINDOW *win, const char *line, int y) {
    if (!win || !line) return; // Basic error handling

    int x = 0; // Start from the beginning of the line
    int i = 0;
    int len = strlen(line);

    while (i < len) {
        if (isspace(line[i])) {
            mvwprintw(win, y, x++, "%c", line[i++]); // Print spaces as is
        } else if (line[i] == '<') {
            if (strncmp(&line[i], "<!--", 4) == 0) {
                handle_html_comment(win, line, &i, y, &x); // Handle comments
            } else {
                handle_html_tag(win, line, &i, y, &x); // Handle tags
            }
        } else {
            // Print any other characters as is
            mvwprintw(win, y, x++, "%c", line[i++]);
        }
    }
}
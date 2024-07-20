#ifndef SYNTAX_H
#define SYNTAX_H

#include "editor.h"

#define NO_SYNTAX 0
#define C_SYNTAX 1
#define HTML_SYNTAX 2
#define PYTHON_SYNTAX 3

extern int current_syntax_mode;

void set_syntax_highlighting(int mode);
void apply_syntax_highlighting(WINDOW *win, const char *line, int y);
void highlight_c_syntax(WINDOW *win, const char *line, int y);
void highlight_html_syntax(WINDOW *win, const char *line, int y);
void handle_html_tag(WINDOW *win, const char *line, int *i, int y, int *x);
void handle_html_comment(WINDOW *win, const char *line, int *i, int y, int *x);
void print_char_with_attr(WINDOW *win, int y, int *x, char c, int attr);
void highlight_no_syntax(WINDOW *win, const char *line, int y);
void highlight_python_syntax(WINDOW *win, const char *line, int y);

#endif // SYNTAX_H

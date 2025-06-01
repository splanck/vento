#ifndef SYNTAX_H
#define SYNTAX_H

#include "editor.h"
#include <regex.h>

#define NO_SYNTAX 0
#define C_SYNTAX 1
#define HTML_SYNTAX 2
#define PYTHON_SYNTAX 3
#define CSHARP_SYNTAX 4
#define JS_SYNTAX 5
#define CSS_SYNTAX 6
#define SHELL_SYNTAX 7

typedef enum {
    SYNTAX_BG = 1,
    SYNTAX_KEYWORD,
    SYNTAX_COMMENT,
    SYNTAX_STRING,
    SYNTAX_TYPE,
    SYNTAX_SYMBOL,
    SYNTAX_SEARCH
} SyntaxColor;

typedef struct {
    const char *pattern;
    int attr;
    regex_t regex;
    int compiled;
} SyntaxRegex;

int compile_regex_set(SyntaxRegex *set, int count);
void free_regex_set(SyntaxRegex *set, int count);
void highlight_regex_line(WINDOW *win, int y, int x_start, const char *line,
                          SyntaxRegex *patterns, int count);

void apply_syntax_highlighting(struct FileState *fs, WINDOW *win, const char *line, int y);

typedef struct {
    const char *ext;
    SyntaxRegex *patterns;
    int count;
} SyntaxDef;

void syntax_register(int mode, const SyntaxDef *def);
const SyntaxDef *syntax_get(int mode);
void highlight_by_patterns(struct FileState *fs, WINDOW *win, const char *line, int y,
                           const SyntaxDef *def);

void highlight_html_syntax(struct FileState *fs, WINDOW *win, const char *line, int y);
void handle_html_tag(WINDOW *win, const char *line, int *i, int y, int *x);
void handle_html_comment(WINDOW *win, const char *line, int *i, int y, int *x);
void print_char_with_attr(WINDOW *win, int y, int *x, char c, int attr);
void highlight_no_syntax(WINDOW *win, const char *line, int y);
void highlight_with_keywords(struct FileState *fs, WINDOW *win, const char *line,
                             int y, const char **keywords, int keyword_count);
void sync_multiline_comment(struct FileState *fs, int line);
void mark_comment_state_dirty(struct FileState *fs);

/* Token scanning helpers shared by highlight implementations */
int scan_identifier(const char *line, int start);
int scan_number(const char *line, int start);
int scan_string(const char *line, int start, char quote, bool *closed);
int scan_multiline_string(const char *line, int start, char quote, bool *closed);

/* Free any compiled regular expressions */
void syntax_cleanup(void);

#endif // SYNTAX_H

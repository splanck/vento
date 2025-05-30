#include <ncurses.h>
#include <string.h>
#include <ctype.h>
#include "syntax.h"
#include "files.h"

#define JS_KEYWORDS_PATTERN \
    "^(break|case|catch|class|const|continue|debugger|default|delete|do|else|export|extends|finally|for|function|if|import|in|instanceof|let|new|return|super|switch|this|throw|try|typeof|var|void|while|with|yield|static)\\b"

static SyntaxRegex JS_PATTERNS[] = {
    { .pattern = "^//.*", .attr = COLOR_PAIR(SYNTAX_COMMENT) | A_BOLD },
    { .pattern = "^/\\*.*\\*/", .attr = COLOR_PAIR(SYNTAX_COMMENT) | A_BOLD },
    { .pattern = "^(\"([^\"\\]|\\.)*\"|'([^'\\]|\\.)*')", .attr = COLOR_PAIR(SYNTAX_STRING) | A_BOLD },
    { .pattern = "^(0[xX][0-9A-Fa-f]+|[0-9]+)", .attr = COLOR_PAIR(SYNTAX_TYPE) | A_BOLD },
    { .pattern = JS_KEYWORDS_PATTERN, .attr = COLOR_PAIR(SYNTAX_KEYWORD) | A_BOLD },
};
static int js_regex_compiled = 0;
static const int JS_PATTERNS_COUNT = sizeof(JS_PATTERNS) / sizeof(JS_PATTERNS[0]);

void highlight_js_syntax(FileState *fs, WINDOW *win, const char *line, int y) {
    (void)fs;
    if (!js_regex_compiled) {
        js_regex_compiled = compile_regex_set(JS_PATTERNS, JS_PATTERNS_COUNT);
    }
    if (js_regex_compiled > 0) {
        highlight_regex_line(win, y, 1, line, JS_PATTERNS, JS_PATTERNS_COUNT);
    } else {
        static const char *JS_KEYWORDS[] = {
            "break","case","catch","class","const","continue","debugger","default",
            "delete","do","else","export","extends","finally","for","function",
            "if","import","in","instanceof","let","new","return","super","switch",
            "this","throw","try","typeof","var","void","while","with","yield","static"
        };
        static const int COUNT = sizeof(JS_KEYWORDS)/sizeof(JS_KEYWORDS[0]);
        highlight_with_keywords(fs, win, line, y, JS_KEYWORDS, COUNT);
    }
}

#include <ncurses.h>
#include <string.h>
#include <ctype.h>
#include "syntax.h"
#include "files.h"

static const char *JS_KEYWORDS[] = {
    "break", "case", "catch", "class", "const", "continue", "debugger", "default",
    "delete", "do", "else", "export", "extends", "finally", "for", "function",
    "if", "import", "in", "instanceof", "let", "new", "return", "super",
    "switch", "this", "throw", "try", "typeof", "var", "void", "while",
    "with", "yield", "static"
};
static const int JS_KEYWORDS_COUNT = sizeof(JS_KEYWORDS) / sizeof(JS_KEYWORDS[0]);

void highlight_js_syntax(FileState *fs, WINDOW *win, const char *line, int y) {
    highlight_with_keywords(fs, win, line, y, JS_KEYWORDS, JS_KEYWORDS_COUNT);
}

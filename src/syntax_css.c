#include <ncurses.h>
#include <string.h>
#include <ctype.h>
#include "syntax.h"
#include "files.h"

#define CSS_KEYWORDS_PATTERN \
    "^(color|background|margin|padding|border|font|display|position|top|left|right|bottom|width|height|flex|grid|float|clear)\\b"

static SyntaxRegex CSS_PATTERNS[] = {
    { .pattern = "^/\\*.*\\*/", .attr = COLOR_PAIR(SYNTAX_COMMENT) | A_BOLD },
    { .pattern = "^(\"([^\"\\]|\\.)*\"|'([^'\\]|\\.)*')", .attr = COLOR_PAIR(SYNTAX_STRING) | A_BOLD },
    { .pattern = "^(0[xX][0-9A-Fa-f]+|[0-9]+[a-zA-Z%]*)", .attr = COLOR_PAIR(SYNTAX_TYPE) | A_BOLD },
    { .pattern = CSS_KEYWORDS_PATTERN, .attr = COLOR_PAIR(SYNTAX_KEYWORD) | A_BOLD },
};
static int css_regex_compiled = 0;
static const int CSS_PATTERNS_COUNT = sizeof(CSS_PATTERNS) / sizeof(CSS_PATTERNS[0]);

void highlight_css_syntax(FileState *fs, WINDOW *win, const char *line, int y) {
    (void)fs;
    if (!css_regex_compiled) {
        css_regex_compiled = compile_regex_set(CSS_PATTERNS, CSS_PATTERNS_COUNT);
    }
    if (css_regex_compiled > 0) {
        highlight_regex_line(win, y, 1, line, CSS_PATTERNS, CSS_PATTERNS_COUNT);
    } else {
        static const char *CSS_KEYWORDS[] = {
            "color","background","margin","padding","border","font","display","position","top","left","right","bottom",
            "width","height","flex","grid","float","clear"
        };
        static const int COUNT = sizeof(CSS_KEYWORDS)/sizeof(CSS_KEYWORDS[0]);
        highlight_with_keywords(fs, win, line, y, CSS_KEYWORDS, COUNT);
    }
}

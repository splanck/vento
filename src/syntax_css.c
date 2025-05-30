#include <ncurses.h>
#include <string.h>
#include <ctype.h>
#include "syntax.h"
#include "files.h"

static const char *CSS_KEYWORDS[] = {
    "color", "background", "margin", "padding", "border", "font",
    "display", "position", "top", "left", "right", "bottom",
    "width", "height", "flex", "grid", "float", "clear"
};
static const int CSS_KEYWORDS_COUNT = sizeof(CSS_KEYWORDS) / sizeof(CSS_KEYWORDS[0]);

void highlight_css_syntax(FileState *fs, WINDOW *win, const char *line, int y) {
    highlight_with_keywords(fs, win, line, y, CSS_KEYWORDS, CSS_KEYWORDS_COUNT);
}

#include <ncurses.h>
#include <string.h>
#include <ctype.h>
#include "syntax.h"
#include "files.h"

static const char *C_KEYWORDS[] = {
    "auto", "break", "case", "char", "const", "continue", "default", "do", "double", "else", "enum", "extern", "float",
    "for", "goto", "if", "inline", "int", "long", "register", "restrict", "return", "short", "signed", "sizeof", "static",
    "struct", "switch", "typedef", "union", "unsigned", "void", "volatile", "while", "_Alignas", "_Alignof", "_Atomic",
    "_Bool", "_Complex", "_Generic", "_Imaginary", "_Noreturn", "_Static_assert", "_Thread_local"
};
static const int C_KEYWORDS_COUNT = sizeof(C_KEYWORDS) / sizeof(C_KEYWORDS[0]);

void highlight_c_syntax(FileState *fs, WINDOW *win, const char *line, int y) {
    highlight_with_keywords(fs, win, line, y, C_KEYWORDS, C_KEYWORDS_COUNT);
}

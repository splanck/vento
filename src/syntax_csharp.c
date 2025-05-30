#include <ncurses.h>
#include <string.h>
#include <ctype.h>
#include "syntax.h"
#include "files.h"

static const char *CSHARP_KEYWORDS[] = {
    "abstract", "as", "base", "bool", "break", "byte", "case", "catch", "char",
    "checked", "class", "const", "continue",
    "decimal", "default", "delegate", "do", "double", "else", "enum", "event", "explicit", "extern", "false", "finally",
    "fixed", "float", "for", "foreach", "goto", "if", "implicit", "in", "int", "interface", "internal", "is", "lock",
    "long", "namespace", "new", "null", "object", "operator", "out", "override",
    "params", "private", "protected",
    "public", "readonly", "ref", "return", "sbyte", "sealed", "short", "sizeof",
    "stackalloc", "static", "string",
    "struct", "switch", "this", "throw", "true", "try", "typeof", "uint", "ulong",
    "unchecked", "unsafe", "ushort",
    "using", "virtual", "void", "volatile", "while"
};
static const int CSHARP_KEYWORDS_COUNT = sizeof(CSHARP_KEYWORDS) / sizeof(CSHARP_KEYWORDS[0]);

void highlight_csharp_syntax(FileState *fs, WINDOW *win, const char *line, int y) {
    highlight_with_keywords(fs, win, line, y, CSHARP_KEYWORDS, CSHARP_KEYWORDS_COUNT);
}

#include <ncurses.h>
#include <string.h>
#include <ctype.h>
#include "syntax.h"
#include "files.h"

#define C_KEYWORDS_PATTERN \
    "^(auto|break|case|char|const|continue|default|do|double|else|enum|extern|float|" \
    "for|goto|if|inline|int|long|register|restrict|return|short|signed|sizeof|static|" \
    "struct|switch|typedef|union|unsigned|void|volatile|while|_Alignas|_Alignof|_Atomic|" \
    "_Bool|_Complex|_Generic|_Imaginary|_Noreturn|_Static_assert|_Thread_local)\\b"

static SyntaxRegex C_PATTERNS[] = {
    { .pattern = "^//.*", .attr = COLOR_PAIR(SYNTAX_COMMENT) | A_BOLD },
    { .pattern = "^/\\*.*\\*/", .attr = COLOR_PAIR(SYNTAX_COMMENT) | A_BOLD },
    { .pattern = "^(\"([^\"\\]|\\.)*\"|'([^'\\]|\\.)*')", .attr = COLOR_PAIR(SYNTAX_STRING) | A_BOLD },
    { .pattern = "^(0[xX][0-9A-Fa-f]+|[0-9]+)", .attr = COLOR_PAIR(SYNTAX_TYPE) | A_BOLD },
    { .pattern = C_KEYWORDS_PATTERN, .attr = COLOR_PAIR(SYNTAX_KEYWORD) | A_BOLD },
};
static int c_regex_compiled = 0;
static const int C_PATTERNS_COUNT = sizeof(C_PATTERNS) / sizeof(C_PATTERNS[0]);

void highlight_c_syntax(FileState *fs, WINDOW *win, const char *line, int y) {
    if (!c_regex_compiled) {
        c_regex_compiled = compile_regex_set(C_PATTERNS, C_PATTERNS_COUNT);
    }
    if (c_regex_compiled > 0) {
        highlight_regex_line(win, y, 1, line, C_PATTERNS, C_PATTERNS_COUNT);
    } else {
        static const char *C_KEYWORDS[] = {
            "auto","break","case","char","const","continue","default","do","double","else","enum","extern","float",
            "for","goto","if","inline","int","long","register","restrict","return","short","signed","sizeof","static",
            "struct","switch","typedef","union","unsigned","void","volatile","while","_Alignas","_Alignof","_Atomic",
            "_Bool","_Complex","_Generic","_Imaginary","_Noreturn","_Static_assert","_Thread_local"
        };
        static const int COUNT = sizeof(C_KEYWORDS)/sizeof(C_KEYWORDS[0]);
        highlight_with_keywords(fs, win, line, y, C_KEYWORDS, COUNT);
    }
}

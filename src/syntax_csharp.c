#include <ncurses.h>
#include <string.h>
#include <ctype.h>
#include "syntax.h"
#include "files.h"

#define CSHARP_KEYWORDS_PATTERN \
    "^(abstract|as|base|bool|break|byte|case|catch|char|checked|class|const|continue|decimal|default|delegate|do|double|else|enum|event|explicit|extern|false|finally|fixed|float|for|foreach|goto|if|implicit|in|int|interface|internal|is|lock|long|namespace|new|null|object|operator|out|override|params|private|protected|public|readonly|ref|return|sbyte|sealed|short|sizeof|stackalloc|static|string|struct|switch|this|throw|true|try|typeof|uint|ulong|unchecked|unsafe|ushort|using|virtual|void|volatile|while)\\b"

static SyntaxRegex CSHARP_PATTERNS[] = {
    { .pattern = "^//.*", .attr = COLOR_PAIR(SYNTAX_COMMENT) | A_BOLD },
    { .pattern = "^/\\*.*\\*/", .attr = COLOR_PAIR(SYNTAX_COMMENT) | A_BOLD },
    { .pattern = "^(\"([^\"\\]|\\.)*\"|'([^'\\]|\\.)*')", .attr = COLOR_PAIR(SYNTAX_STRING) | A_BOLD },
    { .pattern = "^(0[xX][0-9A-Fa-f]+|[0-9]+)", .attr = COLOR_PAIR(SYNTAX_TYPE) | A_BOLD },
    { .pattern = CSHARP_KEYWORDS_PATTERN, .attr = COLOR_PAIR(SYNTAX_KEYWORD) | A_BOLD },
};
static int csharp_regex_compiled = 0;
static const int CSHARP_PATTERNS_COUNT = sizeof(CSHARP_PATTERNS)/sizeof(CSHARP_PATTERNS[0]);

void highlight_csharp_syntax(FileState *fs, WINDOW *win, const char *line, int y) {
    (void)fs;
    if (!csharp_regex_compiled) {
        csharp_regex_compiled = compile_regex_set(CSHARP_PATTERNS, CSHARP_PATTERNS_COUNT);
    }
    if (csharp_regex_compiled > 0) {
        highlight_regex_line(win, y, 1, line, CSHARP_PATTERNS, CSHARP_PATTERNS_COUNT);
    } else {
        static const char *CSHARP_KEYWORDS[] = {
            "abstract","as","base","bool","break","byte","case","catch","char","checked","class","const","continue",
            "decimal","default","delegate","do","double","else","enum","event","explicit","extern","false","finally",
            "fixed","float","for","foreach","goto","if","implicit","in","int","interface","internal","is","lock",
            "long","namespace","new","null","object","operator","out","override","params","private","protected",
            "public","readonly","ref","return","sbyte","sealed","short","sizeof","stackalloc","static","string",
            "struct","switch","this","throw","true","try","typeof","uint","ulong","unchecked","unsafe","ushort",
            "using","virtual","void","volatile","while"
        };
        static const int COUNT = sizeof(CSHARP_KEYWORDS)/sizeof(CSHARP_KEYWORDS[0]);
        highlight_with_keywords(fs, win, line, y, CSHARP_KEYWORDS, COUNT);
    }
}

/*
 * syntax_registry.c
 * -----------------
 * Stores available syntax definitions and provides helper functions for
 * registering and retrieving them.  Built-in regex based definitions are
 * registered at startup via init_registry().
 */
#include <ncurses.h>
#include "syntax.h"
#include "files.h"
#include <string.h>

#define SYNTAX_MODE_COUNT 8

/*
 * Table of available syntax definitions indexed by syntax mode.  Entries are
 * populated via syntax_register() during initialization and looked up with
 * syntax_get().
 */
static SyntaxDef registry[SYNTAX_MODE_COUNT];

/*
 * Register a syntax definition.
 *
 * The definition is stored in the `registry` array at the index
 * corresponding to `mode`.  Later lookups through syntax_get() will
 * return a pointer to this stored entry.
 */
void syntax_register(int mode, const SyntaxDef *def) {
    if (mode >= 0 && mode < SYNTAX_MODE_COUNT && def)
        registry[mode] = *def;
}

/*
 * Retrieve a previously registered syntax definition.
 *
 * Returns NULL if `mode` is out of range or if no definition has been
 * registered for that slot.
 */
const SyntaxDef *syntax_get(int mode) {
    if (mode >= 0 && mode < SYNTAX_MODE_COUNT && registry[mode].patterns)
        return &registry[mode];
    return NULL;
}

/*
 * Generic regex based highlighter used by several built-in definitions.
 * The patterns from `def` are compiled on first use and then applied to
 * `line`.  When no patterns are present the line is printed without
 * highlighting.
 */
void highlight_by_patterns(FileState *fs, WINDOW *win, const char *line, int y,
                           const SyntaxDef *def) {
    (void)fs;
    if (!def) {
        highlight_no_syntax(win, line, y);
        return;
    }
    if (def->count > 0 && def->patterns && !def->patterns[0].compiled) {
        compile_regex_set(def->patterns, def->count);
    }
    if (def->count > 0 && def->patterns && def->patterns[0].compiled) {
        highlight_regex_line(win, y, 1, line, def->patterns, def->count);
    } else {
        highlight_no_syntax(win, line, y);
    }
}

/*
 * Built-in syntax definitions
 * ---------------------------
 * Each *_PATTERNS array describes the regex highlighting rules for one
 * language.  The init_registry() constructor registers these definitions so
 * they are available through syntax_get().
 */

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
static const SyntaxDef C_DEF = { ".c", C_PATTERNS, sizeof(C_PATTERNS)/sizeof(C_PATTERNS[0]) };

#define PYTHON_KEYWORDS_PATTERN \
    "^(False|None|True|and|as|assert|async|await|break|class|continue|def|del|elif|else|except|finally|for|from|global|if|import|in|is|lambda|nonlocal|not|or|pass|raise|return|try|while|with|yield)\\b"
static SyntaxRegex PYTHON_PATTERNS[] = {
    { .pattern = "^#.*", .attr = COLOR_PAIR(SYNTAX_COMMENT) | A_BOLD },
    { .pattern = "^(\"([^\"\\]|\\.)*\"|'([^'\\]|\\.)*')", .attr = COLOR_PAIR(SYNTAX_STRING) | A_BOLD },
    { .pattern = "^(0[xX][0-9A-Fa-f]+|[0-9]+)", .attr = COLOR_PAIR(SYNTAX_TYPE) | A_BOLD },
    { .pattern = PYTHON_KEYWORDS_PATTERN, .attr = COLOR_PAIR(SYNTAX_KEYWORD) | A_BOLD },
};
static const SyntaxDef PYTHON_DEF = { ".py", PYTHON_PATTERNS, sizeof(PYTHON_PATTERNS)/sizeof(PYTHON_PATTERNS[0]) };

#define CSHARP_KEYWORDS_PATTERN \
    "^(abstract|as|base|bool|break|byte|case|catch|char|checked|class|const|continue|decimal|default|delegate|do|double|else|enum|event|explicit|extern|false|finally|fixed|float|for|foreach|goto|if|implicit|in|int|interface|internal|is|lock|long|namespace|new|null|object|operator|out|override|params|private|protected|public|readonly|ref|return|sbyte|sealed|short|sizeof|stackalloc|static|string|struct|switch|this|throw|true|try|typeof|uint|ulong|unchecked|unsafe|ushort|using|virtual|void|volatile|while)\\b"
static SyntaxRegex CSHARP_PATTERNS[] = {
    { .pattern = "^//.*", .attr = COLOR_PAIR(SYNTAX_COMMENT) | A_BOLD },
    { .pattern = "^/\\*.*\\*/", .attr = COLOR_PAIR(SYNTAX_COMMENT) | A_BOLD },
    { .pattern = "^(\"([^\"\\]|\\.)*\"|'([^'\\]|\\.)*')", .attr = COLOR_PAIR(SYNTAX_STRING) | A_BOLD },
    { .pattern = "^(0[xX][0-9A-Fa-f]+|[0-9]+)", .attr = COLOR_PAIR(SYNTAX_TYPE) | A_BOLD },
    { .pattern = CSHARP_KEYWORDS_PATTERN, .attr = COLOR_PAIR(SYNTAX_KEYWORD) | A_BOLD },
};
static const SyntaxDef CSHARP_DEF = { ".cs", CSHARP_PATTERNS, sizeof(CSHARP_PATTERNS)/sizeof(CSHARP_PATTERNS[0]) };

#define JS_KEYWORDS_PATTERN \
    "^(break|case|catch|class|const|continue|debugger|default|delete|do|else|export|extends|finally|for|function|if|import|in|instanceof|let|new|return|super|switch|this|throw|try|typeof|var|void|while|with|yield|static)\\b"
static SyntaxRegex JS_PATTERNS[] = {
    { .pattern = "^//.*", .attr = COLOR_PAIR(SYNTAX_COMMENT) | A_BOLD },
    { .pattern = "^/\\*.*\\*/", .attr = COLOR_PAIR(SYNTAX_COMMENT) | A_BOLD },
    { .pattern = "^(\"([^\"\\]|\\.)*\"|'([^'\\]|\\.)*')", .attr = COLOR_PAIR(SYNTAX_STRING) | A_BOLD },
    { .pattern = "^(0[xX][0-9A-Fa-f]+|[0-9]+)", .attr = COLOR_PAIR(SYNTAX_TYPE) | A_BOLD },
    { .pattern = JS_KEYWORDS_PATTERN, .attr = COLOR_PAIR(SYNTAX_KEYWORD) | A_BOLD },
};
static const SyntaxDef JS_DEF = { ".js", JS_PATTERNS, sizeof(JS_PATTERNS)/sizeof(JS_PATTERNS[0]) };

#define CSS_KEYWORDS_PATTERN \
    "^(color|background|margin|padding|border|font|display|position|top|left|right|bottom|width|height|flex|grid|float|clear)\\b"
static SyntaxRegex CSS_PATTERNS[] = {
    { .pattern = "^/\\*.*\\*/", .attr = COLOR_PAIR(SYNTAX_COMMENT) | A_BOLD },
    { .pattern = "^(\"([^\"\\]|\\.)*\"|'([^'\\]|\\.)*')", .attr = COLOR_PAIR(SYNTAX_STRING) | A_BOLD },
    { .pattern = "^(0[xX][0-9A-Fa-f]+|[0-9]+[a-zA-Z%]*)", .attr = COLOR_PAIR(SYNTAX_TYPE) | A_BOLD },
    { .pattern = CSS_KEYWORDS_PATTERN, .attr = COLOR_PAIR(SYNTAX_KEYWORD) | A_BOLD },
};
static const SyntaxDef CSS_DEF = { ".css", CSS_PATTERNS, sizeof(CSS_PATTERNS)/sizeof(CSS_PATTERNS[0]) };

#define SHELL_KEYWORDS_PATTERN \
    "^(if|then|else|fi|for|in|do|done|while|case|esac|function)\\b"
static SyntaxRegex SHELL_PATTERNS[] = {
    { .pattern = "^#.*", .attr = COLOR_PAIR(SYNTAX_COMMENT) | A_BOLD },
    { .pattern = "^(\"([^\"\\]|\\.)*\"|'([^'\\]|\\.)*')", .attr = COLOR_PAIR(SYNTAX_STRING) | A_BOLD },
    { .pattern = "^(0[xX][0-9A-Fa-f]+|[0-9]+)", .attr = COLOR_PAIR(SYNTAX_TYPE) | A_BOLD },
    { .pattern = SHELL_KEYWORDS_PATTERN, .attr = COLOR_PAIR(SYNTAX_KEYWORD) | A_BOLD },
};
static const SyntaxDef SHELL_DEF = { ".sh", SHELL_PATTERNS, sizeof(SHELL_PATTERNS)/sizeof(SHELL_PATTERNS[0]) };

__attribute__((constructor))
static void init_registry(void) {
    syntax_register(C_SYNTAX, &C_DEF);
    syntax_register(PYTHON_SYNTAX, &PYTHON_DEF);
    syntax_register(CSHARP_SYNTAX, &CSHARP_DEF);
    syntax_register(JS_SYNTAX, &JS_DEF);
    syntax_register(CSS_SYNTAX, &CSS_DEF);
    syntax_register(SHELL_SYNTAX, &SHELL_DEF);
}


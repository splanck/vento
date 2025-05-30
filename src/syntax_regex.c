#include "syntax.h"
#include <string.h>

int compile_regex_set(SyntaxRegex *set, int count) {
    int ok = 0;
    for (int i = 0; i < count; i++) {
        if (!set[i].pattern) {
            set[i].compiled = 0;
            continue;
        }
        if (regcomp(&set[i].regex, set[i].pattern, REG_EXTENDED)) {
            set[i].compiled = 0;
        } else {
            set[i].compiled = 1;
            ok++;
        }
    }
    return ok;
}

void free_regex_set(SyntaxRegex *set, int count) {
    for (int i = 0; i < count; i++) {
        if (set[i].compiled) {
            regfree(&set[i].regex);
            set[i].compiled = 0;
        }
    }
}

void highlight_regex_line(WINDOW *win, int y, int x_start, const char *line,
                          SyntaxRegex *patterns, int count) {
    int x = x_start;
    wattrset(win, COLOR_PAIR(SYNTAX_BG));
    const char *p = line;
    while (*p) {
        int matched = 0;
        for (int i = 0; i < count; i++) {
            if (!patterns[i].compiled)
                continue;
            regmatch_t m;
            if (regexec(&patterns[i].regex, p, 1, &m, 0) == 0 && m.rm_so == 0) {
                wattron(win, patterns[i].attr);
                mvwprintw(win, y, x, "%.*s", (int)m.rm_eo, p);
                wattroff(win, patterns[i].attr);
                x += m.rm_eo;
                p += m.rm_eo;
                matched = 1;
                break;
            }
        }
        if (!matched) {
            mvwprintw(win, y, x++, "%c", *p);
            p++;
        }
    }
}


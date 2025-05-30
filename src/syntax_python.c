#include <ncurses.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "syntax.h"
#include "files.h"

#define PYTHON_KEYWORDS_PATTERN \
    "^(False|None|True|and|as|assert|async|await|break|class|continue|def|del|elif|else|except|finally|for|from|global|if|import|in|is|lambda|nonlocal|not|or|pass|raise|return|try|while|with|yield)\\b"

static SyntaxRegex PYTHON_PATTERNS[] = {
    { .pattern = "^#.*", .attr = COLOR_PAIR(SYNTAX_COMMENT) | A_BOLD },
    { .pattern = "^(\"([^\"\\]|\\.)*\"|'([^'\\]|\\.)*')", .attr = COLOR_PAIR(SYNTAX_STRING) | A_BOLD },
    { .pattern = "^(0[xX][0-9A-Fa-f]+|[0-9]+)", .attr = COLOR_PAIR(SYNTAX_TYPE) | A_BOLD },
    { .pattern = PYTHON_KEYWORDS_PATTERN, .attr = COLOR_PAIR(SYNTAX_KEYWORD) | A_BOLD },
};
static int python_regex_compiled = 0;
static const int PYTHON_PATTERNS_COUNT = sizeof(PYTHON_PATTERNS)/sizeof(PYTHON_PATTERNS[0]);

void highlight_python_syntax(FileState *fs, WINDOW *win, const char *line, int y) {
    int i = 0;
    int x = 1;
    wattrset(win, COLOR_PAIR(SYNTAX_BG));

    if (fs->in_multiline_string) {
        bool closed;
        int len = scan_multiline_string(line, 0, fs->string_delim, &closed);
        wattron(win, COLOR_PAIR(SYNTAX_STRING) | A_BOLD);
        mvwprintw(win, y, x, "%.*s", len, line);
        wattroff(win, COLOR_PAIR(SYNTAX_STRING) | A_BOLD);
        fs->in_multiline_string = !closed;
        x += len;
        i += len;
    } else if ((line[0]=='"' || line[0]=='\'') && line[1]==line[0] && line[2]==line[0]) {
        fs->in_multiline_string = true;
        fs->string_delim = line[0];
        bool closed;
        int len = scan_multiline_string(line, 0, fs->string_delim, &closed);
        wattron(win, COLOR_PAIR(SYNTAX_STRING) | A_BOLD);
        mvwprintw(win, y, x, "%.*s", len, line);
        wattroff(win, COLOR_PAIR(SYNTAX_STRING) | A_BOLD);
        fs->in_multiline_string = !closed;
        x += len;
        i += len;
    }

    if (!python_regex_compiled) {
        python_regex_compiled = compile_regex_set(PYTHON_PATTERNS, PYTHON_PATTERNS_COUNT);
    }
    if (python_regex_compiled > 0) {
        highlight_regex_line(win, y, x, line + i, PYTHON_PATTERNS, PYTHON_PATTERNS_COUNT);
    } else {
        int length = strlen(line);
        while (i < length) {
            if (fs->in_multiline_string) {
                bool closed;
                int len = scan_multiline_string(line, i, fs->string_delim, &closed);
                wattron(win, COLOR_PAIR(SYNTAX_STRING) | A_BOLD);
                mvwprintw(win, y, x, "%.*s", len, &line[i]);
                wattroff(win, COLOR_PAIR(SYNTAX_STRING) | A_BOLD);
                fs->in_multiline_string = !closed;
                x += len;
                i += len;
                continue;
            }
            if ((line[i] == '"' || line[i] == '\'' ) && i + 2 < length && line[i+1]==line[i] && line[i+2]==line[i]) {
                fs->in_multiline_string = true;
                fs->string_delim = line[i];
                bool closed;
                int len = scan_multiline_string(line, i, fs->string_delim, &closed);
                wattron(win, COLOR_PAIR(SYNTAX_STRING) | A_BOLD);
                mvwprintw(win, y, x, "%.*s", len, &line[i]);
                wattroff(win, COLOR_PAIR(SYNTAX_STRING) | A_BOLD);
                fs->in_multiline_string = !closed;
                x += len;
                i += len;
            } else if (isalpha((unsigned char)line[i]) || line[i] == '_') {
                int len = scan_identifier(line, i);
                char word[256];
                if (len >= (int)sizeof(word))
                    len = (int)sizeof(word) - 1;
                strncpy(word, &line[i], len);
                word[len] = '\0';
                int is_keyword = 0;
                static const char *KW[] = {"False","None","True","and","as","assert","async","await","break","class","continue","def","del","elif","else","except","finally","for","from","global","if","import","in","is","lambda","nonlocal","not","or","pass","raise","return","try","while","with","yield"};
                for (int j=0;j<35;j++){ if(strcmp(word,KW[j])==0){ is_keyword=1; break; }}
                if (is_keyword){ wattron(win,COLOR_PAIR(SYNTAX_KEYWORD)|A_BOLD); mvwprintw(win,y,x,"%s",word); wattroff(win,COLOR_PAIR(SYNTAX_KEYWORD)|A_BOLD);} else { mvwprintw(win,y,x,"%s",word); }
                x+=len; i+=len;
            } else if (isdigit((unsigned char)line[i])) {
                int len = scan_number(line, i);
                wattron(win, COLOR_PAIR(SYNTAX_TYPE) | A_BOLD);
                mvwprintw(win, y, x, "%.*s", len, &line[i]);
                wattroff(win, COLOR_PAIR(SYNTAX_TYPE) | A_BOLD);
                x+=len; i+=len;
            } else if (line[i] == '#') {
                wattron(win, COLOR_PAIR(SYNTAX_COMMENT)|A_BOLD);
                mvwprintw(win, y, x, "%s", &line[i]);
                wattroff(win, COLOR_PAIR(SYNTAX_COMMENT)|A_BOLD);
                break;
            } else if (line[i]=='"' || line[i]=='\'') {
                bool closed; int len = scan_string(line,i,line[i],&closed);
                wattron(win,COLOR_PAIR(SYNTAX_STRING)|A_BOLD);
                mvwprintw(win,y,x,"%.*s",len,&line[i]);
                wattroff(win,COLOR_PAIR(SYNTAX_STRING)|A_BOLD);
                x+=len; i+=len;
            } else {
                mvwprintw(win,y,x++,"%c",line[i++]);
            }
        }
    }
}

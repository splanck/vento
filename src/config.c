#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <pwd.h>
#include <unistd.h>
#include "editor.h"
#include "config.h"
#include "syntax.h"

int enable_color = 1; // global flag used throughout the editor

// default application configuration
AppConfig app_config = {
    .background_color = "BLACK",
    .keyword_color = "CYAN",
    .comment_color = "GREEN",
    .string_color = "YELLOW",
    .type_color = "MAGENTA",
    .symbol_color = "RED",
    .enable_color = 1
};

// Helper to map color name to ncurses constant
short get_color_code(const char *color_name) {
    if (strcmp(color_name, "BLACK") == 0) return COLOR_BLACK;
    if (strcmp(color_name, "RED") == 0) return COLOR_RED;
    if (strcmp(color_name, "GREEN") == 0) return COLOR_GREEN;
    if (strcmp(color_name, "YELLOW") == 0) return COLOR_YELLOW;
    if (strcmp(color_name, "BLUE") == 0) return COLOR_BLUE;
    if (strcmp(color_name, "MAGENTA") == 0) return COLOR_MAGENTA;
    if (strcmp(color_name, "CYAN") == 0) return COLOR_CYAN;
    if (strcmp(color_name, "WHITE") == 0) return COLOR_WHITE;
    return -1;
}

static void get_config_path(char *buf, size_t size) {
    struct passwd *pw = getpwuid(getuid());
    const char *homedir = pw ? pw->pw_dir : getenv("HOME");
    if (!homedir) homedir = "";
    snprintf(buf, size, "%s/.ventorc", homedir);
}

static void trim(char *str) {
    char *start = str;
    while (isspace((unsigned char)*start)) start++;
    char *end = start + strlen(start);
    while (end > start && isspace((unsigned char)*(end - 1))) end--;
    size_t len = end - start;
    memmove(str, start, len);
    str[len] = '\0';
}

void config_save(const AppConfig *cfg) {
    char path[256];
    get_config_path(path, sizeof(path));
    FILE *f = fopen(path, "w");
    if (!f) return;
    fprintf(f, "background_color=%s\n", cfg->background_color);
    fprintf(f, "keyword_color=%s\n", cfg->keyword_color);
    fprintf(f, "comment_color=%s\n", cfg->comment_color);
    fprintf(f, "string_color=%s\n", cfg->string_color);
    fprintf(f, "type_color=%s\n", cfg->type_color);
    fprintf(f, "symbol_color=%s\n", cfg->symbol_color);
    fprintf(f, "enable_color=%s\n", cfg->enable_color ? "true" : "false");
    fclose(f);
}

void config_load(AppConfig *cfg) {
    char path[256];
    get_config_path(path, sizeof(path));
    FILE *file = fopen(path, "r");
    if (!file) {
        config_save(cfg); // create default config
        file = fopen(path, "r");
        if (!file) return;
    }

    AppConfig tmp = *cfg;
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char *p = line;
        while (isspace((unsigned char)*p)) p++;
        if (*p == '#' || *p == '\0' || *p == '\n')
            continue;
        line[strcspn(line, "\n")] = '\0';
        char *eq = strchr(p, '=');
        if (!eq) continue;
        *eq++ = '\0';
        char *key = p;
        char *value = eq;
        trim(key);
        trim(value);

        if (strcmp(key, "background_color") == 0) {
            strncpy(tmp.background_color, value, sizeof(tmp.background_color)-1);
            tmp.background_color[sizeof(tmp.background_color)-1] = '\0';
        } else if (strcmp(key, "keyword_color") == 0) {
            strncpy(tmp.keyword_color, value, sizeof(tmp.keyword_color)-1);
            tmp.keyword_color[sizeof(tmp.keyword_color)-1] = '\0';
        } else if (strcmp(key, "comment_color") == 0) {
            strncpy(tmp.comment_color, value, sizeof(tmp.comment_color)-1);
            tmp.comment_color[sizeof(tmp.comment_color)-1] = '\0';
        } else if (strcmp(key, "string_color") == 0) {
            strncpy(tmp.string_color, value, sizeof(tmp.string_color)-1);
            tmp.string_color[sizeof(tmp.string_color)-1] = '\0';
        } else if (strcmp(key, "type_color") == 0) {
            strncpy(tmp.type_color, value, sizeof(tmp.type_color)-1);
            tmp.type_color[sizeof(tmp.type_color)-1] = '\0';
        } else if (strcmp(key, "symbol_color") == 0) {
            strncpy(tmp.symbol_color, value, sizeof(tmp.symbol_color)-1);
            tmp.symbol_color[sizeof(tmp.symbol_color)-1] = '\0';
        } else if (strcmp(key, "enable_color") == 0) {
            tmp.enable_color = (strcmp(value, "true") == 0 || strcmp(value, "1") == 0);
        } else {
            // Unknown key, ignore
            continue;
        }
    }
    fclose(file);

    *cfg = tmp;
    enable_color = cfg->enable_color;

    if (enable_color) {
        start_color();
        short code;
        code = get_color_code(cfg->background_color);
        if (code != -1) init_pair(SYNTAX_BG, COLOR_WHITE, code);
        code = get_color_code(cfg->keyword_color);
        if (code != -1) init_pair(SYNTAX_KEYWORD, code, COLOR_BLACK);
        code = get_color_code(cfg->comment_color);
        if (code != -1) init_pair(SYNTAX_COMMENT, code, COLOR_BLACK);
        code = get_color_code(cfg->string_color);
        if (code != -1) init_pair(SYNTAX_STRING, code, COLOR_BLACK);
        code = get_color_code(cfg->type_color);
        if (code != -1) init_pair(SYNTAX_TYPE, code, COLOR_BLACK);
        code = get_color_code(cfg->symbol_color);
        if (code != -1) init_pair(SYNTAX_SYMBOL, code, COLOR_BLACK);
    }
}

// Compatibility wrapper
void read_config_file() {
    config_load(&app_config);
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <pwd.h>
#include <unistd.h>
#include "editor.h"
#include "config.h"
#include "syntax.h"

int enable_color = 1; // global flag used throughout the editor
int enable_mouse = 1; // global mouse flag
int show_line_numbers = 0; // global line number flag

// default application configuration
AppConfig app_config = {
    .background_color = "BLACK",
    .keyword_color = "CYAN",
    .comment_color = "GREEN",
    .string_color = "YELLOW",
    .type_color = "MAGENTA",
    .symbol_color = "RED",
    .theme = "",
    .enable_color = 1,
    .enable_mouse = 1,
    .show_line_numbers = 0
};

// Helper to map color name to ncurses constant
short get_color_code(const char *color_name) {
    if (strcasecmp(color_name, "BLACK") == 0) return COLOR_BLACK;
    if (strcasecmp(color_name, "RED") == 0) return COLOR_RED;
    if (strcasecmp(color_name, "GREEN") == 0) return COLOR_GREEN;
    if (strcasecmp(color_name, "YELLOW") == 0) return COLOR_YELLOW;
    if (strcasecmp(color_name, "BLUE") == 0) return COLOR_BLUE;
    if (strcasecmp(color_name, "MAGENTA") == 0) return COLOR_MAGENTA;
    if (strcasecmp(color_name, "CYAN") == 0) return COLOR_CYAN;
    if (strcasecmp(color_name, "WHITE") == 0) return COLOR_WHITE;
    return -1;
}

static void get_config_path(char *buf, size_t size) {
    struct passwd *pw = getpwuid(getuid());
    const char *homedir = pw ? pw->pw_dir : getenv("HOME");
    if (!homedir || homedir[0] == '\0')
        homedir = ".";
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

void load_theme(const char *name, AppConfig *cfg) {
    if (!name || !*name || !cfg)
        return;

    char path[256];
    snprintf(path, sizeof(path), "themes/%s.theme", name);
    FILE *f = fopen(path, "r");
    if (!f)
        return;

    char line[256];
    while (fgets(line, sizeof(line), f)) {
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
            strncpy(cfg->background_color, value, sizeof(cfg->background_color)-1);
            cfg->background_color[sizeof(cfg->background_color)-1] = '\0';
        } else if (strcmp(key, "keyword_color") == 0) {
            strncpy(cfg->keyword_color, value, sizeof(cfg->keyword_color)-1);
            cfg->keyword_color[sizeof(cfg->keyword_color)-1] = '\0';
        } else if (strcmp(key, "comment_color") == 0) {
            strncpy(cfg->comment_color, value, sizeof(cfg->comment_color)-1);
            cfg->comment_color[sizeof(cfg->comment_color)-1] = '\0';
        } else if (strcmp(key, "string_color") == 0) {
            strncpy(cfg->string_color, value, sizeof(cfg->string_color)-1);
            cfg->string_color[sizeof(cfg->string_color)-1] = '\0';
        } else if (strcmp(key, "type_color") == 0) {
            strncpy(cfg->type_color, value, sizeof(cfg->type_color)-1);
            cfg->type_color[sizeof(cfg->type_color)-1] = '\0';
        } else if (strcmp(key, "symbol_color") == 0) {
            strncpy(cfg->symbol_color, value, sizeof(cfg->symbol_color)-1);
            cfg->symbol_color[sizeof(cfg->symbol_color)-1] = '\0';
        }
    }
    fclose(f);
}

void config_save(const AppConfig *cfg) {
    static const char *keys[] = {
        "background_color",
        "keyword_color",
        "comment_color",
        "string_color",
        "type_color",
        "symbol_color",
        "theme",
        "enable_color",
        "enable_mouse",
        "show_line_numbers"
    };

    char path[256];
    get_config_path(path, sizeof(path));
    FILE *f = fopen(path, "w");
    if (!f) return;

    fprintf(f, "%s=%s\n", keys[0], cfg->background_color);
    fprintf(f, "%s=%s\n", keys[1], cfg->keyword_color);
    fprintf(f, "%s=%s\n", keys[2], cfg->comment_color);
    fprintf(f, "%s=%s\n", keys[3], cfg->string_color);
    fprintf(f, "%s=%s\n", keys[4], cfg->type_color);
    fprintf(f, "%s=%s\n", keys[5], cfg->symbol_color);
    fprintf(f, "%s=%s\n", keys[6], cfg->theme);
    fprintf(f, "%s=%s\n", keys[7], cfg->enable_color ? "true" : "false");
    fprintf(f, "%s=%s\n", keys[8], cfg->enable_mouse ? "true" : "false");
    fprintf(f, "%s=%s\n", keys[9], cfg->show_line_numbers ? "true" : "false");
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
    char theme_name[sizeof(tmp.theme)] = "";
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

        if (strcmp(key, "theme") == 0) {
            strncpy(theme_name, value, sizeof(theme_name) - 1);
            theme_name[sizeof(theme_name) - 1] = '\0';
        }
    }
    fclose(file);

    if (theme_name[0]) {
        strncpy(tmp.theme, theme_name, sizeof(tmp.theme) - 1);
        tmp.theme[sizeof(tmp.theme) - 1] = '\0';
        load_theme(theme_name, &tmp);
    }

    file = fopen(path, "r");
    if (!file) {
        *cfg = tmp;
        return;
    }

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
        } else if (strcmp(key, "theme") == 0) {
            strncpy(tmp.theme, value, sizeof(tmp.theme)-1);
            tmp.theme[sizeof(tmp.theme)-1] = '\0';
        } else if (strcmp(key, "enable_color") == 0) {
            tmp.enable_color = (strcmp(value, "true") == 0 || strcmp(value, "1") == 0);
        } else if (strcmp(key, "enable_mouse") == 0) {
            tmp.enable_mouse = (strcmp(value, "true") == 0 || strcmp(value, "1") == 0);
        } else if (strcmp(key, "show_line_numbers") == 0) {
            tmp.show_line_numbers = (strcmp(value, "true") == 0 || strcmp(value, "1") == 0);
        } else {
            // Unknown key, ignore
            continue;
        }
    }
    fclose(file);

    *cfg = tmp;
    enable_color = cfg->enable_color;
    enable_mouse = cfg->enable_mouse;
    show_line_numbers = cfg->show_line_numbers;

    if (enable_color) {
        if (!has_colors()) {
            enable_color = 0;
        } else {
            start_color();
            use_default_colors();
            short code;
            short bg = get_color_code(cfg->background_color);
            if (bg == -1) bg = -1;
            init_pair(SYNTAX_BG, COLOR_WHITE, bg);
            code = get_color_code(cfg->keyword_color);
            if (code != -1) init_pair(SYNTAX_KEYWORD, code, -1);
            code = get_color_code(cfg->comment_color);
            if (code != -1) init_pair(SYNTAX_COMMENT, code, -1);
            code = get_color_code(cfg->string_color);
            if (code != -1) init_pair(SYNTAX_STRING, code, -1);
            code = get_color_code(cfg->type_color);
            if (code != -1) init_pair(SYNTAX_TYPE, code, -1);
            code = get_color_code(cfg->symbol_color);
            if (code != -1) init_pair(SYNTAX_SYMBOL, code, -1);
        }
    }
}

// Compatibility wrapper
void read_config_file() {
    config_load(&app_config);
}

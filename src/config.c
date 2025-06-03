#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <pwd.h>
#include <dirent.h>
#include <unistd.h>
#include <limits.h>
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif
#include "editor.h"
#include "config.h"
#include "editor_state.h"
#include "syntax.h"
#include "macro.h"
#include <wchar.h>

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
    const char *cfg = getenv("VENTO_CONFIG");
    if (cfg && *cfg) {
        strncpy(buf, cfg, size - 1);
        buf[size - 1] = '\0';
        return;
    }

    struct passwd *pw = getpwuid(getuid());
    const char *homedir = pw ? pw->pw_dir : getenv("HOME");
    if (!homedir || homedir[0] == '\0')
        homedir = ".";
    snprintf(buf, size, "%s/.ventorc", homedir);
}

static void get_macros_path(char *buf, size_t size) {
    const char *mp = getenv("VENTO_MACROS");
    if (mp && *mp) {
        strncpy(buf, mp, size - 1);
        buf[size - 1] = '\0';
        return;
    }

    struct passwd *pw = getpwuid(getuid());
    const char *homedir = pw ? pw->pw_dir : getenv("HOME");
    if (!homedir || homedir[0] == '\0')
        homedir = ".";
    snprintf(buf, size, "%s/.ventomacros", homedir);
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

    char path[PATH_MAX] = "";

    const char *dirs[3];
    size_t dir_count = 0;

    const char *env_dir = getenv("VENTO_THEME_DIR");
    if (env_dir && *env_dir)
        dirs[dir_count++] = env_dir;

    dirs[dir_count++] = THEME_DIR;

    if (strcmp(THEME_DIR, "themes") != 0)
        dirs[dir_count++] = "themes";

    for (size_t i = 0; i < dir_count; ++i) {
        DIR *dir = opendir(dirs[i]);
        if (!dir)
            continue;
        struct dirent *ent;
        while ((ent = readdir(dir)) != NULL) {
            if (ent->d_name[0] == '.')
                continue;
            const char *dot = strrchr(ent->d_name, '.');
            if (!dot || strcasecmp(dot, ".theme") != 0)
                continue;
            size_t len = dot - ent->d_name;
            if (len == strlen(name) && strncasecmp(ent->d_name, name, len) == 0) {
                int n = snprintf(path, sizeof(path), "%s/%s", dirs[i], ent->d_name);
                if (n < 0 || (size_t)n >= sizeof(path))
                    path[0] = '\0';
                break;
            }
        }
        closedir(dir);
        if (path[0])
            break;
    }

    if (path[0] == '\0') {
        int n = snprintf(path, sizeof(path), "%s/%s.theme", dirs[0], name);
        if (n < 0 || (size_t)n >= sizeof(path))
            path[0] = '\0';
    }

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
        } else if (strcmp(key, "text_color") == 0) {
            strncpy(cfg->text_color, value, sizeof(cfg->text_color)-1);
            cfg->text_color[sizeof(cfg->text_color)-1] = '\0';
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
        } else if (strcmp(key, "search_color") == 0) {
            strncpy(cfg->search_color, value, sizeof(cfg->search_color)-1);
            cfg->search_color[sizeof(cfg->search_color)-1] = '\0';
        }
    }
    fclose(f);
}

void config_save(const AppConfig *cfg) {
    static const char *keys[] = {
        "background_color",
        "text_color",
        "keyword_color",
        "comment_color",
        "string_color",
        "type_color",
        "symbol_color",
        "search_color",
        "theme",
        "enable_color",
        "enable_mouse",
        "show_line_numbers",
        "show_startup_warning",
        "search_ignore_case",
        "tab_width",
        "macros_file",
        "macro_record_key",
        "macro_play_key"
    };

    char path[PATH_MAX];
    get_config_path(path, sizeof(path));
    FILE *f = fopen(path, "w");
    if (!f) return;

    fprintf(f, "%s=%s\n", keys[0], cfg->background_color);
    fprintf(f, "%s=%s\n", keys[1], cfg->text_color);
    fprintf(f, "%s=%s\n", keys[2], cfg->keyword_color);
    fprintf(f, "%s=%s\n", keys[3], cfg->comment_color);
    fprintf(f, "%s=%s\n", keys[4], cfg->string_color);
    fprintf(f, "%s=%s\n", keys[5], cfg->type_color);
    fprintf(f, "%s=%s\n", keys[6], cfg->symbol_color);
    fprintf(f, "%s=%s\n", keys[7], cfg->search_color);
    fprintf(f, "%s=%s\n", keys[8], cfg->theme);
    fprintf(f, "%s=%s\n", keys[9], cfg->enable_color ? "true" : "false");
    fprintf(f, "%s=%s\n", keys[10], cfg->enable_mouse ? "true" : "false");
    fprintf(f, "%s=%s\n", keys[11], cfg->show_line_numbers ? "true" : "false");
    fprintf(f, "%s=%s\n", keys[12], cfg->show_startup_warning ? "true" : "false");
    fprintf(f, "%s=%s\n", keys[13], cfg->search_ignore_case ? "true" : "false");
    fprintf(f, "%s=%d\n", keys[14], cfg->tab_width);
    fprintf(f, "%s=%s\n", keys[15], cfg->macros_file);
    fprintf(f, "%s=%d\n", keys[16], cfg->macro_record_key);
    fprintf(f, "%s=%d\n", keys[17], cfg->macro_play_key);
    fclose(f);
}

void config_load(AppConfig *cfg) {
    char path[PATH_MAX];
    get_config_path(path, sizeof(path));
    FILE *file = fopen(path, "r");
    if (!file) {
        config_save(cfg); // create default config
        file = fopen(path, "r");
        if (!file) return;
    }

    AppConfig tmp = *cfg;
    if (tmp.macros_file[0] == '\0')
        get_macros_path(tmp.macros_file, sizeof(tmp.macros_file));
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
        } else if (strcmp(key, "text_color") == 0) {
            strncpy(tmp.text_color, value, sizeof(tmp.text_color)-1);
            tmp.text_color[sizeof(tmp.text_color)-1] = '\0';
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
        } else if (strcmp(key, "search_color") == 0) {
            strncpy(tmp.search_color, value, sizeof(tmp.search_color)-1);
            tmp.search_color[sizeof(tmp.search_color)-1] = '\0';
        } else if (strcmp(key, "theme") == 0) {
            strncpy(tmp.theme, value, sizeof(tmp.theme)-1);
            tmp.theme[sizeof(tmp.theme)-1] = '\0';
        } else if (strcmp(key, "enable_color") == 0) {
            tmp.enable_color = (strcmp(value, "true") == 0 || strcmp(value, "1") == 0);
        } else if (strcmp(key, "enable_mouse") == 0) {
            tmp.enable_mouse = (strcmp(value, "true") == 0 || strcmp(value, "1") == 0);
        } else if (strcmp(key, "show_line_numbers") == 0) {
            tmp.show_line_numbers = (strcmp(value, "true") == 0 || strcmp(value, "1") == 0);
        } else if (strcmp(key, "show_startup_warning") == 0) {
            tmp.show_startup_warning = (strcmp(value, "true") == 0 || strcmp(value, "1") == 0);
        } else if (strcmp(key, "search_ignore_case") == 0) {
            tmp.search_ignore_case = (strcmp(value, "true") == 0 || strcmp(value, "1") == 0);
        } else if (strcmp(key, "tab_width") == 0) {
            tmp.tab_width = atoi(value);
            if (tmp.tab_width <= 0)
                tmp.tab_width = 4;
        } else if (strcmp(key, "macros_file") == 0) {
            strncpy(tmp.macros_file, value, sizeof(tmp.macros_file) - 1);
            tmp.macros_file[sizeof(tmp.macros_file) - 1] = '\0';
        } else if (strcmp(key, "macro_record_key") == 0) {
            tmp.macro_record_key = atoi(value);
        } else if (strcmp(key, "macro_play_key") == 0) {
            tmp.macro_play_key = atoi(value);
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
    key_macro_record = cfg->macro_record_key;
    key_macro_play = cfg->macro_play_key;

    if (enable_color) {
        if (!has_colors()) {
            enable_color = 0;
        } else {
            start_color();
            use_default_colors();
            short code;
            short bg = get_color_code(cfg->background_color);
            if (bg == -1) bg = -1;
            short fg = get_color_code(cfg->text_color);
            if (fg == -1) fg = COLOR_WHITE;
            init_pair(SYNTAX_BG, fg, bg);

            code = get_color_code(cfg->keyword_color);
            if (code != -1) init_pair(SYNTAX_KEYWORD, code, bg);

            code = get_color_code(cfg->comment_color);
            if (code != -1) init_pair(SYNTAX_COMMENT, code, bg);

            code = get_color_code(cfg->string_color);
            if (code != -1) init_pair(SYNTAX_STRING, code, bg);

            code = get_color_code(cfg->type_color);
            if (code != -1) init_pair(SYNTAX_TYPE, code, bg);

            code = get_color_code(cfg->symbol_color);
            if (code != -1) init_pair(SYNTAX_SYMBOL, code, bg);

            code = get_color_code(cfg->search_color);
            if (code != -1) init_pair(SYNTAX_SEARCH, code, bg);
        }
    }

    macros_load(cfg);
}

// Compatibility wrapper
void read_config_file(AppConfig *cfg) {
    if (cfg)
        config_load(cfg);
}

void macros_load(AppConfig *cfg) {
    if (!cfg)
        return;
    if (cfg->macros_file[0] == '\0')
        get_macros_path(cfg->macros_file, sizeof(cfg->macros_file));
    FILE *f = fopen(cfg->macros_file, "r");
    if (!f)
        return;
    char line[4096];
    while (fgets(line, sizeof(line), f)) {
        char *p = line;
        while (isspace((unsigned char)*p)) p++;
        if (*p == '#' || *p == '\0' || *p == '\n')
            continue;
        line[strcspn(line, "\n")] = '\0';
        char *name = strtok(p, " \t");
        char *len_s = strtok(NULL, " \t");
        if (!name || !len_s)
            continue;
        int len = atoi(len_s);
        Macro *m = macro_get(name);
        if (!m)
            m = macro_create(name);
        if (!m)
            continue;
        m->length = 0;
        for (int i = 0; i < len; ++i) {
            char *tok = strtok(NULL, " \t");
            if (!tok)
                break;
            m->keys[m->length++] = (wint_t)strtoul(tok, NULL, 10);
        }
        m->recording = false;
    }
    fclose(f);
}

void macros_save(const AppConfig *cfg) {
    if (!cfg)
        return;
    char path[PATH_MAX];
    if (cfg->macros_file[0] == '\0')
        get_macros_path(path, sizeof(path));
    else
        strncpy(path, cfg->macros_file, sizeof(path) - 1), path[sizeof(path)-1]='\0';

    FILE *f = fopen(path, "w");
    if (!f)
        return;
    int count = macro_count();
    for (int i = 0; i < count; ++i) {
        Macro *m = macro_at(i);
        if (!m)
            continue;
        fprintf(f, "%s %d", m->name, m->length);
        for (int j = 0; j < m->length; ++j)
            fprintf(f, " %u", (unsigned int)m->keys[j]);
        fputc('\n', f);
    }
    fclose(f);
}

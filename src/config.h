#ifndef CONFIG_H
#define CONFIG_H

#define VERSION "0.1.3"

extern int enable_color;

typedef struct {
    char background_color[16];
    char keyword_color[16];
    char comment_color[16];
    char string_color[16];
    char type_color[16];
    char symbol_color[16];
    int enable_color;
} AppConfig;

extern AppConfig app_config;

short get_color_code(const char *color_name);
void config_load(AppConfig *cfg);
void config_save(const AppConfig *cfg);
void read_config_file(void);

#endif // CONFIG_H

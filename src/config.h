#ifndef CONFIG_H
#define CONFIG_H

#define VERSION "0.1.3"

/* Directory containing installed color themes. Can be overridden at compile
 * time by defining THEME_DIR. Defaults to "themes" which resolves to the
 * local directory at runtime. */
#ifndef THEME_DIR
#define THEME_DIR "themes"
#endif

extern int enable_color;
extern int enable_mouse;
extern int show_line_numbers;

typedef struct {
    char background_color[16];
    char text_color[16];
    char keyword_color[16];
    char comment_color[16];
    char string_color[16];
    char type_color[16];
    char symbol_color[16];
    char search_color[16];
    char theme[32];
    int enable_color;
    int enable_mouse;
    int show_line_numbers;
    int show_startup_warning;
    int search_ignore_case;
    int tab_width;
} AppConfig;

extern AppConfig app_config;

short get_color_code(const char *color_name);
void load_theme(const char *name, AppConfig *cfg);
void config_load(AppConfig *cfg);
void config_save(const AppConfig *cfg);
void read_config_file(AppConfig *cfg);

#endif // CONFIG_H

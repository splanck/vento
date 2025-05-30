#ifndef UI_H
#define UI_H

#include "config.h"
#include <ncurses.h>

void show_help(void);
void show_about(void);
void create_dialog(const char *message, char *output, int max_input_len);
void show_warning_dialog(void);
int show_find_dialog(char *output, int max_input_len, const char *preset);
int show_replace_dialog(char *search, int max_search_len,
                        char *replace, int max_replace_len);
int show_open_file_dialog(char *path, int max_len);
int show_save_file_dialog(char *path, int max_len);
int show_settings_dialog(AppConfig *cfg);

#endif // UI_H

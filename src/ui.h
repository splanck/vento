#ifndef UI_H
#define UI_H

#include "config.h"

void show_help(void);
void show_about(void);
void create_dialog(const char *message, char *output, int max_input_len);
void show_warning_dialog(void);
void get_dir_contents(const char *dir_path, char ***choices, int *n_choices);
void free_dir_contents(char **choices, int n_choices);
void show_find_dialog(char *output, int max_input_len);
int show_open_file_dialog(char *path, int max_len);
int show_save_file_dialog(char *path, int max_len);
void show_settings_dialog(AppConfig *cfg);

#endif // UI_H

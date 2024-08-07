#ifndef UI_H
#define UI_H

void show_help();
void show_about();
void create_dialog(const char *message, char *output, int max_input_len);
void show_warning_dialog();
void get_dir_contents(const char *dir_path, char ***choices, int *n_choices);
void free_dir_contents(char **choices, int n_choices);
void show_find_dialog(char *output, int max_input_len);
int show_select_file(char *selected_path, int max_len);

#endif // UI_H

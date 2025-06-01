#ifndef UI_H
#define UI_H

#include "config.h"
#include "editor_state.h"
#include <ncurses.h>

void show_help(EditorContext *ctx);
void show_about(EditorContext *ctx);
void create_dialog(EditorContext *ctx, const char *message, char *output,
                   int max_input_len);
void show_warning_dialog(EditorContext *ctx);
int show_find_dialog(EditorContext *ctx, char *output, int max_input_len,
                     const char *preset);
int show_replace_dialog(EditorContext *ctx, char *search, int max_search_len,
                        char *replace, int max_replace_len);
int show_goto_dialog(EditorContext *ctx, int *line_number);
int show_open_file_dialog(EditorContext *ctx, char *path, int max_len);
int show_save_file_dialog(EditorContext *ctx, char *path, int max_len);
int show_settings_dialog(EditorContext *ctx, AppConfig *cfg);

#endif // UI_H

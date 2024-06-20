#ifndef EDITOR_H
#define EDITOR_H

#include <ncurses.h>

#define MAX_LINES 1000

// Define custom key constants for CTRL-Left, CTRL-Right, CTRL-Page Up, CTRL-Page Down
#define KEY_CTRL_LEFT  1000
#define KEY_CTRL_RIGHT 1001
#define KEY_CTRL_PGUP  1002
#define KEY_CTRL_PGDN  1003
#define KEY_CTRL_UP    1004
#define KEY_CTRL_DOWN  1005
#define KEY_CTRL_Q     1007 

void initialize();
void draw_text_buffer(WINDOW *win);
void clear_text_buffer();
void run_editor();
void initialize_buffer();
void create_dialog(const char *message, char *output, int max_input_len);
void save_file();
void load_file();
void new_file();
void update_status_bar(int cursor_y, int cursor_x);
void handle_resize(int sig);

extern WINDOW *text_win; // Add this line
extern char *text_buffer[MAX_LINES];
extern int line_count;
extern int start_line;

#endif

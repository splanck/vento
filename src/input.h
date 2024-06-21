#ifndef INPUT_H
#define INPUT_H

void handle_ctrl_backtick();
void handle_key_up(int *cursor_y, int *start_line);
void handle_key_down(int *cursor_y, int *start_line);
void handle_key_left(int *cursor_x);
void handle_key_right(int *cursor_x, int cursor_y);
void handle_key_backspace(int *cursor_x, int *cursor_y, int *start_line);
void handle_key_delete(int *cursor_x, int cursor_y);
void handle_key_enter(int *cursor_x, int *cursor_y, int *start_line);
void handle_key_page_up(int *cursor_y, int *start_line);
void handle_key_page_down(int *cursor_y, int *start_line);
void handle_ctrl_key_left(int *cursor_x);
void handle_ctrl_key_right(int *cursor_x, int cursor_y);
void handle_ctrl_key_pgup(int *cursor_y, int *start_line);
void handle_ctrl_key_pgdn(int *cursor_y, int *start_line);
void handle_ctrl_key_up(int *cursor_y);
void handle_ctrl_key_down(int *cursor_y);
void handle_default_key(int ch, int *cursor_x, int cursor_y);

#endif
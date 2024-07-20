#ifndef INPUT_H
#define INPUT_H

#define BOTTOM_MARGIN 4 // Define the bottom UI margin

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
void move_forward_to_next_word(int *cursor_x, int *cursor_y);
void move_backward_to_previous_word(int *cursor_x, int *cursor_y);

#endif
#ifndef INPUT_H
#define INPUT_H

#define BOTTOM_MARGIN 4 // Define the bottom UI margin

#include "files.h"
void handle_ctrl_backtick();
void handle_key_up(struct FileState *fs);
void handle_key_down(struct FileState *fs);
void handle_key_left(struct FileState *fs);
void handle_key_right(struct FileState *fs);
void handle_key_backspace(struct FileState *fs);
void handle_key_delete(struct FileState *fs);
void handle_key_enter(struct FileState *fs);
void handle_key_page_up(struct FileState *fs);
void handle_key_page_down(struct FileState *fs);
void handle_ctrl_key_left(struct FileState *fs);
void handle_ctrl_key_right(struct FileState *fs);
void handle_ctrl_key_pgup(struct FileState *fs);
void handle_ctrl_key_pgdn(struct FileState *fs);
void handle_ctrl_key_up(struct FileState *fs);
void handle_ctrl_key_down(struct FileState *fs);
void handle_default_key(struct FileState *fs, int ch);
void move_forward_to_next_word(struct FileState *fs);
void move_backward_to_previous_word(struct FileState *fs);

#endif
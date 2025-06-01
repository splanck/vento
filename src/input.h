#ifndef INPUT_H
#define INPUT_H

#define BOTTOM_MARGIN 4 // Define the bottom UI margin

#include "files.h"
#include "editor_state.h"
void handle_ctrl_backtick(EditorContext *ctx);
void handle_key_up(EditorContext *ctx, struct FileState *fs);
void handle_key_down(EditorContext *ctx, struct FileState *fs);
void handle_key_left(EditorContext *ctx, struct FileState *fs);
void handle_key_right(EditorContext *ctx, struct FileState *fs);
void handle_key_home(EditorContext *ctx, struct FileState *fs);
void handle_key_end(EditorContext *ctx, struct FileState *fs);
void handle_key_backspace(EditorContext *ctx, struct FileState *fs);
void handle_key_delete(EditorContext *ctx, struct FileState *fs);
void handle_key_enter(EditorContext *ctx, struct FileState *fs);
void handle_key_page_up(EditorContext *ctx, struct FileState *fs);
void handle_key_page_down(EditorContext *ctx, struct FileState *fs);
void handle_ctrl_key_left(EditorContext *ctx, struct FileState *fs);
void handle_ctrl_key_right(EditorContext *ctx, struct FileState *fs);
void handle_ctrl_key_pgup(EditorContext *ctx, struct FileState *fs);
void handle_ctrl_key_pgdn(EditorContext *ctx, struct FileState *fs);
void handle_ctrl_key_up(EditorContext *ctx, struct FileState *fs);
void handle_ctrl_key_down(EditorContext *ctx, struct FileState *fs);
void handle_tab_key(EditorContext *ctx, struct FileState *fs);
void handle_default_key(EditorContext *ctx, struct FileState *fs, int ch);
void move_forward_to_next_word(EditorContext *ctx, struct FileState *fs);
void move_backward_to_previous_word(EditorContext *ctx, struct FileState *fs);
void handle_mouse_event(EditorContext *ctx, struct FileState *fs, MEVENT *ev);
void update_selection_mouse(EditorContext *ctx, struct FileState *fs, int x, int y);

#endif

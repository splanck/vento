#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "editor.h"
#include "syntax.h"
#include "files.h"
#include "file_manager.h"
#include "editor_state.h"
#include "undo.h"
#include "file_ops.h"
#include "macro.h"

/*
 * editor_actions.c
 * ----------------
 * Helper routines that implement the editing commands invoked from the
 * various menus and key bindings.  Each function manipulates the current
 * FileState, updates global editor state when necessary and usually redraws
 * parts of the screen so the user immediately sees the effect of the
 * operation.
 */

/*
 * Delete the line under the cursor.
 *
 * ctx - Editor context used for drawing.
 * fs  - FileState whose buffer is modified.
 *
 * The removed text is pushed on the undo stack so the action can be
 * reverted.  `fs->modified` is set and the cursor/start_line fields may be
 * adjusted to keep the view in range.  The text window is cleared and
 * redrawn and comment highlighting is marked dirty.
 */
void delete_current_line(EditorContext *ctx, FileState *fs) {
    if (fs->buffer.count == 0) {
        return;
    }
    int line_to_delete = fs->cursor_y - 1 + fs->start_line;
    const char *cur_line = lb_get(&fs->buffer, line_to_delete);
    char *old_text = cur_line ? strdup(cur_line) : strdup("");
    if (!old_text) {
        allocation_failed("strdup failed");
        return;
    }
    push(&fs->undo_stack, (Change){line_to_delete, old_text, NULL});
    lb_delete(&fs->buffer, line_to_delete);
    fs->modified = true;
    if (fs->cursor_y < LINES - 4 && fs->cursor_y <= fs->buffer.count) {
        fs->cursor_y++;
    } else if (fs->start_line + fs->cursor_y > fs->buffer.count) {
        if (fs->cursor_y > 1) {
            fs->cursor_y--;
        } else if (fs->start_line > 0) {
            fs->start_line--;
        }
    }
    werase(ctx->text_win);
    box(ctx->text_win, 0, 0);
    draw_text_buffer(fs, ctx->text_win);
    wrefresh(ctx->text_win);
    mark_comment_state_dirty(fs);
}

/*
 * Insert a blank line at the current cursor position.
 *
 * ctx - Editor context (currently unused).
 * fs  - FileState describing the buffer to edit.
 *
 * A new empty line is inserted into `fs->buffer` and recorded on the undo
 * stack.  `fs->modified` is set and comment state is invalidated.  The
 * cursor is moved to the new line and `redraw()` is called so the change is
 * visible immediately.
 */
void insert_new_line(EditorContext *ctx, FileState *fs) {
    (void)ctx;
    int idx = fs->cursor_y + fs->start_line - 1;
    if (lb_insert(&fs->buffer, idx, "") < 0)
        allocation_failed("lb_insert failed");
    char *p = realloc(fs->buffer.lines[idx], fs->line_capacity);
    if (!p)
        allocation_failed("realloc failed");
    fs->buffer.lines[idx] = p;
    fs->buffer.lines[idx][0] = '\0';
    Change change;
    change.line = fs->cursor_y + fs->start_line - 1;
    change.old_text = NULL;
    change.new_text = strdup("");
    if (!change.new_text) {
        allocation_failed("strdup failed");
        return;
    }
    push(&fs->undo_stack, change);
    fs->modified = true;
    mark_comment_state_dirty(fs);
    fs->cursor_x = 1;
    if (fs->cursor_y == LINES - 4 && fs->start_line + LINES - 4 < fs->buffer.count) {
        fs->start_line++;
    } else {
        fs->cursor_y++;
    }
    redraw();
}

void handle_redo_wrapper(FileState *fs, int *cx, int *cy) {
    (void)cx; (void)cy;
    redo(fs);
}

void handle_undo_wrapper(FileState *fs, int *cx, int *cy) {
    (void)cx; (void)cy;
    undo(fs);
}

/*
 * Switch the editor to the next loaded file.
 *
 * ctx - Editor context used for updating screen state.
 *
 * The cursor position of the current FileState is saved and its file handle
 * closed if it was lazily loaded.  `file_manager.active_index`, `active_file`
 * and `text_win` are updated to reference the newly selected file.  The
 * function redraws the screen and updates the status bar before returning the
 * new cursor position.
 */
CursorPos next_file(EditorContext *ctx) {
    CursorPos pos = {0, 0};
    if (file_manager.count <= 1) {
        return pos;
    }
    if (!confirm_switch())
        return pos;
    FileState *cur = fm_current(&file_manager);
    if (cur) {
        cur->saved_cursor_x = cur->cursor_x;
        cur->saved_cursor_y = cur->cursor_y;
        if (cur->fp && !cur->file_complete) {
            cur->file_pos = ftell(cur->fp);
            fclose(cur->fp);
            cur->fp = NULL;
        }
    }
    int prev_index = file_manager.active_index;
    int idx = prev_index;
    for (int i = 1; i < file_manager.count; i++) {
        int cand = (prev_index + i) % file_manager.count;
        FileState *fs = file_manager.files[cand];
        if (fs && fs->filename[0] != '\0') {
            idx = cand;
            break;
        }
    }
    if (idx == prev_index) {
        if (cur) {
            pos.x = cur->cursor_x;
            pos.y = cur->cursor_y;
        }
        return pos;
    }
    int res = fm_switch(&file_manager, idx);
    if (res < 0) {
        file_manager.active_index = prev_index;
        if (cur && !cur->fp && !cur->file_complete) {
            cur->fp = fopen(cur->filename, "r");
            if (cur->fp)
                fseek(cur->fp, cur->file_pos, SEEK_SET);
        }
        active_file = cur;
        text_win = cur ? cur->text_win : NULL;
        if (active_file) {
            pos.x = active_file->cursor_x;
            pos.y = active_file->cursor_y;
        }
        sync_editor_context(ctx);
        redraw();
        update_status_bar(ctx, active_file);
        return pos;
    }
    active_file = fm_current(&file_manager);
    if (active_file) {
        active_file->cursor_x = active_file->saved_cursor_x;
        active_file->cursor_y = active_file->saved_cursor_y;
    }
    text_win = active_file ? active_file->text_win : NULL;
    clamp_scroll_x(active_file);
    if (active_file) {
        pos.x = active_file->cursor_x;
        pos.y = active_file->cursor_y;
    }
    sync_editor_context(ctx);
    redraw();
    update_status_bar(ctx, active_file);
    return pos;
}

/*
 * Switch the editor to the previous loaded file.
 *
 * ctx - Editor context used for updating screen state.
 *
 * The current file's cursor position is saved and any lazily loaded file
 * handle closed.  `file_manager.active_index`, `active_file` and `text_win`
 * are updated to the newly selected file.  The display is redrawn and the
 * status bar refreshed before the new cursor position is returned.
 */
CursorPos prev_file(EditorContext *ctx) {
    CursorPos pos = {0, 0};
    if (file_manager.count <= 1) {
        return pos;
    }
    if (!confirm_switch())
        return pos;
    FileState *cur = fm_current(&file_manager);
    if (cur) {
        cur->saved_cursor_x = cur->cursor_x;
        cur->saved_cursor_y = cur->cursor_y;
        if (cur->fp && !cur->file_complete) {
            cur->file_pos = ftell(cur->fp);
            fclose(cur->fp);
            cur->fp = NULL;
        }
    }
    int prev_index = file_manager.active_index;
    int idx = prev_index;
    for (int i = 1; i < file_manager.count; i++) {
        int cand = prev_index - i;
        if (cand < 0)
            cand += file_manager.count;
        FileState *fs = file_manager.files[cand];
        if (fs && fs->filename[0] != '\0') {
            idx = cand;
            break;
        }
    }
    if (idx == prev_index) {
        if (cur) {
            pos.x = cur->cursor_x;
            pos.y = cur->cursor_y;
        }
        return pos;
    }
    int res = fm_switch(&file_manager, idx);
    if (res < 0) {
        file_manager.active_index = prev_index;
        if (cur && !cur->fp && !cur->file_complete) {
            cur->fp = fopen(cur->filename, "r");
            if (cur->fp)
                fseek(cur->fp, cur->file_pos, SEEK_SET);
        }
        active_file = cur;
        text_win = cur ? cur->text_win : NULL;
        if (active_file) {
            pos.x = active_file->cursor_x;
            pos.y = active_file->cursor_y;
        }
        sync_editor_context(ctx);
        redraw();
        update_status_bar(ctx, active_file);
        return pos;
    }
    active_file = fm_current(&file_manager);
    if (active_file) {
        active_file->cursor_x = active_file->saved_cursor_x;
        active_file->cursor_y = active_file->saved_cursor_y;
    }
    text_win = active_file ? active_file->text_win : NULL;
    clamp_scroll_x(active_file);
    if (active_file) {
        pos.x = active_file->cursor_x;
        pos.y = active_file->cursor_y;
    }
    sync_editor_context(ctx);
    redraw();
    update_status_bar(ctx, active_file);
    return pos;
}

/*
 * Refresh the status bar showing file information and cursor location.
 *
 * ctx - Editor context used to access the file manager.
 * fs  - Currently active FileState.
 *
 * The function prints the file name, modification flag and cursor position at
 * the top and bottom of the screen.  Macro recording/playing state is also
 * indicated.  It calls `wnoutrefresh` on `stdscr` so the status area is
 * redrawn during the next `doupdate` call.
 */
void update_status_bar(EditorContext *ctx, FileState *fs) {
    sync_editor_context(ctx);
    move(0, 0);
    int idx = ctx->file_manager.active_index + 1;
    int total = ctx->file_manager.count > 0 ? ctx->file_manager.count : 1;
    const char *name = "untitled";
    if (fs && fs->filename[0] != '\0') {
        name = fs->filename;
    }
    const char *fmt = (fs && fs->modified) ? "%s* [%d/%d]" : "%s [%d/%d]";
    size_t base_len = snprintf(NULL, 0, fmt, name, idx, total);
    size_t extra_len = 0;
    if (macro_state.recording)
        extra_len += strlen(" [REC]");
    else if (macro_state.playing)
        extra_len += strlen(" [PLAY]");
    if (current_macro && current_macro->name)
        extra_len += strlen(" {") + strlen(current_macro->name) + strlen("}");

    char *display = malloc(base_len + extra_len + 1);
    if (!display) {
        allocation_failed("update_status_bar malloc failed");
        return;
    }

    snprintf(display, base_len + 1, fmt, name, idx, total);
    if (macro_state.recording)
        strcat(display, " [REC]");
    else if (macro_state.playing)
        strcat(display, " [PLAY]");
    if (current_macro && current_macro->name) {
        strcat(display, " {");
        strcat(display, current_macro->name);
        strcat(display, "}");
    }
    int center_position = (COLS - (int)strlen(display)) / 2;
    if (center_position < 0) center_position = 0;
    mvprintw(1, center_position, "%s", display);
    free(display);
    move(LINES - 1, 0);
    clrtoeol();
    int actual_line_number = fs ? (fs->cursor_y + fs->start_line) : 0;
    mvprintw(LINES - 1, 0, "Lines: %d  Current Line: %d  Column: %d", fs ? fs->buffer.count : 0, actual_line_number, fs ? fs->cursor_x : 0);
    int help_col = COLS - 15;
    if (help_col < 0) help_col = 0;
    mvprintw(LINES - 1, help_col, "CTRL-H - Help");
    wnoutrefresh(stdscr);
}

/*
 * Jump the cursor to a specific line number.
 *
 * ctx  - Editor context providing the text window.
 * fs   - FileState being navigated.
 * line - 1-based line number to jump to.
 *
 * The function clamps the requested line to the valid range, adjusts
 * `fs->start_line` so the target line is centered when possible and updates
 * the cursor location.  The text window is cleared, redrawn and positioned on
 * the new line.
 */
void go_to_line(EditorContext *ctx, FileState *fs, int line) {
    if (fs->buffer.count == 0)
        return;

    ensure_line_loaded(fs, line - 1);

    if (line < 1)
        line = 1;
    if (line > fs->buffer.count)
        line = fs->buffer.count;

    int lines_per_screen = LINES - 3;
    int middle_line = lines_per_screen / 2;
    int idx = line - 1;

    if (fs->buffer.count <= lines_per_screen) {
        fs->start_line = 0;
    } else if (idx < middle_line) {
        fs->start_line = 0;
    } else if (idx > fs->buffer.count - middle_line) {
        fs->start_line = fs->buffer.count - lines_per_screen;
    } else {
        fs->start_line = idx - middle_line;
    }

    fs->cursor_y = idx - fs->start_line + 1;
    fs->cursor_x = 1;

    werase(ctx->text_win);
    box(ctx->text_win, 0, 0);
    draw_text_buffer(fs, ctx->text_win);
    wmove(ctx->text_win, fs->cursor_y,
          fs->cursor_x + get_line_number_offset(fs));
    wnoutrefresh(ctx->text_win);
}

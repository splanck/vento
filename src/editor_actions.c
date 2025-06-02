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
    mark_comment_state_dirty(fs);
}

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
    if (fs->cursor_y > 1) {
        fs->cursor_y--;
    } else if (fs->start_line > 0) {
        fs->start_line--;
    }
}

void handle_redo_wrapper(FileState *fs, int *cx, int *cy) {
    (void)cx; (void)cy;
    redo(fs);
}

void handle_undo_wrapper(FileState *fs, int *cx, int *cy) {
    (void)cx; (void)cy;
    undo(fs);
}

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

void update_status_bar(EditorContext *ctx, FileState *fs) {
    move(0, 0);
    int idx = ctx->file_manager.active_index + 1;
    int total = ctx->file_manager.count > 0 ? ctx->file_manager.count : 1;
    const char *name = "untitled";
    if (fs && fs->filename[0] != '\0') {
        name = fs->filename;
    }
    char display[512];
    if (fs && fs->modified)
        snprintf(display, sizeof(display), "%s* [%d/%d]", name, idx, total);
    else
        snprintf(display, sizeof(display), "%s [%d/%d]", name, idx, total);
    int center_position = (COLS - (int)strlen(display)) / 2;
    if (center_position < 0) center_position = 0;
    mvprintw(1, center_position, "%s", display);
    move(LINES - 1, 0);
    clrtoeol();
    int actual_line_number = fs ? (fs->cursor_y + fs->start_line) : 0;
    mvprintw(LINES - 1, 0, "Lines: %d  Current Line: %d  Column: %d", fs ? fs->buffer.count : 0, actual_line_number, fs ? fs->cursor_x : 0);
    mvprintw(LINES - 1, COLS - 15, "CTRL-H - Help");
    wnoutrefresh(stdscr);
}

void go_to_line(EditorContext *ctx, FileState *fs, int line) {
    if (fs->buffer.count == 0)
        return;

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

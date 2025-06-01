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
    if (fs->line_count == 0) {
        return;
    }
    int line_to_delete = fs->cursor_y - 1 + fs->start_line;
    char *old_text = strdup(fs->text_buffer[line_to_delete]);
    if (!old_text) {
        allocation_failed("strdup failed");
        return;
    }
    push(&fs->undo_stack, (Change){line_to_delete, old_text, NULL});
    for (int i = line_to_delete; i < fs->line_count - 1; ++i) {
        strcpy(fs->text_buffer[i], fs->text_buffer[i + 1]);
    }
    memset(fs->text_buffer[fs->line_count - 1], 0, fs->line_capacity);
    fs->line_count--;
    fs->modified = true;
    if (fs->cursor_y < LINES - 4 && fs->cursor_y <= fs->line_count) {
        fs->cursor_y++;
    } else if (fs->start_line + fs->cursor_y > fs->line_count) {
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
    if (ensure_line_capacity(fs, fs->line_count + 1) < 0)
        allocation_failed("ensure_line_capacity failed");
    for (int i = fs->line_count; i > fs->cursor_y + fs->start_line - 1; --i) {
        strcpy(fs->text_buffer[i], fs->text_buffer[i - 1]);
    }
    fs->line_count++;
    fs->text_buffer[fs->cursor_y + fs->start_line - 1][0] = '\0';
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
    if (fs->cursor_y == LINES - 4 && fs->start_line + LINES - 4 < fs->line_count) {
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

void next_file(EditorContext *ctx, FileState *fs_unused, int *cx, int *cy) {
    (void)fs_unused;
    if (ctx->file_manager.count == 0) {
        return;
    }
    if (!confirm_switch())
        return;
    FileState *cur = fm_current(&ctx->file_manager);
    if (cur) {
        cur->saved_cursor_x = cur->cursor_x;
        cur->saved_cursor_y = cur->cursor_y;
    }
    int idx = ctx->file_manager.active_index + 1;
    if (idx >= ctx->file_manager.count) idx = 0;
    fm_switch(&ctx->file_manager, idx);
    ctx->active_file = fm_current(&ctx->file_manager);
    if (ctx->active_file) {
        ctx->active_file->cursor_x = ctx->active_file->saved_cursor_x;
        ctx->active_file->cursor_y = ctx->active_file->saved_cursor_y;
    }
    ctx->text_win = ctx->active_file ? ctx->active_file->text_win : NULL;
    clamp_scroll_x(ctx->active_file);
    if (cx && cy && ctx->active_file) {
        *cx = ctx->active_file->cursor_x;
        *cy = ctx->active_file->cursor_y;
    }
    redraw();
    update_status_bar(ctx, ctx->active_file);

    active_file = ctx->active_file;
    text_win = ctx->text_win;
    file_manager = ctx->file_manager;
}

void prev_file(EditorContext *ctx, FileState *fs_unused, int *cx, int *cy) {
    (void)fs_unused;
    if (ctx->file_manager.count == 0) {
        return;
    }
    if (!confirm_switch())
        return;
    FileState *cur = fm_current(&ctx->file_manager);
    if (cur) {
        cur->saved_cursor_x = cur->cursor_x;
        cur->saved_cursor_y = cur->cursor_y;
    }
    int idx = ctx->file_manager.active_index - 1;
    if (idx < 0) idx = ctx->file_manager.count - 1;
    fm_switch(&ctx->file_manager, idx);
    ctx->active_file = fm_current(&ctx->file_manager);
    if (ctx->active_file) {
        ctx->active_file->cursor_x = ctx->active_file->saved_cursor_x;
        ctx->active_file->cursor_y = ctx->active_file->saved_cursor_y;
    }
    ctx->text_win = ctx->active_file ? ctx->active_file->text_win : NULL;
    clamp_scroll_x(ctx->active_file);
    if (cx && cy && ctx->active_file) {
        *cx = ctx->active_file->cursor_x;
        *cy = ctx->active_file->cursor_y;
    }
    redraw();
    update_status_bar(ctx, ctx->active_file);

    active_file = ctx->active_file;
    text_win = ctx->text_win;
    file_manager = ctx->file_manager;
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
    mvprintw(LINES - 1, 0, "Lines: %d  Current Line: %d  Column: %d", fs ? fs->line_count : 0, actual_line_number, fs ? fs->cursor_x : 0);
    mvprintw(LINES - 1, COLS - 15, "CTRL-H - Help");
    wnoutrefresh(stdscr);
}

void go_to_line(EditorContext *ctx, FileState *fs, int line) {
    if (fs->line_count == 0)
        return;

    if (line < 1)
        line = 1;
    if (line > fs->line_count)
        line = fs->line_count;

    int lines_per_screen = LINES - 3;
    int middle_line = lines_per_screen / 2;
    int idx = line - 1;

    if (fs->line_count <= lines_per_screen) {
        fs->start_line = 0;
    } else if (idx < middle_line) {
        fs->start_line = 0;
    } else if (idx > fs->line_count - middle_line) {
        fs->start_line = fs->line_count - lines_per_screen;
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

/*
 * Keyboard input handling
 * -----------------------
 * This module maps raw key codes to editing actions. The handlers
 * update cursor positions, modify the buffer, push undo entries and
 * trigger redraws as needed.
 */
#include "editor.h"
#include <ncurses.h>
#include <wchar.h>
#include <wctype.h>
#include "undo.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "config.h"
#include "editor_state.h"
#include "input.h"
#include "clipboard.h"
#include "syntax.h"
#include "files.h"

__attribute__((weak)) AppConfig app_config = {
    .tab_width = 4,
    .macro_record_key = KEY_F(2),
    .macro_play_key = KEY_F(4)
};

/*
 * Ensure the cursor column remains visible by adjusting the horizontal
 * scroll offset.
 *
 * ctx - editor context used for redrawing when the view changes
 * fs  - file whose scroll_x field is updated
 *
 * When scroll_x is modified the text window is redrawn. No undo entry
 * is created by this helper.
 */
static void update_scroll_x(EditorContext *ctx, FileState *fs) {
    int offset = get_line_number_offset(fs);
    int visible_width = COLS - 2 - offset;
    if (visible_width < 1)
        visible_width = 1;
    if (fs->cursor_x - 1 < fs->scroll_x) {
        fs->scroll_x = fs->cursor_x - 1;
        if (fs->scroll_x < 0)
            fs->scroll_x = 0;
        draw_text_buffer(ctx->active_file, text_win);
    } else if (fs->cursor_x - 1 >= fs->scroll_x + visible_width) {
        fs->scroll_x = fs->cursor_x - visible_width;
        draw_text_buffer(ctx->active_file, text_win);
    }
}

/*
 * Ignore CTRL+` key presses.
 *
 * ctx - editor context (unused)
 *
 * This handler intentionally performs no action so terminals that send
 * this control sequence do not cause a crash. The buffer and undo stack
 * are untouched and nothing is redrawn.
 */
void handle_ctrl_backtick(EditorContext *ctx) {
    (void)ctx;
}

/*
 * Move the cursor one line up or scroll the buffer when already at the
 * top of the visible area.
 *
 * ctx - active editor context for redraw when scrolling occurs
 * fs  - file being edited
 *
 * Only cursor_y or start_line is modified and the buffer is untouched.
 * draw_text_buffer() is called when start_line changes. No undo entry
 * is created.
 */
void handle_key_up(EditorContext *ctx, FileState *fs) {
    if (fs->cursor_y > 1) {
        fs->cursor_y--;
    } else if (fs->start_line > 0) {
        fs->start_line--;
        draw_text_buffer(ctx->active_file, text_win);
    }
}

/*
 * Move the cursor one line down or scroll the buffer when reaching the
 * bottom of the visible area.
 *
 * ctx - active editor context for redraw when scrolling occurs
 * fs  - file being edited
 *
 * Adjusts cursor_y or start_line without modifying the text. The line
 * under the new cursor is loaded on demand. Redraw happens when the
 * viewport is moved and no undo entry is recorded.
 */
void handle_key_down(EditorContext *ctx, FileState *fs) {
    if (ensure_line_loaded(fs, fs->start_line + fs->cursor_y) < 0) {
        mvprintw(LINES - 2, 2, "Error loading file: %s", strerror(fs->io_errno));
        refresh();
        getch();
        mvprintw(LINES - 2, 2, "                            ");
        refresh();
        return;
    }
    if (fs->cursor_y < LINES - BOTTOM_MARGIN && fs->cursor_y < fs->buffer.count) {
        fs->cursor_y++;
    } else if (fs->start_line + fs->cursor_y < fs->buffer.count) {
        fs->start_line++;
        draw_text_buffer(ctx->active_file, text_win);
    }
}

/*
 * Move the cursor one character to the left.
 *
 * ctx - active editor context used for horizontal scrolling
 * fs  - file being edited
 *
 * Updates cursor_x and calls update_scroll_x. The buffer is not modified
 * and no undo entry is created. Redraw occurs only if scrolling changes.
 */
void handle_key_left(EditorContext *ctx, FileState *fs) {
    if (fs->cursor_x > 1) {
        fs->cursor_x--;
    }
    update_scroll_x(ctx, fs);
}

/*
 * Move the cursor one character to the right within the current line.
 *
 * ctx - active editor context used for horizontal scrolling
 * fs  - file being edited
 *
 * Updates cursor_x if not past the end of the line and calls
 * update_scroll_x. This does not modify the buffer or create an undo
 * entry. Redraw only happens if scrolling occurs.
 */
void handle_key_right(EditorContext *ctx, FileState *fs) {
    const char *line = lb_get(&fs->buffer, fs->cursor_y - 1 + fs->start_line);
    if (line && fs->cursor_x < (int)strlen(line) + 1) {
        fs->cursor_x++;
    }
    update_scroll_x(ctx, fs);
}

/*
 * Delete the character before the cursor or join the line with the
 * previous one when at column 1.
 *
 * ctx - active editor context used for redrawing
 * fs  - file being edited
 *
 * The buffer is modified but no undo entry is pushed. The cursor moves
 * left or up accordingly and the text window is redrawn. Comment state
 * is marked dirty for syntax highlighting.
 */
void handle_key_backspace(EditorContext *ctx, FileState *fs) {
    if (fs->cursor_x > 1) {
        fs->cursor_x--;
        memmove(&fs->buffer.lines[fs->cursor_y - 1 + fs->start_line][fs->cursor_x - 1],
                &fs->buffer.lines[fs->cursor_y - 1 + fs->start_line][fs->cursor_x],
                strlen(fs->buffer.lines[fs->cursor_y - 1 + fs->start_line]) - fs->cursor_x + 1);
    } else if (fs->cursor_y > 1 || fs->start_line > 0) {
        int idx = fs->cursor_y - 1 + fs->start_line;
        char *prev = fs->buffer.lines[idx - 1];
        char *curr = fs->buffer.lines[idx];
        size_t prev_len = strlen(prev);
        if (prev_len + strlen(curr) < (size_t)fs->line_capacity) {
            strcat(prev, curr);
            lb_delete(&fs->buffer, idx);
            if (fs->cursor_y > 1) {
                fs->cursor_y--;
            } else {
                fs->start_line--;
                draw_text_buffer(ctx->active_file, text_win);
            }
            fs->cursor_x = prev_len + 1;
        }
    }
    werase(text_win);
    box(text_win, 0, 0);
    draw_text_buffer(ctx->active_file, text_win);
    mark_comment_state_dirty(fs);
}

/*
 * Delete the character under the cursor or merge with the next line if
 * at the end of the current one.
 *
 * ctx - active editor context for redraw
 * fs  - file being edited
 *
 * The buffer content changes but no undo entry is pushed. After the
 * deletion the text window is redrawn and the syntax comment state is
 * marked dirty.
 */
void handle_key_delete(EditorContext *ctx, FileState *fs) {
    const char *line_curr = lb_get(&fs->buffer, fs->cursor_y - 1 + fs->start_line);
    if (line_curr && fs->cursor_x < (int)strlen(line_curr)) {
        memmove(&fs->buffer.lines[fs->cursor_y - 1 + fs->start_line][fs->cursor_x - 1],
                &fs->buffer.lines[fs->cursor_y - 1 + fs->start_line][fs->cursor_x],
                strlen(fs->buffer.lines[fs->cursor_y - 1 + fs->start_line]) - fs->cursor_x + 1);
    } else if (fs->cursor_y + fs->start_line < fs->buffer.count) {
        int idx = fs->cursor_y - 1 + fs->start_line;
        char *current = fs->buffer.lines[idx];
        char *next = fs->buffer.lines[idx + 1];
        size_t total_len = strlen(current) + strlen(next);
        if (total_len >= (size_t)fs->line_capacity) {
            size_t space = fs->line_capacity - strlen(current) - 1;
            if (space > 0)
                strncat(current, next, space);
            current[fs->line_capacity - 1] = '\0';
        } else {
            strcat(current, next);
        }
        lb_delete(&fs->buffer, idx + 1);
    }
    werase(text_win);
    box(text_win, 0, 0);
    draw_text_buffer(ctx->active_file, text_win);
    mark_comment_state_dirty(fs);
}

/*
 * Insert a newline at the cursor, splitting the current line and
 * preserving indentation.
 *
 * ctx - active editor context for redraw
 * fs  - file being edited
 *
 * The text before the cursor becomes the old line while the remainder is
 * inserted as a new line. Two undo entries record the modification and
 * the insertion. The cursor is moved to the start of the new line and
 * the window is redrawn with comment state marked dirty.
 */
void handle_key_enter(EditorContext *ctx, FileState *fs) {
    int line_idx = fs->cursor_y - 1 + fs->start_line;
    char *line = fs->buffer.lines[line_idx];
    char *old_text = strdup(line);
    if (!old_text) {
        allocation_failed("strdup failed");
        return;
    }

    int indent_len = 0;
    while (line[indent_len] == ' ' || line[indent_len] == '\t')
        indent_len++;
    char *indent = malloc(indent_len + 1);
    if (!indent) {
        free(old_text);
        allocation_failed("malloc failed");
        return;
    }
    strncpy(indent, line, indent_len);
    indent[indent_len] = '\0';

    char new_line_tmp[fs->line_capacity];
    new_line_tmp[0] = '\0';
    strncat(new_line_tmp, indent, fs->line_capacity - 1);

    char *remainder = &line[fs->cursor_x - 1];
    int remaining_indent = indent_len - (fs->cursor_x - 1);
    if (remaining_indent > 0)
        remainder += remaining_indent;
    strncat(new_line_tmp, remainder, fs->line_capacity - strlen(new_line_tmp) - 1);

    line[fs->cursor_x - 1] = '\0';

    char *new_text = strdup(line);
    if (!new_text) {
        free(old_text);
        free(indent);
        allocation_failed("strdup failed");
        return;
    }
    push(&fs->undo_stack, (Change){ line_idx, old_text, new_text });

    if (lb_insert(&fs->buffer, line_idx + 1, new_line_tmp) < 0) {
        free(old_text);
        free(indent);
        allocation_failed("lb_insert failed");
        return;
    }
    char *p = realloc(fs->buffer.lines[line_idx + 1], fs->line_capacity);
    if (!p) {
        allocation_failed("realloc failed");
        return;
    }
    fs->buffer.lines[line_idx + 1] = p;

    char *insert_text = strdup(new_line_tmp);
    if (!insert_text) {
        free(indent);
        allocation_failed("strdup failed");
        return;
    }
    push(&fs->undo_stack, (Change){ line_idx + 1, NULL, insert_text });

    fs->cursor_x = indent_len + 1;
    if (fs->cursor_y >= LINES - 6) {
        fs->start_line++;
    } else {
        fs->cursor_y++;
    }
    werase(text_win);
    box(text_win, 0, 0);
    draw_text_buffer(ctx->active_file, text_win);
    mark_comment_state_dirty(fs);
    free(indent);
}

/*
 * Scroll one page up and move the cursor to the first visible line.
 *
 * ctx - active editor context for redraw
 * fs  - file being edited
 *
 * Only start_line and cursor_y are changed. The screen is redrawn when
 * the starting line moves. The buffer contents remain unchanged and no
 * undo information is stored.
 */
void handle_key_page_up(EditorContext *ctx, FileState *fs) {
    int page_size = LINES - 4;
    if (fs->start_line > 0) {
        fs->start_line -= page_size;
        if (fs->start_line < 0) {
            fs->start_line = 0;
        }
        draw_text_buffer(ctx->active_file, text_win);
    }
    fs->cursor_y = 1;
}

/*
 * Scroll one page down and move the cursor to the last visible line.
 *
 * ctx - active editor context (unused here)
 * fs  - file being edited
 *
 * Updates start_line and cursor_y after ensuring the necessary lines are
 * loaded. No buffer modifications, undo entries or immediate redraws
 * occur in this handler.
 */
void handle_key_page_down(EditorContext *ctx, FileState *fs) {
    (void)ctx;
    if (ensure_line_loaded(fs, fs->start_line + (LINES - 4)) < 0) {
        mvprintw(LINES - 2, 2, "Error loading file: %s", strerror(fs->io_errno));
        refresh();
        getch();
        mvprintw(LINES - 2, 2, "                            ");
        refresh();
        return;
    }
    int max_lines = LINES - 4;
    if (fs->start_line + max_lines < fs->buffer.count) {
        fs->start_line += max_lines;
    } else {
        fs->start_line = fs->buffer.count - max_lines;
        if (fs->start_line < 0) {
            fs->start_line = 0;
        }
    }

    fs->cursor_y = max_lines;

    if (fs->cursor_y + fs->start_line >= fs->buffer.count) {
        if (fs->buffer.count > 0) {
            fs->cursor_y = fs->buffer.count - fs->start_line;
        } else {
            fs->cursor_y = 1;
        }
    }
}

void handle_ctrl_key_pgup(EditorContext *ctx, FileState *fs) {
    fs->cursor_y = 1;
    fs->start_line = 0;
    draw_text_buffer(ctx->active_file, text_win);
}

void handle_ctrl_key_pgdn(EditorContext *ctx, FileState *fs) {
    load_all_remaining_lines(fs);
    fs->cursor_y = LINES - 4;
    if (fs->buffer.count > LINES - 4) {
        fs->start_line = fs->buffer.count - (LINES - 4);
    } else {
        fs->start_line = 0;
        fs->cursor_y = fs->buffer.count;
    }
    draw_text_buffer(ctx->active_file, text_win);
}

void handle_ctrl_key_up(EditorContext *ctx, FileState *fs) {
    (void)ctx;
    fs->cursor_y = 1;
}

void handle_ctrl_key_down(EditorContext *ctx, FileState *fs) {
    (void)ctx;
    fs->cursor_y = LINES - 4;
}

void handle_ctrl_key_left(EditorContext *ctx, FileState *fs) {
    fs->cursor_x = 1;
    update_scroll_x(ctx, fs);
}

void handle_ctrl_key_right(EditorContext *ctx, FileState *fs) {
    const char *line = lb_get(&fs->buffer, fs->cursor_y - 1 + fs->start_line);
    fs->cursor_x = line ? strlen(line) + 1 : 1;
    update_scroll_x(ctx, fs);
}

/*
 * Move the cursor to the beginning of the current line.
 *
 * ctx - active editor context for horizontal scrolling
 * fs  - file being edited
 *
 * Only cursor_x is updated via update_scroll_x. No undo information is
 * recorded and the buffer is unchanged.
 */
void handle_key_home(EditorContext *ctx, FileState *fs) {
    fs->cursor_x = 1;
    update_scroll_x(ctx, fs);
}

/*
 * Move the cursor to the end of the current line.
 *
 * ctx - active editor context for horizontal scrolling
 * fs  - file being edited
 *
 * cursor_x is set to just past the last character. update_scroll_x is
 * called to adjust the viewport. No undo entry is created.
 */
void handle_key_end(EditorContext *ctx, FileState *fs) {
    const char *line = lb_get(&fs->buffer, fs->cursor_y - 1 + fs->start_line);
    fs->cursor_x = line ? strlen(line) + 1 : 1;
    update_scroll_x(ctx, fs);
}

void handle_tab_key(EditorContext *ctx, FileState *fs) {
    int tabsize = app_config.tab_width > 0 ? app_config.tab_width : 4;
    int inserted = 0;

    if (fs->cursor_x >= fs->line_capacity - 1)
        return;

    char *old_text = strdup(fs->buffer.lines[fs->cursor_y - 1 + fs->start_line]);
    if (!old_text) {
        allocation_failed("strdup failed");
        return;
    }

    while (inserted < tabsize && fs->cursor_x < fs->line_capacity - 1) {
        int len = strlen(fs->buffer.lines[fs->cursor_y - 1 + fs->start_line]);
        if (len > fs->line_capacity - 1)
            len = fs->line_capacity - 1;

        if (fs->cursor_x <= len) {
            memmove(&fs->buffer.lines[fs->cursor_y - 1 + fs->start_line][fs->cursor_x],
                    &fs->buffer.lines[fs->cursor_y - 1 + fs->start_line][fs->cursor_x - 1],
                    len - fs->cursor_x + 1);
        }

        fs->buffer.lines[fs->cursor_y - 1 + fs->start_line][fs->cursor_x - 1] = ' ';
        if (len + 1 < fs->line_capacity)
            fs->buffer.lines[fs->cursor_y - 1 + fs->start_line][len + 1] = '\0';
        fs->buffer.lines[fs->cursor_y - 1 + fs->start_line][fs->line_capacity - 1] = '\0';
        fs->cursor_x++;
        inserted++;
    }

    if (inserted > 0) {
        char *new_text = strdup(fs->buffer.lines[fs->cursor_y - 1 + fs->start_line]);
        if (!new_text) {
            free(old_text);
            allocation_failed("strdup failed");
            return;
        }
        Change change = { fs->cursor_y - 1 + fs->start_line, old_text, new_text };
        push(&fs->undo_stack, change);
        mark_comment_state_dirty(fs);

        werase(text_win);
        box(text_win, 0, 0);
        draw_text_buffer(ctx->active_file, text_win);
    } else {
        free(old_text);
    }
}

void handle_default_key(EditorContext *ctx, FileState *fs, wint_t ch) {
#ifdef KEY_TAB
    if (ch == KEY_TAB || ch == '\t') {
#else
    if (ch == '\t') {
#endif
        handle_tab_key(ctx, fs);
        return;
    }
    if (ch >= KEY_MIN || !iswprint(ch))
        return; /* ignore non-printable or unmapped special keys */
    char mb[MB_CUR_MAX];
    int mblen = wcrtomb(mb, ch, NULL);
    if (mblen <= 0)
        return;
    int len = strlen(fs->buffer.lines[fs->cursor_y - 1 + fs->start_line]);
    if (len > fs->line_capacity - 1)
        len = fs->line_capacity - 1;
    if (len + mblen >= fs->line_capacity)
        return;
    char *old_text = strdup(fs->buffer.lines[fs->cursor_y - 1 + fs->start_line]);
    if (!old_text) {
        allocation_failed("strdup failed");
        return;
    }

    if (fs->cursor_x - 1 <= len) {
        memmove(&fs->buffer.lines[fs->cursor_y - 1 + fs->start_line][fs->cursor_x - 1 + mblen],
                &fs->buffer.lines[fs->cursor_y - 1 + fs->start_line][fs->cursor_x - 1],
                len - (fs->cursor_x - 1) + 1);
    }

    memcpy(&fs->buffer.lines[fs->cursor_y - 1 + fs->start_line][fs->cursor_x - 1], mb, mblen);
    fs->buffer.lines[fs->cursor_y - 1 + fs->start_line][fs->line_capacity - 1] = '\0';
    fs->cursor_x += mblen;

    char *new_text = strdup(fs->buffer.lines[fs->cursor_y - 1 + fs->start_line]);
    if (!new_text) {
        free(old_text);
        allocation_failed("strdup failed");
        return;
    }
    Change change = { fs->cursor_y - 1 + fs->start_line, old_text, new_text };
    push(&fs->undo_stack, change);
    mark_comment_state_dirty(fs);

    werase(text_win);
    box(text_win, 0, 0);
    draw_text_buffer(ctx->active_file, text_win);
}

/*
 * Advance the cursor to the start of the next word, potentially moving
 * to the following line.
 *
 * ctx - editor context (unused)
 * fs  - file being edited
 *
 * This routine only moves the cursor; it does not modify the buffer or
 * create undo information and it performs no redraws.
 */
void move_forward_to_next_word(EditorContext *ctx, FileState *fs) {
    (void)ctx;
    while (fs->cursor_y - 1 + fs->start_line < fs->buffer.count) {
        const char *line = lb_get(&fs->buffer, fs->cursor_y - 1 + fs->start_line);
        if (!line)
            break;
        int len = strlen(line);
        while (fs->cursor_x < len && isalnum(line[fs->cursor_x - 1])) {
            fs->cursor_x++;
        }
        while (fs->cursor_x < len && !isalnum(line[fs->cursor_x - 1])) {
            fs->cursor_x++;
        }
        if (fs->cursor_x < len) {
            return;
        }
        fs->cursor_x = 1;
        fs->cursor_y++;
    }
}

/*
 * Move the cursor backward to the beginning of the previous word.
 *
 * ctx - editor context (unused)
 * fs  - file being edited
 *
 * The cursor may cross line boundaries but the buffer is untouched and
 * no undo entry or redraw is produced.
 */
void move_backward_to_previous_word(EditorContext *ctx, FileState *fs) {
    (void)ctx;
    while (fs->cursor_y - 1 + fs->start_line >= 0) {
        const char *line = lb_get(&fs->buffer, fs->cursor_y - 1 + fs->start_line);
        if (!line)
            break;
        while (fs->cursor_x > 1 && isalnum(line[fs->cursor_x - 2])) {
            fs->cursor_x--;
        }
        while (fs->cursor_x > 1 && !isalnum(line[fs->cursor_x - 2])) {
            fs->cursor_x--;
        }
        if (fs->cursor_x > 1) {
            return;
        }
        if (fs->cursor_y > 1) {
            fs->cursor_y--;
            line = lb_get(&fs->buffer, fs->cursor_y - 1 + fs->start_line);
            fs->cursor_x = line ? strlen(line) + 1 : 1;
        } else {
            fs->cursor_x = 1;
            break;
        }
    }
}

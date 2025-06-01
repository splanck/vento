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

__attribute__((weak)) AppConfig app_config = { .tab_width = 4 };

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

void handle_ctrl_backtick(EditorContext *ctx) {
    (void)ctx;
    // Do nothing on CTRL+Backtick to avoid segmentation fault
}

void handle_key_up(EditorContext *ctx, FileState *fs) {
    if (fs->cursor_y > 1) {
        fs->cursor_y--;
    } else if (fs->start_line > 0) {
        fs->start_line--;
        draw_text_buffer(ctx->active_file, text_win);
    }
}

void handle_key_down(EditorContext *ctx, FileState *fs) {
    ensure_line_loaded(fs, fs->start_line + fs->cursor_y);
    if (fs->cursor_y < LINES - BOTTOM_MARGIN && fs->cursor_y < fs->buffer.count) {
        fs->cursor_y++;
    } else if (fs->start_line + fs->cursor_y < fs->buffer.count) {
        fs->start_line++;
        draw_text_buffer(ctx->active_file, text_win);
    }
}

void handle_key_left(EditorContext *ctx, FileState *fs) {
    if (fs->cursor_x > 1) {
        fs->cursor_x--;
    }
    update_scroll_x(ctx, fs);
}

void handle_key_right(EditorContext *ctx, FileState *fs) {
    const char *line = lb_get(&fs->buffer, fs->cursor_y - 1 + fs->start_line);
    if (line && fs->cursor_x < (int)strlen(line) + 1) {
        fs->cursor_x++;
    }
    update_scroll_x(ctx, fs);
}

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

void handle_key_delete(EditorContext *ctx, FileState *fs) {
    const char *line_curr = lb_get(&fs->buffer, fs->cursor_y - 1 + fs->start_line);
    if (line_curr && fs->cursor_x < (int)strlen(line_curr)) {
        memmove(&fs->buffer.lines[fs->cursor_y - 1 + fs->start_line][fs->cursor_x - 1],
                &fs->buffer.lines[fs->cursor_y - 1 + fs->start_line][fs->cursor_x],
                strlen(fs->buffer.lines[fs->cursor_y - 1 + fs->start_line]) - fs->cursor_x + 1);
    } else if (fs->cursor_y + fs->start_line < fs->buffer.count) {
        int idx = fs->cursor_y - 1 + fs->start_line;
        strcat(fs->buffer.lines[idx], fs->buffer.lines[idx + 1]);
        lb_delete(&fs->buffer, idx + 1);
    }
    werase(text_win);
    box(text_win, 0, 0);
    draw_text_buffer(ctx->active_file, text_win);
    mark_comment_state_dirty(fs);
}

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

void handle_key_page_down(EditorContext *ctx, FileState *fs) {
    (void)ctx;
    ensure_line_loaded(fs, fs->start_line + (LINES - 4));
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

void handle_key_home(EditorContext *ctx, FileState *fs) {
    fs->cursor_x = 1;
    update_scroll_x(ctx, fs);
}

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

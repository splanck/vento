#include "editor.h"
#include <ncurses.h>
#include "undo.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "input.h"
#include "clipboard.h"
#include "syntax.h"

void handle_ctrl_backtick() {
    // Do nothing on CTRL+Backtick to avoid segmentation fault
}

void handle_key_up(FileState *fs) {
    if (fs->cursor_y > 1) {
        fs->cursor_y--;
    } else if (fs->start_line > 0) {
        fs->start_line--;
        draw_text_buffer(active_file, text_win);
    }
}

void handle_key_down(FileState *fs) {
    if (fs->cursor_y < LINES - BOTTOM_MARGIN && fs->cursor_y < fs->line_count) {
        fs->cursor_y++;
    } else if (fs->start_line + fs->cursor_y < fs->line_count) {
        fs->start_line++;
        draw_text_buffer(active_file, text_win);
    }
}

void handle_key_left(FileState *fs) {
    if (fs->cursor_x > 1) {
        fs->cursor_x--;
    }
}

void handle_key_right(FileState *fs) {
    if (fs->cursor_x < (int)strlen(fs->text_buffer[fs->cursor_y - 1 + fs->start_line]) + 1) {
        fs->cursor_x++;
    }
}

void handle_key_backspace(FileState *fs) {
    if (fs->cursor_x > 1) {
        fs->cursor_x--;
        memmove(&fs->text_buffer[fs->cursor_y - 1 + fs->start_line][fs->cursor_x - 1],
                &fs->text_buffer[fs->cursor_y - 1 + fs->start_line][fs->cursor_x],
                strlen(fs->text_buffer[fs->cursor_y - 1 + fs->start_line]) - fs->cursor_x + 1);
    } else if (fs->cursor_y > 1 || fs->start_line > 0) {
        size_t prev_len = strlen(fs->text_buffer[fs->cursor_y - 2 + fs->start_line]);
        if (prev_len + strlen(fs->text_buffer[fs->cursor_y - 1 + fs->start_line]) < (size_t)fs->line_capacity) {
            strcat(fs->text_buffer[fs->cursor_y - 2 + fs->start_line], fs->text_buffer[fs->cursor_y - 1 + fs->start_line]);
            for (int i = fs->cursor_y - 1 + fs->start_line; i < fs->line_count - 1; ++i) {
                strcpy(fs->text_buffer[i], fs->text_buffer[i + 1]);
            }
            fs->text_buffer[fs->line_count - 1][0] = '\0';
            fs->line_count--;
            if (fs->cursor_y > 1) {
                fs->cursor_y--;
            } else {
                fs->start_line--;
                draw_text_buffer(active_file, text_win);
            }
            fs->cursor_x = prev_len + 1;
        }
    }
    werase(text_win);
    box(text_win, 0, 0);
    draw_text_buffer(active_file, text_win);
    mark_comment_state_dirty(fs);
}

void handle_key_delete(FileState *fs) {
    if (fs->cursor_x < (int)strlen(fs->text_buffer[fs->cursor_y - 1 + fs->start_line])) {
        memmove(&fs->text_buffer[fs->cursor_y - 1 + fs->start_line][fs->cursor_x - 1],
                &fs->text_buffer[fs->cursor_y - 1 + fs->start_line][fs->cursor_x],
                strlen(fs->text_buffer[fs->cursor_y - 1 + fs->start_line]) - fs->cursor_x + 1);
    } else if (fs->cursor_y + fs->start_line < fs->line_count) {
        strcat(fs->text_buffer[fs->cursor_y - 1 + fs->start_line], fs->text_buffer[fs->cursor_y + fs->start_line]);
        for (int i = fs->cursor_y + fs->start_line; i < fs->line_count - 1; ++i) {
            strcpy(fs->text_buffer[i], fs->text_buffer[i + 1]);
        }
        fs->text_buffer[fs->line_count - 1][0] = '\0';
        fs->line_count--;
    }
    werase(text_win);
    box(text_win, 0, 0);
    draw_text_buffer(active_file, text_win);
    mark_comment_state_dirty(fs);
}

void handle_key_enter(FileState *fs) {
    if (ensure_line_capacity(fs, fs->line_count + 1) < 0)
        allocation_failed("ensure_line_capacity failed");
    for (int i = fs->line_count; i > fs->cursor_y + fs->start_line; --i) {
        strcpy(fs->text_buffer[i], fs->text_buffer[i - 1]);
    }
    fs->line_count++;

    if (fs->cursor_x > 1) {
        strcpy(fs->text_buffer[fs->cursor_y + fs->start_line],
               &fs->text_buffer[fs->cursor_y - 1 + fs->start_line][fs->cursor_x - 1]);
        fs->text_buffer[fs->cursor_y - 1 + fs->start_line][fs->cursor_x - 1] = '\0';
    } else {
        strcpy(fs->text_buffer[fs->cursor_y + fs->start_line],
               fs->text_buffer[fs->cursor_y - 1 + fs->start_line]);
        fs->text_buffer[fs->cursor_y - 1 + fs->start_line][0] = '\0';
    }

    fs->cursor_x = 1;
    if (fs->cursor_y >= LINES - 6) {
        fs->start_line++;
    } else {
        fs->cursor_y++;
    }
    werase(text_win);
    box(text_win, 0, 0);
    draw_text_buffer(active_file, text_win);
    mark_comment_state_dirty(fs);
}

void handle_key_page_up(FileState *fs) {
    int page_size = LINES - 4;
    if (fs->start_line > 0) {
        fs->start_line -= page_size;
        if (fs->start_line < 0) {
            fs->start_line = 0;
        }
        draw_text_buffer(active_file, text_win);
    }
    fs->cursor_y = 1;
}

void handle_key_page_down(FileState *fs) {
    int max_lines = LINES - 4;
    if (fs->start_line + max_lines < fs->line_count) {
        fs->start_line += max_lines;
    } else {
        fs->start_line = fs->line_count - max_lines;
        if (fs->start_line < 0) {
            fs->start_line = 0;
        }
    }

    fs->cursor_y = max_lines;

    if (fs->cursor_y + fs->start_line >= fs->line_count) {
        if (fs->line_count > 0) {
            fs->cursor_y = fs->line_count - fs->start_line;
        } else {
            fs->cursor_y = 1;
        }
    }
}

void handle_ctrl_key_pgup(FileState *fs) {
    fs->cursor_y = 1;
    fs->start_line = 0;
    draw_text_buffer(active_file, text_win);
}

void handle_ctrl_key_pgdn(FileState *fs) {
    fs->cursor_y = LINES - 4;
    if (fs->line_count > LINES - 4) {
        fs->start_line = fs->line_count - (LINES - 4);
    } else {
        fs->start_line = 0;
        fs->cursor_y = fs->line_count;
    }
    draw_text_buffer(active_file, text_win);
}

void handle_ctrl_key_up(FileState *fs) {
    fs->cursor_y = 1;
}

void handle_ctrl_key_down(FileState *fs) {
    fs->cursor_y = LINES - 4;
}

void handle_ctrl_key_left(FileState *fs) {
    fs->cursor_x = 1;
}

void handle_ctrl_key_right(FileState *fs) {
    fs->cursor_x = strlen(fs->text_buffer[fs->cursor_y - 1 + fs->start_line]) + 1;
}

void handle_tab_key(FileState *fs) {
    const int TAB_SIZE = 4;
    int inserted = 0;

    if (fs->cursor_x >= fs->line_capacity - 1)
        return;

    char *old_text = strdup(fs->text_buffer[fs->cursor_y - 1 + fs->start_line]);
    if (!old_text) {
        allocation_failed("strdup failed");
        return;
    }

    while (inserted < TAB_SIZE && fs->cursor_x < fs->line_capacity - 1) {
        int len = strlen(fs->text_buffer[fs->cursor_y - 1 + fs->start_line]);

        if (fs->cursor_x <= len) {
            memmove(&fs->text_buffer[fs->cursor_y - 1 + fs->start_line][fs->cursor_x],
                    &fs->text_buffer[fs->cursor_y - 1 + fs->start_line][fs->cursor_x - 1],
                    len - fs->cursor_x + 1);
        }

        fs->text_buffer[fs->cursor_y - 1 + fs->start_line][fs->cursor_x - 1] = ' ';
        fs->text_buffer[fs->cursor_y - 1 + fs->start_line][len + 1] = '\0';
        fs->cursor_x++;
        inserted++;
    }

    if (inserted > 0) {
        char *new_text = strdup(fs->text_buffer[fs->cursor_y - 1 + fs->start_line]);
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
        draw_text_buffer(active_file, text_win);
    } else {
        free(old_text);
    }
}

void handle_default_key(FileState *fs, int ch) {
#ifdef KEY_TAB
    if (ch == KEY_TAB || ch == '\t') {
#else
    if (ch == '\t') {
#endif
        handle_tab_key(fs);
        return;
    }
    if (fs->cursor_x < fs->line_capacity - 1) {
        int len = strlen(fs->text_buffer[fs->cursor_y - 1 + fs->start_line]);
        char *old_text = strdup(fs->text_buffer[fs->cursor_y - 1 + fs->start_line]);
        if (!old_text) {
            allocation_failed("strdup failed");
            return;
        }

        if (fs->cursor_x <= len) {
            memmove(&fs->text_buffer[fs->cursor_y - 1 + fs->start_line][fs->cursor_x],
                    &fs->text_buffer[fs->cursor_y - 1 + fs->start_line][fs->cursor_x - 1],
                    len - fs->cursor_x + 1);
        }

        fs->text_buffer[fs->cursor_y - 1 + fs->start_line][fs->cursor_x - 1] = ch;
        fs->text_buffer[fs->cursor_y - 1 + fs->start_line][len + 1] = '\0';
        fs->cursor_x++;

        char *new_text = strdup(fs->text_buffer[fs->cursor_y - 1 + fs->start_line]);
        if (!new_text) {
            free(old_text);
            allocation_failed("strdup failed");
            return;
        }
        Change change = { fs->cursor_y - 1 + fs->start_line, old_text, new_text };
        push(&fs->undo_stack, change);
        mark_comment_state_dirty(fs);
    }

    werase(text_win);
    box(text_win, 0, 0);
    draw_text_buffer(active_file, text_win);
}

void move_forward_to_next_word(FileState *fs) {
    while (fs->cursor_y - 1 + fs->start_line < fs->line_count) {
        char *line = fs->text_buffer[fs->cursor_y - 1 + fs->start_line];
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

void move_backward_to_previous_word(FileState *fs) {
    while (fs->cursor_y - 1 + fs->start_line >= 0) {
        char *line = fs->text_buffer[fs->cursor_y - 1 + fs->start_line];
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
            line = fs->text_buffer[fs->cursor_y - 1 + fs->start_line];
            fs->cursor_x = strlen(line) + 1;
        } else {
            fs->cursor_x = 1;
            break;
        }
    }
}

#include <string.h>
#include <ncurses.h>
#include "editor.h"
#include "clipboard.h"
#include "files.h"
#include "syntax.h"

/* Clipboard and selection state are now stored in FileState */

void start_selection_mode(FileState *fs, int cursor_x, int cursor_y) {
    fs->selection_mode = true;
    fs->sel_start_x = cursor_x;
    fs->sel_start_y = cursor_y;
    fs->sel_end_x = cursor_x;
    fs->sel_end_y = cursor_y;
}

void end_selection_mode(FileState *fs) {
    fs->selection_mode = false;
    copy_selection(fs);
}

void copy_selection(FileState *fs) {
    int start_x = fs->sel_start_x;
    int end_x = fs->sel_end_x;
    int start_y, end_y;

    if (fs->sel_start_y < fs->sel_end_y) {
        start_y = fs->sel_start_y;
        end_y = fs->sel_end_y;
    } else {
        start_y = fs->sel_end_y;
        end_y = fs->sel_start_y;
    }

    if (fs->clipboard == NULL) {
        return;
    }

    fs->clipboard[0] = '\0';  // Clear clipboard
    size_t clip_len = 0;
    for (int y = start_y; y <= end_y && clip_len < CLIPBOARD_SIZE - 1; y++) {
        if (y == start_y) {
            const char *src = &fs->text_buffer[y - 1 + fs->start_line][start_x - 1];
            size_t to_copy = end_x - start_x + 1;
            if (to_copy > CLIPBOARD_SIZE - 1 - clip_len) {
                to_copy = CLIPBOARD_SIZE - 1 - clip_len;
            }
            strncpy(fs->clipboard + clip_len, src, to_copy);
            clip_len += to_copy;
            fs->clipboard[clip_len] = '\0';
        } else {
            if (clip_len < CLIPBOARD_SIZE - 1) {
                fs->clipboard[clip_len++] = '\n';
                fs->clipboard[clip_len] = '\0';
            }
            const char *src = fs->text_buffer[y - 1 + fs->start_line];
            size_t to_copy = strlen(src);
            if (to_copy > CLIPBOARD_SIZE - 1 - clip_len) {
                to_copy = CLIPBOARD_SIZE - 1 - clip_len;
            }
            strncpy(fs->clipboard + clip_len, src, to_copy);
            clip_len += to_copy;
            fs->clipboard[clip_len] = '\0';
        }
    }
}

void paste_clipboard(FileState *fs, int *cursor_x, int *cursor_y) {
    if (fs->clipboard == NULL) {
        return;
    }

    char tmp[CLIPBOARD_SIZE];
    strncpy(tmp, fs->clipboard, sizeof(tmp) - 1);
    tmp[sizeof(tmp) - 1] = '\0';

    char *line = strtok(tmp, "\n");
    while (line) {
        size_t len = strlen(line);
        char *dest = fs->text_buffer[*cursor_y - 1 + fs->start_line];
        size_t dest_len = strlen(dest);

        if (dest_len + len >= (size_t)fs->line_capacity) {
            if (dest_len >= (size_t)(fs->line_capacity - 1)) {
                len = 0;  // No space to paste
            } else {
                len = fs->line_capacity - dest_len - 1;
            }
        }

        memmove(&dest[*cursor_x - 1 + len],
                &dest[*cursor_x - 1],
                dest_len - (*cursor_x - 1) + 1);
        memcpy(&dest[*cursor_x - 1], line, len);
        *cursor_x += len;

        line = strtok(NULL, "\n");
        if (line) {
            /* Move to the next line and create it in the buffer */
            (*cursor_y)++;
            fs->cursor_x = *cursor_x = 1;
            fs->cursor_y = *cursor_y;
            if (ensure_line_capacity(fs, fs->line_count + 1) < 0)
                allocation_failed("ensure_line_capacity failed");
            insert_new_line(fs);
        }
    }

    fs->cursor_x = *cursor_x;
    fs->cursor_y = *cursor_y;
}

void handle_selection_mode(FileState *fs, int ch, int *cursor_x, int *cursor_y) {
    if (ch == KEY_UP) {
        if (*cursor_y > 1) (*cursor_y)--;
        fs->sel_end_y = *cursor_y;
    }
    if (ch == KEY_DOWN) {
        if (*cursor_y < LINES - 4) (*cursor_y)++;
        fs->sel_end_y = *cursor_y;
    }
    if (ch == KEY_LEFT) {
        if (*cursor_x > 1) (*cursor_x)--;
        fs->sel_end_x = *cursor_x;
    }
    if (ch == KEY_RIGHT) {
        /* Do not allow the cursor to move past the buffer width */
        if (*cursor_x < fs->line_capacity - 1) (*cursor_x)++;
        fs->sel_end_x = *cursor_x;
    }
    if (ch == 10) {
        end_selection_mode(fs);
    }
}

void update_selection_mouse(FileState *fs, int x, int y) {
    fs->sel_end_x = x;
    fs->sel_end_y = y;

    werase(text_win);
    box(text_win, 0, 0);
    draw_text_buffer(fs, text_win);
    wmove(text_win, fs->cursor_y, fs->cursor_x);
    wrefresh(text_win);
}


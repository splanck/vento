#include <string.h>
#include <ncurses.h>
#include "editor.h"
#include "clipboard.h"
#include "files.h"

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

        if (dest_len + len >= (size_t)(COLS - 3)) {
            if (dest_len >= (size_t)(COLS - 3) - 1) {
                break;  // No space to paste
            }
            len = (COLS - 3) - dest_len - 1;
        }

        memmove(&dest[*cursor_x - 1 + len],
                &dest[*cursor_x - 1],
                dest_len - (*cursor_x - 1) + 1);
        memcpy(&dest[*cursor_x - 1], line, len);
        line = strtok(NULL, "\n");
        (*cursor_y)++;
        *cursor_x = 1;
    }
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
        if (*cursor_x < COLS - 6) (*cursor_x)++;
        fs->sel_end_x = *cursor_x;
    }
    if (ch == 10) {
        end_selection_mode(fs);
    }
}


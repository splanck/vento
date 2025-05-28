#include <string.h>
#include <ncurses.h>
#include "editor.h"
#include "clipboard.h"
#include "files.h"

char *clipboard;
bool selection_mode = false;
int sel_start_x = 0, sel_start_y = 0;
int sel_end_x = 0, sel_end_y = 0;

void start_selection_mode(FileState *fs, int cursor_x, int cursor_y) {
    (void)fs;
    selection_mode = true;
    sel_start_x = cursor_x;
    sel_start_y = cursor_y;
    sel_end_x = cursor_x;
    sel_end_y = cursor_y;
}

void end_selection_mode(FileState *fs) {
    selection_mode = false;
    copy_selection(fs);
}

void copy_selection(FileState *fs) {
    int start_x = sel_start_x;
    int end_x = sel_end_x;
    int start_y, end_y;

    if (sel_start_y < sel_end_y) {
        start_y = sel_start_y;
        end_y = sel_end_y;
    } else {
        start_y = sel_end_y;
        end_y = sel_start_y;
    }

    if (clipboard == NULL) {
        return;
    }

    clipboard[0] = '\0';  // Clear clipboard
    size_t clip_len = 0;
    for (int y = start_y; y <= end_y && clip_len < CLIPBOARD_SIZE - 1; y++) {
        if (y == start_y) {
            const char *src = &text_buffer[y - 1 + fs->start_line][start_x - 1];
            size_t to_copy = end_x - start_x + 1;
            if (to_copy > CLIPBOARD_SIZE - 1 - clip_len) {
                to_copy = CLIPBOARD_SIZE - 1 - clip_len;
            }
            strncpy(clipboard + clip_len, src, to_copy);
            clip_len += to_copy;
            clipboard[clip_len] = '\0';
        } else {
            if (clip_len < CLIPBOARD_SIZE - 1) {
                clipboard[clip_len++] = '\n';
                clipboard[clip_len] = '\0';
            }
            const char *src = text_buffer[y - 1 + fs->start_line];
            size_t to_copy = strlen(src);
            if (to_copy > CLIPBOARD_SIZE - 1 - clip_len) {
                to_copy = CLIPBOARD_SIZE - 1 - clip_len;
            }
            strncpy(clipboard + clip_len, src, to_copy);
            clip_len += to_copy;
            clipboard[clip_len] = '\0';
        }
    }
}

void paste_clipboard(FileState *fs, int *cursor_x, int *cursor_y) {
    if (clipboard == NULL) {
        return;
    }

    char tmp[CLIPBOARD_SIZE];
    strncpy(tmp, clipboard, sizeof(tmp) - 1);
    tmp[sizeof(tmp) - 1] = '\0';

    char *line = strtok(tmp, "\n");
    while (line) {
        size_t len = strlen(line);
        char *dest = text_buffer[*cursor_y - 1 + fs->start_line];
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
        sel_end_y = *cursor_y;
    }
    if (ch == KEY_DOWN) {
        if (*cursor_y < LINES - 4) (*cursor_y)++;
        sel_end_y = *cursor_y;
    }
    if (ch == KEY_LEFT) {
        if (*cursor_x > 1) (*cursor_x)--;
        sel_end_x = *cursor_x;
    }
    if (ch == KEY_RIGHT) {
        if (*cursor_x < COLS - 6) (*cursor_x)++;
        sel_end_x = *cursor_x;
    }
    if (ch == 10) {
        end_selection_mode(fs);
    }
}


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
    for (int y = start_y; y <= end_y; y++) {
        if (y == start_y) {
            strncat(clipboard,
                    &text_buffer[y - 1 + fs->start_line][start_x - 1],
                    end_x - start_x + 1);
        } else {
            strcat(clipboard, "\n");
            strcat(clipboard, text_buffer[y - 1 + fs->start_line]);
        }
    }
}

void paste_clipboard(FileState *fs, int *cursor_x, int *cursor_y) {
    if (clipboard == NULL) {
        return;
    }

    char tmp[CLIPBOARD_SIZE];
    strcpy(tmp, clipboard);

    char *line = strtok(tmp, "\n");
    while (line) {
        int len = strlen(line);
        memmove(&text_buffer[*cursor_y - 1 + fs->start_line][*cursor_x - 1 + len],
                &text_buffer[*cursor_y - 1 + fs->start_line][*cursor_x - 1],
                strlen(&text_buffer[*cursor_y - 1 + fs->start_line][*cursor_x - 1]) + 1);
        memcpy(&text_buffer[*cursor_y - 1 + fs->start_line][*cursor_x - 1], line, len);
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


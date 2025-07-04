/*
 * Clipboard operations and selection mode management
 * --------------------------------------------------
 * When a user begins selecting text, start_selection_mode records the starting
 * coordinates in the active FileState. Cursor movements while in selection mode
 * update the end coordinates but do not modify any buffers. Once the selection
 * ends, the selected text can be copied or cut into the shared
 * `global_clipboard` array. This buffer is used across files and paste actions
 * simply insert its contents at the current cursor location.
 */
#include <string.h>
#include <ncurses.h>
#include <stdlib.h>
#include <stdbool.h>
#include "editor.h"
#include "clipboard.h"
#include "files.h"
#include "syntax.h"
#include "undo.h"

/* Global clipboard buffer shared by all files */
char global_clipboard[CLIPBOARD_SIZE];

/* Selection state is stored in FileState */

/*
 * Begin text selection.
 *
 * fs       - file state to update
 * cursor_x - starting column (1-based)
 * cursor_y - starting row (1-based)
 *
 * Enables selection_mode and records start/end coordinates.
 */
void start_selection_mode(FileState *fs, int cursor_x, int cursor_y) {
    fs->selection_mode = true;
    fs->sel_start_x = cursor_x;
    fs->sel_start_y = cursor_y;
    fs->sel_end_x = cursor_x;
    fs->sel_end_y = cursor_y;
}

/*
 * Finish selection mode and copy the marked text.
 *
 * fs - file state containing selection coordinates
 *
 * Disables selection_mode and stores the selection in global_clipboard.
 */
void end_selection_mode(FileState *fs) {
    fs->selection_mode = false;
}

/*
 * Copy the currently highlighted region into global_clipboard.
 *
 * fs - file state providing selection coordinates and buffer data
 *
 * Overwrites global_clipboard with the selected text.
 */
void copy_selection(FileState *fs) {
    int start_x = fs->sel_start_x;
    int end_x = fs->sel_end_x;
    int start_y = fs->sel_start_y;
    int end_y = fs->sel_end_y;

    if (start_y > end_y) {
        start_y = fs->sel_end_y;
        end_y = fs->sel_start_y;
        start_x = fs->sel_end_x;
        end_x = fs->sel_start_x;
    } else if (start_y == end_y && start_x > end_x) {
        int tmp = start_x;
        start_x = end_x;
        end_x = tmp;
    }

    global_clipboard[0] = '\0';  // Clear clipboard
    size_t clip_len = 0;
    for (int y = start_y; y <= end_y && clip_len < CLIPBOARD_SIZE - 1; y++) {
        ensure_line_loaded(fs, y - 1 + fs->start_line);
        if (y == start_y) {
            const char *line = lb_get(&fs->buffer, y - 1 + fs->start_line);
            const char *src = line ? line + start_x - 1 : "";
            size_t to_copy = end_x - start_x + 1;
            if (to_copy > CLIPBOARD_SIZE - 1 - clip_len) {
                to_copy = CLIPBOARD_SIZE - 1 - clip_len;
            }
            strncpy(global_clipboard + clip_len, src, to_copy);
            clip_len += to_copy;
            global_clipboard[clip_len] = '\0';
        } else {
            if (clip_len < CLIPBOARD_SIZE - 1) {
                global_clipboard[clip_len++] = '\n';
                global_clipboard[clip_len] = '\0';
            }
            const char *src = lb_get(&fs->buffer, y - 1 + fs->start_line);
            size_t to_copy = strlen(src);
            if (to_copy > CLIPBOARD_SIZE - 1 - clip_len) {
                to_copy = CLIPBOARD_SIZE - 1 - clip_len;
            }
            strncpy(global_clipboard + clip_len, src, to_copy);
            clip_len += to_copy;
            global_clipboard[clip_len] = '\0';
        }
    }
}

/*
 * Insert the contents of global_clipboard at the cursor position.
 *
 * fs        - file receiving the text
 * cursor_x  - column position, updated after paste
 * cursor_y  - row position, updated after paste
 *
 * Modifies the file buffer, moves the cursor and marks the file modified.
 */
void paste_clipboard(FileState *fs, int *cursor_x, int *cursor_y) {
    char tmp[CLIPBOARD_SIZE];
    strncpy(tmp, global_clipboard, sizeof(tmp) - 1);
    tmp[sizeof(tmp) - 1] = '\0';

    char *line = strtok(tmp, "\n");
    bool first = true;
    while (line) {
        size_t len = strlen(line);
        int line_idx = *cursor_y - 1 + fs->start_line;
        char *dest = fs->buffer.lines[line_idx];
        size_t dest_len = strlen(dest);
        char *old_text = NULL;
        
        if (first) {
            old_text = strdup(dest);
            if (!old_text)
                allocation_failed("strdup failed");
        }

        if (ensure_col_capacity(fs, (int)(dest_len + len + 1)) < 0) {
            if (old_text)
                free(old_text);
            allocation_failed("ensure_col_capacity failed");
        }
        dest = fs->buffer.lines[line_idx];

        if (*cursor_x > (int)dest_len + 1)
            *cursor_x = dest_len + 1;
        size_t idx = (size_t)(*cursor_x - 1);
        memmove(&dest[idx + len], &dest[idx], dest_len - idx + 1);
        memcpy(&dest[idx], line, len);
        *cursor_x += len;

        char *new_text = strdup(dest);
        if (!new_text) {
            if (old_text)
                free(old_text);
            allocation_failed("strdup failed");
        }

        if (first) {
            push(&fs->undo_stack, (Change){ line_idx, old_text, new_text });
        } else {
            push(&fs->undo_stack, (Change){ line_idx, NULL, new_text });
            if (old_text)
                free(old_text); /* only free if allocated */
        }

        line = strtok(NULL, "\n");
        if (line) {
            /* Move to the next line and create it in the buffer */
            (*cursor_y)++;
            fs->cursor_x = *cursor_x = 1;
            fs->cursor_y = *cursor_y;
            if (ensure_line_capacity(fs, fs->buffer.count + 1) < 0) {
                show_message("Unable to allocate line");
                break;
            }
            if (lb_insert(&fs->buffer, *cursor_y - 1 + fs->start_line, "") < 0) {
                show_message("Unable to insert line");
                break;
            }
            char *p = realloc(fs->buffer.lines[*cursor_y - 1 + fs->start_line], fs->line_capacity);
            if (!p)
                allocation_failed("realloc failed");
            fs->buffer.lines[*cursor_y - 1 + fs->start_line] = p;
        }
        first = false;
    }

    fs->cursor_x = *cursor_x;
    fs->cursor_y = *cursor_y;
    fs->modified = true;
}

/*
 * Update the selection based on navigation keys while selection_mode is active.
 *
 * fs        - active file
 * ch        - key pressed
 * cursor_x  - current column, modified by the function
 * cursor_y  - current row, modified by the function
 *
 * Arrow keys extend the selection and Enter finalizes it.
 */
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

/*
 * Copy the active selection via keyboard shortcut.
 *
 * fs - file from which to copy
 *
 * Simply calls copy_selection when selection_mode is enabled.
 */
void copy_selection_keyboard(FileState *fs) {
    if (!fs->selection_mode)
        return;
    copy_selection(fs);
}

/*
 * Cut the selected text from the buffer and copy it to global_clipboard.
 *
 * fs - file containing the selection
 *
 * Deletes the selection from the buffer, positions the cursor at the start
 * of the removed region and disables selection_mode.
 */
void cut_selection(FileState *fs) {
    if (!fs->selection_mode)
        return;

    int start_y = fs->sel_start_y;
    int end_y = fs->sel_end_y;
    int start_x = fs->sel_start_x;
    int end_x = fs->sel_end_x;

    if (start_y > end_y) {
        start_y = fs->sel_end_y;
        end_y = fs->sel_start_y;
        start_x = fs->sel_end_x;
        end_x = fs->sel_start_x;
    } else if (start_y == end_y && start_x > end_x) {
        int tmp = start_x;
        start_x = end_x;
        end_x = tmp;
    }

    for (int y = start_y; y <= end_y; ++y)
        ensure_line_loaded(fs, y - 1 + fs->start_line);
    copy_selection(fs);

    char *first = fs->buffer.lines[start_y - 1 + fs->start_line];
    char *last = fs->buffer.lines[end_y - 1 + fs->start_line];
    char *old_first = strdup(first);
    if (!old_first)
        allocation_failed("strdup failed");

    if (start_y == end_y) {
        memmove(&first[start_x - 1], &first[end_x], strlen(first) - end_x + 1);
        char *new_first = strdup(first);
        if (!new_first) {
            free(old_first);
            allocation_failed("strdup failed");
        }
        push(&fs->undo_stack,
             (Change){ start_y - 1 + fs->start_line, old_first, new_first });
    } else {
        first[start_x - 1] = '\0';
        strncat(first, &last[end_x], fs->line_capacity - strlen(first) - 1);
        char *new_first = strdup(first);
        if (!new_first) {
            free(old_first);
            allocation_failed("strdup failed");
        }
        push(&fs->undo_stack,
             (Change){ start_y - 1 + fs->start_line, old_first, new_first });

        int remove_count = end_y - start_y;
        int del_idx = start_y - 1 + fs->start_line + 1;
        for (int i = 0; i < remove_count; ++i) {
            char *old_line = strdup(fs->buffer.lines[del_idx]);
            if (!old_line)
                allocation_failed("strdup failed");
            push(&fs->undo_stack, (Change){ del_idx, old_line, NULL });
            lb_delete(&fs->buffer, del_idx);
        }
    }

    fs->cursor_x = start_x;
    fs->cursor_y = start_y;
    fs->selection_mode = false;
    fs->modified = true;
}


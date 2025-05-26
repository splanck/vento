#include "editor.h"
#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "input.h"

char *strdup(const char *s);  // Explicitly declare strdup

void handle_ctrl_backtick() {
    // Do nothing on CTRL+Backtick to avoid segmentation fault
}

/**
 * Handles the key up event.
 * Moves the cursor up within the visible text area.
 * If the cursor is at the top of the window, scrolls the text buffer up and redraws the view.
 *
 * @param cursor_y Pointer to the current y-coordinate of the cursor.
 * @param start_line Pointer to the starting line of the visible text area.
 */
void handle_key_up(int *cursor_y, int *start_line) {
    // Move cursor up within the visible text area
    if (*cursor_y > 1) {
        (*cursor_y)--;
    }
    // Scroll the text buffer up when the cursor is at the top of the window
    else if (*start_line > 0) {
        (*start_line)--;
        // Redraw the text buffer to reflect the new view
        draw_text_buffer(text_win);
    }
}

/**
 * Handles the key down event.
 * Moves the cursor down within the visible text area.
 * If the cursor is at the bottom of the window, scrolls the text buffer down and redraws the view.
 *
 * @param cursor_y Pointer to the current y-coordinate of the cursor.
 * @param start_line Pointer to the starting line of the visible text area.
 */
void handle_key_down(int *cursor_y, int *start_line) {
    // Check if the cursor can move down within the visible area
    if (*cursor_y < LINES - BOTTOM_MARGIN && *cursor_y < line_count) {
        (*cursor_y)++;
    }
    // Scroll the text buffer if there's more content below the current view
    else if (*start_line + *cursor_y < line_count) {
        (*start_line)++;
        draw_text_buffer(text_win); // Consider optimizing this operation
    }
}

/**
 * Handles the key left event.
 * Moves the cursor left within the current line.
 *
 * @param cursor_x Pointer to the current x-coordinate of the cursor.
 */
void handle_key_left(int *cursor_x) {
    if (*cursor_x > 1) {
        (*cursor_x)--;
    }
}

/**
 * Handles the key right event.
 * Moves the cursor right within the current line.
 *
 * @param cursor_x Pointer to the current x-coordinate of the cursor.
 * @param cursor_y The current y-coordinate of the cursor.
 */
void handle_key_right(int *cursor_x, int cursor_y) {
    // Move cursor right within the current line
    if (*cursor_x < (int)strlen(text_buffer[cursor_y - 1 + start_line]) + 1) {
        (*cursor_x)++;
    }
}

/**
 * Handles the key backspace event.
 * Deletes the character to the left of the cursor.
 * If the cursor is at the beginning of a line, merges the current line with the previous line.
 *
 * @param cursor_x Pointer to the current x-coordinate of the cursor.
 * @param cursor_y Pointer to the current y-coordinate of the cursor.
 * @param start_line Pointer to the starting line of the visible text area.
 */
void handle_key_backspace(int *cursor_x, int *cursor_y, int *start_line) {
    // Delete character to the left of the cursor
    if (*cursor_x > 1) {
        (*cursor_x)--;
        memmove(&text_buffer[*cursor_y - 1 + *start_line][*cursor_x - 1], &text_buffer[*cursor_y - 1 + *start_line][*cursor_x], strlen(text_buffer[*cursor_y - 1 + *start_line]) - *cursor_x + 1);
    }
    // Merge current line with the previous line
    else if (*cursor_y > 1 || *start_line > 0) {
        size_t prev_len = strlen(text_buffer[*cursor_y - 2 + *start_line]);
        // Check if the merged line fits within the screen width
        if (prev_len + strlen(text_buffer[*cursor_y - 1 + *start_line]) < (size_t)(COLS - 6)) {
            // Merge the lines
            strcat(text_buffer[*cursor_y - 2 + *start_line], text_buffer[*cursor_y - 1 + *start_line]);
            // Shift the remaining lines up
            for (int i = *cursor_y - 1 + *start_line; i < line_count - 1; ++i) {
                strcpy(text_buffer[i], text_buffer[i + 1]);
            }
            // Clear the last line
            text_buffer[line_count - 1][0] = '\0';
            line_count--;
            // Update cursor position
            if (*cursor_y > 1) {
                (*cursor_y)--;
            } else {
                (*start_line)--;
                draw_text_buffer(text_win);
            }
            *cursor_x = prev_len + 1;
        }
    }
    // Redraw the text buffer
    werase(text_win);
    box(text_win, 0, 0);
    draw_text_buffer(text_win);
}

/**
 * Handles the key delete event.
 * Deletes the character at the cursor position.
 * If the cursor is at the end of a line, merges the current line with the next line.
 *
 * @param cursor_x Pointer to the current x-coordinate of the cursor.
 * @param cursor_y The current y-coordinate of the cursor.
 */
void handle_key_delete(int *cursor_x, int cursor_y) {
    // Delete character at the cursor position
    if (*cursor_x < (int)strlen(text_buffer[cursor_y - 1 + start_line])) {
        memmove(&text_buffer[cursor_y - 1 + start_line][*cursor_x - 1], &text_buffer[cursor_y - 1 + start_line][*cursor_x], strlen(text_buffer[cursor_y - 1 + start_line]) - *cursor_x + 1);
    }
    // Merge current line with the next line
    else if (cursor_y + start_line < line_count) {
        strcat(text_buffer[cursor_y - 1 + start_line], text_buffer[cursor_y + start_line]);
        for (int i = cursor_y + start_line; i < line_count - 1; ++i) {
            strcpy(text_buffer[i], text_buffer[i + 1]);
        }
        text_buffer[line_count - 1][0] = '\0';
        line_count--;
    }
    werase(text_win);
    box(text_win, 0, 0);
    draw_text_buffer(text_win);
}

/**
 * Handles the key enter event.
 * Inserts a new line at the current cursor position.
 * If the maximum number of lines has been reached, the function does nothing.
 *
 * @param cursor_x Pointer to the current x-coordinate of the cursor.
 * @param cursor_y Pointer to the current y-coordinate of the cursor.
 * @param start_line Pointer to the starting line of the visible text area.
 */
void handle_key_enter(int *cursor_x, int *cursor_y, int *start_line) {
    // Check if the maximum number of lines has been reached
    if (line_count < MAX_LINES - 1) {
        // Move lines below the current line down by one
        for (int i = line_count; i > *cursor_y + *start_line; --i) {
            strcpy(text_buffer[i], text_buffer[i - 1]);
        }
        line_count++;

        // Handle case where Enter is pressed at the current position
        if (*cursor_x > 1) {
            strcpy(text_buffer[*cursor_y + *start_line], &text_buffer[*cursor_y - 1 + *start_line][*cursor_x - 1]);
            text_buffer[*cursor_y - 1 + *start_line][*cursor_x - 1] = '\0';
        } else {
            strcpy(text_buffer[*cursor_y + *start_line], text_buffer[*cursor_y - 1 + *start_line]);
            text_buffer[*cursor_y - 1 + *start_line][0] = '\0';
        }

        *cursor_x = 1;
        if (*cursor_y >= LINES - 6) {
            (*start_line)++;
        } else {
            (*cursor_y)++;
        }
        werase(text_win);
        box(text_win, 0, 0);
        draw_text_buffer(text_win);
    }
}

/**
 * Handles the key page up event.
 * Moves the starting line up by one page size.
 * If the starting line is already at the top, the function does nothing.
 *
 * @param cursor_y Pointer to the current y-coordinate of the cursor.
 * @param start_line Pointer to the starting line of the visible text area.
 */
void handle_key_page_up(int *cursor_y, int *start_line) {
    int page_size = LINES - 4; // Adjust for the status bar

    // Move the starting line up by one page size
    if (*start_line > 0) {
        *start_line -= page_size;
        if (*start_line < 0) {
            *start_line = 0;
        }
        draw_text_buffer(text_win); // Redraw the text buffer to reflect the new view
    }

    *cursor_y = 1; // Move the cursor to the top of the screen
}

/**
 * Handles the key page down event.
 * Moves the starting line down by one page size.
 * If the starting line is already at the bottom, the function does nothing.
 *
 * @param cursor_y Pointer to the current y-coordinate of the cursor.
 * @param start_line Pointer to the starting line of the visible text area.
 */
void handle_key_page_down(int *cursor_y, int *start_line) {
    int max_lines = LINES - 4; // Adjust for the status bar

    // Move the starting line down
    if (*start_line + max_lines < line_count) {
        *start_line += max_lines;
    } else {
        *start_line = line_count - max_lines;
        if (*start_line < 0) {
            *start_line = 0;
        }
    }

    // Move the cursor to the bottom of the screen
    *cursor_y = max_lines;

    // Ensure the cursor doesn't go past the end of the file
    if (*cursor_y + *start_line >= line_count) {
        if (line_count > 0) {
            *cursor_y = line_count - *start_line;
        } else {
            *cursor_y = 1;
        }
    }
}

/**
 * Handles the Ctrl + Page Up key event.
 * Moves the cursor to the top of the screen and sets the starting line to the first line.
 *
 * @param cursor_y Pointer to the current y-coordinate of the cursor.
 * @param start_line Pointer to the starting line of the visible text area.
 */
void handle_ctrl_key_pgup(int *cursor_y, int *start_line) {
    *cursor_y = 1;
    *start_line = 0;
    draw_text_buffer(text_win);
}

/**
 * Handles the Ctrl + Page Down key event.
 * Moves the cursor to the bottom of the screen and sets the starting line to show the last lines.
 *
 * @param cursor_y Pointer to the current y-coordinate of the cursor.
 * @param start_line Pointer to the starting line of the visible text area.
 */
void handle_ctrl_key_pgdn(int *cursor_y, int *start_line) {
    *cursor_y = LINES - 4; // Adjust for the status bar
    if (line_count > LINES - 4) {
        *start_line = line_count - (LINES - 4);
    } else {
        *start_line = 0;
        *cursor_y = line_count;
    }
    draw_text_buffer(text_win);
}

/**
 * Handles the Ctrl + Up key event.
 * Moves the cursor to the top of the screen.
 *
 * @param cursor_y Pointer to the current y-coordinate of the cursor.
 */
void handle_ctrl_key_up(int *cursor_y) {
    *cursor_y = 1;
}

/**
 * Handles the Ctrl + Down key event.
 * Moves the cursor to the bottom of the screen.
 *
 * @param cursor_y Pointer to the current y-coordinate of the cursor.
 */
void handle_ctrl_key_down(int *cursor_y) {
    *cursor_y = LINES - 4; // Adjust for the status bar
}

/**
 * Handles the Ctrl + Left key event.
 * Moves the cursor to the beginning of the line.
 *
 * @param cursor_x Pointer to the current x-coordinate of the cursor.
 */
void handle_ctrl_key_left(int *cursor_x) {
    *cursor_x = 1;
}

/**
 * Handles the Ctrl + Right key event.
 * Moves the cursor to the end of the line.
 *
 * @param cursor_x Pointer to the current x-coordinate of the cursor.
 * @param cursor_y The current y-coordinate of the cursor.
 */
void handle_ctrl_key_right(int *cursor_x, int cursor_y) {
    *cursor_x = strlen(text_buffer[cursor_y - 1 + start_line]) + 1;
}

/**
 * Handles the default key event.
 * Inserts the character at the current cursor position.
 * If the cursor is at the end of a line, the character is appended to the line.
 *
 * @param ch The character to be inserted.
 * @param cursor_x Pointer to the current x-coordinate of the cursor.
 * @param cursor_y The current y-coordinate of the cursor.
 */
void handle_default_key(int ch, int *cursor_x, int cursor_y) {
#ifdef KEY_TAB
    if (ch == KEY_TAB || ch == '\t') {
#else
    if (ch == '\t') {
#endif
        const int TAB_SIZE = 4;
        for (int i = 0; i < TAB_SIZE; i++) {
            handle_default_key(' ', cursor_x, cursor_y);
        }
        return;
    }
    if (*cursor_x < COLS - 6) {
        int len = strlen(text_buffer[cursor_y - 1 + start_line]);
        char *old_text = strdup(text_buffer[cursor_y - 1 + start_line]);

        // Shift the characters to the right of the cursor to make space for the new character
        if (*cursor_x <= len) {
            memmove(&text_buffer[cursor_y - 1 + start_line][*cursor_x], &text_buffer[cursor_y - 1 + start_line][*cursor_x - 1], len - *cursor_x + 1);
        }

        // Insert the new character at the cursor position
        text_buffer[cursor_y - 1 + start_line][*cursor_x - 1] = ch;
        text_buffer[cursor_y - 1 + start_line][len + 1] = '\0';  // Ensure null termination
        (*cursor_x)++;

        char *new_text = strdup(text_buffer[cursor_y - 1 + start_line]);
        Change change = { cursor_y - 1 + start_line, old_text, new_text };
        push(&undo_stack, change);
    }

    // Redraw the text buffer
    werase(text_win);
    box(text_win, 0, 0);
    draw_text_buffer(text_win);
}

/**
 * Moves the cursor forward to the beginning of the next word.
 * If the cursor is already at the end of the line, it moves to the start of the next line.
 *
 * @param cursor_x Pointer to the current x-coordinate of the cursor.
 * @param cursor_y Pointer to the current y-coordinate of the cursor.
 */
void move_forward_to_next_word(int *cursor_x, int *cursor_y) {
    while (*cursor_y - 1 + start_line < line_count) {
        char *line = text_buffer[*cursor_y - 1 + start_line];
        int len = strlen(line);

        // Move cursor to the end of the current word
        while (*cursor_x < len && isalnum(line[*cursor_x - 1])) {
            (*cursor_x)++;
        }

        // Move cursor to the beginning of the next word
        while (*cursor_x < len && !isalnum(line[*cursor_x - 1])) {
            (*cursor_x)++;
        }

        // If the cursor is within the line, break the loop
        if (*cursor_x < len) {
            return;
        }

        // Move to the start of the next line
        *cursor_x = 1;
        (*cursor_y)++;
    }
}

/**
 * Moves the cursor backward to the beginning of the previous word.
 * If the cursor is already at the start of the line, it moves to the end of the previous line.
 *
 * @param cursor_x Pointer to the current x-coordinate of the cursor.
 * @param cursor_y Pointer to the current y-coordinate of the cursor.
 */
void move_backward_to_previous_word(int *cursor_x, int *cursor_y) {
    while (*cursor_y - 1 + start_line >= 0) {
        char *line = text_buffer[*cursor_y - 1 + start_line];

        // Move cursor to the beginning of the current word
        while (*cursor_x > 1 && isalnum(line[*cursor_x - 2])) {
            (*cursor_x)--;
        }

        // Move cursor to the beginning of the previous word
        while (*cursor_x > 1 && !isalnum(line[*cursor_x - 2])) {
            (*cursor_x)--;
        }

        // If the cursor is within the line, break the loop
        if (*cursor_x > 1) {
            return;
        }

        // Move to the end of the previous line
        if (*cursor_y > 1) {
            (*cursor_y)--;
            line = text_buffer[*cursor_y - 1 + start_line];
            *cursor_x = strlen(line) + 1;
        } else {
            *cursor_x = 1;
            break;
        }
    }
}


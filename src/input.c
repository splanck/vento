#include "editor.h"
#include <ncurses.h>
#include "undo.h"
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
void handle_key_up(FileState *fs) {
    if (fs->cursor_y > 1) {
        fs->cursor_y--;
    } else if (fs->start_line > 0) {
        fs->start_line--;
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
void handle_key_down(FileState *fs) {
    if (fs->cursor_y < LINES - BOTTOM_MARGIN && fs->cursor_y < line_count) {
        fs->cursor_y++;
    } else if (fs->start_line + fs->cursor_y < line_count) {
        fs->start_line++;
        draw_text_buffer(text_win);
    }
}

/**
 * Handles the key left event.
 * Moves the cursor left within the current line.
 *
 * @param cursor_x Pointer to the current x-coordinate of the cursor.
 */
void handle_key_left(FileState *fs) {
    if (fs->cursor_x > 1) {
        fs->cursor_x--;
    }
}

/**
 * Handles the key right event.
 * Moves the cursor right within the current line.
 *
 * @param cursor_x Pointer to the current x-coordinate of the cursor.
 * @param cursor_y The current y-coordinate of the cursor.
 */
void handle_key_right(FileState *fs) {
    if (fs->cursor_x < (int)strlen(text_buffer[fs->cursor_y - 1 + start_line]) + 1) {
        fs->cursor_x++;
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
void handle_key_backspace(FileState *fs) {
    if (fs->cursor_x > 1) {
        fs->cursor_x--;
        memmove(&text_buffer[fs->cursor_y - 1 + fs->start_line][fs->cursor_x - 1],
                &text_buffer[fs->cursor_y - 1 + fs->start_line][fs->cursor_x],
                strlen(text_buffer[fs->cursor_y - 1 + fs->start_line]) - fs->cursor_x + 1);
    } else if (fs->cursor_y > 1 || fs->start_line > 0) {
        size_t prev_len = strlen(text_buffer[fs->cursor_y - 2 + fs->start_line]);
        // Check if the merged line fits within the screen width
        if (prev_len + strlen(text_buffer[fs->cursor_y - 1 + fs->start_line]) < (size_t)(COLS - 6)) {
            strcat(text_buffer[fs->cursor_y - 2 + fs->start_line], text_buffer[fs->cursor_y - 1 + fs->start_line]);
            for (int i = fs->cursor_y - 1 + fs->start_line; i < line_count - 1; ++i) {
                strcpy(text_buffer[i], text_buffer[i + 1]);
            }
            // Clear the last line
            text_buffer[line_count - 1][0] = '\0';
            line_count--;
            if (fs->cursor_y > 1) {
                fs->cursor_y--;
            } else {
                fs->start_line--;
                draw_text_buffer(text_win);
            }
            fs->cursor_x = prev_len + 1;
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
void handle_key_delete(FileState *fs) {
    if (fs->cursor_x < (int)strlen(text_buffer[fs->cursor_y - 1 + start_line])) {
        memmove(&text_buffer[fs->cursor_y - 1 + start_line][fs->cursor_x - 1],
                &text_buffer[fs->cursor_y - 1 + start_line][fs->cursor_x],
                strlen(text_buffer[fs->cursor_y - 1 + start_line]) - fs->cursor_x + 1);
    } else if (fs->cursor_y + start_line < line_count) {
        strcat(text_buffer[fs->cursor_y - 1 + start_line], text_buffer[fs->cursor_y + start_line]);
        for (int i = fs->cursor_y + start_line; i < line_count - 1; ++i) {
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
void handle_key_enter(FileState *fs) {
    if (line_count < MAX_LINES - 1) {
        for (int i = line_count; i > fs->cursor_y + fs->start_line; --i) {
            strcpy(text_buffer[i], text_buffer[i - 1]);
        }
        line_count++;

        if (fs->cursor_x > 1) {
            strcpy(text_buffer[fs->cursor_y + fs->start_line],
                   &text_buffer[fs->cursor_y - 1 + fs->start_line][fs->cursor_x - 1]);
            text_buffer[fs->cursor_y - 1 + fs->start_line][fs->cursor_x - 1] = '\0';
        } else {
            strcpy(text_buffer[fs->cursor_y + fs->start_line],
                   text_buffer[fs->cursor_y - 1 + fs->start_line]);
            text_buffer[fs->cursor_y - 1 + fs->start_line][0] = '\0';
        }

        fs->cursor_x = 1;
        if (fs->cursor_y >= LINES - 6) {
            fs->start_line++;
        } else {
            fs->cursor_y++;
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
void handle_key_page_up(FileState *fs) {
    int page_size = LINES - 4; // Adjust for the status bar

    // Move the starting line up by one page size
    if (fs->start_line > 0) {
        fs->start_line -= page_size;
        if (fs->start_line < 0) {
            fs->start_line = 0;
        }
        draw_text_buffer(text_win); // Redraw the text buffer to reflect the new view
    }

    fs->cursor_y = 1;
}

/**
 * Handles the key page down event.
 * Moves the starting line down by one page size.
 * If the starting line is already at the bottom, the function does nothing.
 *
 * @param cursor_y Pointer to the current y-coordinate of the cursor.
 * @param start_line Pointer to the starting line of the visible text area.
 */
void handle_key_page_down(FileState *fs) {
    int max_lines = LINES - 4; // Adjust for the status bar

    // Move the starting line down
    if (fs->start_line + max_lines < line_count) {
        fs->start_line += max_lines;
    } else {
        fs->start_line = line_count - max_lines;
        if (fs->start_line < 0) {
            fs->start_line = 0;
        }
    }

    // Move the cursor to the bottom of the screen
    fs->cursor_y = max_lines;

    // Ensure the cursor doesn't go past the end of the file
    if (fs->cursor_y + fs->start_line >= line_count) {
        if (line_count > 0) {
            fs->cursor_y = line_count - fs->start_line;
        } else {
            fs->cursor_y = 1;
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
void handle_ctrl_key_pgup(FileState *fs) {
    fs->cursor_y = 1;
    fs->start_line = 0;
    draw_text_buffer(text_win);
}

/**
 * Handles the Ctrl + Page Down key event.
 * Moves the cursor to the bottom of the screen and sets the starting line to show the last lines.
 *
 * @param cursor_y Pointer to the current y-coordinate of the cursor.
 * @param start_line Pointer to the starting line of the visible text area.
 */
void handle_ctrl_key_pgdn(FileState *fs) {
    fs->cursor_y = LINES - 4; // Adjust for the status bar
    if (line_count > LINES - 4) {
        fs->start_line = line_count - (LINES - 4);
    } else {
        fs->start_line = 0;
        fs->cursor_y = line_count;
    }
    draw_text_buffer(text_win);
}

/**
 * Handles the Ctrl + Up key event.
 * Moves the cursor to the top of the screen.
 *
 * @param cursor_y Pointer to the current y-coordinate of the cursor.
 */
void handle_ctrl_key_up(FileState *fs) {
    fs->cursor_y = 1;
}

/**
 * Handles the Ctrl + Down key event.
 * Moves the cursor to the bottom of the screen.
 *
 * @param cursor_y Pointer to the current y-coordinate of the cursor.
 */
void handle_ctrl_key_down(FileState *fs) {
    fs->cursor_y = LINES - 4; // Adjust for the status bar
}

/**
 * Handles the Ctrl + Left key event.
 * Moves the cursor to the beginning of the line.
 *
 * @param cursor_x Pointer to the current x-coordinate of the cursor.
 */
void handle_ctrl_key_left(FileState *fs) {
    fs->cursor_x = 1;
}

/**
 * Handles the Ctrl + Right key event.
 * Moves the cursor to the end of the line.
 *
 * @param cursor_x Pointer to the current x-coordinate of the cursor.
 * @param cursor_y The current y-coordinate of the cursor.
 */
void handle_ctrl_key_right(FileState *fs) {
    fs->cursor_x = strlen(text_buffer[fs->cursor_y - 1 + start_line]) + 1;
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
void handle_default_key(FileState *fs, int ch) {
#ifdef KEY_TAB
    if (ch == KEY_TAB || ch == '\t') {
#else
    if (ch == '\t') {
#endif
        const int TAB_SIZE = 4;
        for (int i = 0; i < TAB_SIZE; i++) {
            handle_default_key(fs, ' ');
        }
        return;
    }
    if (fs->cursor_x < COLS - 6) {
        int len = strlen(text_buffer[fs->cursor_y - 1 + start_line]);
        char *old_text = strdup(text_buffer[fs->cursor_y - 1 + start_line]);

        // Shift the characters to the right of the cursor to make space for the new character
        if (fs->cursor_x <= len) {
            memmove(&text_buffer[fs->cursor_y - 1 + start_line][fs->cursor_x],
                    &text_buffer[fs->cursor_y - 1 + start_line][fs->cursor_x - 1],
                    len - fs->cursor_x + 1);
        }

        // Insert the new character at the cursor position
        text_buffer[fs->cursor_y - 1 + start_line][fs->cursor_x - 1] = ch;
        text_buffer[fs->cursor_y - 1 + start_line][len + 1] = '\0';
        fs->cursor_x++;

        char *new_text = strdup(text_buffer[fs->cursor_y - 1 + start_line]);
        Change change = { fs->cursor_y - 1 + start_line, old_text, new_text };
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
void move_forward_to_next_word(FileState *fs) {
    while (fs->cursor_y - 1 + start_line < line_count) {
        char *line = text_buffer[fs->cursor_y - 1 + start_line];
        int len = strlen(line);

        // Move cursor to the end of the current word
        while (fs->cursor_x < len && isalnum(line[fs->cursor_x - 1])) {
            fs->cursor_x++;
        }

        // Move cursor to the beginning of the next word
        while (fs->cursor_x < len && !isalnum(line[fs->cursor_x - 1])) {
            fs->cursor_x++;
        }

        // If the cursor is within the line, break the loop
        if (fs->cursor_x < len) {
            return;
        }

        // Move to the start of the next line
        fs->cursor_x = 1;
        fs->cursor_y++;
    }
}

/**
 * Moves the cursor backward to the beginning of the previous word.
 * If the cursor is already at the start of the line, it moves to the end of the previous line.
 *
 * @param cursor_x Pointer to the current x-coordinate of the cursor.
 * @param cursor_y Pointer to the current y-coordinate of the cursor.
 */
void move_backward_to_previous_word(FileState *fs) {
    while (fs->cursor_y - 1 + start_line >= 0) {
        char *line = text_buffer[fs->cursor_y - 1 + start_line];

        // Move cursor to the beginning of the current word
        while (fs->cursor_x > 1 && isalnum(line[fs->cursor_x - 2])) {
            fs->cursor_x--;
        }

        // Move cursor to the beginning of the previous word
        while (fs->cursor_x > 1 && !isalnum(line[fs->cursor_x - 2])) {
            fs->cursor_x--;
        }

        // If the cursor is within the line, break the loop
        if (fs->cursor_x > 1) {
            return;
        }

        // Move to the end of the previous line
        if (fs->cursor_y > 1) {
            fs->cursor_y--;
            line = text_buffer[fs->cursor_y - 1 + start_line];
            fs->cursor_x = strlen(line) + 1;
        } else {
            fs->cursor_x = 1;
            break;
        }
    }
}


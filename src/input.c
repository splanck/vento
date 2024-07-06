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

void handle_key_up(int *cursor_y, int *start_line) {
    if (*cursor_y > 1) {
        (*cursor_y)--;
    } else if (*start_line > 0) {
        (*start_line)--;
        draw_text_buffer(text_win);
    }
}

void handle_key_down(int *cursor_y, int *start_line) {
    if (*cursor_y < LINES - 4 && *cursor_y < line_count) {
        (*cursor_y)++;
    } else if (*start_line + *cursor_y < line_count) {
        (*start_line)++;
        draw_text_buffer(text_win);
    }
}

void handle_key_left(int *cursor_x) {
    if (*cursor_x > 1)
        (*cursor_x)--;
}

void handle_key_right(int *cursor_x, int cursor_y) {
    if (*cursor_x < (int)strlen(text_buffer[cursor_y - 1 + start_line]) + 1)
        (*cursor_x)++;
}

void handle_key_backspace(int *cursor_x, int *cursor_y, int *start_line) {
    if (*cursor_x > 1) {
        (*cursor_x)--;
        memmove(&text_buffer[*cursor_y - 1 + *start_line][*cursor_x - 1], &text_buffer[*cursor_y - 1 + *start_line][*cursor_x], strlen(text_buffer[*cursor_y - 1 + *start_line]) - *cursor_x + 1);
    } else if (*cursor_y > 1 || *start_line > 0) {
        size_t prev_len = strlen(text_buffer[*cursor_y - 2 + *start_line]);
        if (prev_len + strlen(text_buffer[*cursor_y - 1 + *start_line]) < (size_t)(COLS - 6)) {
            strcat(text_buffer[*cursor_y - 2 + *start_line], text_buffer[*cursor_y - 1 + *start_line]);
            for (int i = *cursor_y - 1 + *start_line; i < line_count - 1; ++i) {
                strcpy(text_buffer[i], text_buffer[i + 1]);
            }
            text_buffer[line_count - 1][0] = '\0';
            line_count--;
            if (*cursor_y > 1) {
                (*cursor_y)--;
            } else {
                (*start_line)--;
                draw_text_buffer(text_win);
            }
            *cursor_x = prev_len + 1;
        }
    }
    werase(text_win);
    box(text_win, 0, 0);
    draw_text_buffer(text_win);
}

void handle_key_delete(int *cursor_x, int cursor_y) {
    if (*cursor_x < (int)strlen(text_buffer[cursor_y - 1 + start_line])) {
        memmove(&text_buffer[cursor_y - 1 + start_line][*cursor_x - 1], &text_buffer[cursor_y - 1 + start_line][*cursor_x], strlen(text_buffer[cursor_y - 1 + start_line]) - *cursor_x + 1);
    } else if (cursor_y + start_line < line_count) {
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

void handle_key_enter(int *cursor_x, int *cursor_y, int *start_line) {
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

void handle_key_page_up(int *cursor_y, int *start_line) {
    int page_size = LINES - 4;
    if (*start_line > 0) {
        *start_line -= page_size;
        if (*start_line < 0) {
            *start_line = 0;
        }
        draw_text_buffer(text_win);
    }
    *cursor_y = 1;
}

void handle_key_page_down(int *cursor_y, int *start_line) {
    int max_lines = LINES - 4; // Adjust for the status bar

    // Move the starting line down
    if (*start_line + max_lines < line_count) {
        *start_line += max_lines;
    }

    // If this would scroll past the end of the file, adjust
    if (*start_line + max_lines > line_count) {
        *start_line = line_count - max_lines;
    }

    // Move the cursor to the bottom of the screen
    *cursor_y = max_lines;

    // Ensure the cursor doesn't go past the end of the file
    if (*cursor_y + *start_line >= line_count) {
        *cursor_y = line_count - *start_line;
    }
}

void handle_ctrl_key_pgup(int *cursor_y, int *start_line) {
    *cursor_y = 1;
    *start_line = 0;
    draw_text_buffer(text_win);
}

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

void handle_ctrl_key_up(int *cursor_y) {
    *cursor_y = 1;
}

void handle_ctrl_key_down(int *cursor_y) {
    *cursor_y = LINES - 4; // Adjust for the status bar
}

void handle_ctrl_key_left(int *cursor_x) {
    *cursor_x = 1;
}

void handle_ctrl_key_right(int *cursor_x, int cursor_y) {
    *cursor_x = strlen(text_buffer[cursor_y - 1 + start_line]) + 1;
}

void handle_default_key(int ch, int *cursor_x, int cursor_y) {
    if (*cursor_x < COLS - 6) {
        int len = strlen(text_buffer[cursor_y - 1 + start_line]);
        char *old_text = strdup(text_buffer[cursor_y - 1 + start_line]);
        
        if (*cursor_x <= len) {
            memmove(&text_buffer[cursor_y - 1 + start_line][*cursor_x], &text_buffer[cursor_y - 1 + start_line][*cursor_x - 1], len - *cursor_x + 1);
        }
        text_buffer[cursor_y - 1 + start_line][*cursor_x - 1] = ch;
        text_buffer[cursor_y - 1 + start_line][len + 1] = '\0';  // Ensure null termination
        (*cursor_x)++;

        char *new_text = strdup(text_buffer[cursor_y - 1 + start_line]);
        Change change = { cursor_y - 1 + start_line, old_text, new_text };
        push(&undo_stack, change);
    }
    werase(text_win);
    box(text_win, 0, 0);
    draw_text_buffer(text_win);
}

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



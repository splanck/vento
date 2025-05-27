#include <ncurses.h>
#include <string.h>
#include <stdlib.h>

#include "editor.h"
#include "ui.h"
#include "search.h"

void find_next_occurrence(const char *word, int *cursor_x, int *cursor_y) {
    int found = 0;
    int lines_per_screen = LINES - 3;  // Lines available in a single screen view
    int middle_line = lines_per_screen / 2; // Calculate middle line position
    int start_search = *cursor_y + start_line;

    // Search from the current cursor position to the end of the document
    for (int line = start_search; line < line_count; ++line) {
        const char *line_text = text_buffer[line];
        const char *found_position = strstr(line == start_search ? line_text + *cursor_x : line_text, word);

        if (found_position != NULL) {
            // Calculate new cursor positions
            *cursor_y = line - start_line + 1;
            *cursor_x = found_position - line_text + 1;

            // Adjust start_line based on document size and found line position
            if (line_count <= lines_per_screen) {
                start_line = 0;  // The entire document fits on one screen
            } else {
                if (line < middle_line) {
                    start_line = 0;  // Avoid scrolling past the top
                } else if (line > line_count - middle_line) {
                    start_line = line_count - lines_per_screen;  // Keep the last line visible
                } else {
                    start_line = line - middle_line;  // Center the found line
                }
            }

            // Update cursor position to the line in the middle of the screen
            *cursor_y = line - start_line + 1;

            found = 1;
            break;
        }
    }

    // If not found, wrap around and search from the start to the initial cursor position
    if (!found) {
        for (int line = 0; line < start_search; ++line) {
            const char *line_text = text_buffer[line];
            const char *found_position = strstr(line_text, word);

            if (found_position != NULL) {
                // Calculate new cursor positions
                *cursor_y = line - start_line + 1;
                *cursor_x = found_position - line_text + 1;

                // Adjust start_line based on document size and found line position
                if (line_count <= lines_per_screen) {
                    start_line = 0;  // The entire document fits on one screen
                } else {
                    if (line < middle_line) {
                        start_line = 0;  // Avoid scrolling past the top
                    } else if (line > line_count - middle_line) {
                        start_line = line_count - lines_per_screen;  // Keep the last line visible
                    } else {
                        start_line = line - middle_line;  // Center the found line
                    }
                }

                // Update cursor position to the line in the middle of the screen
                *cursor_y = line - start_line + 1;

                found = 1;
                break;
            }
        }
    }

    if (!found) {
        mvprintw(LINES - 2, 0, "Word not found.");
    } else {
        mvprintw(LINES - 2, 0, "Found at Line: %d, Column: %d", *cursor_y + start_line + 1, *cursor_x + 1);
    }
    refresh();
    wmove(text_win, *cursor_y, *cursor_x);
    wrefresh(text_win);
}

void find(int new_search)
{
    (void)new_search;
    char *output = malloc(256 * sizeof(char));
    *output = '\0';
    show_find_dialog(output, 20);

    find_next_occurrence(output, &cursor_x, &cursor_y);
}


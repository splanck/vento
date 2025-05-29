#include <ncurses.h>
#include <string.h>
#include <stdlib.h>

#include "editor.h"
#include "ui.h"
#include "search.h"
#include "files.h"

extern char search_text[256];

void find_next_occurrence(FileState *fs, const char *word) {
    int *cursor_x = &fs->cursor_x;
    int *cursor_y = &fs->cursor_y;
    int found = 0;
    int lines_per_screen = LINES - 3;  // Lines available in a single screen view
    int middle_line = lines_per_screen / 2; // Calculate middle line position
    int start_search = *cursor_y + fs->start_line;

    // Search from the current cursor position to the end of the document
    for (int line = start_search; line < fs->line_count; ++line) {
        const char *line_text = fs->text_buffer[line];
        const char *found_position = strstr(line == start_search ? line_text + *cursor_x : line_text, word);

        if (found_position != NULL) {
            // Calculate new cursor positions
            *cursor_y = line - fs->start_line + 1;
            *cursor_x = found_position - line_text + 1;

            // Adjust start_line based on document size and found line position
            if (fs->line_count <= lines_per_screen) {
                fs->start_line = 0;
            } else {
                if (line < middle_line) {
                    fs->start_line = 0;
                } else if (line > fs->line_count - middle_line) {
                    fs->start_line = fs->line_count - lines_per_screen;
                } else {
                    fs->start_line = line - middle_line;
                }
            }

            // Update cursor position to the line in the middle of the screen
            *cursor_y = line - fs->start_line + 1;

            found = 1;
            break;
        }
    }

    // If not found, wrap around and search from the start to the initial cursor position
    if (!found) {
        for (int line = 0; line < start_search; ++line) {
            const char *line_text = fs->text_buffer[line];
            const char *found_position = strstr(line_text, word);

            if (found_position != NULL) {
                // Calculate new cursor positions
                *cursor_y = line - fs->start_line + 1;
                *cursor_x = found_position - line_text + 1;

                // Adjust start_line based on document size and found line position
                if (fs->line_count <= lines_per_screen) {
                    fs->start_line = 0;
                } else {
                    if (line < middle_line) {
                        fs->start_line = 0;
                    } else if (line > fs->line_count - middle_line) {
                        fs->start_line = fs->line_count - lines_per_screen;
                    } else {
                        fs->start_line = line - middle_line;
                    }
                }

                // Update cursor position to the line in the middle of the screen
                *cursor_y = line - fs->start_line + 1;

                found = 1;
                break;
            }
        }
    }

    if (!found) {
        mvprintw(LINES - 2, 0, "Word not found.");
    } else {
        mvprintw(LINES - 2, 0, "Found at Line: %d, Column: %d", *cursor_y + fs->start_line + 1, *cursor_x + 1);
    }
    refresh();
    wmove(text_win, *cursor_y, *cursor_x);
    wrefresh(text_win);
}

void find(FileState *fs, int new_search)
{
    char output[256];

    if (new_search) {
        while (1) {
            show_find_dialog(search_text, output, sizeof(output));
            if (output[0] == '\0')
                break;

            strncpy(search_text, output, sizeof(search_text) - 1);
            search_text[sizeof(search_text) - 1] = '\0';
            find_next_occurrence(fs, search_text);
        }
    } else {
        if (search_text[0] != '\0')
            find_next_occurrence(fs, search_text);
    }
}


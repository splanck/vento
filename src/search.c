#include <ncurses.h>
#include <string.h>
#include <stdlib.h>

#include "editor.h"
#include "ui.h"
#include "search.h"
#include "files.h"
#include "undo.h"
#include "syntax.h"

extern char search_text[256];

/*
 * Scan the text buffer starting from the given line and column for the next
 * occurrence of the specified word. The search wraps around to the beginning of
 * the document if necessary. On success, the found line index is stored in
 * `found_line` and a pointer to the position within the line is returned.
 * Returns NULL if the word is not found.
 */
static char *scan_next(FileState *fs, const char *word, int start_search,
                       int cursor_x, int *found_line) {
    for (int line = start_search; line < fs->line_count; ++line) {
        char *line_text = fs->text_buffer[line];
        char *pos = strstr(line == start_search ? line_text + cursor_x : line_text,
                           word);
        if (pos) {
            *found_line = line;
            return pos;
        }
    }

    for (int line = 0; line < start_search; ++line) {
        char *line_text = fs->text_buffer[line];
        char *pos = strstr(line_text, word);
        if (pos) {
            *found_line = line;
            return pos;
        }
    }

    return NULL;
}

void find_next_occurrence(FileState *fs, const char *word) {
    int *cursor_x = &fs->cursor_x;
    int *cursor_y = &fs->cursor_y;
    int lines_per_screen = LINES - 3;  // Lines available in a single screen view
    int middle_line = lines_per_screen / 2; // Calculate middle line position
    int start_search = *cursor_y + fs->start_line;

    int found_line = -1;
    char *found_position = scan_next(fs, word, start_search, *cursor_x, &found_line);

    if (!found_position) {
        mvprintw(LINES - 2, 0, "Word not found.");
        fs->match_start_y = fs->match_end_y = -1;
        fs->match_start_x = fs->match_end_x = -1;
    } else {
        *cursor_y = found_line - fs->start_line + 1;
        *cursor_x = found_position - fs->text_buffer[found_line] + 1;

        if (fs->line_count <= lines_per_screen) {
            fs->start_line = 0;
        } else {
            if (found_line < middle_line) {
                fs->start_line = 0;
            } else if (found_line > fs->line_count - middle_line) {
                fs->start_line = fs->line_count - lines_per_screen;
            } else {
                fs->start_line = found_line - middle_line;
            }
        }

        *cursor_y = found_line - fs->start_line + 1;

        fs->match_start_y = found_line;
        fs->match_end_y = found_line;
        fs->match_start_x = found_position - fs->text_buffer[found_line];
        fs->match_end_x = fs->match_start_x + strlen(word) - 1;

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
            int confirmed = show_find_dialog(output, sizeof(output),
                                            search_text[0] ? search_text : NULL);
            if (!confirmed)
                break; /* ESC pressed */

            if (output[0] != '\0') {
                strncpy(search_text, output, sizeof(search_text) - 1);
                search_text[sizeof(search_text) - 1] = '\0';
            }

            if (search_text[0] != '\0')
                find_next_occurrence(fs, search_text);
        }
    } else {
        if (search_text[0] != '\0')
            find_next_occurrence(fs, search_text);
    }
}

static void replace_in_line(FileState *fs, int line, char *pos,
                             const char *search, const char *replacement) {
    char *line_text = fs->text_buffer[line];
    char *old_text = strdup(line_text);

    size_t prefix_len = pos - line_text;
    size_t search_len = strlen(search);
    size_t suffix_len = strlen(pos + search_len);
    size_t replacement_len = strlen(replacement);

    char new_line[fs->line_capacity];
    if (prefix_len >= (size_t)fs->line_capacity)
        prefix_len = fs->line_capacity - 1;
    strncpy(new_line, line_text, prefix_len);
    size_t idx = prefix_len;

    if (idx < (size_t)fs->line_capacity - 1) {
        size_t to_copy = replacement_len;
        if (idx + to_copy >= (size_t)fs->line_capacity)
            to_copy = fs->line_capacity - idx - 1;
        memcpy(new_line + idx, replacement, to_copy);
        idx += to_copy;
    }

    if (idx < (size_t)fs->line_capacity - 1) {
        size_t to_copy = suffix_len;
        if (idx + to_copy >= (size_t)fs->line_capacity)
            to_copy = fs->line_capacity - idx - 1;
        memcpy(new_line + idx, pos + search_len, to_copy);
        idx += to_copy;
    }
    new_line[idx] = '\0';

    char *new_text = strdup(new_line);
    Change change = { line, old_text, new_text };
    push(&fs->undo_stack, change);
    strncpy(fs->text_buffer[line], new_line, fs->line_capacity - 1);
    fs->text_buffer[line][fs->line_capacity - 1] = '\0';
    mark_comment_state_dirty(fs);
}

void replace_next_occurrence(FileState *fs, const char *search,
                             const char *replacement) {
    int *cursor_x = &fs->cursor_x;
    int *cursor_y = &fs->cursor_y;
    int lines_per_screen = LINES - 3;
    int middle_line = lines_per_screen / 2;
    int start_search = *cursor_y + fs->start_line;

    int found_line = -1;
    char *found_position = scan_next(fs, search, start_search, *cursor_x, &found_line);

    if (!found_position) {
        mvprintw(LINES - 2, 0, "Word not found.");
        refresh();
        return;
    }

    replace_in_line(fs, found_line, found_position, search, replacement);

    if (fs->line_count <= lines_per_screen) {
        fs->start_line = 0;
    } else {
        if (found_line < middle_line) {
            fs->start_line = 0;
        } else if (found_line > fs->line_count - middle_line) {
            fs->start_line = fs->line_count - lines_per_screen;
        } else {
            fs->start_line = found_line - middle_line;
        }
    }

    *cursor_y = found_line - fs->start_line + 1;
    *cursor_x = (found_position - fs->text_buffer[found_line]) + strlen(replacement) + 1;

    mvprintw(LINES - 2, 0, "Replaced at Line: %d, Column: %d", *cursor_y + fs->start_line + 1, *cursor_x);
    refresh();
    werase(text_win);
    box(text_win, 0, 0);
    draw_text_buffer(active_file, text_win);
    wmove(text_win, *cursor_y, *cursor_x);
    wrefresh(text_win);
}

void replace_all_occurrences(FileState *fs, const char *search,
                             const char *replacement) {
    for (int line = 0; line < fs->line_count; ++line) {
        char *line_text = fs->text_buffer[line];
        char *pos = strstr(line_text, search);
        if (!pos)
            continue;

        char *old_text = strdup(line_text);
        if (!old_text) {
            mvprintw(LINES - 2, 0, "Memory allocation failed");
            refresh();
            continue;
        }

        size_t search_len = strlen(search);
        size_t replacement_len = strlen(replacement);
        size_t buf_size = fs->line_capacity;
        char *new_line = calloc(buf_size, 1);
        if (!new_line) {
            free(old_text);
            mvprintw(LINES - 2, 0, "Memory allocation failed");
            refresh();
            continue;
        }
        size_t idx = 0;
        char *cursor = line_text;
        while (pos) {
            size_t prefix_len = pos - cursor;
            if (idx + prefix_len >= buf_size - 1) {
                prefix_len = buf_size - 1 - idx;
            }
            memcpy(new_line + idx, cursor, prefix_len);
            idx += prefix_len;

            if (idx < buf_size - 1) {
                size_t to_copy = replacement_len;
                if (idx + to_copy >= buf_size - 1)
                    to_copy = buf_size - 1 - idx;
                memcpy(new_line + idx, replacement, to_copy);
                idx += to_copy;
            }

            cursor = pos + search_len;
            pos = strstr(cursor, search);
        }
        size_t tail_len = strlen(cursor);
        if (idx + tail_len >= buf_size - 1)
            tail_len = buf_size - 1 - idx;
        memcpy(new_line + idx, cursor, tail_len);
        idx += tail_len;
        new_line[idx] = '\0';

        char *new_text = strdup(new_line);
        if (!new_text) {
            free(old_text);
            free(new_line);
            mvprintw(LINES - 2, 0, "Memory allocation failed");
            refresh();
            continue;
        }
        push(&fs->undo_stack, (Change){ line, old_text, new_text });
        strncpy(fs->text_buffer[line], new_line, fs->line_capacity - 1);
        fs->text_buffer[line][fs->line_capacity - 1] = '\0';
        free(new_line);
        mark_comment_state_dirty(fs);
    }

    werase(text_win);
    box(text_win, 0, 0);
    draw_text_buffer(active_file, text_win);
    mvprintw(LINES - 2, 0, "All occurrences replaced.");
    refresh();
    wrefresh(text_win);
}

void replace(FileState *fs) {
    char search[256];
    char replacement[256];

    if (!show_replace_dialog(search, sizeof(search), replacement, sizeof(replacement)))
        return;

    const char *options[] = {"Replace Next", "Replace All", "Cancel"};
    int option = 0;
    int ch;
    int opt_count = 3;
    int win_height = opt_count + 2;
    int win_width = 20;
    int win_y = (LINES - win_height) / 2;
    int win_x = (COLS - win_width) / 2;
    WINDOW *win = newwin(win_height, win_width, win_y, win_x);
    keypad(win, TRUE);
    box(win, 0, 0);

    while (1) {
        for (int i = 0; i < opt_count; ++i) {
            if (i == option)
                wattron(win, A_REVERSE);
            mvwprintw(win, i + 1, 2, "%s", options[i]);
            wattroff(win, A_REVERSE);
        }
        wrefresh(win);
        ch = wgetch(win);
        if (ch == KEY_UP) {
            if (option > 0)
                option--;
        } else if (ch == KEY_DOWN) {
            if (option < opt_count - 1)
                option++;
        } else if (ch == '\n') {
            break;
        } else if (ch == 27) {
            option = 2;
            break;
        }
    }

    wclear(win);
    wrefresh(win);
    delwin(win);
    wrefresh(stdscr);

    switch (option) {
        case 0:
            replace_next_occurrence(fs, search, replacement);
            break;
        case 1:
            replace_all_occurrences(fs, search, replacement);
            break;
        default:
            break;
    }
}


#include <ncurses.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <ctype.h>

#include "editor.h"
#include "ui.h"
#include "search.h"
#include "files.h"
#include "undo.h"
#include "syntax.h"
#include "config.h"
#include "editor_state.h"

/*
 * Search and replace implementation.
 *
 * Case sensitivity is controlled by the global option
 * `app_config.search_ignore_case`. When enabled the helper
 * `strcasestr_simple` is used to compare text in a case-insensitive way.
 *
 * All search operations use `scan_next` which scans forward from the current
 * cursor position and wraps to the start of the buffer when the end is
 * reached.  The coordinates of the most recent match are stored in the
 * FileState fields `match_start_x`, `match_end_x`, `match_start_y` and
 * `match_end_y`.  The editor highlights these coordinates after a successful
 * search or replace.  A value of `-1` in these fields indicates that no match
 * is currently highlighted.
 */

extern char search_text[256];

static char *strcasestr_simple(const char *h, const char *n) {
    if (!*n) return (char *)h;
    for (; *h; ++h) {
        const char *hp = h;
        const char *np = n;
        while (*hp && *np && tolower((unsigned char)*hp) == tolower((unsigned char)*np)) {
            ++hp;
            ++np;
        }
        if (*np == '\0')
            return (char *)h;
    }
    return NULL;
}

/**
 * Scan the buffer for the next occurrence of `word`.
 *
 * The scan begins at `start_search`/`cursor_x` and proceeds to the end of the
 * document, wrapping to the beginning if necessary. Case sensitivity follows
 * `app_config.search_ignore_case`.  When a match is found `*found_line` is set
 * to the matching line index and a pointer to the match inside that line is
 * returned.  The function performs no cursor movement or state updates and
 * returns NULL when no match exists.
 */
static char *scan_next(FileState *fs, const char *word, int start_search,
                       int cursor_x, int *found_line) {
    for (int line = start_search; line < fs->buffer.count; ++line) {
        char *line_text = (char *)lb_get(&fs->buffer, line);
        char *pos;
        if (app_config.search_ignore_case)
            pos = strcasestr_simple(line == start_search ? line_text + cursor_x : line_text,
                                    word);
        else
            pos = strstr(line == start_search ? line_text + cursor_x : line_text,
                         word);
        if (pos) {
            *found_line = line;
            return pos;
        }
    }

    for (int line = 0; line < start_search; ++line) {
        char *line_text = (char *)lb_get(&fs->buffer, line);
        char *pos = app_config.search_ignore_case ?
                        strcasestr_simple(line_text, word) : strstr(line_text, word);
        if (pos) {
            *found_line = line;
            return pos;
        }
    }

    return NULL;
}

/**
 * Move the cursor to the next occurrence of `word`.
 *
 * This uses `scan_next` starting from the current cursor position.  When a
 * match is found the cursor and viewport are updated so that the match is
 * visible, and the match coordinates in `fs` are set for highlighting.  If no
 * match exists a status message is printed and the highlight coordinates are
 * cleared.
 */
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
        clrtoeol();
        refresh();
        fs->match_start_y = fs->match_end_y = -1;
        fs->match_start_x = fs->match_end_x = -1;
    } else {
        *cursor_y = found_line - fs->start_line + 1;
        const char *found_line_text = lb_get(&fs->buffer, found_line);
        *cursor_x = found_position - found_line_text + 1;

        if (fs->buffer.count <= lines_per_screen) {
            fs->start_line = 0;
        } else {
            if (found_line < middle_line) {
                fs->start_line = 0;
            } else if (found_line > fs->buffer.count - middle_line) {
                fs->start_line = fs->buffer.count - lines_per_screen;
            } else {
                fs->start_line = found_line - middle_line;
            }
        }

        *cursor_y = found_line - fs->start_line + 1;

        fs->match_start_y = found_line;
        fs->match_end_y = found_line;
        fs->match_start_x = found_position - found_line_text;
        fs->match_end_x = fs->match_start_x + strlen(word) - 1;

        mvprintw(LINES - 2, 0, "Found at Line: %d, Column: %d", *cursor_y + fs->start_line + 1, *cursor_x + 1);
        clrtoeol();
        refresh();
    }
    int off = get_line_number_offset ? get_line_number_offset(fs) : 0;
    wmove(text_win, *cursor_y,
          *cursor_x + off);
    wrefresh(text_win);
}

/**
 * Entry point for interactive searches.
 *
 * When `new_search` is non-zero a dialog prompts the user for a search string
 * and repeated presses of Enter continue searching forward.  When zero, the
 * previously entered `search_text` is reused.  Cursor movement and match
 * highlighting are handled by `find_next_occurrence`.  This function does not
 * modify the undo stack.
 */
void find(EditorContext *ctx, FileState *fs, int new_search)
{
    char output[256];

    if (new_search) {
        while (1) {
            int confirmed = show_find_dialog(ctx, output, sizeof(output),
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
    char *line_text = (char *)lb_get(&fs->buffer, line);
    char *old_text = strdup(line_text);

    size_t prefix_len = pos - line_text;
    size_t search_len = strlen(search);
    size_t suffix_len = strlen(pos + search_len);
    size_t replacement_len = strlen(replacement);

    char *new_line = malloc(fs->line_capacity);
    if (!new_line) {
        free(old_text);
        allocation_failed("replace_in_line malloc");
    }

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
    strncpy(fs->buffer.lines[line], new_line, fs->line_capacity - 1);
    fs->buffer.lines[line][fs->line_capacity - 1] = '\0';
    free(new_line);
    fs->modified = true;
    mark_comment_state_dirty(fs);
}

/**
 * Replace the next occurrence of `search` with `replacement`.
 *
 * Starting from the current cursor position this searches forward using
 * `scan_next`.  When a match is found the text of that line is replaced and the
 * modification is pushed onto the undo stack.  The cursor is moved to the end
 * of the inserted text and the viewport is adjusted to keep the line visible.
 * Search highlight fields are cleared when the operation completes.
 */
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
        clrtoeol();
        refresh();
        return;
    }

    replace_in_line(fs, found_line, found_position, search, replacement);
    fs->modified = true;

    if (fs->buffer.count <= lines_per_screen) {
        fs->start_line = 0;
    } else {
        if (found_line < middle_line) {
            fs->start_line = 0;
        } else if (found_line > fs->buffer.count - middle_line) {
            fs->start_line = fs->buffer.count - lines_per_screen;
        } else {
            fs->start_line = found_line - middle_line;
        }
    }

    *cursor_y = found_line - fs->start_line + 1;
    const char *found_line_text2 = lb_get(&fs->buffer, found_line);
    *cursor_x = (found_position - found_line_text2) + strlen(replacement) + 1;

    mvprintw(LINES - 2, 0, "Replaced at Line: %d, Column: %d", *cursor_y + fs->start_line + 1, *cursor_x);
    clrtoeol();
    refresh();
    werase(text_win);
    box(text_win, 0, 0);
    draw_text_buffer(active_file, text_win);
    int off = get_line_number_offset ? get_line_number_offset(fs) : 0;
    wmove(text_win, *cursor_y,
          *cursor_x + off);
    wrefresh(text_win);

    fs->match_start_x = fs->match_end_x = -1;
    fs->match_start_y = fs->match_end_y = -1;
}

/**
 * Replace every occurrence of `search` in the buffer.
 *
 * Each line is scanned for matches (respecting case sensitivity) and whenever
 * a match is found the text is rewritten.  Every change is pushed onto the undo
 * stack.  When finished the current match highlight is cleared, `fs->modified`
 * is set if replacements occurred and the window is redrawn to reflect the
 * changes.
 */
void replace_all_occurrences(FileState *fs, const char *search,
                             const char *replacement) {
    bool replaced = false;
    for (int line = 0; line < fs->buffer.count; ++line) {
        char *line_text = (char *)lb_get(&fs->buffer, line);
        char *pos = app_config.search_ignore_case
                        ? strcasestr_simple(line_text, search)
                        : strstr(line_text, search);
        if (!pos)
            continue;

        char *old_text = strdup(line_text);
        if (!old_text) {
            mvprintw(LINES - 2, 0, "Memory allocation failed");
            clrtoeol();
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
            clrtoeol();
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
            pos = app_config.search_ignore_case ?
                    strcasestr_simple(cursor, search) :
                    strstr(cursor, search);
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
            clrtoeol();
            refresh();
            continue;
        }
        push(&fs->undo_stack, (Change){ line, old_text, new_text });
        strncpy(fs->buffer.lines[line], new_line, fs->line_capacity - 1);
        fs->buffer.lines[line][fs->line_capacity - 1] = '\0';
        free(new_line);
        mark_comment_state_dirty(fs);
        replaced = true;
    }

    fs->match_start_x = fs->match_end_x = -1;
    fs->match_start_y = fs->match_end_y = -1;

    if (replaced)
        fs->modified = true;

    werase(text_win);
    box(text_win, 0, 0);
    draw_text_buffer(active_file, text_win);
    mvprintw(LINES - 2, 0, "All occurrences replaced.");
    clrtoeol();
    refresh();
    wrefresh(text_win);
}

/**
 * Interactive entry point for text replacement.
 *
 * Displays a dialog prompting for the search and replacement strings and then
 * asks the user whether to replace only the next occurrence or all of them.
 * The actual modifications and cursor movements are performed by
 * `replace_next_occurrence` or `replace_all_occurrences` depending on the
 * chosen option.
 */
void replace(EditorContext *ctx, FileState *fs) {
    char search[256];
    char replacement[256];

    if (!show_replace_dialog(ctx, search, sizeof(search), replacement,
                             sizeof(replacement)))
        return;

    const char *options[] = {"Replace Next", "Replace All", "Cancel"};
    int option = 0;
    int ch;
    int opt_count = 3;
    int win_height = opt_count + 2;
    int win_width = 20;
    if (win_width > COLS - 2)
        win_width = COLS - 2;
    if (win_width < 2)
        win_width = 2;
    int win_y = (LINES - win_height) / 2;
    int win_x = (COLS - win_width) / 2;
    if (win_x < 0)
        win_x = 0;
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


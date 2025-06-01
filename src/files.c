#include <stdlib.h>
#include <string.h>
#include "files.h"
#include "editor.h"
#include "syntax.h"
#include "config.h"
#include "editor_state.h"
#include "line_buffer.h"
#include <limits.h>

// Function to initialize a new FileState for a given filename
FileState *initialize_file_state(const char *filename, int max_lines, int max_cols) {
    FileState *file_state = malloc(sizeof(FileState));
    if (!file_state) {
        return NULL;
    }

    strncpy(file_state->filename, filename, sizeof(file_state->filename) - 1);
    file_state->filename[sizeof(file_state->filename) - 1] = '\0';

    // Initialize text buffer
    lb_init(&file_state->buffer, max_lines);
    if (!file_state->buffer.lines) {
        free(file_state);
        return NULL;
    }
    for (int i = 0; i < max_lines; i++) {
        file_state->buffer.lines[i] = calloc(max_cols, sizeof(char));
        if (!file_state->buffer.lines[i]) {
            for (int j = 0; j < i; j++)
                free(file_state->buffer.lines[j]);
            lb_free(&file_state->buffer);
            free(file_state);
            return NULL;
        }
    }

    file_state->buffer.count = 1; // Start with a single empty line ready for editing
    file_state->buffer.capacity = max_lines;
    file_state->line_capacity = max_cols;
    file_state->start_line = 0;
    file_state->scroll_x = 0;
    file_state->cursor_x = 1;
    file_state->cursor_y = 1;
    file_state->saved_cursor_x = 1;
    file_state->saved_cursor_y = 1;
    file_state->undo_stack = NULL; // Initialize your undo stack
    file_state->redo_stack = NULL; // Initialize your redo stack
    file_state->selection_mode = false;
    file_state->sel_start_x = file_state->sel_start_y = 0;
    file_state->sel_end_x = file_state->sel_end_y = 0;
    file_state->match_start_x = file_state->match_start_y = -1;
    file_state->match_end_x = file_state->match_end_y = -1;
    file_state->syntax_mode = NO_SYNTAX; // Set to NO_SYNTAX initially
    file_state->in_multiline_comment = false;
    file_state->in_multiline_string = false;
    file_state->string_delim = '\0';
    file_state->nested_mode = 0;
    file_state->last_scanned_line = 0;
    file_state->last_comment_state = false;
    file_state->text_win = newwin(LINES - 2, COLS, 1, 0); // Create a new window for the file
    if (!file_state->text_win) {
        lb_free(&file_state->buffer);
        free(file_state);
        return NULL;
    }
    wtimeout(file_state->text_win, 10);
    wbkgd(file_state->text_win, enable_color ? COLOR_PAIR(SYNTAX_BG) : A_NORMAL);

    file_state->fp = NULL;
    file_state->file_complete = true;
    file_state->modified = false;

    return file_state;
}

// Function to free allocated resources in FileState
void free_file_state(FileState *file_state) {
    lb_free(&file_state->buffer);
    if (file_state->fp) {
        fclose(file_state->fp);
        file_state->fp = NULL;
    }
    file_state->last_scanned_line = 0;
    file_state->last_comment_state = false;
    delwin(file_state->text_win);
    free(file_state);
}

int ensure_line_capacity(FileState *fs, int min_needed) {
    if (min_needed < fs->buffer.capacity)
        return 0;

    int new_max = fs->buffer.capacity * 2;
    if (new_max <= min_needed)
        new_max = min_needed + 1;

    char **new_buffer = realloc(fs->buffer.lines, new_max * sizeof(char *));
    if (!new_buffer)
        return -1;
    fs->buffer.lines = new_buffer;

    for (int i = fs->buffer.capacity; i < new_max; ++i) {
        fs->buffer.lines[i] = calloc(fs->line_capacity, sizeof(char));
        if (!fs->buffer.lines[i]) {
            for (int j = fs->buffer.capacity; j < i; ++j)
                free(fs->buffer.lines[j]);
            fs->buffer.lines = realloc(fs->buffer.lines, fs->buffer.capacity * sizeof(char *));
            return -1;
        }
    }

    fs->buffer.capacity = new_max;
    return 0;
}

int ensure_col_capacity(FileState *fs, int cols) {
    if (cols <= fs->line_capacity)
        return 0;

    int old_capacity = fs->line_capacity;
    for (int i = 0; i < fs->buffer.capacity; ++i) {
        char *tmp = realloc(fs->buffer.lines[i], cols);
        if (!tmp) {
            for (int j = 0; j < i; ++j) {
                char *restore = realloc(fs->buffer.lines[j], old_capacity);
                if (restore)
                    fs->buffer.lines[j] = restore;
            }
            return -1;
        }
        fs->buffer.lines[i] = tmp;
        memset(fs->buffer.lines[i] + old_capacity, 0, cols - old_capacity);
    }

    fs->line_capacity = cols;
    return 0;
}

static int read_line_into(FileState *fs, const char *line) {
    if (ensure_line_capacity(fs, fs->buffer.count + 1) < 0)
        return -1;

    size_t len = strlen(line);
    if (len > 0 && line[len - 1] == '\n')
        len--;

    if (ensure_col_capacity(fs, (int)len + 1) < 0)
        return -1;

    char tmp[PATH_MAX];
    if (len >= sizeof(tmp))
        len = sizeof(tmp) - 1;
    memcpy(tmp, line, len);
    tmp[len] = '\0';

    int idx = fs->buffer.count;
    if (lb_insert(&fs->buffer, idx, tmp) < 0)
        return -1;

    if ((size_t)fs->line_capacity > len + 1) {
        char *p = realloc(fs->buffer.lines[idx], fs->line_capacity);
        if (!p)
            return -1;
        fs->buffer.lines[idx] = p;
    }

    return 0;
}

int load_next_lines(FileState *fs, int count) {
    if (!fs->fp)
        return 0;

    char *line = NULL;
    size_t len = 0;
    int loaded = 0;
    ssize_t nread;
    while (loaded < count && (nread = getline(&line, &len, fs->fp)) != -1) {
        (void)nread;
        if (read_line_into(fs, line) < 0) {
            free(line);
            return -1;
        }
        loaded++;
    }
    free(line);
    if (feof(fs->fp)) {
        fclose(fs->fp);
        fs->fp = NULL;
        fs->file_complete = true;
    } else {
        fs->file_complete = false;
    }
    return loaded;
}

void ensure_line_loaded(FileState *fs, int idx) {
    if (idx < fs->buffer.count)
        return;
    int to_load = idx - fs->buffer.count + 1;
    if (to_load < 0)
        to_load = 0;
    load_next_lines(fs, to_load);
}

void load_all_remaining_lines(FileState *fs) {
    while (!fs->file_complete && fs->fp) {
        load_next_lines(fs, INT_MAX);
    }
}

// Function to load entire file content (used in tests)
int load_file_into_buffer(FileState *file_state) {
    file_state->fp = fopen(file_state->filename, "r");
    if (!file_state->fp)
        return -1;
    file_state->file_complete = false;
    file_state->buffer.count = 0;
    int res = load_next_lines(file_state, INT_MAX);
    file_state->last_scanned_line = 0;
    file_state->last_comment_state = false;
    file_state->in_multiline_comment = false;
    file_state->in_multiline_string = false;
    file_state->string_delim = '\0';
    file_state->nested_mode = 0;
    return (res >= 0) ? 0 : -1;
}

#include <stdlib.h>
#include <string.h>
#include "files.h"
#include "editor.h"
#include "syntax.h"
#include "clipboard.h"
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
    file_state->text_buffer = malloc(max_lines * sizeof(char *));
    if (!file_state->text_buffer) {
        free(file_state);
        return NULL;
    }
    for (int i = 0; i < max_lines; i++) {
        file_state->text_buffer[i] = calloc(max_cols, sizeof(char));
        if (!file_state->text_buffer[i]) {
            for (int j = 0; j < i; j++) {
                free(file_state->text_buffer[j]);
            }
            free(file_state->text_buffer);
            free(file_state);
            return NULL;
        }
    }

    file_state->line_count = 1; // Start with a single empty line ready for editing
    file_state->max_lines = max_lines;
    file_state->line_capacity = max_cols;
    file_state->start_line = 0;
    file_state->cursor_x = 1;
    file_state->cursor_y = 1;
    file_state->undo_stack = NULL; // Initialize your undo stack
    file_state->redo_stack = NULL; // Initialize your redo stack
    file_state->selection_mode = false;
    file_state->sel_start_x = file_state->sel_start_y = 0;
    file_state->sel_end_x = file_state->sel_end_y = 0;
    file_state->clipboard = malloc(CLIPBOARD_SIZE);
    if (!file_state->clipboard) {
        for (int j = 0; j < max_lines; j++) {
            free(file_state->text_buffer[j]);
        }
        free(file_state->text_buffer);
        free(file_state);
        return NULL;
    }
    file_state->clipboard[0] = '\0';
    file_state->syntax_mode = NO_SYNTAX; // Set to NO_SYNTAX initially
    file_state->in_multiline_comment = false;
    file_state->in_multiline_string = false;
    file_state->string_delim = '\0';
    file_state->nested_mode = 0;
    file_state->last_scanned_line = 0;
    file_state->last_comment_state = false;
    file_state->text_win = newwin(LINES - 2, COLS, 1, 0); // Create a new window for the file
    if (!file_state->text_win) {
        for (int j = 0; j < max_lines; j++) {
            free(file_state->text_buffer[j]);
        }
        free(file_state->text_buffer);
        free(file_state->clipboard);
        free(file_state);
        return NULL;
    }
    wbkgd(file_state->text_win, COLOR_PAIR(SYNTAX_BG));

    file_state->fp = NULL;
    file_state->file_complete = true;

    return file_state;
}

// Function to free allocated resources in FileState
void free_file_state(FileState *file_state, int max_lines) {
    (void)max_lines;
    for (int i = 0; i < file_state->max_lines; i++) {
        free(file_state->text_buffer[i]);
    }
    free(file_state->text_buffer);
    if (file_state->clipboard) {
        free(file_state->clipboard);
    }
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
    if (min_needed < fs->max_lines)
        return 0;

    int new_max = fs->max_lines * 2;
    if (new_max <= min_needed)
        new_max = min_needed + 1;

    char **new_buffer = realloc(fs->text_buffer, new_max * sizeof(char *));
    if (!new_buffer)
        return -1;
    fs->text_buffer = new_buffer;

    for (int i = fs->max_lines; i < new_max; ++i) {
        fs->text_buffer[i] = calloc(fs->line_capacity, sizeof(char));
        if (!fs->text_buffer[i]) {
            for (int j = fs->max_lines; j < i; ++j)
                free(fs->text_buffer[j]);
            fs->text_buffer = realloc(fs->text_buffer, fs->max_lines * sizeof(char *));
            return -1;
        }
    }

    fs->max_lines = new_max;
    return 0;
}

int ensure_col_capacity(FileState *fs, int cols) {
    if (cols <= fs->line_capacity)
        return 0;

    int old_capacity = fs->line_capacity;
    for (int i = 0; i < fs->max_lines; ++i) {
        char *tmp = realloc(fs->text_buffer[i], cols);
        if (!tmp) {
            for (int j = 0; j < i; ++j) {
                char *restore = realloc(fs->text_buffer[j], old_capacity);
                if (restore)
                    fs->text_buffer[j] = restore;
            }
            return -1;
        }
        fs->text_buffer[i] = tmp;
        memset(fs->text_buffer[i] + old_capacity, 0, cols - old_capacity);
    }

    fs->line_capacity = cols;
    return 0;
}

static int read_line_into(FileState *fs, const char *line) {
    if (ensure_line_capacity(fs, fs->line_count + 1) < 0)
        return -1;
    size_t len = strlen(line);
    if (len > 0) {
        size_t copy_len = len - 1;
        if (copy_len > (size_t)(fs->line_capacity - 1))
            copy_len = fs->line_capacity - 1;
        strncpy(fs->text_buffer[fs->line_count], line, copy_len);
        fs->text_buffer[fs->line_count][copy_len] = '\0';
    } else {
        fs->text_buffer[fs->line_count][0] = '\0';
    }
    fs->line_count++;
    return 0;
}

int load_next_lines(FileState *fs, int count) {
    if (!fs->fp)
        return 0;

    char line[1024];
    int loaded = 0;
    while (loaded < count && fgets(line, sizeof(line), fs->fp)) {
        if (read_line_into(fs, line) < 0)
            return -1;
        loaded++;
    }
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
    if (idx < fs->line_count)
        return;
    int to_load = idx - fs->line_count + 1;
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
    file_state->line_count = 0;
    int res = load_next_lines(file_state, INT_MAX);
    file_state->last_scanned_line = 0;
    file_state->last_comment_state = false;
    file_state->in_multiline_comment = false;
    file_state->in_multiline_string = false;
    file_state->string_delim = '\0';
    file_state->nested_mode = 0;
    return (res >= 0) ? 0 : -1;
}

/*
 * files.c
 * -------
 * FileState management and file loading helpers.
 *
 * FileState represents a single open document. It owns all strings in
 * its LineBuffer and releases them in free_file_state(). Large files are
 * read lazily: a FILE handle is opened on demand, additional lines are
 * loaded with load_next_lines(), and the handle is closed once the end is
 * reached.
 */
#include <stdlib.h>
#include <string.h>
#include "files.h"
#include "editor.h"
#include "syntax.h"
#include "config.h"
#include "editor_state.h"
#include "line_buffer.h"
#include "undo.h"
#include <limits.h>
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif
#include <stddef.h>
char *realpath(const char *path, char *resolved_path);
/**
 * canonicalize_path - resolve PATH to an absolute form.
 * @path: input file path, may be NULL or empty.
 * @out: buffer receiving the canonical path.
 * @out_size: size of OUT in bytes.
 *
 * If realpath succeeds the resolved path is copied to OUT, otherwise the
 * original path or an empty string is used. The output is always null
 * terminated.
 *
 * Returns: none.
 * Side effects: writes to OUT only.
 */

void canonicalize_path(const char *path, char *out, size_t out_size) {
    char resolved[PATH_MAX];
    if (path && path[0] != '\0' && realpath(path, resolved)) {
        strncpy(out, resolved, out_size - 1);
    } else {
        if (!path)
            path = "";
        strncpy(out, path, out_size - 1);
    }
    out[out_size - 1] = '\0';
}
/**
 * initialize_file_state - allocate and setup a new FileState.
 * @filename: path to load, may be NULL for an empty buffer.
 * @max_lines: initial line capacity.
 * @max_cols: initial column capacity for each line.
 *
 * Allocates all memory for the text buffer and creates an ncurses window.
 * The FileState takes ownership of the allocated lines which are freed
 * by free_file_state(). The file is not opened here; fp is set to NULL.
 *
 * Returns: a pointer to the new FileState or NULL on allocation failure.
 * Side effects: allocates memory and creates an ncurses window.
 */

FileState *initialize_file_state(const char *filename, int max_lines, int max_cols) {
    FileState *file_state = malloc(sizeof(FileState));
    if (!file_state) {
        return NULL;
    }

    canonicalize_path(filename ? filename : "", file_state->filename,
                     sizeof(file_state->filename));

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
    keypad(file_state->text_win, TRUE);
    meta(file_state->text_win, TRUE);
    wtimeout(file_state->text_win, 10);
    wbkgd(file_state->text_win, enable_color ? COLOR_PAIR(SYNTAX_BG) : A_NORMAL);

    file_state->fp = NULL;
    file_state->file_pos = 0;
    file_state->file_complete = true;
    file_state->modified = false;

    return file_state;
}
/**
 * free_file_state - release all resources owned by a FileState.
 * @file_state: FileState to destroy.
 *
 * All line strings in the buffer are freed and the ncurses window is
 * destroyed. Any open FILE handle is closed and undo/redo stacks are
 * disposed of.
 *
 * Returns: none.
 * Side effects: deallocates memory and closes the associated FILE.
 */

void free_file_state(FileState *file_state) {
    lb_free(&file_state->buffer);
    if (file_state->fp) {
        fclose(file_state->fp);
        file_state->fp = NULL;
    }
    if (file_state->undo_stack) {
        free_stack(file_state->undo_stack);
        file_state->undo_stack = NULL;
    }
    if (file_state->redo_stack) {
        free_stack(file_state->redo_stack);
        file_state->redo_stack = NULL;
    }
    file_state->last_scanned_line = 0;
    file_state->last_comment_state = false;
    delwin(file_state->text_win);
    free(file_state);
}
/**
 * ensure_line_capacity - expand the buffer to hold at least MIN_NEEDED lines.
 * @fs: file whose buffer will grow.
 * @min_needed: minimum number of lines required.
 *
 * Allocates additional line slots and associated memory when necessary.
 * Newly created lines are zero-initialised and owned by @fs.
 *
 * Returns: 0 on success or -1 on allocation failure.
 * Side effects: reallocates fs->buffer.lines and may allocate new line strings.
 */

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
/**
 * ensure_col_capacity - grow lines to hold at least COLS characters.
 * @fs: file whose lines may be reallocated.
 * @cols: desired minimum column capacity.
 *
 * Each line buffer is resized using realloc. On failure previously
 * resized lines are restored.
 *
 * Returns: 0 on success or -1 on allocation failure.
 * Side effects: reallocates memory for each line in the buffer.
 */

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
            if (fs->fp) {
                fclose(fs->fp);
                fs->fp = NULL;
            }
            fs->file_complete = false;
            return -1;
        }
        loaded++;
        if (fs->fp)
            fs->file_pos = ftell(fs->fp);
    }
    if (fs->fp)
        fs->file_pos = ftell(fs->fp);
    free(line);
    if (fs->fp && feof(fs->fp)) {
        fclose(fs->fp);
        fs->fp = NULL;
        fs->file_complete = true;
    } else if (fs->fp) {
        fs->file_complete = false;
    }
    return loaded;
}
/**
 * ensure_line_loaded - guarantee that line IDX is available in memory.
 * @fs: FileState to load from.
 * @idx: 0-based index of the requested line.
 *
 * Opens the file on demand and reads additional lines as needed using
 * load_next_lines().
 *
 * Returns: none.
 * Side effects: may open fs->fp, read from disk and update file_pos.
 */

void ensure_line_loaded(FileState *fs, int idx) {
    if (idx < fs->buffer.count)
        return;
    int to_load = idx - fs->buffer.count + 1;
    if (to_load < 0)
        to_load = 0;
    if (!fs->fp && !fs->file_complete) {
        fs->fp = fopen(fs->filename, "r");
        if (fs->fp)
            fseek(fs->fp, fs->file_pos, SEEK_SET);
    }
    load_next_lines(fs, to_load);
}
/**
 * load_all_remaining_lines - read the rest of the file into memory.
 * @fs: FileState whose file should be fully loaded.
 *
 * Repeatedly calls load_next_lines() until the end of the file is reached.
 * The file handle is closed when reading completes.
 *
 * Returns: none.
 * Side effects: reads from disk and may close fs->fp.
 */

void load_all_remaining_lines(FileState *fs) {
    while (!fs->file_complete && fs->fp) {
        load_next_lines(fs, INT_MAX);
    }
}

/**
 * load_file_into_buffer - read an entire file into the buffer.
 * @file_state: FileState whose filename is used.
 *
 * Opens the file, loads all lines and resets syntax state. The file
 * handle is closed when finished and file_complete is set accordingly.
 *
 * Returns: 0 on success or -1 on failure to open or read the file.
 * Side effects: replaces existing buffer contents and modifies fp.
 */
int load_file_into_buffer(FileState *file_state) {
    file_state->fp = fopen(file_state->filename, "r");
    if (!file_state->fp)
        return -1;
    file_state->file_pos = 0;
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

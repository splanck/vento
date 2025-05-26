#include <stdlib.h>
#include "files.h"
#include "editor.h"
#include "syntax.h"

// Function to initialize a new FileState for a given filename
FileState *initialize_file_state(const char *filename, int max_lines, int max_cols) {
    FileState *file_state = malloc(sizeof(FileState));
    strncpy(file_state->filename, filename, sizeof(file_state->filename) - 1);

    // Initialize text buffer
    file_state->text_buffer = malloc(max_lines * sizeof(char *));
    for (int i = 0; i < max_lines; i++) {
        file_state->text_buffer[i] = calloc(max_cols, sizeof(char));
    }

    file_state->line_count = 0;
    file_state->start_line = 0;
    file_state->cursor_x = 1;
    file_state->cursor_y = 1;
    file_state->undo_stack = NULL; // Initialize your undo stack
    file_state->redo_stack = NULL; // Initialize your redo stack
    file_state->selection_mode = false;
    file_state->sel_start_x = file_state->sel_start_y = 0;
    file_state->sel_end_x = file_state->sel_end_y = 0;
    file_state->clipboard = NULL; // Initialize clipboard if needed
    file_state->syntax_mode = NO_SYNTAX; // Set to NO_SYNTAX initially
    file_state->text_win = newwin(LINES - 2, COLS, 1, 0); // Create a new window for the file

    return file_state;
}

// Function to free allocated resources in FileState
void free_file_state(FileState *file_state, int max_lines) {
    for (int i = 0; i < max_lines; i++) {
        free(file_state->text_buffer[i]);
    }
    free(file_state->text_buffer);
    if (file_state->clipboard) {
        free(file_state->clipboard);
    }
    delwin(file_state->text_win);
    free(file_state);
}

// Function to load file content into the text buffer
int load_file_into_buffer(FileState *file_state) {
    FILE *fp = fopen(file_state->filename, "r");
    if (!fp) {
        return -1; // Return error if file cannot be opened
    }

    char line[1024];
    while (fgets(line, sizeof(line), fp)) {
        // Copy line to text buffer
        strncpy(file_state->text_buffer[file_state->line_count], line, strlen(line) - 1);
        file_state->line_count++;
    }
    fclose(fp);
    return 0; // Success
}

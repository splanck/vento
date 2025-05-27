#include <ncurses.h>
#include <stdlib.h>
#include <signal.h>
#include "editor.h"
#include "file_ops.h"
#include "undo.h"
#include "input.h"
#include "ui.h"
#include "files.h"


/**
 * The main function of the program.
 * 
 * @param argc The number of command-line arguments.
 * @param argv An array of strings containing the command-line arguments.
 * @return 0 on successful execution.
 */
int main(int argc, char *argv[]) {
    initialize();

    active_file = initialize_file_state("", MAX_LINES, COLS - 3);
    initialize_buffer();

    // Check if a filename was provided as a parameter
    if (argc > 1) {
        load_file(active_file, argv[1]);
    } else {
        new_file(active_file);
    }

    // Show the warning dialog before entering the main editor loop
    show_warning_dialog();

    // Finishes initializing editor window and begin accepting keyboard input.
    run_editor();
    
    endwin();

    // Free the undo stack
    free_stack(undo_stack);
    undo_stack = NULL; // Prevent double free

    // Free the redo stack
    free_stack(redo_stack);
    redo_stack = NULL; // Prevent double free

    cleanup_on_exit();
    return 0;
}

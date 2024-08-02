#include <ncurses.h>
#include <stdlib.h>
#include <signal.h>
#include "editor.h"
#include "input.h"
#include "ui.h"

/**
 * Frees the memory allocated for the undo stack.
 * 
 * @param stack The undo stack to be freed.
 */
void free_stack(Node *stack) {
    while (stack) {
        Node *next = stack->next;
        
        // Free the old_text if it exists
        if (stack->change.old_text) {
            free(stack->change.old_text);
            stack->change.old_text = NULL; // Prevent double free
        }
        
        // Free the new_text if it exists
        if (stack->change.new_text) {
            free(stack->change.new_text);
            stack->change.new_text = NULL; // Prevent double free
        }
        
        // Free the stack node
        free(stack);
        stack = next;
    }
}

/**
 * The main function of the program.
 * 
 * @param argc The number of command-line arguments.
 * @param argv An array of strings containing the command-line arguments.
 * @return 0 on successful execution.
 */
int main(int argc, char *argv[]) {
    initialize();

    initialize_buffer();

    // Check if a filename was provided as a parameter
    if (argc > 1) {
        load_file(argv[1]);
    } else {
        new_file();
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

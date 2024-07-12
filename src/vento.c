#include <ncurses.h>
#include <stdlib.h>
#include <signal.h>
#include "editor.h"
#include "input.h"

void free_stack(Node *stack) {
    while (stack) {
        Node *next = stack->next;
        if (stack->change.old_text) {
            //printf("Freeing old_text at address: %p\n", (void*)stack->change.old_text);
            free(stack->change.old_text);
            stack->change.old_text = NULL; // Prevent double free
        }
        if (stack->change.new_text) {
            //printf("Freeing new_text at address: %p\n", (void*)stack->change.new_text);
            free(stack->change.new_text);
            stack->change.new_text = NULL; // Prevent double free
        }
        printf("Freeing stack node at address: %p\n", (void*)stack);
        free(stack);
        stack = next;
    }
}

int main(int argc, char *argv[]) {
    initialize();

    initialize_buffer();

    // Check if a filename was provided as a parameter
    if (argc > 1) {
        load_file(argv[1]);
    } else {
        new_file();
    }

    run_editor();

    endwin();

    //printf("Freeing undo stack\n");
    free_stack(undo_stack);
    undo_stack = NULL; // Prevent double free
    //printf("Freeing redo stack\n");
    free_stack(redo_stack);
    redo_stack = NULL; // Prevent double free

    cleanup_on_exit();
    return 0;
}

#include <ncurses.h>
#include <stdlib.h>
#include <signal.h>
#include "editor.h"
#include "input.h"

void free_stack(Node *stack) {
    while (stack) {
        Node *next = stack->next;
        free(stack->change.old_text);
        free(stack->change.new_text);
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

    free_stack(undo_stack);
    free_stack(redo_stack);

    endwin();
    return 0;
}

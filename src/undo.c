#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include "editor.h"
#include "undo.h"
#include "files.h"
#include "editor_state.h"
#include "line_buffer.h"

/**
 * Insert a new change at the top of a stack.
 *
 * The stack parameter points to either a FileState undo or redo list. This
 * function allocates a new Node, stores the Change and makes it the new head.
 * Ownership of any text in the Change is transferred to the created node.
 */
void push(Node **stack, Change change) {
    Node *new_node = (Node *)malloc(sizeof(Node));
    if (new_node == NULL) {
        allocation_failed("push malloc failed");
        return;
    }
    new_node->change = change;
    new_node->next = *stack;
    *stack = new_node;
}

/**
 * Remove and return the most recent change from a stack.
 *
 * If the stack is empty an empty Change with NULL pointers is returned. The
 * caller becomes responsible for freeing the returned Change's text fields.
 */
Change pop(Node **stack) {
    if (*stack == NULL) {
        Change empty_change = {0, NULL, NULL};
        return empty_change;
    }
    Node *top = *stack;
    Change change = top->change;
    *stack = top->next;
    free(top);
    return change;
}

/** Check whether a stack has no elements. */
int is_empty(Node *stack) {
    return stack == NULL;
}

/**
 * Undo the most recent action on `fs`.
 *
 * The change popped from `fs->undo_stack` is reversed in the file buffer and a
 * corresponding entry is pushed onto `fs->redo_stack`. If the undo stack is
 * empty the function returns immediately.
 */
void undo(FileState *fs) {
    if (fs->undo_stack == NULL)
        return;

    Change change = pop(&fs->undo_stack);

    if (change.old_text && !change.new_text) { /* Deletion */
        if (lb_insert(&fs->buffer, change.line, change.old_text) < 0)
            allocation_failed("lb_insert failed");
        char *p = realloc(fs->buffer.lines[change.line], fs->line_capacity);
        if (!p)
            allocation_failed("realloc failed");
        fs->buffer.lines[change.line] = p;
        fs->buffer.lines[change.line][fs->line_capacity - 1] = '\0';

        push(&fs->redo_stack, (Change){ change.line, strdup(change.old_text), NULL });
        free(change.old_text);
    } else if (!change.old_text && change.new_text) { /* Insertion */
        lb_delete(&fs->buffer, change.line);

        push(&fs->redo_stack, (Change){ change.line, NULL, strdup(change.new_text) });
        free(change.new_text);
    } else if (change.old_text && change.new_text) { /* Edit */
        char *line = (char *)lb_get(&fs->buffer, change.line);
        if (line) {
            strncpy(line, change.old_text, fs->line_capacity - 1);
            line[fs->line_capacity - 1] = '\0';
        }

        push(&fs->redo_stack,
             (Change){ change.line, strdup(change.old_text), strdup(change.new_text) });
        free(change.old_text);
        free(change.new_text);
    }

    werase(text_win);
    box(text_win, 0, 0);
    draw_text_buffer(active_file, text_win);
    wrefresh(text_win);
    fs->modified = true;
}

/**
 * Redo the last undone action on `fs`.
 *
 * The change popped from `fs->redo_stack` is applied to the file buffer and a
 * corresponding entry is pushed back onto `fs->undo_stack`. If the redo stack
 * is empty the function simply returns.
 */
void redo(FileState *fs) {
    if (fs->redo_stack == NULL)
        return;

    Change change = pop(&fs->redo_stack);

    if (change.old_text && !change.new_text) { /* Deletion */
        push(&fs->undo_stack, (Change){ change.line, strdup(change.old_text), NULL });
        lb_delete(&fs->buffer, change.line);
        free(change.old_text);
    } else if (!change.old_text && change.new_text) { /* Insertion */
        if (lb_insert(&fs->buffer, change.line, change.new_text) < 0)
            allocation_failed("lb_insert failed");
        char *p = realloc(fs->buffer.lines[change.line], fs->line_capacity);
        if (!p)
            allocation_failed("realloc failed");
        fs->buffer.lines[change.line] = p;
        fs->buffer.lines[change.line][fs->line_capacity - 1] = '\0';
        push(&fs->undo_stack, (Change){ change.line, NULL, strdup(change.new_text) });
        free(change.new_text);
    } else if (change.old_text && change.new_text) { /* Edit */
        push(&fs->undo_stack,
             (Change){ change.line, strdup(change.old_text), strdup(change.new_text) });
        char *line = (char *)lb_get(&fs->buffer, change.line);
        if (line) {
            strncpy(line, change.new_text, fs->line_capacity - 1);
            line[fs->line_capacity - 1] = '\0';
        }
        free(change.old_text);
        free(change.new_text);
    }

    werase(text_win);
    box(text_win, 0, 0);
    draw_text_buffer(active_file, text_win);
    fs->modified = true;
}

/**
 * Free an entire change stack.
 *
 * All nodes are removed and any text stored within them is released.
 */
void free_stack(Node *stack) {
    while (stack) {
        Node *next = stack->next;
        if (stack->change.old_text) {
            free(stack->change.old_text);
            stack->change.old_text = NULL;
        }
        if (stack->change.new_text) {
            free(stack->change.new_text);
            stack->change.new_text = NULL;
        }
        free(stack);
        stack = next;
    }
}


#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include "editor.h"
#include "undo.h"
#include "files.h"
#include "editor_state.h"
#include "line_buffer.h"

/*
 * Undo/Redo Data Structures
 * -------------------------
 * The editor maintains two singly linked stacks per file: one for undo and one
 * for redo.  Each stack node stores a `Change` describing how a single line was
 * modified.  A `Change` contains dynamically allocated strings for the text
 * before and after the modification.  When a node is pushed onto a stack it
 * assumes ownership of those strings and releases them when popped or when the
 * stack is destroyed.  The head pointers of these stacks may be NULL when no
 * history exists.
 */

/**
 * Insert a new change at the top of a stack.
 *
 * The stack parameter points to either a FileState undo or redo list.  The
 * function allocates a new Node, stores the Change and makes it the new head.
 * Only the pointers inside the Change are stored; the Node becomes responsible
 * for freeing that text when removed.
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
 * If the stack is empty an empty Change with NULL pointers is returned.  The
 * caller becomes the owner of any text in the returned Change and must free it
 * after applying the change.
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

/**
 * Check whether a stack has no elements.
 *
 * It is safe for the head pointer of either stack to be NULL, which this
 * function interprets as empty history.
 */
int is_empty(Node *stack) {
    return stack == NULL;
}

/**
 * Undo the most recent action on `fs`.
 *
 * A change is popped from `fs->undo_stack` and applied in reverse to
 * `fs->buffer`.  The inverse of that change is pushed onto `fs->redo_stack` so
 * it can be redone later.  If there is no undo history the function simply
 * returns.  A screen redraw is triggered and `fs->modified` is set whenever a
 * change is undone.
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

        char *dup = strdup(change.old_text);
        if (!dup) {
            allocation_failed("strdup failed");
        } else {
            push(&fs->redo_stack, (Change){ change.line, dup, NULL });
        }
        free(change.old_text);
    } else if (!change.old_text && change.new_text) { /* Insertion */
        lb_delete(&fs->buffer, change.line);

        char *dup = strdup(change.new_text);
        if (!dup) {
            allocation_failed("strdup failed");
        } else {
            push(&fs->redo_stack, (Change){ change.line, NULL, dup });
        }
        free(change.new_text);
    } else if (change.old_text && change.new_text) { /* Edit */
        char *line = (char *)lb_get(&fs->buffer, change.line);
        if (line) {
            strncpy(line, change.old_text, fs->line_capacity - 1);
            line[fs->line_capacity - 1] = '\0';
        }

        char *dup_old = strdup(change.old_text);
        char *dup_new = strdup(change.new_text);
        if (!dup_old || !dup_new) {
            if (dup_old)
                free(dup_old);
            if (dup_new)
                free(dup_new);
            allocation_failed("strdup failed");
        } else {
            push(&fs->redo_stack,
                 (Change){ change.line, dup_old, dup_new });
        }
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
 * A change is popped from `fs->redo_stack` and re-applied to `fs->buffer`.  The
 * inverse operation is then pushed back onto `fs->undo_stack`.  If there is no
 * redo history the function returns immediately.  As with undo, the text window
 * is redrawn and the file marked modified whenever a redo occurs.
 */
void redo(FileState *fs) {
    if (fs->redo_stack == NULL)
        return;

    Change change = pop(&fs->redo_stack);

    if (change.old_text && !change.new_text) { /* Deletion */
        char *dup = strdup(change.old_text);
        if (!dup) {
            allocation_failed("strdup failed");
        } else {
            push(&fs->undo_stack, (Change){ change.line, dup, NULL });
        }
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
        char *dup = strdup(change.new_text);
        if (!dup) {
            allocation_failed("strdup failed");
        } else {
            push(&fs->undo_stack, (Change){ change.line, NULL, dup });
        }
        free(change.new_text);
    } else if (change.old_text && change.new_text) { /* Edit */
        char *dup_old = strdup(change.old_text);
        char *dup_new = strdup(change.new_text);
        if (!dup_old || !dup_new) {
            if (dup_old)
                free(dup_old);
            if (dup_new)
                free(dup_new);
            allocation_failed("strdup failed");
        } else {
            push(&fs->undo_stack,
                 (Change){ change.line, dup_old, dup_new });
        }
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
    wrefresh(text_win);
    fs->modified = true;
}

/**
 * Free an entire change stack.
 *
 * All nodes are removed and any text stored within them is released.
 * This should be called when a FileState is destroyed to avoid leaking
 * memory associated with undo and redo history.
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


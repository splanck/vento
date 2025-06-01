#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include "editor.h"
#include "undo.h"
#include "files.h"
#include "editor_state.h"
#include "line_buffer.h"

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

int is_empty(Node *stack) {
    return stack == NULL;
}

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


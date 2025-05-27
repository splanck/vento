#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include "editor.h"
#include "undo.h"
#include "files.h"
char *strdup(const char *s);  // Explicitly declare strdup

void push(Node **stack, Change change) {
    Node *new_node = (Node *)malloc(sizeof(Node));
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
    if (undo_stack == NULL) return;

    Change change = pop(&undo_stack);

    if (change.old_text) {
        for (int i = fs->line_count; i > change.line; --i) {
            strcpy(text_buffer[i], text_buffer[i - 1]);
        }
        fs->line_count++;

        strcpy(text_buffer[change.line], change.old_text);

        push(&redo_stack, (Change){change.line, strdup(change.old_text), NULL});
        free(change.old_text);
    } else {
        for (int i = change.line; i < fs->line_count - 1; ++i) {
            strcpy(text_buffer[i], text_buffer[i + 1]);
        }
        text_buffer[fs->line_count - 1][0] = '\0';
        fs->line_count--;

        push(&redo_stack, change);
    }

    werase(text_win);
    box(text_win, 0, 0);
    draw_text_buffer(text_win);
    wrefresh(text_win);
}

void redo(FileState *fs) {
    (void)fs;
    if (redo_stack == NULL) return;

    Change change = pop(&redo_stack);

    if (change.new_text) {
        push(&undo_stack, (Change){change.line, strdup(text_buffer[change.line]), strdup(change.new_text)});
        strcpy(text_buffer[change.line], change.new_text);
        free(change.new_text);
    }

    werase(text_win);
    box(text_win, 0, 0);
    draw_text_buffer(text_win);
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


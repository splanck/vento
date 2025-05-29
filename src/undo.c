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
    if (fs->undo_stack == NULL)
        return;

    Change change = pop(&fs->undo_stack);

    if (change.old_text && !change.new_text) { /* Deletion */
        ensure_line_capacity(fs, fs->line_count + 1);
        for (int i = fs->line_count; i > change.line; --i) {
            strncpy(fs->text_buffer[i], fs->text_buffer[i - 1], fs->line_capacity - 1);
            fs->text_buffer[i][fs->line_capacity - 1] = '\0';
        }
        fs->line_count++;

        strncpy(fs->text_buffer[change.line], change.old_text, fs->line_capacity - 1);
        fs->text_buffer[change.line][fs->line_capacity - 1] = '\0';

        push(&fs->redo_stack, (Change){ change.line, strdup(change.old_text), NULL });
        free(change.old_text);
    } else if (!change.old_text && change.new_text) { /* Insertion */
        for (int i = change.line; i < fs->line_count - 1; ++i) {
            strncpy(fs->text_buffer[i], fs->text_buffer[i + 1], fs->line_capacity - 1);
            fs->text_buffer[i][fs->line_capacity - 1] = '\0';
        }
        fs->text_buffer[fs->line_count - 1][0] = '\0';
        fs->line_count--;

        push(&fs->redo_stack, (Change){ change.line, NULL, strdup(change.new_text) });
        free(change.new_text);
    } else if (change.old_text && change.new_text) { /* Edit */
        strncpy(fs->text_buffer[change.line], change.old_text, fs->line_capacity - 1);
        fs->text_buffer[change.line][fs->line_capacity - 1] = '\0';

        push(&fs->redo_stack,
             (Change){ change.line, strdup(change.old_text), strdup(change.new_text) });
        free(change.old_text);
        free(change.new_text);
    }

    werase(text_win);
    box(text_win, 0, 0);
    draw_text_buffer(active_file, text_win);
    wrefresh(text_win);
}

void redo(FileState *fs) {
    if (fs->redo_stack == NULL)
        return;

    Change change = pop(&fs->redo_stack);

    if (change.old_text && !change.new_text) { /* Deletion */
        push(&fs->undo_stack, (Change){ change.line, strdup(change.old_text), NULL });
        for (int i = change.line; i < fs->line_count - 1; ++i) {
            strncpy(fs->text_buffer[i], fs->text_buffer[i + 1], fs->line_capacity - 1);
            fs->text_buffer[i][fs->line_capacity - 1] = '\0';
        }
        fs->text_buffer[fs->line_count - 1][0] = '\0';
        fs->line_count--;
        free(change.old_text);
    } else if (!change.old_text && change.new_text) { /* Insertion */
        ensure_line_capacity(fs, fs->line_count + 1);
        for (int i = fs->line_count; i > change.line; --i) {
            strncpy(fs->text_buffer[i], fs->text_buffer[i - 1], fs->line_capacity - 1);
            fs->text_buffer[i][fs->line_capacity - 1] = '\0';
        }
        fs->line_count++;
        strncpy(fs->text_buffer[change.line], change.new_text, fs->line_capacity - 1);
        fs->text_buffer[change.line][fs->line_capacity - 1] = '\0';
        push(&fs->undo_stack, (Change){ change.line, NULL, strdup(change.new_text) });
        free(change.new_text);
    } else if (change.old_text && change.new_text) { /* Edit */
        push(&fs->undo_stack,
             (Change){ change.line, strdup(change.old_text), strdup(change.new_text) });
        strncpy(fs->text_buffer[change.line], change.new_text, fs->line_capacity - 1);
        fs->text_buffer[change.line][fs->line_capacity - 1] = '\0';
        free(change.old_text);
        free(change.new_text);
    }

    werase(text_win);
    box(text_win, 0, 0);
    draw_text_buffer(active_file, text_win);
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


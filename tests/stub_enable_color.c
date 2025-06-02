#include <ncurses.h>
#include <stdlib.h>
#include "editor.h"

int enable_color = 1;

void wtimeout(WINDOW *w, int t){ (void)w; (void)t; }

void free_stack(Node *stack){
    while (stack){
        Node *next = stack->next;
        free(stack->change.old_text);
        free(stack->change.new_text);
        free(stack);
        stack = next;
    }
}

#ifndef UNDO_H
#define UNDO_H

#include "editor.h"

void push(Node **stack, Change change);
Change pop(Node **stack);
int is_empty(Node *stack);
void free_stack(Node *stack);
void undo();
void redo();

#endif

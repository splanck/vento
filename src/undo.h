#ifndef UNDO_H
#define UNDO_H

#include "editor.h"
#include "files.h"

void push(Node **stack, Change change);
Change pop(Node **stack);
int is_empty(Node *stack);
void free_stack(Node *stack);
struct FileState;
void undo(FileState *fs);
void redo(FileState *fs);

#endif

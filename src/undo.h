#ifndef UNDO_H
#define UNDO_H

#include "editor.h"
#include "files.h"

/**
 * Pushes a change onto the given stack.
 *
 * Ownership of any strings inside the Change is transferred to the stack. The
 * function allocates a new Node and places it at the head of the list.
 */
void push(Node **stack, Change change);

/**
 * Pops the most recent change from the stack.
 *
 * If the stack is empty an empty Change with NULL strings is returned. The
 * caller becomes responsible for freeing the returned Change contents.
 */
Change pop(Node **stack);

/** Check whether the stack contains no entries. */
int is_empty(Node *stack);

/**
 * Frees all nodes in the stack and any text they own.
 */
void free_stack(Node *stack);

struct FileState;

/**
 * Reverses the most recent action recorded in `fs->undo_stack` and moves that
 * entry to `fs->redo_stack`.
 */
void undo(FileState *fs);

/**
 * Reapplies the most recently undone action from `fs->redo_stack` and moves it
 * back to `fs->undo_stack`.
 */
void redo(FileState *fs);

#endif

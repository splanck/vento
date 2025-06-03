/*
 * line_buffer.c
 * -------------
 * Implementation of the LineBuffer structure, a dynamic array of strings
 * used to hold the text contents of a file. Each entry in the array
 * represents one line. The helpers here manage allocation, resizing and
 * basic editing operations on that array.
 */

#include "line_buffer.h"
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 16

/**
 * Ensure the line buffer has at least MIN_CAPACITY slots.
 *
 * The function reallocates the internal array when needed and initialises
 * any newly created entries to NULL. On success the buffer's capacity is
 * updated. Returns 0 on success or -1 if memory allocation fails.
 */
static int lb_grow(LineBuffer *lb, int min_capacity) {
    int new_cap = lb->capacity ? lb->capacity * 2 : INITIAL_CAPACITY;
    if (new_cap < min_capacity)
        new_cap = min_capacity;
    char **tmp = realloc(lb->lines, new_cap * sizeof(char *));
    if (!tmp)
        return -1;
    lb->lines = tmp;
    for (int i = lb->capacity; i < new_cap; ++i)
        lb->lines[i] = NULL;
    lb->capacity = new_cap;
    return 0;
}

/**
 * Allocate and initialise a new LineBuffer.
 *
 * If INITIAL_CAPACITY is non-positive a default value is used. The returned
 * buffer owns its internal memory and should be released with lb_free().
 * Returns NULL on allocation failure.
 */
LineBuffer *lb_create(int initial_capacity) {
    LineBuffer *lb = malloc(sizeof(LineBuffer));
    if (!lb)
        return NULL;
    lb_init(lb, initial_capacity);
    if (!lb->lines) {
        free(lb);
        return NULL;
    }
    return lb;
}

/**
 * Initialise an existing LineBuffer structure.
 *
 * Memory for the line array is allocated using calloc so all slots start as
 * NULL. The count is reset to zero and capacity reflects the number of
 * allocated slots. A non-positive INITIAL_CAPACITY falls back to the default.
 */
void lb_init(LineBuffer *lb, int initial_capacity) {
    if (!lb)
        return;
    if (initial_capacity <= 0)
        initial_capacity = INITIAL_CAPACITY;
    lb->lines = calloc(initial_capacity, sizeof(char *));
    if (!lb->lines) {
        lb->count = 0;
        lb->capacity = 0;
        return;
    }
    lb->count = 0;
    lb->capacity = initial_capacity;
}

/**
 * Release all memory owned by LB.
 *
 * Each stored line is freed and the underlying array is released. The
 * structure's count and capacity fields are reset to zero so the buffer can
 * be safely reused or discarded.
 */
void lb_free(LineBuffer *lb) {
    if (!lb)
        return;
    for (int i = 0; i < lb->capacity; ++i)
        free(lb->lines[i]);
    free(lb->lines);
    lb->lines = NULL;
    lb->count = 0;
    lb->capacity = 0;
}

/**
 * Retrieve the string at INDEX from the buffer.
 *
 * Returns NULL if INDEX is out of range or if LB is NULL. The returned
 * pointer remains owned by the buffer.
 */
const char *lb_get(LineBuffer *lb, int index) {
    if (!lb || index < 0 || index >= lb->count)
        return NULL;
    return lb->lines[index];
}

/**
 * Insert LINE at the given INDEX within the buffer.
 *
 * The buffer grows if necessary and newly created slots are initialised
 * to NULL. When inserting past the current end, missing lines are created
 * and count is updated to index + 1. Returns 0 on success or -1 on memory
 * allocation failure.
 */
int lb_insert(LineBuffer *lb, int index, const char *line) {
    if (!lb || !line)
        return -1;
    if (index < 0)
        index = 0;

    /* Insert beyond current count means append and ensure space */
    if (index >= lb->count) {
        if (index >= lb->capacity) {
            if (lb_grow(lb, index + 1) < 0)
                return -1;
        }
        for (int i = lb->count; i < index; ++i)
            lb->lines[i] = NULL;
        lb->lines[index] = strdup(line);
        if (!lb->lines[index])
            return -1;
        lb->count = index + 1;
        return 0;
    }

    /* Regular insertion in the middle */
    if (lb->count >= lb->capacity) {
        if (lb_grow(lb, lb->count + 1) < 0)
            return -1;
    }
    memmove(&lb->lines[index + 1], &lb->lines[index],
            (lb->count - index) * sizeof(char *));
    lb->lines[index] = strdup(line);
    if (!lb->lines[index])
        return -1;
    lb->count++;
    return 0;
}

/**
 * Remove the line at INDEX from the buffer.
 *
 * The stored string is freed and subsequent lines are shifted up to fill the
 * gap. The count field is decremented; capacity is left unchanged.
 */
void lb_delete(LineBuffer *lb, int index) {
    if (!lb || index < 0 || index >= lb->count)
        return;
    free(lb->lines[index]);
    for (int i = index; i < lb->count - 1; ++i)
        lb->lines[i] = lb->lines[i + 1];
    lb->lines[lb->count - 1] = NULL;
    lb->count--;
}

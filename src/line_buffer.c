#include "line_buffer.h"
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 16

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

const char *lb_get(LineBuffer *lb, int index) {
    if (!lb || index < 0 || index >= lb->count)
        return NULL;
    return lb->lines[index];
}

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

void lb_delete(LineBuffer *lb, int index) {
    if (!lb || index < 0 || index >= lb->count)
        return;
    free(lb->lines[index]);
    for (int i = index; i < lb->count - 1; ++i)
        lb->lines[i] = lb->lines[i + 1];
    lb->lines[lb->count - 1] = NULL;
    lb->count--;
}

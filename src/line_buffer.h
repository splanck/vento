#ifndef LINE_BUFFER_H
#define LINE_BUFFER_H

#include <stddef.h>

/*
 * LineBuffer
 * ----------
 * A lightweight dynamic array of strings used by the editor to store
 * the contents of files. Each entry corresponds to a single line and
 * the structure tracks how many lines are in use as well as the
 * currently allocated capacity.
 */

typedef struct LineBuffer {
    char **lines;   /* array of allocated strings */
    int count;      /* number of valid lines stored */
    int capacity;   /* total slots allocated in lines */
} LineBuffer;

LineBuffer *lb_create(int initial_capacity);
void lb_init(LineBuffer *lb, int initial_capacity);
void lb_free(LineBuffer *lb);
const char *lb_get(LineBuffer *lb, int index);
int lb_insert(LineBuffer *lb, int index, const char *line);
void lb_delete(LineBuffer *lb, int index);

#endif /* LINE_BUFFER_H */

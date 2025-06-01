#ifndef LINE_BUFFER_H
#define LINE_BUFFER_H

#include <stddef.h>

typedef struct LineBuffer {
    char **lines;
    int count;
    int capacity;
} LineBuffer;

LineBuffer *lb_create(int initial_capacity);
void lb_init(LineBuffer *lb, int initial_capacity);
void lb_free(LineBuffer *lb);
const char *lb_get(LineBuffer *lb, int index);
int lb_insert(LineBuffer *lb, int index, const char *line);
void lb_delete(LineBuffer *lb, int index);

#endif /* LINE_BUFFER_H */

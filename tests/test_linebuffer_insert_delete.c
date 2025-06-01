#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <string.h>
#include "line_buffer.h"

int main(void){
    LineBuffer lb;
    lb_init(&lb, 2);

    assert(lb_insert(&lb, 0, "foo") == 0);
    assert(lb.count == 1);
    assert(strcmp(lb_get(&lb,0), "foo") == 0);

    assert(lb_insert(&lb, 1, "bar") == 0);
    assert(lb.count == 2);
    assert(strcmp(lb_get(&lb,1), "bar") == 0);

    /* trigger automatic resizing */
    assert(lb_insert(&lb, 2, "baz") == 0);
    assert(lb.count == 3);
    assert(strcmp(lb_get(&lb,2), "baz") == 0);
    assert(lb.capacity >= 3);

    lb_delete(&lb, 1);
    assert(lb.count == 2);
    assert(strcmp(lb_get(&lb,1), "baz") == 0);

    lb_free(&lb);
    return 0;
}

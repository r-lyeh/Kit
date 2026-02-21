#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

typedef void* (*reallocator)(void *, size_t);

#if TEST
#define BUDDY_ALLOC_IMPLEMENTATION
#include "buddy.h"
void *my_buddy_realloc(void *ptr, size_t requested_size) {
    static struct buddy *b = 0;
    if(!b) b = buddy_embed(malloc(64*1024),64*1024);
    return buddy_realloc(b, ptr, requested_size, 0);
}
#endif

reallocator arena(unsigned sz) {
    if( !sz ) return realloc;
    return my_buddy_realloc;
}

int main() {
    reallocator mem = arena(4);

    int *p = mem(NULL, sizeof(int));
    *p = 42;
    printf("%d\n", *p);
    p = mem(p, 0);
}

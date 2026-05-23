#ifndef ZREALLOC_PREREQUISITES
#define ZREALLOC_PREREQUISITES

// redirect all allocation code paths, so:
// - ensure we have a single code path for all allocations (we only use realloc; no malloc,calloc,free).
// - ensure we zero-clear all our allocs (zrealloc). especially important while enlarging dynamic buffers.

#include <string.h>
#include <stdlib.h>
#include ifdef(KIT_BSD, <malloc/malloc.h>, <malloc.h>) // freebsd: <malloc_np.h>
ifdef(KIT_ANDROID, size_t dlmalloc_usable_size(void*));

static void*  (*sys_realloc)(void*, size_t) = realloc;
static size_t (*sys_msize)(void *) = ifdef(KIT_BSD, (size_t (*)(void *))malloc_size, ifdef(KIT_WINDOWS, _msize, ifdef(KIT_ANDROID, dlmalloc_usable_size, malloc_usable_size)));

static void*  (*sys_malloc)(size_t) = malloc;
static void*  (*sys_calloc)(size_t, size_t) = calloc;
static void   (*sys_free)(void*) = free;
static char*  (*sys_strdup)(const char*) = strdup;

#endif


#ifndef ZREALLOC_H
#define ZREALLOC_H

void* zrealloc(void* p, size_t sz);

#endif

#if KIT_CODE
#pragma once

void* zrealloc(void* p, size_t siz) {
    if( siz != ~0u ) {
        if( siz > 0 ) {
            size_t old = p ? sys_msize(p) : 0u;
            if( siz > old ) {
                size_t diff = siz - old;
                p = sys_realloc(p, siz);
                if( p ) {
                    memset((char*)p + siz - diff, 0, diff);
                } else {
                    os_die("!Error: no memory enough.");
                }
                return p;
            }
        }
        return sys_realloc(p, siz);
    }
    return (void*)(uintptr_t)sys_msize(p);
}

#endif

#ifndef ALLOC_H
#define ALLOC_H

// memory (vector based allocator; x1.75 enlarge factor) -----------------------

void* vrealloc(void* p, size_t sz);
size_t vlen( void* p );

#define strdup(s)         (strcpy(vrealloc(0,strlen(s)+1),s))
#define free(p)           (void)vrealloc((p), 0)
#define malloc(sz)        vrealloc(0, (sz))
#define calloc(n,sz)      vrealloc(0,(n)*(sz)) // no need to memset(malloc(0,n*s),0,n*s) since vrealloc>zrealloc does it for us
#define realloc           vrealloc

// memory leaks ----------------------------------------------------------------

#if REPORT_MEMORY_LEAKS
#define vrealloc(ptr,len) memleak(vrealloc(memleak(ptr,0,0,0,0),len),len,__func__,__FILE__,__LINE__)
void* memleak(void *ptr, int len, const char *func, const char *file, int line);
#endif

#elif KIT_CODE
#pragma once

#if 0
// ----------------------------------------------------------------------
// Fixed vrealloc (no floating-point truncation)
// ----------------------------------------------------------------------
static THREAD size_t vstats = 0;

void* (vrealloc)(void* p, size_t sz) {
    size_t *ret;

    if (!sz) {
        if (p) {
            ret = (size_t*)p - 2;
            vstats -= sizeof(size_t) * 2 + ret[1];
            ret[0] = 0;
            ret[1] = 0;
            zrealloc(ret, 0);
        }
        return NULL;
    }

    if (!p) {
        size_t alloc_sz = sizeof(size_t) * 2 + sz;
        ret = (size_t*)zrealloc(NULL, alloc_sz);
        if (!ret) return NULL;
        ret[0] = sz;
        ret[1] = 0;
        vstats += alloc_sz;
    } else {
        ret = (size_t*)p - 2;
        size_t osz = ret[0];
        size_t cap = ret[1];

        if (sz == (size_t)-1)
            return (void*)osz;

        if (sz <= osz + cap) {
            ret[0] = sz;
            ret[1] = cap - (sz - osz);
        } else {
            // grow 1.75x using integer arithmetic
            size_t new_data = sz;
            size_t new_alloc = sizeof(size_t)*2 + (new_data * 7 / 4);

            ret = (size_t*)zrealloc(ret, new_alloc);
            if (!ret) return NULL;

            ret[0] = sz;
            ret[1] = (new_data * 7 / 4) - sz;
            vstats += ret[1] - osz;
        }
    }
    return &ret[2];
}

size_t vlen(void* p) {
    return p ? (size_t)vrealloc(p, (size_t)-1) : 0;
}
#else
static THREAD size_t vstats = 0;
void* (vrealloc)( void* p, size_t sz ) {
    size_t *ret;
    if( !sz ) {
        if( p ) {
            ret = (size_t*)p - 2; vstats -= sizeof(size_t) * 2 + ret[1];
            ret[0] = 0;
            ret[1] = 0;
            ret = (size_t*)zrealloc( ret, 0 );
        }
        return 0;
    } else {
        if( !p ) {
            ret = (size_t*)zrealloc( 0, sizeof(size_t) * 2 + sz ); vstats += sizeof(size_t) * 2 + sz;
            ret[0] = sz;
            ret[1] = 0;
        } else {
            ret = (size_t*)p - 2;
            size_t osz = ret[0], cap = ret[1]; // original size and capacity
            if( sz == (size_t)-1 ) return (void*)osz;
            if( sz <= (osz + cap) ) {
                ret[0] = sz;
                ret[1] = cap - (sz - osz);
            } else {
                ret = (size_t*)zrealloc( ret, sizeof(size_t) * 2 + sz * 1.75 );
                ret[0] = sz;
                ret[1] = (size_t)(sz * 1.75) - sz; vstats += ret[1] - osz;
            }
        }
        return &ret[2];
    }
}
size_t vlen( void* p ) {
    return p == (void*)-1 ? vstats : p ? (size_t)(vrealloc)( p, (size_t)-1 ) : 0; // @addme to fwk
}
#endif

// memory leaks ----------------------------------------------------------------

#if REPORT_MEMORY_LEAKS
void  memreport(void) { system(KIT_WINDOWS ? 
    "type 0*.mem 2> nul       > report.mem && del 0*.mem 2> nul       && type report.mem | sort && find /V /C \"\" report.mem" :
    "cat 0*.mem  2> /dev/null > report.mem ;  rm  0*.mem 2> /dev/null ;  cat  report.mem | sort ;  wc -l report.mem"
); }
void* memleak(void *ptr, int len, const char *func, const char *file, int line) {
    ONCE system(KIT_WINDOWS ? "del *.mem 2> nul" : "rm *.mem 2> /dev/null"), atexit(memreport);
    char n[sizeof(void*)*2+8] = {0}; sprintf(n,"%p.mem",ptr);
    for(FILE *fp = line ? fopen(n,"wb") : 0; fp; fprintf(fp, "%8d bytes, %-20s, %s:%d\n", len,func,file,line), fclose(fp), fp = 0) {}
    return line ? ptr : (unlink(n), ptr);
}
#endif

#endif

#ifndef PTR_H
#define PTR_H

void *(ptr)(const char *bin, const char *type); 

#define obj(type, ...) *(type*)ptr(__VA_ARGS__, #type)  // returns a constructed struct
#define ptr(bin, ...)          ptr(bin, "" __VA_ARGS__) // returns a constructed temporary struct*

#elif KIT_CODE
#pragma once

void *(ptr)(const char *bin, const char *type) { // returns a temporary struct*
    char buf[64] = {0}; 
    if( !type || !type[0] ) { // try to infer from bin
        if( bin[0] == '[' ) { // assume .ini with [type] section
            if( sscanf(bin+1, "%63[^]]", buf) != 1 ) return NULL;
            type = buf;
        }
    }
    if( !type || !type[0] ) return NULL;  // we did our best at this point

    rtti *R = reflected(type);
    if( !R ) return NULL;

    char *s = va("%.*s", R->size, "");
    (objload)((void*)s, bin, type);

    return s;
}

#if defined TEST || defined TESTS
AUTOTEST { // fvec3 is reflected in a previous test
    fvec3 r = obj(fvec3, "[fvec3]\nx=1\ny=2\nz=3");
    test( r.x == 1 );
    test( r.y == 2 );
    test( r.z == 3 );

    fvec3 s = obj(fvec3, "x=-1\ny=-2\nz=-3");
    test( s.x == -1 );
    test( s.y == -2 );
    test( s.z == -3 );
}
#endif // TEST
#endif // KIT_CODE

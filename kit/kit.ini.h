// .ini reader that handles [sections], #comments and key=value pairs

#ifndef INI_H
#define INI_H

const char** ini(const char *contents, int *pairs);

#elif KIT_CODE
#pragma once

const char** ini(const char *contents, int *pairs) {
    static array_(char*) vec = 0;
    for( int i = 0, ii = array_count(vec); i < ii; ++i ) if(vec[i]) free(vec[i]);
    array_clear(vec);

    char section[128] = {0}, slen = 0;
    for each_line(l, contents, ",") { // \r\n and ,
        // skip key whitespaces, then EOS||comment check
        // read [section] if present
        // skip key token, then EOS||comment check, terminate keystr
        // skip whitespaces, then assign value
        char *key = (char*)l + strspn((char*)l, " \t"); if(!*key || *key == '#') continue;
        if( *key == '[' ) { snprintf(section,128,"%s",key+1), slen = strcspn(section, "= []\t"); continue; }
        char *end = key + strcspn(key, "= ]\t"); if(!*end || *end == '#') continue; *end++ = 0;
        char *val = end + strspn(end, "= [\t");

        if(section[0]) snprintf(section+slen,128-slen,".%s",key), key=section;
        array_push(vec, strdup(key));
        array_push(vec, strdup(val));
    }

    if(pairs) *pairs = array_count(vec) / 2;

    array_push(vec,0);
    array_push(vec,0);

    return (const char **)vec;
}

#endif

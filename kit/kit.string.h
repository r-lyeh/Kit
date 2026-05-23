#ifndef STRING_H
#define STRING_H

// strings ---------------------------------------------------------------------

#define string_(t) t // for REFLECT()

#define each_line(l, text, ...) each_string(l, text, "\r\n" __VA_ARGS__)
#define each_word(w, line, ...) each_string(w, line, "\t " __VA_ARGS__)

// iterator: split string into tokens, including separators. @fixme
#define each_string(tok, s_, sep__) \
    (const char *copy_ = va("%s", (s_)), *sep_ = (sep__); copy_; copy_ = 0) \
        for( char* tok = strsep((char**)&copy_,sep_); tok; tok = strsep((char**)&copy_,sep_) )
#define each_string_notworking(tok, s_, sep_) \
    (char *copy = va("%s", (s_)), *saveptr = NULL, is_sep = 0; copy; copy = 0) \
    for(const char *tok = strtok_r(copy, sep_, &saveptr); \
         tok || (saveptr && *saveptr); \
         is_sep = !is_sep, \
         tok = is_sep ? ((saveptr && *saveptr) ? (char[]){ *saveptr++, '\0' } : NULL) \
                      : strtok_r(NULL, sep_, &saveptr) )

// iterator: split string into tokens, excluding separators
#define each_string_ex(token_, s_, sep_) \
    (char *copy_ = va("%s", (s_)); copy_; copy_ = 0) \
        for( const char *next_ = 0, *token_ = strtok_r(copy_, sep_, (char**)&next_); token_; token_ = strtok_r(NULL, sep_, (char**)&next_) )


#define va(...) ((printf || printf(__VA_ARGS__), va(__VA_ARGS__)))  // vs2015 check trick
//#define va(...) (((&printf) || printf(__VA_ARGS__), va(__VA_ARGS__)))  // vs2015 check trick

static inline
char* (va)(const char *fmt, ...) {
    va_list vl, copy;
    va_start(vl, fmt);

        va_copy(copy, vl);
        int sz = /*stbsp_*/vsnprintf( 0, 0, fmt, copy ) + 1;
        va_end(copy);

        enum { STACK_ALLOC = 64*1024 }; assert(sz < STACK_ALLOC && "no stack enough, increase STACK_ALLOC value");
        //static THREAD char *buf = 0; if(!buf) buf = (vrealloc)(0, STACK_ALLOC); // @leak
        static THREAD char buf[STACK_ALLOC] = {0};
        static THREAD int cur = 0, len = STACK_ALLOC - 1;

        char* ptr = buf + (cur *= (cur+sz) < len, (cur += sz) - sz);
        /*stbsp_*/vsnprintf( ptr, sz, fmt, vl );

    va_end(vl);
    return ptr;
}

#ifdef _WIN32
char *strsep(char **sp, const char *sep); // better than strtok(), as it preserves empty strings within delimiters
#endif
char *strswap(char *text, const char *target, const char *replacement); // replace all occurences of `target` in `text`, only if `replacement` is shorter than `target`
int strmatch(const char *s, const char *wildcard); // returns true if wildcard matches
int strmatchi(const char *s, const char *wildcard); // returns true if wildcard matches (case insensitive)
int strcnt(const char *s, int ch); // count repetitions of a character in a string
const char *strvalid(const char *p);

#if _MSC_VER
#define strtok_r strtok_s
#endif

#elif KIT_CODE
#pragma once

#ifdef _WIN32
char *strsep(char **sp, const char *sep) { // better than strtok(), as it preserves empty strings within delimiters
    if( sp && *sp && **sp ) {
        char *p = *sp + strcspn(*sp, sep), *s = *sp;
        if( *p ) *p++ = '\0';
        *sp = p;
        return s;
    }
    return NULL;
}
#endif
int strmatch(const char *s, const char *wildcard) { // returns true if wildcard matches
    if( *wildcard == '\0' ) return !*s;
    if( *wildcard ==  '*' ) return strmatch(s, wildcard+1) || (*s && strmatch(s+1, wildcard));
    if( *wildcard ==  '?' ) return *s && (*s != '.') && strmatch(s+1, wildcard+1);
    return (*s == *wildcard) && strmatch(s+1, wildcard+1);
}
int strmatchi(const char *s, const char *wildcard) { // returns true if wildcard matches (case insensitive)
    if( *wildcard == '\0' ) return !*s;
    if( *wildcard ==  '*' ) return strmatchi(s, wildcard+1) || (*s && strmatchi(s+1, wildcard));
    if( *wildcard ==  '?' ) return *s && (*s != '.') && strmatchi(s+1, wildcard+1);
    return (tolower(*s) == tolower(*wildcard)) && strmatchi(s+1, wildcard+1);
}
int strcnt(const char *s, int ch) { // count repetitions of a character in a string
    int count = 0;
    if(s) while( *s++ ) count += s[-1] == ch;
    return count;
}
char *strswap(char *text, const char *target, const char *replacement) { // replaced only if replacement is shorter than target
    int rlen = strlen(replacement), diff = strlen(target) - rlen;
    if( diff >= 0 )
    for( char *s = text, *e = s + strlen(text); /*s < e &&*/ 0 != (s = strstr(s, target)); ) {
        if( rlen ) s = (char*)memcpy( s, replacement, rlen ) + rlen;
        if( diff ) memmove( s, s + diff, (e - (s + diff)) + 1 );
    }
    return text;
}
const char* strvalid(const char *s) {
    return s ? s : "";
}

#endif

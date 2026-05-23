unsigned    argc();
char*       argv(unsigned index);
const char *arg(const char *option);  // arg("--key=value") returns `value` if no arg is supplied, or actual argument text if supplied
int         argi(const char *option); // same than atoi(arg(...))

#if KIT_CODE
#pragma once

#if KIT_MACOS
#include <crt_externs.h>
#endif

#if KIT_GCC && !KIT_MINGW // also, clang
    int __argc; char **__argv;
    ifdef(KIT_WASM,,__attribute__((constructor)) void init_argcv(int c, char **v) { __argc = c; __argv = v; });
#endif

unsigned argc(void) {
    ifdef(KIT_MACOS, ONCE __argc = *_NSGetArgc());
    return __argc;
}
char* argv(unsigned arg) {
    ifdef(KIT_MACOS, ONCE __argv = *_NSGetArgv());
    return __argv[((arg % __argc) + __argc) % __argc];
}
const char *arg(const char *opt) { // if "--opt" is provided: function returns "1" if --opt found, "0" otherwise. if "--opt=X" is provided: function returns "Y" if --opt=Y is found, "X" otherwise.
    int optlen = strcspn(opt, "=");
    if( optlen ) {
         // '--option' case
        if( !strchr(opt, '=') ) {
            const char *alt = va("%s=*", opt);
            for( int i = 1; i < argc(); ++i ) {
                if( !strcmp(argv(i), opt) ) return "1";
                if( strmatch(argv(i), alt) ) return argv(i) + strcspn(argv(i), "=") + 1;
            }
            return "0";
        }
        else // '--option=value' case
        for( int i = 1; i < argc(); ++i ) {
            if( argv(i)[0] == '-' && !strncmp(argv(i), opt, optlen) ) {
                return argv(i) + optlen + 1;
            }
        }
    }
    return opt + optlen + 1;
}

AUTOTEST {
    test(argc() >= 1);
#ifndef main //< avoid crash if using SDL_main, which does not support __argv at crt0/AUTOTEST time
    test(argv(0) != NULL);
    test(0 == strcmp("yes", arg("--test=yes"))
      || 0 == strcmp("no",  arg("--test=no")));
#endif
}

#endif // KIT_CODE

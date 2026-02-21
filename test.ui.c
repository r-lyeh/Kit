// cl test.c

#include "kit.h"
#include <stdio.h>

#include <stdarg.h>
#define THREAD __declspec(thread)
static const char *va(const char *fmt, ...) {
    static THREAD char tmp[0x10000];
    static THREAD int at = 0;
    char *ret = tmp+at;
    va_list args;
    va_start(args, fmt);
    at += 1 + vsnprintf(ret, sizeof(tmp)-at-1, fmt, args);
    va_end(args);
    if(at > sizeof(tmp) - 0x400) {
        at = 0;
    }
    return ret;
}


#undef  RGB
#define RGB(r,g,b) ((255<<24)|(b<<16)|(g<<8)|(r<<0))

const unsigned gray[] = {
    RGB(8,11,15), // dark
    RGB(37,44,56), // dim
    RGB(50,68,69), // mid
    RGB(111,120,134), // normal
    RGB(214,224,242), // highlight
};
const unsigned pal[] = {
    RGB(15,103,251), // blue
    RGB(255,26,0), // red
    RGB(254,12,115), // pink

    RGB(0,255,0), // green
    RGB(0,210,229), // cyan
    RGB(255,212,0), // yellow
};
unsigned use1,use2;

struct mouse {
    union { int x, X; };
    union { int y, Y; };
    union { int w, W, Wheel; };
    union { int b, B, Buttons; };
    union { int l, L, Left; };
    union { int m, M, Middle; };
    union { int r, R, Right; };
    union { int c, C, Cursor; };
};

typedef struct mouse mouse_t;

mouse_t mouse(int id) {
    int x, y, buttons;
    tigrMouse(window_handle(id), &x, &y, &buttons);

    mouse_t m = {0};
    m.x = x;
    m.y = y;
    m.l = !!(buttons & 1);
    m.m = !!(buttons & 2);
    m.r = !!(buttons & 4);
    m.b = (m.r << 2) | (m.m << 1) | (m.l << 0);

    return m;
}


int main() {
    const char *title = "kit.h ("
        ifdef(KIT_C, "C", "C++")
        ifdef(KIT_CL, ",MSVC")
        ifdef(KIT_GCC, ",GCC")
        ifdef(KIT_CLANG, ",CLANG")
        ifdef(KIT_WINDOWS, ",WINDOWS")
        ifdef(KIT_LINUX, ",LINUX")
        ifdef(KIT_MACOS, ",MACOS")
    ")";
    puts(title);

    window_create(0);
    window_create(1);

    while( window_count() > 0 ) { // ( window_count() == 2 ) {
        if( window_ready(0) ) {
            window_clear(0,gray[use1%countof(gray)]);
            struct mouse m = mouse(0);
            print(0,va("Hi %d %d %d",m.x,m.y,m.l<<2|m.m<<1|m.r));
            window_swap(0);
        }
        if( window_ready(1) ) {
            window_clear(1,pal[use2%countof(pal)]);
            struct mouse m = mouse(1);
            print(1,va("Hi %d %d %d",m.x,m.y,m.l<<2|m.m<<1|m.r));
            window_swap(1);
        }
        if(key(1,'<')) use1++;
        if(key(1,'>')) use2++;
    }

    window_destroy(0);
    window_destroy(1);
}

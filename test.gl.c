// cl test.c

#include "kit.h"
#include <stdio.h>

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

#include <kit_opengl.h>

int main() {
    for( window_create(0); window_count() > 0; ) {
        if( window_ready(0) ) { //gray[use1%countof(gray)])) {
            window_clear(0, pal[0]);

            tigrBeginOpenGL(window_handle(0));

                glClearColor(1, 0, 1, 1);
                glClear(GL_COLOR_BUFFER_BIT);

            window_swap(0);
        }
        if(key(1,'<')) use1++;
        if(key(1,'>')) use2++;
    }

    window_destroy(0);
}

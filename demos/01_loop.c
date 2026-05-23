// This example code creates a window+renderer, and then clears the
// window to a different color every frame, so you'll effectively get a window
// that's smoothly fading between colors.
//
// This code is public domain. Feel free to use it for any purpose!

#include "kit.h"

const char *hints;

void main(event ev) {
    if( ev.init ) {
        if(!render.open(640,480,0.75,0)) { // 640x480 logical coords in a 75% window
            app.quit(-1);
        }
    }
    if( ev.tick ) {
        // choose the color for the frame we will draw, colors fade smoothly via sin() wave.
        const double now = elapsed.ss(); // time in seconds
        const float red = (float) (0.5 + 0.5 * SDL_sin(now));
        const float green = (float) (0.5 + 0.5 * SDL_sin(now + SDL_PI_D * 2 / 3));
        const float blue = (float) (0.5 + 0.5 * SDL_sin(now + SDL_PI_D * 4 / 3));

        // clear the window to the draw color.
        render.clear(color.rgb(red*255,green*255,blue*255));

        // bring the newly-cleared rendering on the screen.
        render.present();
    }
    if( ev.type == SDL_EVENT_QUIT ) {
        app.quit(0);
    }
}

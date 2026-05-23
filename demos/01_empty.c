#include "kit.h"

const char *hints;

void main(event ev) {
    if( ev.init ) {
        render.open(640,480,0.75,0); // 640x480 logical coords in a 75% window
    }
    if( ev.tick ) {
        render.clear(0xFF4D2DD3);
        render.present();
    }
    if( ev.type == SDL_EVENT_QUIT ) {
        app.quit(0);
    }
}

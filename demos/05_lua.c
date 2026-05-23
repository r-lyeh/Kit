#include "kit.h"

void main(event ev) {
    static unsigned L = 0;

    if( ev.init ) {
        if(!render.open(800,600,0,0))
            app.quit(-1);
        
        L = lua.open("demos/art/tick.lua");

        os.log("Press \6`R`\7 to reload");
    }

    if( ev.emit ) {
        SDL_Event *event = ev.emit;
        if (event->type == SDL_EVENT_QUIT) {
            app.quit(0);
        }
        if (event->type == SDL_EVENT_KEY_DOWN && event->key.key == SDLK_R) {
            os.log("\6Reloading Lua script...");
            lua.reload(L);
        }
    }

    if( ev.tick ) {
        render.clear(color.hex("#001"));

        lua.call(L, "tick");

        render.present();
    }

    if( ev.quit ) {
        lua.close(&L);
    }
}

const char *hints;

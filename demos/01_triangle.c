#include "kit.h"
#include "_mesh.h"

void main(event ev) {
    int _640 = 640;
    int _480 = 480;
    if( ev.init ) {
        if( !render.open(_640, _480, 0.85, 0) )
            app.quit(-1);
        window.title("mesh triangle + transparent bacgrkound");
    }
    if( ev.emit ) {
        if( ev.emit->type == SDL_EVENT_QUIT ) {
            //if( dialog.prompt("Exit Game","Are you sure that you want to quit?", 2, "No", "Yes") == 2 )
                app.quit(0); /* end the program, reporting success to the OS. */
        }
        if( ev.emit->type == SDL_EVENT_KEY_DOWN ) {
            if( keyboard.down("F11") || (keyboard.down("enter") && keyboard.held("Alt")) ) window_fullscreen( window_fullscreen(-1) ^ 1 ); // on()
        }
    }
    if( ev.tick ) {
        float size = _480/3;
        float2 c = float2(_640/2,_480/2);

        float2 up = float2(c.x, c.y-size*0.60);
        float2 left = float2(c.x-size*0.75, c.y+size*0.60);
        float2 right = float2(c.x+size*0.75, c.y+size*0.60);

        /* Start with a dark blue transparent canvas. */
        render.clear(color.hex("#000011bb"));

        /* Draw a single triangle with a different color at each vertex. */
        mesh.clear();
        mesh.vertex( float2(up.x, up.y), float4(1,0,0,1), float2(0,0));
        mesh.vertex( float2(left.x, left.y), float4(0,1,0,1), float2(0,0));
        mesh.vertex( float2(right.x, right.y), float4(0,0,1,1), float2(0,0));
        mesh.push(0);

        render.present();
    }
}

const char *hints;

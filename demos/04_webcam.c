#include "kit.h"
#include "_mesh.h"
const char *hints;

void main(event ev) {
    if( ev.init ) {
        if(!render.open(640,480, 0.85, 0))
            app.quit(-1);
    }
    if( ev.emit ) {
        if( ev.emit->type == SDL_EVENT_QUIT )
            app.quit(0);
    }
    if( ev.tick ) {
        unsigned tex = webcam_capture(1); // capture texture from webcam #1

        render.clear(color.hex("#001"));

        mesh.clear();
        mesh.vertex(float2(  0,  0),float4(1,1,1,1),float2(0,0)); // xy, tint, uv
        mesh.vertex(float2(640,  0),float4(1,1,1,1),float2(1,0)); // xy, tint, uv
        mesh.vertex(float2(  0,480),float4(1,1,1,1),float2(0,1)); // xy, tint, uv
        mesh.vertex(float2(640,480),float4(1,1,1,1),float2(1,1)); // xy, tint, uv
        mesh.quad(0,1,2,3);
        mesh.push(tex); // use texture from webcam capture
  
        render.present();
    }
}

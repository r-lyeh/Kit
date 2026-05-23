#include "kit.h"
const char *hints;

void main(event ev)
{
    if( ev.init ) {
        float scale = 0.85;
        int _1280 = display.width(0), _800 = display.height(0);
        if( !render.open(_1280*scale,_800*scale,scale,0) ) {
            app.quit(-1);
        }
        imgui.open();
    }

    // events
    if( ev.emit ) {
        // consume imgui event
        SDL_Event *event = imgui.event(ev.emit);
        if( !event ) return;

        // process remaining event
        if (event->type == SDL_EVENT_QUIT)
            //if( dialog.prompt("Exit app", "Are you sure?", 2, "No", "Yes") == 2 )
                app.quit(0);
    }

    // main loop
    if( ev.tick ) {
        render.clear(color.hex("#001"));

        imgui.begin();

        ui2_tick();
        ui2_demo(1);

        if( ui.button("this is a button") )
            os.log("clicked");

        imgui.end();

        render.present();
    }

    // bye
    if( ev.quit ) {
        imgui.close();
        app.quit(0);
    }
}

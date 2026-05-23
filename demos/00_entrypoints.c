#include <kit.h>

void main(event ev) {
    puts("1st entrypoint in this file. you can call following functions at any time:");
    puts("- app.quit(rc), to quit the app gracefully.");
    puts("- app.reload(), to restart the app if needed.");
    //< your code here...
    puts("bye!\n");
    app.quit(0);
}

void main(event ev) {
    puts("2nd entrypoint in this file. user can decide which entrypoint to run via --main=N arg");
    puts("also, this could be run in parallel in addition to previous entrypoint");
    //< your parallel code here...
    puts("bye!\n");
    app.quit(0);
}

void main(event ev) {
    ONCE {
        puts("3rd entrypoint in this file. classic/blocking main loop that:");
        puts("- wont run on emscripten");
        puts("- will freeze rendering and audio while app is being dragged or resized");
        if(!render.open(640,480, 0.75, 0)) {
            app.quit(-1);
        }
        while( kit.loop(1) ) {
            if( keyboard.down("F5") ) app.reload();
            if( keyboard.down("escape") || (keyboard.down("F4") && keyboard.held("alt")) )
                //if( dialog.prompt("Exit game","Are you sure?", 2, "No", "Yes") == 2 )
                    app.quit(0);

            render.clear(color.hex("#102"));
            render.present();
        }
        puts("bye!\n");
        app.quit(0);
    }
}

void main(event ev) {
    if( ev.init ) {
        puts("4th entrypoint in this file. this one is event based and thus:");
        puts("- can run on emscripten");
        puts("- rendering and audio will work even while window is being dragged or resized");
        if(!render.open(640,480, 0.75, 0))
            app.quit(-1);
    }
    if( ev.emit ) {
        if( ev.emit->type == SDL_EVENT_QUIT )
            app.quit(0);
    }
    if( ev.tick ) {
        if( keyboard.down("F5") ) app.reload();
        if( keyboard.down("escape") || (keyboard.down("F4") && keyboard.held("alt")) )
            //if( dialog.prompt("Exit game","Are you sure?", 2, "No", "Yes") == 2 )
                app.quit(0);

        render.clear(color.hex("#102"));
        render.present();
    }
    if( ev.quit ) {
        puts("bye!\n");
    }
}

const char *hints;

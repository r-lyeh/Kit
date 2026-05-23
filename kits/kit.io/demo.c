// lua-evaluated input expressions
// - rlyeh, public domain

#include <kit.h>

// ---------------------------------------------------------------------------
// demo

void main(event ev) {

    if (ev.init) {
        if (!render.open(0, 0, 0.50, 0))
            app.quit(-1);
    }

    if (ev.emit) {
        if (ev.emit->type == SDL_EVENT_QUIT)
            app.quit(0);
    }

    if (ev.tick) {
        if (input_keyboard(0, "escape"))   app.quit(0);
        if (input_keyboard(0, "down(f1)")) dialog.alert("F1 pressed");

        render.clear(color.hex("#001"));
        render.present();
    }

    if (ev.quit) {
        printf("\ndone.\n");
    }
}

const char *hints;

#include "kit.h"
const char *hints;

void main(event ev)
{
    // Our state
    static bool show_demo_window = true;
    static bool show_another_window = false;
    static ImVec4 clear_color = {10/255.f, 64/255.f, 100/255.f, 0.50f};

    if( ev.init ) {
        float scale = 0.80;
        if(!render.open(display.width(0)*scale,display.height(0)*scale,scale,0)) {
            app.quit(-1);
        }

        imgui.open();
    }
    // Events
    if( ev.emit ) {
        SDL_Event *event = imgui.event(ev.emit);
        if( !event ) return; // if event consumed

        if (event->type == SDL_EVENT_QUIT)
            //if( dialog.prompt("Exit app", "Are you sure?", 2, "No", "Yes") == 2 )
                app.quit(0);
    }
    // Main loop
    if( ev.tick ) {

        render.clear(color.hex("#001"));
        imgui.begin();

        // 1. Show the big demo window (Most of the sample code is in igShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            igShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            igBegin("Hello, world!", NULL, 0);                 // Create a window called "Hello, world!" and append into it.

            igText("This is some useful text.");               // Display some text (you can use a format strings too)
            igCheckbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            igCheckbox("Another Window", &show_another_window);

            igSliderFloat("Float", &f, 0.0f, 1.0f, "%.3f", 0);    // Edit 1 float using a slider from 0.0f to 1.0f
            igColorEdit4("clear color", (float*)&clear_color, 0); // Edit 3 floats representing a color

            if (igButton("Button",ImVec2(0,0)))                               // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            igSameLine(0,-1);
            igText("counter = %d", counter);

            ImGuiIO* io = igGetIO_Nil();
            igText("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io->Framerate, io->Framerate);
            igEnd();
        }

        // 3. Show another simple window.
        if (show_another_window)
        {
            igBegin("Another Window", &show_another_window, 0);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            igText("Hello from another window!");
            if (igButton("Close Me",ImVec2(0,0)))
                show_another_window = false;
            igEnd();
        }

        imgui.end();

        render.present();
    }
    if( ev.quit ) {
        imgui.close();
        app.quit(0);
    }
}

// demo that displays keyboard, mouse, gamepad, touch and high-level input bindings.
// @todo: showcase rumble api
// - rlyeh, public domain

#include "kit.h"

const char *hints;

// ---------------------------------------------------------------------------
// tiny colored rectangle helper

static void draw_rect(float x, float y, float w, float h, unsigned col) {
    SDL_SetRenderDrawColor(render.handle,
        color.r(col), color.g(col), color.b(col), color.a(col));
    SDL_FRect r = { x, y, w, h };
    SDL_RenderFillRect(render.handle, &r);
}
static void draw_rectb(float x, float y, float w, float h, unsigned col) {
    SDL_SetRenderDrawColor(render.handle,
        color.r(col), color.g(col), color.b(col), color.a(col));
    SDL_FRect r = { x, y, w, h };
    SDL_RenderRect(render.handle, &r);
}

// ---------------------------------------------------------------------------
// label shorthand via imgui

static void label(const char *fmt, ...) {
    char buf[256];
    va_list ap; va_start(ap, fmt); vsnprintf(buf, sizeof buf, fmt, ap); va_end(ap);
    igText("%s", buf);
}
static void colored_label(float r, float g, float b, const char *fmt, ...) {
    char buf[256];
    va_list ap; va_start(ap, fmt); vsnprintf(buf, sizeof buf, fmt, ap); va_end(ap);
    igTextColored((ImVec4){r,g,b,1}, "%s", buf);
}

// ---------------------------------------------------------------------------
// section helpers

static void section(const char *title) {
    igSeparator();
    igTextColored((ImVec4){1,0.8f,0.2f,1}, "[ %s ]", title);
}

// ---------------------------------------------------------------------------
// key indicator: a small colored pill per key

static void key_pill(const char *label_str, float val) {
    unsigned col = val > 0 ? color.hex("#4f8") : color.hex("#444");
    igPushStyleColor_Vec4(ImGuiCol_Button,      (ImVec4){color.r(col)/255.f, color.g(col)/255.f, color.b(col)/255.f, 0.9f});
    igPushStyleColor_Vec4(ImGuiCol_ButtonHovered,(ImVec4){color.r(col)/255.f, color.g(col)/255.f, color.b(col)/255.f, 1.f});
    igPushStyleColor_Vec4(ImGuiCol_ButtonActive, (ImVec4){color.r(col)/255.f, color.g(col)/255.f, color.b(col)/255.f, 1.f});
    igButton(label_str, (ImVec2){0,0});
    igPopStyleColor(3);
    igSameLine(0,-1);
}

// ---------------------------------------------------------------------------
// draw the mouse cursor visualisation

static void draw_mouse_vis(float ox, float oy) {
    float mx = mouse.get("x");
    float my = mouse.get("y");
    // crosshair at mouse position
    unsigned col = mouse.held("left") ? color.hex("#f84") :
                   mouse.held("right") ? color.hex("#48f") : color.hex("#fff");
    int w,h; SDL_GetWindowSize(window.handle,&w,&h);
    float px = mx, py = my; // already raw pixels
    draw_rect(px-8, py-1, 16, 2, col);
    draw_rect(px-1, py-8,  2,16, col);
    // wheel indicator
    float wheel = mouse.get("wheel");
    if(wheel != 0) {
        draw_rect(px+12, py + (wheel>0?-8:4), 4, 4, color.hex("#ff0"));
    }
}

// ---------------------------------------------------------------------------
// gamepad visualiser

static void draw_gamepad_vis(float ox, float oy, int gid) {
    float connected = gamepad.connected(gid);
    if(!connected) {
        igTextDisabled("Gamepad %d: not connected", gid);
        return;
    }

    igTextColored((ImVec4){0.4f,1,0.4f,1}, "Gamepad %d connected", gid);

    // face buttons
    igText("Face: ");
    key_pill("S",  gamepad.held(gid,"south"));
    key_pill("E",  gamepad.held(gid,"east"));
    key_pill("W",  gamepad.held(gid,"west"));
    key_pill("N",  gamepad.held(gid,"north"));
    igNewLine();

    // shoulders/triggers
    igText("Shoulders: ");
    key_pill("L1", gamepad.held(gid,"L1"));
    key_pill("R1", gamepad.held(gid,"R1"));
    igNewLine();
    igText("Triggers: ");
    float l2 = gamepad.get(gid,"L2");
    float r2 = gamepad.get(gid,"R2");
    igProgressBar(l2, (ImVec2){60,0}, "L2"); igSameLine(0,-1);
    igProgressBar(r2, (ImVec2){60,0}, "R2");

    // sticks
    float lx = gamepad.get(gid,"lx"), ly = gamepad.get(gid,"ly");
    float rx = gamepad.get(gid,"rx"), ry = gamepad.get(gid,"ry");
    igText("Left stick:  x=%.2f y=%.2f", lx, ly);
    igText("Right stick: x=%.2f y=%.2f", rx, ry);

    // dpad
    igText("Dpad: ");
    key_pill("^", gamepad.held(gid,"^"));
    key_pill("v", gamepad.held(gid,"v"));
    key_pill("<", gamepad.held(gid,"<"));
    key_pill(">", gamepad.held(gid,">"));
    igNewLine();

    // stick visualisation (tiny 2D plot)
    ImVec2 cursor = igGetCursorScreenPos();
    float sz = 50;
    ImDrawList *dl = igGetWindowDrawList();
    // left stick box
    ImDrawList_AddRect(dl,
        (ImVec2){cursor.x,     cursor.y},
        (ImVec2){cursor.x+sz,  cursor.y+sz},
        0xFFAAAAAA, 0,0, 1);
    ImDrawList_AddCircleFilled(dl,
        (ImVec2){cursor.x+sz*0.5f + lx*sz*0.45f,
                 cursor.y+sz*0.5f + ly*sz*0.45f},
        5, 0xFF44FF44, 12);
    // right stick box
    ImDrawList_AddRect(dl,
        (ImVec2){cursor.x+sz+8,    cursor.y},
        (ImVec2){cursor.x+sz+8+sz, cursor.y+sz},
        0xFFAAAAAA, 0,0, 1);
    ImDrawList_AddCircleFilled(dl,
        (ImVec2){cursor.x+sz+8+sz*0.5f + rx*sz*0.45f,
                 cursor.y+sz*0.5f       + ry*sz*0.45f},
        5, 0xFF4444FF, 12);
    igDummy((ImVec2){sz*2+8, sz+4});
}

// ---------------------------------------------------------------------------
// touch visualiser (drawn directly onto the renderer)

static void draw_touch_vis(void) {
    int n = touch.count();
    for(int i = 0; i < n && i < 10; i++) {
        if(!touch.held(i)) continue;
        float tx = touch.x(i);
        float ty = touch.y(i);
        float pr = touch.pressure(i);
        // outer ring
        unsigned col = color.rgba(80, 200, 255, 200);
        float r = 20 + pr * 20;
        draw_rectb(tx-r, ty-r, r*2, r*2, col);
        // center dot
        draw_rect(tx-4, ty-4, 8, 8, color.rgba(255,255,255,220));
    }
    // pinch / rotate indicators
    float pinch  = touch.pinch();
    float rot    = touch.rotate();
    if(pinch  != 0) draw_rect(20, 20, 8 + pinch*200, 8, color.hex("#ff0"));
    if(rot    != 0) draw_rect(20, 36, 8 + rot*2,     8, color.hex("#f80"));
}

// ---------------------------------------------------------------------------
// main demo

void main(event ev) {

    // named action bindings (set up once)
    static bool bindings_ready = 0;

    if(ev.init) {
        if(!render.open(0,0, 0.85, 0)) app.quit(-1);
        window.title("kit.input demo");
        imgui.open();

        // high-level bindings
        input.bind("quit",       "keyboard(escape)");
        input.bind("fullscreen", "keyboard(F11)");
        input.bind("jump",       "keyboard(space) || gamepad(0,south)");
        input.bind("fire",       "mouse(left) || gamepad(0,east)");
        input.bind("move_right", "keyboard(right) || keyboard(d)");
        input.bind("move_left",  "keyboard(left)  || keyboard(a)");
        input.bind("move_up",    "keyboard(up)    || keyboard(w)");
        input.bind("move_down",  "keyboard(down)  || keyboard(s)");
        input.bind("dash",       "keyboard(left shift) || gamepad(0,L1)");
        bindings_ready = 1;
    }

    if(ev.emit) {
        SDL_Event *e = imgui.event(ev.emit);
        if(!e) return;
        if(e->type == SDL_EVENT_QUIT) app.quit(0);
    }

    if(ev.tick) {
        // high-level actions
        if(input.action("quit"))       app.quit(0);
        if(input.action("fullscreen")) window_fullscreen(window_fullscreen(-1)^1);

        // --- render background ---
        render.clear(color.hex("#0a0a14"));

        // draw touch points directly on screen (before imgui)
        draw_touch_vis();

        // draw mouse crosshair
        draw_mouse_vis(0, 0);

        // --- imgui panels ---
        imgui.begin();

        // ================================================================
        // KEYBOARD panel
        // ================================================================
        igSetNextWindowPos((ImVec2){10,10}, ImGuiCond_Once, (ImVec2){0,0});
        igSetNextWindowSize((ImVec2){360, 400}, ImGuiCond_Once);
        igBegin("Keyboard", NULL, 0);

        section("State (WASD + arrows)");
        igText("held(w): "); igSameLine(0,-1); key_pill("W", keyboard.held("w")); igNewLine();
        igText("held(a): "); igSameLine(0,-1); key_pill("A", keyboard.held("a")); igNewLine();
        igText("held(s): "); igSameLine(0,-1); key_pill("S", keyboard.held("s")); igNewLine();
        igText("held(d): "); igSameLine(0,-1); key_pill("D", keyboard.held("d")); igNewLine();

        section("Transitions");
        label("down(space)  : %s", keyboard.down("space")  ? "DOWN"    : "-");
        label("held(space)  : %s", keyboard.held("space")  ? "HELD"    : "-");
        label("up(space)    : %s", keyboard.up("space")    ? "UP"      : "-");
        label("tapped(space): %s", keyboard.tapped("space")? "TAPPED"  : "-");
        label("idle(space)  : %s", keyboard.idle("space")  ? "idle"    : "-");

        section("repeat(r, 30 frames)");
        label("repeat(r,30) : %s", keyboard.repeat("r",30) ? "REPEAT!" : "-");

        section("any key");
        label("any()        : %s", keyboard.any() ? "YES" : "no");

        section("chord");
        label("ctrl+z held  : %s", (keyboard.held("left ctrl") && keyboard.held("z")) ? "YES" : "no");

        igEnd();

        // ================================================================
        // MOUSE panel
        // ================================================================
        igSetNextWindowPos((ImVec2){380,10}, ImGuiCond_Once, (ImVec2){0,0});
        igSetNextWindowSize((ImVec2){280, 280}, ImGuiCond_Once);
        igBegin("Mouse", NULL, 0);

        section("Position");
        label("x=%.1f  y=%.1f", mouse.get("x"), mouse.get("y"));
        label("dx=%.2f dy=%.2f", mouse.delta("x"), mouse.delta("y"));
        label("wheel=%.1f",      mouse.get("wheel"));

        section("Buttons");
        igText("left  : "); igSameLine(0,-1); key_pill("L", mouse.held("left"));   igNewLine();
        igText("right : "); igSameLine(0,-1); key_pill("R", mouse.held("right"));  igNewLine();
        igText("middle: "); igSameLine(0,-1); key_pill("M", mouse.held("middle")); igNewLine();

        section("Transitions");
        label("down(left)   : %s", mouse.down("left")  ? "DOWN"   : "-");
        label("up(left)     : %s", mouse.up("left")    ? "UP"     : "-");
        label("tapped(left) : %s", mouse.tapped("left")? "TAPPED" : "-");

        section("Cursor shape");
        igText("click buttons 1-8 to change cursor");
        for(int i=0;i<8;i++) {
            char lbl[4]; SDL_snprintf(lbl,sizeof lbl,"%d",i);
            if(igButton(lbl,(ImVec2){0,0})) mouse.cursor(i);
            igSameLine(0,-1);
        }
        igNewLine();

        igEnd();

        // ================================================================
        // GAMEPAD panel
        // ================================================================
        igSetNextWindowPos((ImVec2){670,10}, ImGuiCond_Once, (ImVec2){0,0});
        igSetNextWindowSize((ImVec2){300, 320}, ImGuiCond_Once);
        igBegin("Gamepad", NULL, 0);
        draw_gamepad_vis(0, 0, 0);
        igEnd();

        // ================================================================
        // TOUCH panel
        // ================================================================
        igSetNextWindowPos((ImVec2){10, 420}, ImGuiCond_Once, (ImVec2){0,0});
        igSetNextWindowSize((ImVec2){360, 220}, ImGuiCond_Once);
        igBegin("Touch", NULL, 0);

        section("Fingers");
        label("count() = %d", touch.count());
        for(int i=0;i<4;i++) {
            label("finger[%d]: held=%s  x=%.2f y=%.2f  pressure=%.2f  tapped=%s",
                i,
                touch.held(i)   ? "YES" : "no",
                touch.x(i), touch.y(i),
                touch.pressure(i),
                touch.tapped(i) ? "TAP" : "-");
        }

        section("Frame-delta gestures");
        label("pinch()     = %.3f", touch.pinch());
        label("rotate()    = %.3f deg", touch.rotate());
        label("swipe_x(0)  = %.3f", touch.swipe_x(0));
        label("swipe_y(0)  = %.3f", touch.swipe_y(0));

        igEnd();

        // ================================================================
        // GESTURE panel
        // ================================================================
        igSetNextWindowPos((ImVec2){380, 300}, ImGuiCond_Once, (ImVec2){0,0});
        igSetNextWindowSize((ImVec2){280, 200}, ImGuiCond_Once);
        igBegin("Gesture", NULL, 0);

        section("Directional swipes");
        label("swipe_up()    : %s", gesture.swipe_up()    ? "<<< SWIPE >>>" : "-");
        label("swipe_down()  : %s", gesture.swipe_down()  ? "<<< SWIPE >>>" : "-");
        label("swipe_left()  : %s", gesture.swipe_left()  ? "<<< SWIPE >>>" : "-");
        label("swipe_right() : %s", gesture.swipe_right() ? "<<< SWIPE >>>" : "-");

        section("Taps");
        label("tap(1)        : %s", gesture.tap(1)        ? "TAP!"  : "-");
        label("tap(2)        : %s", gesture.tap(2)        ? "TAP!"  : "-");
        label("double_tap(1) : %s", gesture.double_tap(1) ? "DTAP!" : "-");

        section("Dollar gesture");
        igText("(hold R to record 'circle', then draw)");
        if(keyboard.held("r")) gesture.record("circle");
        label("recognized('circle') = %.2f", gesture.recognized("circle"));

        igEnd();

        // ================================================================
        // INPUT bindings panel
        // ================================================================
        igSetNextWindowPos((ImVec2){670, 340}, ImGuiCond_Once, (ImVec2){0,0});
        igSetNextWindowSize((ImVec2){300, 300}, ImGuiCond_Once);
        igBegin("Input (high-level)", NULL, 0);

        section("Bound actions");
        label("jump        : %s", input.action("jump")       > 0 ? "YES" : "no");
        label("fire        : %s", input.action("fire")       > 0 ? "YES" : "no");
        label("dash        : %s", input.action("dash")       > 0 ? "YES" : "no");
        label("move_right  : %s", input.action("move_right") > 0 ? "YES" : "no");
        label("move_left   : %s", input.action("move_left")  > 0 ? "YES" : "no");
        label("move_up     : %s", input.action("move_up")    > 0 ? "YES" : "no");
        label("move_down   : %s", input.action("move_down")  > 0 ? "YES" : "no");

        section("Ad-hoc eval");
        label("eval(\"a && d\")    = %.0f", input.eval("keyboard(a) && keyboard(d)"));
        label("eval(\"any key\")   = %.0f", input.eval("__any_key()"));

        section("Rebind demo");
        igText("Press 'J' to toggle jump->gamepad only");
        static int jump_kb = 1;
        if(keyboard.tapped("j")) {
            jump_kb = !jump_kb;
            if(jump_kb) input.bind("jump","keyboard(space) || gamepad(0,south)");
            else        input.bind("jump","gamepad(0,south)");
        }
        label("jump binding: %s", jump_kb ? "kb+gp" : "gp only");

        section("Unbind");
        igText("Press 'U' to unbind all");
        static int unbound = 0;
        if(keyboard.tapped("u") && !unbound) {
            input.unbind("*");
            unbound = 1;
        }
        if(unbound) colored_label(1,0.4f,0.4f,"All bindings cleared!");

        igEnd();

        // ================================================================
        // minimal game object: a box moved by bindings
        // ================================================================
        {
            static float bx = 400, by = 500;
            float speed = 3.f;
            if(input.action("move_right") > 0) bx += speed;
            if(input.action("move_left")  > 0) bx -= speed;
            if(input.action("move_up")    > 0) by -= speed;
            if(input.action("move_down")  > 0) by += speed;

            unsigned col = (input.action("jump") > 0) ? color.hex("#ff4") :
                           (input.action("fire") > 0) ? color.hex("#f44") :
                           (input.action("dash") > 0) ? color.hex("#4af") :
                           color.hex("#4f8");
            draw_rect(bx-16, by-16, 32, 32, col);
            draw_rectb(bx-17, by-17, 34, 34, color.hex("#fff"));
        }

        imgui.end();
        render.present();
    }

    if(ev.quit) {
        input.unbind("*");
        imgui.close();
    }
}


// undecorated chrome-styled window with custom titlebar and buttons. window can be resized and dragged too.
// - rlyeh, public domain

#include "kit.h"

#define TITLEBAR_HEIGHT 32
#define BUTTON_WIDTH 46
#define BUTTON_HEIGHT TITLEBAR_HEIGHT
#define WINDOW_BORDER 5

typedef struct {
    SDL_Rect close;
    SDL_Rect maximize;
    SDL_Rect minimize;
} TitlebarButtons;

// ----------------------------------------------------------------------------
// titlebar layout

TitlebarButtons get_titlebar_buttons(int window_width) {
    TitlebarButtons b;

    b.close    = (SDL_Rect){ window_width - BUTTON_WIDTH, 0, BUTTON_WIDTH, BUTTON_HEIGHT };
    b.maximize = (SDL_Rect){ window_width - BUTTON_WIDTH * 2, 0, BUTTON_WIDTH, BUTTON_HEIGHT };
    b.minimize = (SDL_Rect){ window_width - BUTTON_WIDTH * 3, 0, BUTTON_WIDTH, BUTTON_HEIGHT };

    return b;
}

// ----------------------------------------------------------------------------
// hit test (drag + resize)

SDL_HitTestResult hit_test_callback(SDL_Window *window, const SDL_Point *pt, void *data) {
    int w, h;
    SDL_GetWindowSize(window, &w, &h);

    TitlebarButtons btns = get_titlebar_buttons(w);

    bool left   = pt->x < WINDOW_BORDER;
    bool right  = pt->x >= w - WINDOW_BORDER;
    bool top    = pt->y < WINDOW_BORDER;
    bool bottom = pt->y >= h - WINDOW_BORDER;

    // Resize zones
    if (top && left)     return SDL_HITTEST_RESIZE_TOPLEFT;
    if (top && right)    return SDL_HITTEST_RESIZE_TOPRIGHT;
    if (bottom && left)  return SDL_HITTEST_RESIZE_BOTTOMLEFT;
    if (bottom && right) return SDL_HITTEST_RESIZE_BOTTOMRIGHT;

    if (left)   return SDL_HITTEST_RESIZE_LEFT;
    if (right)  return SDL_HITTEST_RESIZE_RIGHT;
    if (top)    return SDL_HITTEST_RESIZE_TOP;
    if (bottom) return SDL_HITTEST_RESIZE_BOTTOM;

    // Titlebar drag (excluding buttons)
    if (pt->y < TITLEBAR_HEIGHT) {
        if (SDL_PointInRect(pt, &btns.close) ||
            SDL_PointInRect(pt, &btns.maximize) ||
            SDL_PointInRect(pt, &btns.minimize)) {
            return SDL_HITTEST_NORMAL;
        }
        return SDL_HITTEST_DRAGGABLE;
    }

    return SDL_HITTEST_NORMAL;
}

// ----------------------------------------------------------------------------
// button handling

void handle_titlebar_click(SDL_Window* hwnd, int x, int y, int clicks) {
    int w;
    SDL_GetWindowSize(hwnd, &w, NULL);

    SDL_Point p = {x, y};
    TitlebarButtons btns = get_titlebar_buttons(w);

    if( SDL_PointInRect(&p, &btns.minimize) ) {
        window.maximize(0); // minimize
    }
    else if( SDL_PointInRect(&p, &btns.maximize) || (y < TITLEBAR_HEIGHT && clicks == 2) ) { // if button clicked, or double-clicked titlebar
        // if maximized, restore; else maximize
        window.maximize(window.maximize(-1) >= 1 ? 0.5 : 1.0);
    }
    else if( SDL_PointInRect(&p, &btns.close) ) {
        app.quit(0);
    }
}

// ----------------------------------------------------------------------------
// visual button debugging

void visualize(void) {
    int w, h;
    SDL_GetWindowSize(window.handle, &w, &h);

    TitlebarButtons btns = get_titlebar_buttons(w);

    // Background
    SDL_Renderer *r = render.handle;
    SDL_SetRenderDrawColor(r, 30, 30, 30, 255);
    SDL_RenderClear(r);

    // Titlebar
    SDL_FRect title = {0, 0, w, TITLEBAR_HEIGHT};
    SDL_SetRenderDrawColor(r, 45, 45, 45, 255);
    SDL_RenderFillRect(r, &title);

    // Buttons
    SDL_FRect b1 = { btns.close.x, btns.close.y, btns.close.w, btns.close.h };
    SDL_SetRenderDrawColor(r, 200, 80, 80, 255); // close
    SDL_RenderFillRect(r, &b1);

    SDL_FRect b2 = { btns.maximize.x, btns.maximize.y, btns.maximize.w, btns.maximize.h };
    SDL_SetRenderDrawColor(r, 80, 200, 120, 255); // maximize
    SDL_RenderFillRect(r, &b2);

    SDL_FRect b3 = { btns.minimize.x, btns.minimize.y, btns.minimize.w, btns.minimize.h };
    SDL_SetRenderDrawColor(r, 200, 200, 80, 255); // minimize
    SDL_RenderFillRect(r, &b3);

    SDL_RenderPresent(r);
}

// ----------------------------------------------------------------------------
// main

void main(event ev) {
    if( ev.init ) {
        int w = display.width(0) * 0.85;
        int h = display.height(0) * 0.85;
        int flags = SDL_WINDOW_BORDERLESS | SDL_WINDOW_RESIZABLE;

        if(!SDL_CreateWindowAndRenderer("Undecorated window", w, h, flags, &window.handle, &render.handle))
            app.quit(-1);

        SDL_SetWindowHitTest(window.handle, hit_test_callback, NULL);
    }

    if( ev.emit ) {
        if (ev.emit->type == SDL_EVENT_QUIT)
            app.quit(0);

        if (ev.emit->type == SDL_EVENT_MOUSE_BUTTON_DOWN &&
            ev.emit->button.button == SDL_BUTTON_LEFT) {
            handle_titlebar_click(window.handle,
                                  ev.emit->button.x,
                                  ev.emit->button.y,
                                  ev.emit->button.clicks);
        }
    }

    if( ev.tick ) {
        visualize();
    }
}

const char *hints;

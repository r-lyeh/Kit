#include <kit.h>
#include <tigr/tigr.h>
#include <tigr/tigr.c>

Tigr *app[32];

void* window_handle(int id) {
    return app[id];
}

int window_create(int id) {
    if( !app[id] ) {
        app[id] = tigrWindow(320, 240, "hi", 0);
    }
    return 1;
}

int window_ready(int id) {
    if( app[id] && tigrClosed(app[id]) ) {
        window_destroy(id);
    }
    return app[id] && !tigrClosed(app[id]);
}

void window_clear(int id, unsigned color) {
    if( color ) {
        union { TPixel p; unsigned c; } u; u.c = color;
        tigrClear(app[id], u.p);
    }
}

int window_swap(int id) {
    tigrUpdate(app[id]);
    return 1;
}

int window_destroy(int id) {
    if( app[id] ) {
        tigrFree(app[id]);
        app[id] = 0;
    }
    return 0;
}

int window_count() {
    int count = 0;
    for( int i = 0; i < countof(app); ++i ) {
        count += window_ready(i);
    }
    return count;
}

int key(int id, int ch) {
    if( app[id] ) {
    /**/ if( ch == '<' ) ch = TK_LEFT;
    else if( ch == '>' ) ch = TK_RIGHT;
    return tigrKeyDown(app[id], ch);
    return tigrKeyHeld(app[id], ch);
    }
    return 0;
}

int print(int id, const char *fmt, ...) {
    tigrPrint(app[id], tfont, 10,10, tigrRGBA(255,255,255,255), fmt);
    return 1;
}

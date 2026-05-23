extern struct dd {
void        (*color)(unsigned rgba);
void        (*text)(float x, float y, int shadow, text string);
void        (*log)(text message);
} dd;

void dd_color(unsigned rgba) {
    SDL_SetRenderDrawColor(render.handle, color.r(rgba), color.g(rgba), color.b(rgba), SDL_ALPHA_OPAQUE);
}
void dd_text(float x, float y, int shadow, text string) {
    //SDL_RenderCoordinatesFromWindow(render.handle, x, y, &x, &y);
    SDL_SetRenderScale(render.handle, 4.0f, 4.0f); x/=4, y/=4;
    if( shadow ) {
        uint8_t r,g,b,a;
        SDL_GetRenderDrawColor(render.handle, &r, &g, &b, &a);
        SDL_SetRenderDrawColor(render.handle, 0, 0, 0, a);
        SDL_RenderDebugText(render.handle, x+1, y+1, string);
        SDL_SetRenderDrawColor(render.handle, r, g, b, a);
    }
    SDL_RenderDebugText(render.handle, x, y, string);
    SDL_SetRenderScale(render.handle, 1.0f, 1.0f);
}

// ----------------------------------------------------------------------------

int     dd_print(int x, int y, int step, float alpha, const char *str); // returns ending Y
void    dd_log(const char *msg);
#define dd_log(...) dd_log(va(__VA_ARGS__))
void    dd_flush_log(void);

typedef struct DebugMessage {
    struct DebugMessage *next;
    uint64_t begin_tick, end_tick;
    char str[];
} DebugMessage;

static DebugMessage first = {0};
static DebugMessage *last = &first;

int dd_print(int x, int y, int step, float alpha, const char *str) {
    SDL_Renderer* handle = render.handle;
    alpha *= 255;
    for each_string(line, str, "\r\n") {
        /**/ if( *line == 1 ) SDL_SetRenderDrawColor(handle, 0xf,0,0xc6,alpha), ++line;
        else if( *line == 2 ) SDL_SetRenderDrawColor(handle, 0xf,0x32,0,alpha), ++line;
        else if( *line == 3 ) SDL_SetRenderDrawColor(handle, 0xb9,0,0x53,alpha), ++line;
        else if( *line == 4 ) SDL_SetRenderDrawColor(handle, 0x54,0xdf,0,alpha), ++line;
        else if( *line == 5 ) SDL_SetRenderDrawColor(handle, 0,0xc8,0xff,alpha), ++line;
        else if( *line == 6 ) SDL_SetRenderDrawColor(handle, 0xf3,0xe8,1,alpha), ++line;
        else if( *line == 7 ) SDL_SetRenderDrawColor(handle, 0xe6,0xe6,0xff,alpha), ++line;
        else                  SDL_SetRenderDrawColor(handle, 0xe6,0xe6,0xff,alpha);
        SDL_RenderDebugText(handle, x, y, line);
        y += step;
    }
    return y;
}
void (dd_log)(const char *msg) {
    uint64_t now = SDL_GetTicks(); // time_ns();
    os.log("%s", msg);

    int length = strlen(msg) + 1; // length = str + NUL
    DebugMessage *self = SDL_malloc(sizeof(DebugMessage) + length);

    self->next = 0;
    self->end_tick = (self->begin_tick = now) + 3000; // 3e9;
    memcpy(self->str, msg, length);

    last->next = self, last = self;
}

void dd_flush_log(void) {
    const uint64_t now = SDL_GetTicks();

    float2 size = render.size();
    int x = 0, y = 0, h = size.h;

    SDL_SetRenderClipRect(render.handle, NULL);

    DebugMessage *prev = &first;
    for( DebugMessage *msg = first.next; msg ; ) {
        DebugMessage *next = msg->next;

        double lifetime = (now - msg->begin_tick) / (double)(msg->end_tick - msg->begin_tick);
        if( lifetime > 1) lifetime = 1.f;

        h = dd_print(x, h, -8, 1.0f - lifetime, msg->str);

        // delete
        if( lifetime >= 1 ) {
            prev->next = next;
            SDL_free(msg);
            msg = next;            
        } else {
            prev = msg;
            msg = next;
        }
    }

    if (first.next == NULL) {
        last = &first;
    }
}

struct dd dd = { .color = dd_color, .text = dd_text };

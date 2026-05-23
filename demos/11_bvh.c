// Morton BVH Collision Detection Demo
// SDL3 + C port of Matthias Müller's Ten Minute Physics #24
//
// Top-down 2D wireframe view. Green boxes = no collision, Red = colliding.

#include "kit.h"
const char *hints;

// -------------------------------------------------------------
// Constants

#define BOX_COUNT      10000
#define WORLD_SIZE     100.0f
#define MIN_BOX_SIZE   0.5f
#define MAX_BOX_SIZE   3.0f
#define MAX_SPEED      0.3f

#define WINDOW_W  1280
#define WINDOW_H   720

// -------------------------------------------------------------
// Math helpers

static inline float frand(void) { return (float)rand() / (float)RAND_MAX; }
static inline float fminf2(float a, float b) { return a < b ? a : b; }
static inline float fmaxf2(float a, float b) { return a > b ? a : b; }
static inline float fsignf(float x) { return x >= 0.0f ? 1.0f : -1.0f; }

// -------------------------------------------------------------
// Morton code (30-bit, 10 bits per axis)

static uint32_t expand_bits(uint32_t v) {
    v = (v * 0x00010001u) & 0xFF0000FFu;
    v = (v * 0x00000101u) & 0x0F00F00Fu;
    v = (v * 0x00000011u) & 0xC30C30C3u;
    v = (v * 0x00000005u) & 0x49249249u;
    return v;
}

static uint32_t morton3(float x, float y, float z) {
    /* Normalise to [0, 1] */
    x = (x + WORLD_SIZE * 0.5f) / WORLD_SIZE;
    y = (y + WORLD_SIZE * 0.5f) / WORLD_SIZE;
    z = (z + WORLD_SIZE * 0.5f) / WORLD_SIZE;

    x = fminf2(fmaxf2(x, 0.f), 1.f);
    y = fminf2(fmaxf2(y, 0.f), 1.f);
    z = fminf2(fmaxf2(z, 0.f), 1.f);

    uint32_t xi = (uint32_t)(x * 1023.f);
    uint32_t yi = (uint32_t)(y * 1023.f);
    uint32_t zi = (uint32_t)(z * 1023.f);
    if (xi > 1023) xi = 1023;
    if (yi > 1023) yi = 1023;
    if (zi > 1023) zi = 1023;

    return expand_bits(xi) | (expand_bits(yi) << 1) | (expand_bits(zi) << 2);
}

// -------------------------------------------------------------
// AABB

typedef union { struct { float minx, miny, minz, maxx, maxy, maxz; }; struct { float min[3], max[3]; }; } AABB;

static int aabb_intersect(const AABB *a, const AABB *b) {
    return a->minx <= b->maxx && a->maxx >= b->minx &&
           a->miny <= b->maxy && a->maxy >= b->miny &&
           a->minz <= b->maxz && a->maxz >= b->minz;
}

static void aabb_merge(AABB *dst, const AABB *a, const AABB *b) {
    dst->minx = fminf2(a->minx, b->minx);
    dst->miny = fminf2(a->miny, b->miny);
    dst->minz = fminf2(a->minz, b->minz);
    dst->maxx = fmaxf2(a->maxx, b->maxx);
    dst->maxy = fmaxf2(a->maxy, b->maxy);
    dst->maxz = fmaxf2(a->maxz, b->maxz);
}

// -------------------------------------------------------------
// Box

typedef struct {
    float px, py, pz;   /* position (centre) */
    float vx, vy, vz;   /* velocity */
    float w, h, d;      /* half-extents */
    int   colliding;
} Box;

static void box_init(Box *b) {
    b->w = (MIN_BOX_SIZE + frand() * (MAX_BOX_SIZE - MIN_BOX_SIZE)) * 0.5f;
    b->h = (MIN_BOX_SIZE + frand() * (MAX_BOX_SIZE - MIN_BOX_SIZE)) * 0.5f;
    b->d = (MIN_BOX_SIZE + frand() * (MAX_BOX_SIZE - MIN_BOX_SIZE)) * 0.5f;

    b->px = (frand() - 0.5f) * WORLD_SIZE;
    b->py = (frand() - 0.5f) * WORLD_SIZE;
    b->pz = (frand() - 0.5f) * WORLD_SIZE;

    b->vx = (frand() - 0.5f) * MAX_SPEED * 2.f;
    b->vy = (frand() - 0.5f) * MAX_SPEED * 2.f;
    b->vz = (frand() - 0.5f) * MAX_SPEED * 2.f;

    b->colliding = 0;
}

static void box_aabb(const Box *b, AABB *out) {
    out->minx = b->px - b->w;  out->maxx = b->px + b->w;
    out->miny = b->py - b->h;  out->maxy = b->py + b->h;
    out->minz = b->pz - b->d;  out->maxz = b->pz + b->d;
}

static void box_update(Box *b) {
    b->px += b->vx;
    b->py += b->vy;
    b->pz += b->vz;

    float lim;
    lim = WORLD_SIZE * 0.5f - b->w;
    if (b->px >  lim) { b->vx = -fabsf(b->vx); b->px =  lim; }
    if (b->px < -lim) { b->vx =  fabsf(b->vx); b->px = -lim; }
    lim = WORLD_SIZE * 0.5f - b->h;
    if (b->py >  lim) { b->vy = -fabsf(b->vy); b->py =  lim; }
    if (b->py < -lim) { b->vy =  fabsf(b->vy); b->py = -lim; }
    lim = WORLD_SIZE * 0.5f - b->d;
    if (b->pz >  lim) { b->vz = -fabsf(b->vz); b->pz =  lim; }
    if (b->pz < -lim) { b->vz =  fabsf(b->vz); b->pz = -lim; }

    b->colliding = 0;
}

// -------------------------------------------------------------
// BVH  (flat array, 2*N-1 nodes for N leaves)

typedef struct {
    AABB aabb;
    int  left;    /* index into node array; -1 = leaf */
    int  right;
    int  box_id;  /* valid only for leaves */
} BVHNode;

/* Scratch sort list */
typedef struct { int id; uint32_t code; } SortItem;

static int cmp_sort_item(const void *a, const void *b) {
    const SortItem *sa = (const SortItem *)a;
    const SortItem *sb = (const SortItem *)b;
    if (sa->code < sb->code) return -1;
    if (sa->code > sb->code) return  1;
    return 0;
}

/*
 * We build into a pre-allocated flat pool. Returns root index.
 * pool must hold at least 2*n-1 entries.
 */
static int bvh_build_rec(BVHNode *pool, int *next,
                          SortItem *list, int begin, int end,
                          const Box *boxes) {
    int idx = (*next)++;

    if (begin == end) {
        /* Leaf */
        int bid = list[begin].id;
        box_aabb(&boxes[bid], &pool[idx].aabb);
        pool[idx].left   = -1;
        pool[idx].right  = -1;
        pool[idx].box_id = bid;
        return idx;
    }

    int mid = (begin + end) / 2;
    int l = bvh_build_rec(pool, next, list, begin, mid, boxes);
    int r = bvh_build_rec(pool, next, list, mid+1, end, boxes);

    aabb_merge(&pool[idx].aabb, &pool[l].aabb, &pool[r].aabb);
    pool[idx].left   = l;
    pool[idx].right  = r;
    pool[idx].box_id = -1;
    return idx;
}

static int bvh_build(BVHNode *pool, SortItem *scratch,
                     const Box *boxes, int n) {
    for (int i = 0; i < n; i++) {
        scratch[i].id   = i;
        scratch[i].code = morton3(boxes[i].px, boxes[i].py, boxes[i].pz);
    }
    qsort(scratch, n, sizeof(SortItem), cmp_sort_item);
    int next = 0;
    return bvh_build_rec(pool, &next, scratch, 0, n-1, boxes);
}

// -------------------------------------------------------------
// Collision traversal

static long long g_check_count;

static void find_collisions(int query_id, const AABB *query_aabb,
                             int node_idx, const BVHNode *pool,
                             const Box *boxes,
                             int *colliding_flags) {
    const BVHNode *node = &pool[node_idx];

    if (!aabb_intersect(query_aabb, &node->aabb)) return;

    if (node->left == -1) {
        /* Leaf */
        g_check_count++;
        if (node->box_id != query_id) {
            AABB other;
            box_aabb(&boxes[node->box_id], &other);
            if (aabb_intersect(query_aabb, &other)) {
                colliding_flags[query_id]  = 1;
                colliding_flags[node->box_id] = 1;
            }
        }
        return;
    }

    find_collisions(query_id, query_aabb, node->left,  pool, boxes, colliding_flags);
    find_collisions(query_id, query_aabb, node->right, pool, boxes, colliding_flags);
}

// -------------------------------------------------------------
// HUD text via SDL_RenderDebugText (SDL3 built-in, no TTF needed)

static void hud_line(SDL_Renderer *ren, int x, int *y, const char *text) {
    SDL_RenderDebugText(ren, (float)x, (float)*y, text);
    *y += 16;
}

// -------------------------------------------------------------
// Main

void main(event ev) {
    /* Allocated boxes */
    static Box *boxes;

    /* BVH pool: 2*N - 1 nodes worst case */
    static int bvh_pool_size;
    static BVHNode  *bvh_pool;
    static SortItem *scratch ;
    static int      *colliding_flags;

    /* Stats */
    static long long frame_count    = 0;
    static int       fps            = 0;
    static int       collision_count = 0;
    static double    build_ms       = 0.0;
    static double    check_ms       = 0.0;
    static double    render_ms      = 0.0;
    static long long bvh_checks     = 0;
    static uint64_t  fps_tick       = 0;
    static uint64_t  fps_frames     = 0;

    if( ev.init ) {
        //srand((unsigned)time(NULL));

        if(!render.open(WINDOW_W,WINDOW_H,0.85,0))
            app.quit(-1);

        SDL_SetRenderVSync(render.handle, 1);

        dd_createContext(render.handle);

        /* Allocate boxes */
        boxes = (Box *)SDL_malloc(BOX_COUNT * sizeof(Box));
        for (int i = 0; i < BOX_COUNT; i++) box_init(&boxes[i]);

        /* BVH pool: 2*N - 1 nodes worst case */
        bvh_pool_size = 2 * BOX_COUNT;
        bvh_pool = (BVHNode  *)SDL_malloc(bvh_pool_size * sizeof(BVHNode));
        scratch  = (SortItem *)SDL_malloc(BOX_COUNT * sizeof(SortItem));
        colliding_flags = (int *)SDL_malloc(BOX_COUNT * sizeof(int));

        fps_tick = SDL_GetTicks();
    }

    if( ev.emit ) {
        if (ev.emit->type == SDL_EVENT_QUIT) app.quit(0);
    }

    if( ev.tick ) {
        /* ---- Update boxes ---- */
        for (int i = 0; i < BOX_COUNT; i++) box_update(&boxes[i]);
        SDL_memset(colliding_flags, 0, BOX_COUNT * sizeof(int));

        /* ---- Build BVH ---- */
        uint64_t t0 = SDL_GetTicksNS();
        int root = bvh_build(bvh_pool, scratch, boxes, BOX_COUNT);
        uint64_t t1 = SDL_GetTicksNS();
        build_ms = (t1 - t0) * 1e-6;

        /* ---- Collision detection ---- */
        g_check_count = 0;
        uint64_t t2 = SDL_GetTicksNS();
        for (int i = 0; i < BOX_COUNT; i++) {
            AABB qa;
            box_aabb(&boxes[i], &qa);
            find_collisions(i, &qa, root, bvh_pool, boxes, colliding_flags);
        }
        uint64_t t3 = SDL_GetTicksNS();
        check_ms = (t3 - t2) * 1e-6;
        bvh_checks = g_check_count;

        /* Count collisions and propagate flag to boxes */
        collision_count = 0;
        for (int i = 0; i < BOX_COUNT; i++) {
            boxes[i].colliding = colliding_flags[i];
            if (colliding_flags[i]) collision_count++;
        }

        /* ---- FPS ---- */
        fps_frames++;
        uint64_t now = SDL_GetTicks();
        if (now - fps_tick >= 1000) {
            fps = (int)(fps_frames * 1000 / (now - fps_tick));
            fps_frames = 0;
            fps_tick = now;
        }

        /* ---- Render ---- */
        SDL_SetRenderDrawColor(render.handle, 15, 15, 20, 255);
        SDL_RenderClear(render.handle);

        /* Draw boxes */
        uint64_t t4 = SDL_GetTicksNS();
        int last = -1;
        dd_beginFrame(now/1e6, WINDOW_W, WINDOW_H);
        for (int i = 0; i < BOX_COUNT; i++) {
            float sx = boxes[i].px - boxes[i].w;
            float ex = boxes[i].px + boxes[i].w;
            float sy = boxes[i].py - boxes[i].h;
            float ey = boxes[i].py + boxes[i].h;
            float sz = boxes[i].pz - boxes[i].d;
            float ez = boxes[i].pz + boxes[i].d;

            if( boxes[i].colliding != last ) {
                last = boxes[i].colliding;
                dd_color(last ? DD_RED : DD_GREEN);
            }
            dd_aabb(dd3(sx,sy,sz), dd3(ex,ey,ez));
        }
        dd_endFrame();
        uint64_t t5 = SDL_GetTicksNS();
        render_ms = (t5 - t4) * 1e-6;

        /* ---- HUD ---- */
        SDL_FRect hud_bg = { 2, 2, 190, 145 };
        SDL_SetRenderDrawColor(render.handle, 0, 0, 0, 255);
        SDL_RenderFillRect(render.handle, &hud_bg);

        char buf[64];
        int hy = 10;

        SDL_SetRenderDrawColorFloat(render.handle, 0.9f, 0.9f, 1.0f, 1.0f);
        hud_line(render.handle, 8, &hy, "Morton BVH " ifdef(KIT_RELEASE,"(release)","(debug)"));

        SDL_SetRenderDrawColorFloat(render.handle, 0.7f, 0.9f, 0.7f, 1.0f);
        SDL_snprintf(buf, sizeof(buf), "Boxes      : %d", BOX_COUNT);
        hud_line(render.handle, 8, &hy, buf);

        SDL_snprintf(buf, sizeof(buf), "Collisions : %d", collision_count);
        hud_line(render.handle, 8, &hy, buf);

        SDL_snprintf(buf, sizeof(buf), "BVH build  : %.2f ms", build_ms);
        hud_line(render.handle, 8, &hy, buf);

        SDL_snprintf(buf, sizeof(buf), "Col check  : %.2f ms", check_ms);
        hud_line(render.handle, 8, &hy, buf);

        SDL_snprintf(buf, sizeof(buf), "Render : %.2f ms", render_ms);
        hud_line(render.handle, 8, &hy, buf);

        SDL_snprintf(buf, sizeof(buf), "BVH checks : %lld", bvh_checks);
        hud_line(render.handle, 8, &hy, buf);

        SDL_snprintf(buf, sizeof(buf), "FPS        : %d", fps);
        hud_line(render.handle, 8, &hy, buf);

        SDL_RenderPresent(render.handle);
        frame_count++;
    }

    if( ev.quit ) {
        SDL_free(boxes);
        SDL_free(bvh_pool);
        SDL_free(scratch);
        SDL_free(colliding_flags);
    }
}

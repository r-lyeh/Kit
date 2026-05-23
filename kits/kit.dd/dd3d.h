// debugdraw utilities for sdl3_renderer. based on work by @glampert (public domain)
// - rlyeh, public domain

// [x] primitives: 
// [x] flotilla-style 3d locators
// [x] text 2d, text 3d
// [x] perfect 3d spheres, and almost perfect 3d capsules
// [x] variable thickness
// [x] animated stipples and dashes
// [x] bitmasked groups
// [x] expire time
// [x] distance-based fade out

#ifndef DD_H
#define DD_H "0.0.0"

#include <SDL3/SDL.h>

// ----------------------------------------------------------------------
// Math types

typedef struct { float x, y; }       dd_vec2;
typedef struct { float x, y, z; }    dd_vec3;
typedef struct { float x, y, z, w; } dd_vec4;
typedef struct { float m[4][4]; }    dd_mat4;   // column-major

// ----------------------------------------------------------------------
// Camera

typedef struct {
    dd_vec3 position;
    dd_vec3 target;
    dd_vec3 up;
    float   fov_y;
    float   near_z;
    float   far_z;
} dd_camera_t;

// ----------------------------------------------------------------------
// Color helpers

typedef SDL_Color dd_rgba;

#define DD_RGB(r,g,b)    ((dd_rgba){(r),(g),(b),255})
#define DD_RGBA(r,g,b,a) ((dd_rgba){(r),(g),(b),(a)})

#if 0
static const dd_rgba DD_BLUE    = { 80, 160, 255, 255};
static const dd_rgba DD_RED     = {255,  50,  50, 255};
static const dd_rgba DD_PINK    = {220,  80, 220, 255};
static const dd_rgba DD_GREEN   = { 50, 230,  80, 255};
static const dd_rgba DD_CYAN    = { 50, 230, 230, 255};
static const dd_rgba DD_YELLOW  = {255, 230,  50, 255};
static const dd_rgba DD_WHITE   = {255, 255, 255, 255};
static const dd_rgba DD_ORANGE  = {255, 160,  30, 255};
static const dd_rgba DD_GRAY    = {160, 160, 160, 255};
static const dd_rgba DD_PINK    = {255,  80, 160, 255};
static const dd_rgba DD_PURPLE  = {160,  60, 220, 255};
#else
static const dd_rgba DD_BLACK       = {  0,  0, 24,255};
static const dd_rgba DD_BLUE        = {  0, 51,225,255};
static const dd_rgba DD_RED         = {140, 15,  0,255}, DD_BROWN1  = {140, 15,  0,255};
static const dd_rgba DD_BROWN2      = {216,146,  1,255};
static const dd_rgba DD_LIME        = {164,233,  0,255}, DD_GREEN2  = {164,233,  0,255};
static const dd_rgba DD_GREEN       = { 16,180, 20,255};
static const dd_rgba DD_CYAN        = {  0,152,196,255}, DD_BLUE2   = {  0,152,196,255};
static const dd_rgba DD_WHITE       = {230,230,255,255}, DD_GRAY    = {230,230,255,255};

static const dd_rgba DD_GRAY2       = {145,145,145,255};
static const dd_rgba DD_PURPLE      = {120,  0,200,255};
static const dd_rgba DD_MAGENTA     = {185,  0, 83,255};
static const dd_rgba DD_ORANGE      = {240, 75,  0,255};
static const dd_rgba DD_YELLOW      = {243,232,  1,255};
static const dd_rgba DD_TURQUOISE   = {  0,232,145,255};
static const dd_rgba DD_CYAN2       = {  0,200,255,255};
static const dd_rgba DD_WHITE2      = {255,255,200,255}, DD_YELLOW2 = {255,255,200,255};

//static const dd_rgba DD_PAL[16]  = {
//    DD_BLACK, DD_BLUE,  DD_RED,  DD_PINK,  DD_GREEN,  DD_CYAN,  DD_YELLOW,  DD_WHITE,
//    DD_GRAY,  DD_BLUE2, DD_RED2, DD_PINK2, DD_GREEN2, DD_CYAN2, DD_YELLOW2, DD_WHITE2
//};
#endif

// ----------------------------------------------------------------------
// Groups  (bitmask, 64 slots)

#define DD_GROUP_COUNT   64
#define DD_GROUP_ALL     UINT64_C(-1)

// ----------------------------------------------------------------------
// Depth fade

typedef struct {
    bool    enabled;
    float   fade_near;
    float   fade_far;
    uint8_t min_alpha;
} dd_depthFade;

// Style  (thickness + stipple)
//
// Usage:
//   dd_setStyle((dd_style){ .thickness=3.f, .stipple=DD_STIPPLE_DASH });
//   dd_box(...);
//   dd_resetStyle();   // back to solid hairline
//
// Thickness: screen-space pixels. 1.0 = hairline (SDL_RenderLine, fastest).
//            >1.0 uses SDL_RenderGeometry quads with mitered caps.
//            Sensible range: 1.0 – 8.0.
//
// Stipple: 8-bit pattern sampled every `stipple_px` screen pixels along
//          the line. Bit 7 = first pixel, bit 0 = last of the period.
//          Predefined patterns below; supply custom uint8_t for others.
// ----------------------------------------------------------------------
//          0xFF = solid (no stippling).

// Predefined stipple patterns (8-bit, period = stipple_px pixels)
#define DD_STIPPLE_SPARSE    0x88u  // █░░░█░░░
#define DD_STIPPLE_DOT       0xAAu  // █░█░█░█░
#define DD_STIPPLE_DASH      0xF0u  // ████░░░░
#define DD_STIPPLE_DASH_DOT  0xE4u  // ███░█░░░
#define DD_STIPPLE_DASH_DOT2 0xF4u  // ████░█░░
#define DD_STIPPLE_SOLID     0xFFu  // ████████

typedef struct {
    float   thickness;        // screen pixels, default 1.0 (hairline)
    uint8_t stipple;          // pattern mask, default DD_STIPPLE_SOLID
    uint8_t stipple_px;       // pixel period for stipple, default 16
    float   stipple_speed;    // animation: bits/second to rotate pattern (Negative == reverse): 0 = static (default). 8.0 = one full rotation per second.
} dd_style;

// Default style: solid hairline, no stipple, no animation
#define DD_STYLE_DEFAULT ((dd_style){ 1.0f, DD_STIPPLE_SOLID, 16, 0.0f })

// ---- Group control ----

typedef struct dd_context dd_context;

dd_context *dd_createContext (SDL_Renderer *renderer);
void        dd_destroyContext(dd_context *ctx);

void  dd_setCamera(const dd_camera_t *cam);
const dd_camera_t *dd_getCamera(void);

void dd_setViewport(int w, int h);
void dd_getViewport(int *w, int *h);

// ---- Group control ----
void dd_setGroup           (int group_id);
void dd_setGroupVisible    (int group_id, bool visible);
void dd_setAllGroupsVisible(bool visible);

// ---- Depth fade ----
void dd_setDepthFade   (dd_depthFade settings);
void dd_enableDepthFade(bool enable);

// ---- Style ----
void dd_setStyle  (dd_style style);
void dd_resetStyle(void);

// ---- Frame ----
void dd_beginFrame(double now_seconds, int width, int height);
void dd_endFrame  (void);

// color
void dd_color(dd_rgba c); // replaces current color
void dd_pushColor(dd_rgba c); // push a new color into stack
void dd_popColors(int num); // pop N colors from stack

// ----------------------------------------------------------------------
// Core primitives

void dd_line    (dd_vec3 a, dd_vec3 b);
void dd_lineDashed(dd_vec3 a, dd_vec3 b);
void dd_point   (dd_vec3 p, float size);
void dd_aabb    (dd_vec3 mn, dd_vec3 mx);
void dd_box     (dd_vec3 center, dd_vec3 half);
void dd_obb     (dd_vec3 center, dd_vec3 axis_x, dd_vec3 axis_y, dd_vec3 axis_z);
void dd_sphere  (dd_vec3 center, float radius);
void dd_sphereEx(dd_vec3 center, float radius, int segments);
void dd_capsule (dd_vec3 base, dd_vec3 tip, float radius);
void dd_cone    (dd_vec3 apex, dd_vec3 base_center, float radius, int segments);
void dd_circle  (dd_vec3 center, dd_vec3 normal, float radius, int segments);
void dd_ray     (dd_vec3 origin, dd_vec3 direction, float length);
void dd_frustum (const dd_mat4 *inv_view_proj);
void dd_triangle(dd_vec3 a, dd_vec3 b, dd_vec3 c);
void dd_normal  (dd_vec3 pos, dd_vec3 normal);

// dd_axes: XYZ cross at origin.
// Positive extents: solid lines (R/G/B).
// Negative extents: dashed lines, same colours at 50% alpha.
void dd_axes(dd_vec3 origin, dd_vec3 axis_x, dd_vec3 axis_y, dd_vec3 axis_z, float length);

// dd_grid: two-pass grid (major + minor lines).
// major_color  : every (cells/subdivisions) lines.
// minor_color  : subdivision lines (use lower alpha for subtlety).
// subdivisions : how many minor cells per major cell.
// Pass subdivisions=1 for a plain single-pass grid.
void dd_grid(dd_vec3 center, float cell_size, int cells, dd_rgba major_color, dd_rgba minor_color, int subdivisions);

// dd_prism: generalised prism/cylinder/pyramid.
// Draws a polygon ring of `segments` sides at `center` in the plane of `normal`, with radius `radius`.
// height > 0 : cone  : lines from ring to apex offset along normal
// height < 0 : prism : extrudes a second ring along -normal*|height|
// height = 0 : ring only (same as dd_circle with a polygon approximation)
void dd_prism(dd_vec3 center, float radius, float height, dd_vec3 normal, int segments);

// dd_aabbCorners: draws only the corner ticks of a bounding box.
// Each edge has half unit-length tick drawn at each end.
// Less visual clutter than full edges when boxes are densely packed.
void dd_aabbCorners(dd_vec3 mn, dd_vec3 mx);

void dd_camera(dd_vec3 position, dd_vec3 forward, dd_vec3 up); // film-camera body: rectangular body, two half-reels, and a trapezoidal lens shield. `forward` and `up` define camera orientation.

void dd_position   (dd_vec3 pos, float radius); // stacked circle rings with a vertical line to the ground (Y=0). Ring count and radius shrink with height. Useful to locate "where is this actor" quickly.
void dd_positionDir(dd_vec3 pos, dd_vec3 dir, float radius); // same but the bottom ring is oriented toward `dir` (shows facing direction).
void dd_boid(dd_vec3 position, dd_vec3 dir); // cheap directional triangle indicator (front/back).
void dd_diamond(dd_vec3 center, dd_vec3 up, float size); // 6-point octahedron / diamond shape. Good as a position marker with optional orientation.

// ----------------------------------------------------------------------
// Timed primitives

void dd_lineTimed   (dd_vec3 a, dd_vec3 b, float lifetime_seconds);
void dd_pointTimed  (dd_vec3 p, float size, float lifetime_seconds);
void dd_sphereTimed (dd_vec3 center, float radius, float lifetime_seconds);
void dd_boxTimed    (dd_vec3 center, dd_vec3 half, float lifetime_seconds);
void dd_rayTimed    (dd_vec3 origin, dd_vec3 dir, float length, float lifetime_seconds);

void dd_clearTimed(void);

// Text
//
// dd_label: screen-space HUD overlay via SDL_RenderDebugText.
//   Projects `pos` to screen, renders at that pixel position.
//   Fixed 8x8 pixel size regardless of world distance.
//   Good for: per-object stat readouts, coordinates, frame counters.
//
// dd_label3d: world-space Hershey simplex vector font.
//   Rendered as 3D line segments on a camera-facing billboard.
//   Scales with `scale` (world units per character height).
//   Affected by depth fade and groups like any other line primitive.
//   Supports '\n' for multi-line.
//   Good for: world annotations, distance labels, named waypoints.
//
// ----------------------------------------------------------------------
// Both support printf-style formatting and *Timed variants.

// 2D screen-space (SDL_RenderDebugText)
void dd_label(dd_vec3 world_pos, const char *fmt, ...);
void dd_labelTimed(dd_vec3 world_pos, float lifetime_seconds, const char *fmt, ...);

// 3D world-space (Hershey vector font, camera-facing)
void dd_label3d(dd_vec3 world_pos, float scale, const char *fmt, ...);
void dd_label3dTimed(dd_vec3 world_pos, float scale, float lifetime_seconds, const char *fmt, ...);

// ----------------------------------------------------------------------
// Stats

typedef struct {
    int lines_submitted;
    int lines_drawn;
    int lines_clipped;
    int lines_culled;
    int timed_lines_active;
    int labels_drawn;
} dd_stats;

const dd_stats *dd_getStats(void);

// dd_gizmo.h  -  Translation / Rotation / Scale gizmos for dd
//
// INTERACTION MODEL
// -----------------
// The gizmo is stateless from the caller's perspective. You own the transform
// and pass it in each frame. The library returns deltas and the hot part.
//
// Typical per-frame loop:
//
//   dd_gizmoDesc g = {
//       .origin = obj.pos, .axis_x={1,0,0}, .axis_y={0,1,0}, .axis_z={0,0,1},
//       .screen_size = 110.0f, .hot_axis = DD_GIZMO_AXIS_NONE,
//   };
//   dd_gizmoInput inp = {
//       .mouse_x=mx, .mouse_y=my,
//       .mouse_down=lmb, .mouse_pressed=lmb_press, .mouse_released=lmb_rel,
//   };
//   dd_gizmoDelta delta = {0};
//   dd_gizmoUpdateTranslate(&g, &inp, &delta);
//   if (delta.active) obj.pos = add3(obj.pos, delta.translate);
//   dd_beginFrame(now, width, height);
//     dd_gizmoTranslate(&g);
//   dd_endFrame();
//
// Returned deltas:
//   translate : world-space vector to ADD to position
//   rotate    : angle in radians to ADD (CCW positive around chosen axis)
//   scale     : multiplicative factor (scale *= delta.scale)

// Axis / part IDs
typedef enum {
    DD_GIZMO_AXIS_NONE = 0,
    DD_GIZMO_AXIS_X,
    DD_GIZMO_AXIS_Y,
    DD_GIZMO_AXIS_Z,
    DD_GIZMO_AXIS_XY,    // plane handle; also 2D screen translate
    DD_GIZMO_AXIS_XZ,
    DD_GIZMO_AXIS_YZ,
    DD_GIZMO_AXIS_ALL,   // uniform scale centre
    DD_GIZMO_AXIS_VIEW,  // view-plane rotate (outer ring)
} dd_gizmoAxis;

// Caller fills; library writes hot_axis back
typedef struct {
    dd_vec3      origin;
    dd_vec3      axis_x, axis_y, axis_z;  // unit local axes
    float        screen_size;   // desired diameter pixels (80-140)
    float        scale;         // uniform scale (ALL handle): also read by dd_GizmoScale
    float        scale_x;      // per-axis scale factors: read by dd_GizmoScale
    float        scale_y;
    float        scale_z;
    dd_gizmoAxis hot_axis;      // WRITTEN by dd_update*, READ by dd_draw*
} dd_gizmoDesc;

// Input snapshot: fill from SDL events each frame.
//
// IMPORTANT: mouse_pressed and mouse_released are edge-triggered (one frame
// only). With static variables in a framework, you MUST clear them at the
// start of each tick, e.g.:
//   inp.mouse_pressed = inp.mouse_released = false;  // top of tick
//   ... process events, set them on the relevant event ...
//   dd_gizmoUpdate*(g, &inp, &delta);
//
// If mouse_pressed stays true across frames, the gizmo thinks every frame
// is a fresh click and drag-release-redrag will not work.
//
// vp_x / vp_y: viewport offset in OS-window pixels (default 0,0).
// Set these if your renderer viewport doesn't start at the window's top-left
// (e.g. an editor side panel, or mouse coords relative to the display).
// Diagnosis: if hover is off by a fixed N pixels in X, set vp_x = N.

typedef struct {
    float mouse_x, mouse_y;
    bool  mouse_down;
    bool  mouse_pressed;   // went down THIS frame only: clear each tick
    bool  mouse_released;  // went up THIS frame only : clear each tick
    float vp_x, vp_y;      // viewport offset within OS window (usually 0,0)
} dd_gizmoInput;

// What changed this frame
typedef struct {
    bool           active;     // true while a drag is in progress
    dd_gizmoAxis part;       // which part is being dragged
    dd_vec3      translate;  // add to position (translate gizmos)
    float          rotate;     // add to angle in radians (rotate gizmos)
    float          scale;      // multiply by scale (scale gizmo; 1.0=none)
} dd_gizmoDelta;

// Returns true while a gizmo drag is in progress.
// Use this in your tick (NOT in the event handler) to suppress camera orbit:
//   if (!dd_gizmoIsDragging()) { apply_orbit_delta(); }
// See note below about timing: always check this AFTER dd_gizmoUpdate*().
bool dd_gizmoIsDragging(void);

// --- Update (hover + drag): call BEFORE drawing -----------------------

// 3D translation: X/Y/Z arrows + XY/XZ/YZ plane quads
void dd_gizmoUpdateTranslate(dd_gizmoDesc *g, const dd_gizmoInput *inp, dd_gizmoDelta *delta);

// 2D screen-plane translate: drag center square -> camera right/up move
void dd_gizmoUpdateTranslate2D(dd_gizmoDesc *g, const dd_gizmoInput *inp, dd_gizmoDelta *delta);

// 3D rotation: X/Y/Z rings
void dd_gizmoUpdateRotate(dd_gizmoDesc *g, const dd_gizmoInput *inp, dd_gizmoDelta *delta);

// 2D view-plane rotate: drag outer white ring -> angle around cam-forward
void dd_gizmoUpdateRotate2D(dd_gizmoDesc *g, const dd_gizmoInput *inp, dd_gizmoDelta *delta);

// 3D scale: X/Y/Z shafts + uniform centre
void dd_gizmoUpdateScale(dd_gizmoDesc *g, const dd_gizmoInput *inp, dd_gizmoDelta *delta);

// --- Draw (visuals only: call inside BeginFrame/EndFrame) -------------
void dd_gizmoTranslate(const dd_gizmoDesc *g);
void dd_gizmoRotate   (const dd_gizmoDesc *g);
void dd_gizmoScale    (const dd_gizmoDesc *g);

// ----------------------------------------------------------------------------
// math constants

#define DD_PI  3.14159265358979f
#define DD_TAU 6.28318530717959f

// ----------------------------------------------------------------------
// Vec3

static inline float   dot3(dd_vec3 a, dd_vec3 b) { return a.x*b.x + a.y*b.y + a.z*b.z; }
static inline float   len3(dd_vec3 a) { return SDL_sqrtf(dot3(a,a)); }
static inline dd_vec3 dd3(float x, float y, float z) { return (dd_vec3){x, y, z}; }
static inline dd_vec3 add3(dd_vec3 a, dd_vec3 b) { return (dd_vec3){a.x+b.x, a.y+b.y, a.z+b.z}; }
static inline dd_vec3 sub3(dd_vec3 a, dd_vec3 b) { return (dd_vec3){a.x-b.x, a.y-b.y, a.z-b.z}; }
static inline dd_vec3 scale3(dd_vec3 a, float s) { return (dd_vec3){a.x*s, a.y*s, a.z*s}; }
static inline dd_vec3 cross3(dd_vec3 a, dd_vec3 b) { return (dd_vec3){a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x }; }
static inline dd_vec3 norm3(dd_vec3 a) { float l = len3(a); if (l < 1e-8f) return (dd_vec3){0,1,0}; return scale3(a, 1.0f/l); }

// ----------------------------------------------------------------------
// Mat4  (column-major)

static inline dd_mat4 m4_identity(void) { dd_mat4 m = {{{0}}}; m.m[0][0] = m.m[1][1] = m.m[2][2] = m.m[3][3] = 1.0f; return m; }
static inline dd_vec4 m4_mul_vec4(const dd_mat4 *m, dd_vec4 v) {
    dd_vec4 r;
    r.x = m->m[0][0]*v.x + m->m[1][0]*v.y + m->m[2][0]*v.z + m->m[3][0]*v.w;
    r.y = m->m[0][1]*v.x + m->m[1][1]*v.y + m->m[2][1]*v.z + m->m[3][1]*v.w;
    r.z = m->m[0][2]*v.x + m->m[1][2]*v.y + m->m[2][2]*v.z + m->m[3][2]*v.w;
    r.w = m->m[0][3]*v.x + m->m[1][3]*v.y + m->m[2][3]*v.z + m->m[3][3]*v.w;
    return r;
}
static inline dd_mat4 m4_mul(const dd_mat4 *a, const dd_mat4 *b) {
    dd_mat4 r = {{{0}}};
    for (int c = 0; c < 4; ++c)
        for (int row = 0; row < 4; ++row)
            for (int k = 0; k < 4; ++k)
                r.m[c][row] += a->m[k][row] * b->m[c][k];
    return r;
}
// Look-at view matrix
static inline dd_mat4 m4_lookat(dd_vec3 eye, dd_vec3 target, dd_vec3 up) {
    dd_vec3 f = norm3(sub3(target, eye));
    dd_vec3 r = norm3(cross3(f, up));
    dd_vec3 u = cross3(r, f);

    dd_mat4 m = {{{0}}};
    m.m[0][0] =  r.x; m.m[1][0] =  r.y; m.m[2][0] =  r.z;
    m.m[0][1] =  u.x; m.m[1][1] =  u.y; m.m[2][1] =  u.z;
    m.m[0][2] = -f.x; m.m[1][2] = -f.y; m.m[2][2] = -f.z;
    m.m[3][0] = -dot3(r, eye);
    m.m[3][1] = -dot3(u, eye);
    m.m[3][2] =  dot3(f, eye);
    m.m[3][3] = 1.0f;
    return m;
}

// Perspective projection (column-major, clip-z in [-1,1])
static inline dd_mat4 m4_perspective(float fov_y_deg, float aspect, float near_z, float far_z) {
    float f = 1.0f / tanf(fov_y_deg * (3.14159265f / 180.0f) * 0.5f);
    dd_mat4 m = {{{0}}};
    m.m[0][0] = f / aspect;
    m.m[1][1] = f;
    m.m[2][2] = (far_z + near_z) / (near_z - far_z);
    m.m[2][3] = -1.0f;
    m.m[3][2] = (2.0f * far_z * near_z) / (near_z - far_z);
    return m;
}

#endif // DD_H

#if KIT_CODE
#pragma once

// -----------------------------------------------------------------------------
// CAPACITIES

#define DD_LINE_CAPACITY   65536   // per-frame lines
#define DD_TIMED_CAPACITY  16384   // persistent lines
#define DD_LABEL_CAPACITY  1024    // per-frame labels
#define DD_TLABEL_CAPACITY 512     // persistent labels
#define DD_LABEL_MAX_LEN   128     // chars per label
#define DD_COLOR_CAPACITY  128     // number of stacked colors

// -----------------------------------------------------------------------------
// INTERNAL TYPES

typedef struct {
    dd_vec3 a, b;
    dd_rgba color;
    uint8_t group;
    uint8_t no_cull;
    float   thickness;
    uint8_t stipple;
    uint8_t stipple_px;
    float   stipple_speed;  // bits/second, from style at push time. negative == reversed
    double  expire;
} dd_line_t;

typedef struct {
    dd_vec3 pos;
    dd_rgba color;
    uint8_t group;
    double  expire;
    char    text[DD_LABEL_MAX_LEN];
} dd_label_t;

// -----------------------------------------------------------------------------
// CONTEXT

struct dd_context {
    dd_rgba dd_colors[DD_COLOR_CAPACITY], *dd_color_inuse;

    SDL_Renderer *renderer;
    int           vp_w, vp_h;

    dd_camera_t   cam;
    dd_mat4       view, proj, vp;

    // Per-frame line buffer
    dd_line_t    *lines;
    int           line_count;

    // Persistent (timed) line ring buffer
    dd_line_t    *timed;
    int           timed_count;    // active entries: may have gaps
    int           timed_cap;

    // Per-frame label buffer
    dd_label_t   *labels;
    int           label_count;

    // Persistent label buffer
    dd_label_t   *tlabels;
    int           tlabel_count;
    int           tlabel_cap;

    // State
    double        now;
    double        prev_now;      // for computing dt in BeginFrame
    float         stipple_phase; // accumulated bit offset [0,8) for animated stipples
    uint8_t       cur_group;
    uint64_t      group_mask;
    dd_depthFade  depth_fade;
    dd_style      cur_style;

    dd_stats      stats;
};

// -----------------------------------------------------------------------------
// CREATE / DESTROY

dd_context *dd_ctx;

dd_context *dd_createContext(SDL_Renderer *renderer) {
    dd_context *ctx = (dd_context *)SDL_calloc(1, sizeof(dd_context));
    if (!ctx) return NULL;

    ctx->renderer = renderer;

    ctx->lines  = (dd_line_t *)SDL_malloc(sizeof(dd_line_t) * DD_LINE_CAPACITY);
    ctx->timed  = (dd_line_t *)SDL_malloc(sizeof(dd_line_t) * DD_TIMED_CAPACITY);
    ctx->labels = (dd_label_t *)SDL_malloc(sizeof(dd_label_t) * DD_LABEL_CAPACITY);
    ctx->tlabels= (dd_label_t *)SDL_malloc(sizeof(dd_label_t) * DD_TLABEL_CAPACITY);
    if (ctx->lines && ctx->timed && ctx->labels && ctx->tlabels) {
        ctx->timed_cap  = DD_TIMED_CAPACITY;
        ctx->tlabel_cap = DD_TLABEL_CAPACITY;

        ctx->group_mask = DD_GROUP_ALL;
        ctx->cur_group  = 0;
        ctx->cur_style  = DD_STYLE_DEFAULT;

        ctx->depth_fade.enabled   = false;
        ctx->depth_fade.fade_near = 5.0f;
        ctx->depth_fade.fade_far  = 200.0f;
        ctx->depth_fade.min_alpha = 30;

        // Default camera
        ctx->cam.position = dd3(5, 4, 5);
        ctx->cam.target   = dd3(0, 0, 0);
        ctx->cam.up       = dd3(0, 1, 0);
        ctx->cam.fov_y    = 60.0f;
        ctx->cam.near_z   = 0.1f;
        ctx->cam.far_z    = 1000.0f;

        ctx->view = m4_lookat(ctx->cam.position, ctx->cam.target, ctx->cam.up);
        ctx->proj = m4_perspective(ctx->cam.fov_y, 1.0f, ctx->cam.near_z, ctx->cam.far_z);
        ctx->vp   = m4_mul(&ctx->proj, &ctx->view);

        ctx->dd_colors[0] = DD_WHITE;
        ctx->dd_color_inuse = ctx->dd_colors;

        return dd_ctx = ctx;
    }

    SDL_free(ctx->lines); SDL_free(ctx->timed);
    SDL_free(ctx->labels); SDL_free(ctx->tlabels);
    SDL_free(ctx);
    return NULL;
}

void dd_destroyContext(dd_context *ctx) {
    if (!ctx) return;
    SDL_free(ctx->lines); SDL_free(ctx->timed);
    SDL_free(ctx->labels); SDL_free(ctx->tlabels);
    SDL_free(ctx);
}

// -----------------------------------------------------------------------------
// CAMERA / VIEWPORT

void dd_setCamera(const dd_camera_t *cam) {
    float ratio = dd_ctx->vp_w / (float)(dd_ctx->vp_h + !dd_ctx->vp_h);
    dd_ctx->cam  = *cam;
    dd_ctx->view = m4_lookat(cam->position, cam->target, cam->up);
    dd_ctx->proj = m4_perspective(cam->fov_y, ratio, cam->near_z, cam->far_z);
    dd_ctx->vp   = m4_mul(&dd_ctx->proj, &dd_ctx->view);
}

const dd_camera_t *dd_getCamera(void) { return &dd_ctx->cam; }

void dd_setViewport(int w, int h) {
    float ratio = w / (float)(h + !h);
    dd_ctx->vp_w = w; dd_ctx->vp_h = h;
    dd_ctx->proj = m4_perspective(dd_ctx->cam.fov_y, ratio, dd_ctx->cam.near_z, dd_ctx->cam.far_z);
    dd_ctx->vp   = m4_mul(&dd_ctx->proj, &dd_ctx->view);
}

void dd_getViewport(int *w, int *h) {
    if (w) *w = dd_ctx->vp_w;
    if (h) *h = dd_ctx->vp_h;
}

// -----------------------------------------------------------------------------
// GROUPS

void dd_setGroup(int id) {
    dd_ctx->cur_group = (uint8_t)(id & 63);
}

void dd_setGroupVisible(int id, bool visible) {
    uint64_t bit = 1ull << (id & 63);
    if (visible) dd_ctx->group_mask |=  bit;
    else         dd_ctx->group_mask &= ~bit;
}

void dd_setAllGroupsVisible(bool visible) {
    dd_ctx->group_mask = visible ? DD_GROUP_ALL : UINT64_C(0);
}

// -----------------------------------------------------------------------------
// STYLE

void dd_setStyle(dd_style style) {
    // Fill in defaults for zero fields
    if (style.thickness < 0.5f)  style.thickness  = 1.0f;
    if (style.stipple_px == 0)   style.stipple_px  = 16;
    if (style.stipple == 0)      style.stipple      = DD_STIPPLE_SOLID;
    dd_ctx->cur_style = style;
}

void dd_resetStyle(void) {
    dd_ctx->cur_style = DD_STYLE_DEFAULT;
}

void    dd_color(dd_rgba c) { *dd_ctx->dd_color_inuse = c; }
void    dd_pushColor(dd_rgba c) { if( dd_ctx->dd_color_inuse < (dd_ctx->dd_colors + DD_COLOR_CAPACITY) ) dd_ctx->dd_color_inuse++; *dd_ctx->dd_color_inuse = c; }
void    dd_popColors(int num) { while(num-- > 0 ) if( dd_ctx->dd_color_inuse > dd_ctx->dd_colors ) --dd_ctx->dd_color_inuse; }

// -----------------------------------------------------------------------------
// DEPTH FADE

void dd_setDepthFade(dd_depthFade s) { dd_ctx->depth_fade = s; }
void dd_enableDepthFade(bool e)        { dd_ctx->depth_fade.enabled = e; }

// -----------------------------------------------------------------------------
// FRAME

void dd_beginFrame(double now_seconds, int width, int height) {
    dd_ctx->vp_w = width, dd_ctx->vp_h = height;

    double dt = dd_ctx->prev_now > 0.0 ? (now_seconds - dd_ctx->prev_now) : 0.0;
    dd_ctx->prev_now    = now_seconds;
    dd_ctx->now         = now_seconds;
    dd_ctx->line_count  = 0;
    dd_ctx->label_count = 0;

    // Advance global stipple phase: wraps at 8 (one full pattern period)
    dd_ctx->stipple_phase += (float)(dt * 8.0);  // full rotation = 1 second at speed=8
    dd_ctx->stipple_phase  = SDL_fmodf(dd_ctx->stipple_phase, 8.0f);

    SDL_memset(&dd_ctx->stats, 0, sizeof(dd_ctx->stats));
}

// -----------------------------------------------------------------------------
// PROJECTION HELPERS

static dd_vec4 dd__project(const dd_vec3 p) {
    dd_vec4 v = {p.x, p.y, p.z, 1.0f};
    return m4_mul_vec4(&dd_ctx->vp, v);
}

static dd_vec2 dd__ndc_to_screen(const float nx, float ny) {
    return (dd_vec2){
        (nx * 0.5f + 0.5f) * (float)dd_ctx->vp_w,
        (1.0f - (ny * 0.5f + 0.5f)) * (float)dd_ctx->vp_h
    };
}

// Full homogeneous clip against all 6 frustum planes.
// Clips the segment (a,b) in clip space so both endpoints are inside:
//   -w <= x <= w,  -w <= y <= w,  -w <= z <= w,  w > 0
// This prevents astronomically large NDC values when the camera is
// near-coplanar with geometry (e.g. grid lines near the horizon).
// Returns false if the segment is entirely outside any plane.
static bool dd__clip_line(dd_vec4 *a, dd_vec4 *b) {
    // Each plane: dot(v, plane) >= 0 means inside.
    // Planes in homogeneous space:
    //   near:    w > 0  -> w >= 0         (already handled first)
    //   far:     z <= w -> w - z >= 0
    //   left:    x >= -w -> x + w >= 0
    //   right:   x <= w  -> w - x >= 0
    //   bottom:  y >= -w -> y + w >= 0
    //   top:     y <= w  -> w - y >= 0

    // For each plane, da = signed dist of a, db = signed dist of b.
    // Clip parametrically: if da<0 clip a; if db<0 clip b.

    #define CLIP_PLANE(da, db)                      \
        do {                                         \
            if ((da) < 0.0f && (db) < 0.0f)         \
                return false;                        \
            if ((da) < 0.0f) {                       \
                float t = (da) / ((da) - (db));      \
                a->x += t*(b->x - a->x);             \
                a->y += t*(b->y - a->y);             \
                a->z += t*(b->z - a->z);             \
                a->w += t*(b->w - a->w);             \
            } else if ((db) < 0.0f) {                \
                float t = (da) / ((da) - (db));      \
                b->x = a->x + t*(b->x - a->x);      \
                b->y = a->y + t*(b->y - a->y);       \
                b->z = a->z + t*(b->z - a->z);       \
                b->w = a->w + t*(b->w - a->w);       \
            }                                        \
        } while(0)

    // near: w > 0 (use small epsilon to avoid divide-by-near-zero)
    CLIP_PLANE(a->w - 1e-4f,  b->w - 1e-4f);
    // far:    w - z >= 0
    CLIP_PLANE(a->w - a->z,   b->w - b->z);
    // left:   x + w >= 0
    CLIP_PLANE(a->x + a->w,   b->x + b->w);
    // right:  w - x >= 0
    CLIP_PLANE(a->w - a->x,   b->w - b->x);
    // bottom: y + w >= 0
    CLIP_PLANE(a->y + a->w,   b->y + b->w);
    // top:    w - y >= 0
    CLIP_PLANE(a->w - a->y,   b->w - b->y);

    #undef CLIP_PLANE
    return true;
}

static bool dd__in_frustum(float x, float y) {
    return x >= -1.05f && x <= 1.05f && y >= -1.05f && y <= 1.05f;
}

// Liang-Barsky 2D clip: does the segment (ax,ay)->(bx,by) intersect [-1,1]x[-1,1]?
// Returns true if any part of the segment is inside the NDC box.
// This handles grazing-angle cases where both endpoints are outside but the
// segment still crosses the visible area (e.g. a long grid line near the horizon).
static bool dd__segment_visible(float ax, float ay, float bx, float by) {
    if (dd__in_frustum(ax, ay) || dd__in_frustum(bx, by)) return true;

    float dx = bx - ax, dy = by - ay;
    float t0 = 0.0f, t1 = 1.0f;

    // Clip against each of the 4 NDC planes
    float p[4] = { -dx,  dx, -dy,  dy };
    float q[4] = { ax - (-1.05f), 1.05f - ax, ay - (-1.05f), 1.05f - ay };

    for (int i = 0; i < 4; i++) {
        if (SDL_fabsf(p[i]) < 1e-10f) {
            if (q[i] < 0.0f) return false; // parallel and outside
        } else {
            float r = q[i] / p[i];
            if (p[i] < 0.0f) { if (r > t0) t0 = r; }
            else              { if (r < t1) t1 = r; }
        }
        if (t0 > t1) return false;
    }
    return true;
}

// Depth of a world-space point along the camera forward vector.
// Returns distance from camera position.
static float dd__depth(const dd_vec3 p) {
    dd_vec3 fwd = norm3(sub3(dd_ctx->cam.target, dd_ctx->cam.position));
    return dot3(sub3(p, dd_ctx->cam.position), fwd);
}

// Apply depth fade: scale alpha of color by distance.
// Uses midpoint of the line segment.
static dd_rgba dd__fade(dd_rgba c, dd_vec3 a, dd_vec3 b) {
    if (!dd_ctx->depth_fade.enabled) return c;
    dd_vec3 mid = dd3(0.5f*(a.x+b.x), 0.5f*(a.y+b.y), 0.5f*(a.z+b.z));
    float d = dd__depth(mid);
    float n = dd_ctx->depth_fade.fade_near;
    float f = dd_ctx->depth_fade.fade_far;
    float t = (d - n) / (f - n);
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    float min_a = (float)dd_ctx->depth_fade.min_alpha / 255.0f;
    float scale = 1.0f - t * (1.0f - min_a);
    c.a = (uint8_t)(c.a * scale);
    return c;
}

// Render a thick segment as two screen-space triangles (a quad).
// pa, pb: screen-space endpoints. half: half-thickness in pixels.
static void dd__render_thick(SDL_Renderer *r, float ax, float ay, float bx, float by, float half) {
    float dx = bx - ax, dy = by - ay;
    float len = SDL_sqrtf(dx*dx + dy*dy);
    if (len < 0.5f) return; // dismiss
    float px = -dy / len * half;
    float py =  dx / len * half;

    uint8_t cr,cg,cb,ca;
    SDL_GetRenderDrawColor(r, &cr, &cg, &cb, &ca);
    SDL_FColor fc = { cr/255.f,cg/255.f,cb/255.f,ca/255.f };

    SDL_Vertex verts[4];
    verts[0].position.x = ax+px; verts[0].position.y = ay+py;
    verts[1].position.x = ax-px; verts[1].position.y = ay-py;
    verts[2].position.x = bx+px; verts[2].position.y = by+py;
    verts[3].position.x = bx-px; verts[3].position.y = by-py;
    verts[0].color = verts[1].color = verts[2].color = verts[3].color = fc;
    verts[0].tex_coord.x = verts[0].tex_coord.y = 0;
    verts[1].tex_coord.x = verts[1].tex_coord.y = 0;
    verts[2].tex_coord.x = verts[2].tex_coord.y = 0;
    verts[3].tex_coord.x = verts[3].tex_coord.y = 0;

    int indices[6] = { 0,1,2, 1,3,2 };
    SDL_RenderGeometry(r, NULL, verts, 4, indices, 6);
}

// Render a segment applying stipple pattern + animation offset.
// `offset`: integer bit rotation [0,7]: shifts the pattern so the dashes
//   appear to crawl along the line over time.
static void dd__render_stipple(SDL_Renderer *r, float ax, float ay, float bx, float by, float thickness, uint8_t pattern, uint8_t period, int offset) {
    float dx = bx - ax, dy = by - ay;
    float len = SDL_sqrtf(dx*dx + dy*dy);
    if (len < 0.5f) return;

    // Rotate pattern by offset bits (left rotate within 8 bits)
    offset &= 7;
    uint8_t pat = (uint8_t)((pattern << offset) | (pattern >> (8 - offset)));

    float ux = dx / len, uy = dy / len;
    float half = thickness * 0.5f;
    float step = (float)period / 8.0f;   // pixels per bit

    float t = 0.0f;
    while (t < len) {
        float seg_start  = t;
        float bit_offset = t / step;
        int   bit_idx    = (int)bit_offset & 7;
        bool  on         = (pat >> (7 - bit_idx)) & 1;

        float next_boundary = (SDL_floorf(bit_offset) + 1.0f) * step;
        float seg_end = next_boundary < len ? next_boundary : len;

        if (on) {
            float sax = ax + ux * seg_start, say = ay + uy * seg_start;
            float sbx = ax + ux * seg_end,   sby = ay + uy * seg_end;
            if (thickness <= 1.5f)
                SDL_RenderLine(r, sax, say, sbx, sby);
            else
                dd__render_thick(r, sax, say, sbx, sby, half);
        }
        t = seg_end;
        if (t >= len) break;
    }
}

// Draw one line (shared by frame and timed flush)
static void dd__render_line(const dd_line_t *l) {
    if (!(dd_ctx->group_mask & (1ull << l->group))) return;

    dd_rgba c = dd__fade(l->color, l->a, l->b);
    if (c.a == 0) return;

    dd_vec4 ca = dd__project(l->a);
    dd_vec4 cb = dd__project(l->b);

    if (!dd__clip_line(&ca, &cb)) { dd_ctx->stats.lines_culled++; return; }

    float nax = ca.x/ca.w, nay = ca.y/ca.w;
    float nbx = cb.x/cb.w, nby = cb.y/cb.w;

    if (!dd__segment_visible(nax, nay, nbx, nby)) {
        dd_ctx->stats.lines_clipped++; return;
    }

    dd_vec2 sa = dd__ndc_to_screen(nax, nay);
    dd_vec2 sb = dd__ndc_to_screen(nbx, nby);

    SDL_SetRenderDrawColor(dd_ctx->renderer, c.r, c.g, c.b, c.a);

    bool solid    = (l->stipple == DD_STIPPLE_SOLID);
    bool hairline = (l->thickness <= 1.5f);

    if (solid && hairline) {
        SDL_RenderLine(dd_ctx->renderer, sa.x, sa.y, sb.x, sb.y);
    } else if (solid) {
        dd__render_thick(dd_ctx->renderer, sa.x, sa.y, sb.x, sb.y, l->thickness * 0.5f);
    } else {
        // Compute animation offset: the global stipple_phase is in [0,8).
        // Scale by this line's stipple_speed (relative to base speed of 8 b/s).
        // A speed of 1.0 means the line rotates at the same rate as the global phase.
        // Negative speed reverses direction.
        float phase = dd_ctx->stipple_phase * (l->stipple_speed / 8.0f);
        int   offset = (int)SDL_floorf(phase) & 7;
        // For negative speeds, flip direction
        if (l->stipple_speed < 0.0f) offset = (8 - offset) & 7;

        dd__render_stipple(dd_ctx->renderer, sa.x, sa.y, sb.x, sb.y, l->thickness, l->stipple, l->stipple_px, offset);
    }

    dd_ctx->stats.lines_drawn++;
}

void dd_endFrame(void) {
    dd_ctx->stats.lines_submitted = dd_ctx->line_count;

    // --- Per-frame lines ---
    for (int i = 0; i < dd_ctx->line_count; i++)
        dd__render_line(&dd_ctx->lines[i]);

    // --- Timed lines: render alive ones, compact dead ones ---
    int alive = 0;
    for (int i = 0; i < dd_ctx->timed_count; i++) {
        dd_line_t *l = &dd_ctx->timed[i];
        if (l->expire <= dd_ctx->now) continue;  // expired
        dd__render_line(l);
        if (i != alive) dd_ctx->timed[alive] = *l;  // compact
        alive++;
    }
    dd_ctx->timed_count = alive;
    dd_ctx->stats.timed_lines_active = alive;

    // --- Per-frame labels ---
    for (int i = 0; i < dd_ctx->label_count; i++) {
        dd_label_t *lb = &dd_ctx->labels[i];
        if (!(dd_ctx->group_mask & (1ull << lb->group))) continue;

        // Project to screen
        dd_vec4 clip = dd__project(lb->pos);
        if (clip.w <= 0.001f) continue;
        float nx = clip.x / clip.w, ny = clip.y / clip.w;
        if (!dd__in_frustum(nx, ny)) continue;
        dd_vec2 sp = dd__ndc_to_screen(nx, ny);

        dd_rgba c = lb->color;
        if (dd_ctx->depth_fade.enabled) {
            // Fade label alpha by depth
            float d = dd__depth(lb->pos);
            float t = (d - dd_ctx->depth_fade.fade_near) / (dd_ctx->depth_fade.fade_far - dd_ctx->depth_fade.fade_near);
            if (t < 0.0f) t = 0.0f; else if (t > 1.0f) t = 1.0f;
            float min_a = (float)dd_ctx->depth_fade.min_alpha / 255.0f;
            c.a = (uint8_t)(c.a * (1.0f - t * (1.0f - min_a)));
        }
        if (c.a == 0) continue;

        SDL_SetRenderDrawColor(dd_ctx->renderer, c.r, c.g, c.b, c.a);
        SDL_RenderDebugText(dd_ctx->renderer, sp.x, sp.y, lb->text);
        dd_ctx->stats.labels_drawn++;
    }

    // --- Timed labels: render + compact ---
    int talive = 0;
    for (int i = 0; i < dd_ctx->tlabel_count; i++) {
        dd_label_t *lb = &dd_ctx->tlabels[i];
        if (lb->expire <= dd_ctx->now) continue;
        // Reuse label render path: copy into per-frame slot temporarily
        if (dd_ctx->label_count < DD_LABEL_CAPACITY) {
            dd_ctx->labels[dd_ctx->label_count++] = *lb;
            // render it immediately
            dd_label_t *tmp = &dd_ctx->labels[dd_ctx->label_count - 1];
            if (dd_ctx->group_mask & (1ull << tmp->group)) {
                dd_vec4 clip = dd__project(tmp->pos);
                if (clip.w > 0.001f) {
                    float nx = clip.x/clip.w, ny = clip.y/clip.w;
                    if (dd__in_frustum(nx, ny)) {
                        dd_vec2 sp = dd__ndc_to_screen(nx, ny);
                        dd_rgba c = tmp->color;
                        SDL_SetRenderDrawColor(dd_ctx->renderer, c.r, c.g, c.b, c.a);
                        SDL_RenderDebugText(dd_ctx->renderer, sp.x, sp.y, tmp->text);
                        dd_ctx->stats.labels_drawn++;
                    }
                }
            }
            dd_ctx->label_count--; // don't permanently add to frame buffer
        }
        if (i != talive) dd_ctx->tlabels[talive] = *lb;
        talive++;
    }
    dd_ctx->tlabel_count = talive;
}

// -----------------------------------------------------------------------------
// STATS

const dd_stats *dd_getStats(void) { return &dd_ctx->stats; }

// -----------------------------------------------------------------------------
// INTERNAL PUSH HELPERS

static void dd__push(dd_vec3 a, dd_vec3 b) {
    if (dd_ctx->line_count >= DD_LINE_CAPACITY) return;
    dd_line_t *l = &dd_ctx->lines[dd_ctx->line_count++];
    l->a = a; l->b = b; l->color = *dd_ctx->dd_color_inuse;
    l->group         = dd_ctx->cur_group;
    l->no_cull       = 0;
    l->thickness     = dd_ctx->cur_style.thickness;
    l->stipple       = dd_ctx->cur_style.stipple;
    l->stipple_px    = dd_ctx->cur_style.stipple_px;
    l->stipple_speed = dd_ctx->cur_style.stipple_speed;
    l->expire        = 0.0;
}

static void dd__push_nc(dd_vec3 a, dd_vec3 b) {
    if (dd_ctx->line_count >= DD_LINE_CAPACITY) return;
    dd_line_t *l = &dd_ctx->lines[dd_ctx->line_count++];
    l->a = a; l->b = b; l->color = *dd_ctx->dd_color_inuse;
    l->group         = dd_ctx->cur_group;
    l->no_cull       = 1;
    l->thickness     = dd_ctx->cur_style.thickness;
    l->stipple       = dd_ctx->cur_style.stipple;
    l->stipple_px    = dd_ctx->cur_style.stipple_px;
    l->stipple_speed = dd_ctx->cur_style.stipple_speed;
    l->expire        = 0.0;
}

static void dd__push_timed(dd_vec3 a, dd_vec3 b, float lifetime) {
    if (dd_ctx->timed_count >= dd_ctx->timed_cap) return;
    dd_line_t *l = &dd_ctx->timed[dd_ctx->timed_count++];
    l->a = a; l->b = b; l->color = *dd_ctx->dd_color_inuse;
    l->group         = dd_ctx->cur_group;
    l->no_cull       = 0;
    l->thickness     = dd_ctx->cur_style.thickness;
    l->stipple       = dd_ctx->cur_style.stipple;
    l->stipple_px    = dd_ctx->cur_style.stipple_px;
    l->stipple_speed = dd_ctx->cur_style.stipple_speed;
    l->expire        = dd_ctx->now + (double)lifetime;
}

static void dd__push_label(dd_vec3 pos, double expire, const char *fmt, va_list ap) {
    dd_label_t *lb = NULL;
    if( expire > 0.0 ) {
        if (dd_ctx->tlabel_count >= dd_ctx->tlabel_cap) return;
        lb = &dd_ctx->tlabels[dd_ctx->tlabel_count++];
        lb->expire = expire;
    } else {
        if (dd_ctx->label_count >= DD_LABEL_CAPACITY) return;
        lb = &dd_ctx->labels[dd_ctx->label_count++];
        lb->expire = 0.0;
    }
    lb->pos   = pos;
    lb->color = *dd_ctx->dd_color_inuse;
    lb->group = dd_ctx->cur_group;
    SDL_vsnprintf(lb->text, DD_LABEL_MAX_LEN, fmt, ap);
}

// -----------------------------------------------------------------------------
// TIMED CONTROL

void dd_clearTimed(void) {
    dd_ctx->timed_count  = 0;
    dd_ctx->tlabel_count = 0;
}

// -----------------------------------------------------------------------------
// PRIMITIVES  (same geometry as before, just call dd__push)

void dd_line(dd_vec3 a, dd_vec3 b) {
    dd__push(a, b);
}

void dd_point(dd_vec3 p, float size) {
    size *= 0.25f; // assume user wants size == 1 for a small cube, rather than a grid unit
#if 1
    float h = size * 0.5f;
    dd__push(dd3(p.x-h,p.y,  p.z  ), dd3(p.x+h,p.y,  p.z  ));
    dd__push(dd3(p.x,  p.y-h,p.z  ), dd3(p.x,  p.y+h,p.z  ));
    dd__push(dd3(p.x,  p.y,  p.z-h), dd3(p.x,  p.y,  p.z+h));
#else
    dd_vec3 half = dd3(size/2,size/2,size/2);
    dd_aabb(sub3(p,half), add3(p,half));
#endif
}

void dd_aabb(dd_vec3 mn, dd_vec3 mx) {
    dd__push(dd3(mn.x,mn.y,mn.z),dd3(mx.x,mn.y,mn.z));
    dd__push(dd3(mx.x,mn.y,mn.z),dd3(mx.x,mn.y,mx.z));
    dd__push(dd3(mx.x,mn.y,mx.z),dd3(mn.x,mn.y,mx.z));
    dd__push(dd3(mn.x,mn.y,mx.z),dd3(mn.x,mn.y,mn.z));
    dd__push(dd3(mn.x,mx.y,mn.z),dd3(mx.x,mx.y,mn.z));
    dd__push(dd3(mx.x,mx.y,mn.z),dd3(mx.x,mx.y,mx.z));
    dd__push(dd3(mx.x,mx.y,mx.z),dd3(mn.x,mx.y,mx.z));
    dd__push(dd3(mn.x,mx.y,mx.z),dd3(mn.x,mx.y,mn.z));
    dd__push(dd3(mn.x,mn.y,mn.z),dd3(mn.x,mx.y,mn.z));
    dd__push(dd3(mx.x,mn.y,mn.z),dd3(mx.x,mx.y,mn.z));
    dd__push(dd3(mx.x,mn.y,mx.z),dd3(mx.x,mx.y,mx.z));
    dd__push(dd3(mn.x,mn.y,mx.z),dd3(mn.x,mx.y,mx.z));
}

void dd_box(dd_vec3 center, dd_vec3 half) {
    dd_aabb(sub3(center,half), add3(center,half));
}

void dd_obb(dd_vec3 ctr, dd_vec3 ax, dd_vec3 ay, dd_vec3 az) {
    dd_vec3 corners[8];
    for (int i = 0; i < 8; i++) {
        float sx = i&1 ? 1 : -1, sy = i&2 ? 1 : -1, sz = i&4 ? 1 : -1;
        corners[i] = add3(ctr, add3(add3(scale3(ax,sx),scale3(ay,sy)),scale3(az,sz)));
    }
    int edges[12][2]={{0,1},{2,3},{4,5},{6,7},{0,2},{1,3},{4,6},{5,7},{0,4},{1,5},{2,6},{3,7}};
    for (int e=0;e<12;e++) dd__push(corners[edges[e][0]],corners[edges[e][1]]);
}

void dd_triangle(dd_vec3 a, dd_vec3 b, dd_vec3 c) {
    dd__push(a,b); dd__push(b,c); dd__push(c,a);
}

// Circle ring helper: shared by sphere, capsule, circle, cone
static void dd__ring(dd_vec3 ctr, dd_vec3 right, dd_vec3 up, float r, int segs) {
    dd_vec3 prev = add3(ctr, scale3(right, r));
    for (int i = 1; i <= segs; i++) {
        float t = (float)i/(float)segs * DD_TAU;
        dd_vec3 cur = add3(ctr, add3(scale3(right,SDL_cosf(t)*r), scale3(up,SDL_sinf(t)*r)));
        dd__push(prev, cur);
        prev = cur;
    }
}

// Silhouette circle (perspective-correct, camera-facing)
static void dd__silhouette(dd_vec3 ctr, float r, int segs) {
    dd_vec3 cam = dd_ctx->cam.position;
    dd_vec3 to  = sub3(ctr, cam);
    float d = len3(to);
    if (d <= r) return;
    dd_vec3 vd = scale3(to, 1.f/d);
    float dp     = r*r/d;
    float rp     = r*SDL_sqrtf(1.f-(r/d)*(r/d));
    dd_vec3 sc = add3(cam, scale3(vd, d-dp));
    float pen = -dot3(sub3(sc, ctr), vd);
    // (no outward_axis clamp needed for full sphere)
    dd_vec3 wu = (SDL_fabsf(vd.y)<0.99f)?dd3(0,1,0):dd3(1,0,0);
    dd_vec3 rt = norm3(cross3(wu, vd));
    dd_vec3 up = cross3(vd, rt);
    (void)pen;
    dd__ring(sc, rt, up, rp, segs);
}

// Half-silhouette for capsule caps
static void dd__silhouette_half(dd_vec3 pole, float r, dd_vec3 out_ax, int segs) {
    dd_vec3 cam = dd_ctx->cam.position;
    dd_vec3 to  = sub3(pole, cam);
    float d = len3(to);
    if (d <= r) return;
    dd_vec3 vd = scale3(to, 1.f/d);
    float dp = r*r/d, rp = r*SDL_sqrtf(1.f-(r/d)*(r/d));
    dd_vec3 sc = add3(cam, scale3(vd, d-dp));
    float pen = -dot3(sub3(sc, pole), out_ax);
    if (pen > 0.f) { sc = add3(sc, scale3(out_ax, pen)); rp = r; }
    dd_vec3 wu = (SDL_fabsf(vd.y)<0.99f)?dd3(0,1,0):dd3(1,0,0);
    dd_vec3 rt = norm3(cross3(wu, vd));
    dd_vec3 up = cross3(vd, rt);
    dd_vec3 prev = add3(sc, scale3(rt, rp));
    for (int i = 1; i <= segs; i++) {
        float t = (float)i/(float)segs * DD_TAU;
        dd_vec3 cur = add3(sc, add3(scale3(rt,SDL_cosf(t)*rp), scale3(up,SDL_sinf(t)*rp)));
        dd_vec3 mid = dd3(0.5f*(prev.x+cur.x),0.5f*(prev.y+cur.y),0.5f*(prev.z+cur.z));
        if (dot3(sub3(mid, pole), out_ax) >= 0.f) dd__push(prev, cur);
        prev = cur;
    }
}

void dd_sphereEx(dd_vec3 ctr, float r, int segs) {
    dd__ring(ctr, dd3(1,0,0), dd3(0,1,0), r, segs);
    dd__ring(ctr, dd3(1,0,0), dd3(0,0,1), r, segs);
    dd__ring(ctr, dd3(0,1,0), dd3(0,0,1), r, segs);
    dd__silhouette(ctr, r, segs);
}

void dd_sphere(dd_vec3 ctr, float r) {
    dd_sphereEx(ctr, r, 24);
}

void dd_circle(dd_vec3 ctr, dd_vec3 normal, float r, int segs) {
    dd_vec3 ref  = SDL_fabsf(normal.x) < 0.9f ? dd3(1,0,0) : dd3(0,1,0);
    dd_vec3 rt   = norm3(cross3(ref, normal));
    dd_vec3 up   = cross3(normal, rt);
    dd__ring(ctr, rt, up, r, segs);
}

void dd_cone(dd_vec3 apex, dd_vec3 base_ctr, float r, int segs) {
    dd_vec3 ax = norm3(sub3(base_ctr, apex));
    dd_vec3 ref = SDL_fabsf(ax.x) < 0.9f ? dd3(1,0,0) : dd3(0,1,0);
    dd_vec3 rt  = norm3(cross3(ref, ax));
    dd_vec3 up  = cross3(ax, rt);
    dd__ring(base_ctr, rt, up, r, segs);
    for (int i = 0; i < segs; i++) {
        float t = (float)i/(float)segs * DD_TAU;
        dd_vec3 bp = add3(base_ctr, add3(scale3(rt,SDL_cosf(t)*r), scale3(up,SDL_sinf(t)*r)));
        dd__push(apex, bp);
    }
}

void dd_capsule(dd_vec3 base, dd_vec3 tip, float r) {
    int segs = 20;
    dd_vec3 ax  = norm3(sub3(tip, base));
    dd_vec3 ref = (SDL_fabsf(ax.x)<0.9f)?dd3(1,0,0):dd3(0,1,0);
    dd_vec3 rt  = norm3(cross3(ref, ax));
    dd_vec3 up  = cross3(ax, rt);
    for (int i = 0; i < 8; i++) {
        float t = (float)i/8.f * DD_TAU;
        dd_vec3 off = add3(scale3(rt,SDL_cosf(t)*r), scale3(up,SDL_sinf(t)*r));
        dd__push(add3(base,off), add3(tip,off));
    }
    dd__ring(base, rt, up, r, segs);
    dd__ring(tip,  rt, up, r, segs);
    for (int side = 0; side < 2; side++) {
        dd_vec3 pole = side?tip:base;
        float dir = side?1.f:-1.f;
        for (int arc = 0; arc < 4; arc++) {
            float phi = (float)arc/4.f * DD_TAU;
            dd_vec3 rv = add3(scale3(rt,SDL_cosf(phi)), scale3(up,SDL_sinf(phi)));
            dd_vec3 prev2 = add3(pole, scale3(rv, r));
            int hs = segs/2;
            for (int j = 1; j <= hs; j++) {
                float theta = (float)j/(float)hs * (DD_PI*0.5f);
                dd_vec3 cur = add3(pole, add3(scale3(rv,SDL_cosf(theta)*r), scale3(ax,SDL_sinf(theta)*r*dir)));
                dd__push(prev2, cur);
                prev2 = cur;
            }
        }
    }
    dd__silhouette_half(base, r, scale3(ax,-1.f), segs);
    dd__silhouette_half(tip,  r, ax,                segs);
}

void dd_ray(dd_vec3 origin, dd_vec3 dir, float length) {
    dd_vec3 d   = norm3(dir);
    dd_vec3 end = add3(origin, scale3(d, length));
    dd__push(origin, end);
    float head = length*0.12f, rad = head*0.35f;
    dd_vec3 base = add3(origin, scale3(d, length-head));
    dd_cone(end, base, rad, 6);
}

void dd_frustum(const dd_mat4 *inv_vp) {
    static const float ndc[8][4] = {
        {-1,-1,-1,1},{1,-1,-1,1},{1,1,-1,1},{-1,1,-1,1},
        {-1,-1, 1,1},{1,-1, 1,1},{1,1, 1,1},{-1,1, 1,1},
    };
    dd_vec3 wp[8];
    for (int i=0;i<8;i++) {
        dd_vec4 v={ndc[i][0],ndc[i][1],ndc[i][2],ndc[i][3]};
        dd_vec4 r=m4_mul_vec4(inv_vp,v);
        float iw = SDL_fabsf(r.w) > 1e-8f ? (1.f/r.w) : 0.f;
        wp[i]=dd3(r.x*iw,r.y*iw,r.z*iw);
    }
    for (int i=0;i<4;i++) { dd__push(wp[i],wp[(i+1)%4]); dd__push(wp[i+4],wp[(i+1)%4+4]); dd__push(wp[i],wp[i+4]); }
}

// -----------------------------------------------------------------------------
// TIMED PRIMITIVES

// Helper: temporarily override push to go to timed buffer
// We do this by swapping line_count pointer temporarily: cleanest approach
// is a small wrapper that pushes directly.

// Macro: run draw call then move its new lines to timed buffer
#define DD__PUSH_TIMED_BEGIN(lifetime)  \
    int _base = dd_ctx->line_count;             \
    double _exp = dd_ctx->now + (double)(lifetime);

#define DD__PUSH_TIMED_END()                                   \
    for (int _i = _base; _i < dd_ctx->line_count; _i++) {           \
        dd_line_t _l = dd_ctx->lines[_i];                           \
        _l.expire = _exp;                                           \
        if (dd_ctx->timed_count < dd_ctx->timed_cap)                  \
            dd_ctx->timed[dd_ctx->timed_count++] = _l;               \
    }                                                               \
    dd_ctx->line_count = _base;  /* rewind frame buffer */

void dd_lineTimed(dd_vec3 a, dd_vec3 b, float lt) {
    dd__push_timed(a, b, lt);
}

void dd_pointTimed(dd_vec3 p, float size, float lt) {
    DD__PUSH_TIMED_BEGIN(lt)
    dd_point(p, size);
    DD__PUSH_TIMED_END()
}

void dd_sphereTimed(dd_vec3 ctr, float r, float lt) {
    DD__PUSH_TIMED_BEGIN(lt)
    dd_sphere(ctr, r);
    DD__PUSH_TIMED_END()
}

void dd_boxTimed(dd_vec3 ctr, dd_vec3 half, float lt) {
    DD__PUSH_TIMED_BEGIN(lt)
    dd_box(ctr, half);
    DD__PUSH_TIMED_END()
}

void dd_rayTimed(dd_vec3 origin, dd_vec3 dir, float length, float lt) {
    DD__PUSH_TIMED_BEGIN(lt)
    dd_ray(origin, dir, length);
    DD__PUSH_TIMED_END()
}

// -----------------------------------------------------------------------------
// TEXT 2D  (SDL_RenderDebugText, screen-space)

void dd_label(dd_vec3 pos, const char *fmt, ...) {
    va_list ap; va_start(ap, fmt);
    dd__push_label(pos, 0.0, fmt, ap);
    va_end(ap);
}

void dd_labelTimed(dd_vec3 pos, float lt, const char *fmt, ...) {
    va_list ap; va_start(ap, fmt);
    dd__push_label(pos, dd_ctx->now + (double)lt, fmt, ap);
    va_end(ap);
}

// -----------------------------------------------------------------------------
// NEW PRIMITIVES

// --- Dashed line ---
void dd_lineDashed(dd_vec3 a, dd_vec3 b) {
    dd_vec3 diff = sub3(b, a);
    float len = len3(diff);
    if (len < 1e-6f) return;
    dd_vec3 unit = scale3(diff, 1.0f / len);
    // dash + gap = 2 world units; adjust dash_len for frequency
    float dash = 0.5f, gap = 0.5f, pos = 0.0f;
    while (pos < len) {
        float end = pos + dash;
        if (end > len) end = len;
        dd__push(
            add3(a, scale3(unit, pos)),
            add3(a, scale3(unit, end)));
        pos += dash + gap;
    }
}

void dd_axes(dd_vec3 origin, dd_vec3 ax, dd_vec3 ay, dd_vec3 az, float length) {
    // Positive solid && Negative dashed at half alpha
    dd_pushColor(DD_MAGENTA); dd_ray(origin, ax, length); dd_ctx->dd_color_inuse->a = 128; dd_lineDashed(origin, add3(origin, scale3(ax,-length)));
    dd_color(DD_GREEN);       dd_ray(origin, ay, length); dd_ctx->dd_color_inuse->a = 128; dd_lineDashed(origin, add3(origin, scale3(ay,-length)));
    dd_color(DD_BLUE);        dd_ray(origin, az, length); dd_ctx->dd_color_inuse->a = 128; dd_lineDashed(origin, add3(origin, scale3(az,-length)));
    dd_popColors(1);
}

void dd_grid(dd_vec3 center, float cell_size, int cells, dd_rgba major_color, dd_rgba minor_color, int subdivisions) {
    float half = cell_size * (float)cells * 0.5f;

    // minor lines first (painted underneath)
    if (subdivisions > 1) {
        dd_rgba bak = *dd_ctx->dd_color_inuse;
        *dd_ctx->dd_color_inuse = minor_color;
        float sub = cell_size / (float)subdivisions;
        for (float t = -half; t <= half + 1e-4f; t += sub) {
            float rem = (t + half) / cell_size;
            float frac = rem - SDL_floorf(rem);
            if (frac < 1e-4f || frac > 1.0f - 1e-4f) continue; // skip major
            dd__push_nc(dd3(center.x-half, center.y, center.z+t), dd3(center.x+half, center.y, center.z+t));
            dd__push_nc(dd3(center.x+t, center.y, center.z-half), dd3(center.x+t, center.y, center.z+half));
        }
        *dd_ctx->dd_color_inuse = bak;
    }

    // major lines (painted over)
    dd_rgba bak = *dd_ctx->dd_color_inuse;
    *dd_ctx->dd_color_inuse = major_color;
    for (int i = 0; i <= cells; i++) {
        float t = -half + (float)i * cell_size;
        dd__push_nc(dd3(center.x-half, center.y, center.z+t), dd3(center.x+half, center.y, center.z+t));
        dd__push_nc(dd3(center.x+t, center.y, center.z-half), dd3(center.x+t, center.y, center.z+half));
    }
    *dd_ctx->dd_color_inuse = bak;

    // draw nswe points
    dd_normal(add3(center, dd3(-half,0,0)), dd3(0,0.5,0));
    dd_normal(add3(center, dd3(+half,0,0)), dd3(0,0.5,0));
    dd_normal(add3(center, dd3(0,0,-half)), dd3(0,0.5,0));
    dd_normal(add3(center, dd3(0,0,+half)), dd3(0,0.5,0));

    // draw axes (painted over). length = full grid half-extent in both + and - directions
    dd_axes(center, dd3(1,0,0), dd3(0,1,0), dd3(0,0,1), half);
}

// --- Normal ---
void dd_normal(dd_vec3 pos, dd_vec3 dir) {
    dd__push(pos, add3(pos, dir)); // do not normalize dir because it will help debugging unnormalised normals
}

// --- Prism (generalised) ---
void dd_prism(dd_vec3 center, float radius, float height, dd_vec3 normal, int segments) {
    dd_vec3 n   = norm3(normal);
    dd_vec3 ref = (SDL_fabsf(n.x) < 0.9f) ? dd3(1,0,0) : dd3(0,1,0);
    dd_vec3 rt  = norm3(cross3(ref, n));
    dd_vec3 up2 = norm3(cross3(n, rt));

    // Generate base ring
    dd_vec3 pts[256]; // max segments
    if (segments > 255) segments = 255;
    for (int i = 0; i < segments; i++) {
        float t = (float)i / (float)segments * DD_TAU;
        pts[i] = add3(center, add3(scale3(rt, SDL_cosf(t)*radius),scale3(up2, SDL_sinf(t)*radius)));
    }

    // Draw base ring
    for (int i = 0; i < segments; i++)
        dd__push(pts[i], pts[(i+1) % segments]);

    if (SDL_fabsf(height) > 1e-6f) {
        if (height > 0.0f) {
            // Cone: lines to apex
            dd_vec3 apex = add3(center, scale3(n, height));
            for (int i = 0; i < segments; i++)
                dd__push(pts[i], apex);
        } else {
            // Prism: extrude second ring along -n
            dd_vec3 top = add3(center, scale3(n, -height));
            dd_vec3 pts2[256];
            for (int i = 0; i < segments; i++)
                pts2[i] = add3(pts[i], scale3(n, -height));
            // Top ring + verticals
            for (int i = 0; i < segments; i++) {
                dd__push(pts2[i], pts2[(i+1) % segments]);
                dd__push(pts[i],  pts2[i]);
            }
            (void)top;
        }
    }
}

// --- AABB corner ticks ---
void dd_aabbCorners(dd_vec3 mn, dd_vec3 mx) {
    dd_vec3 pts[8];
    pts[0]=dd3(mn.x,mn.y,mn.z); pts[1]=dd3(mx.x,mn.y,mn.z);
    pts[2]=dd3(mx.x,mn.y,mx.z); pts[3]=dd3(mn.x,mn.y,mx.z);
    pts[4]=dd3(mn.x,mx.y,mn.z); pts[5]=dd3(mx.x,mx.y,mn.z);
    pts[6]=dd3(mx.x,mx.y,mx.z); pts[7]=dd3(mn.x,mx.y,mx.z);
    // 12 edges, draw only unit-length tick at each end
    int edges[12][2]={{0,1},{1,2},{2,3},{3,0},{4,5},{5,6},{6,7},{7,4},{0,4},{1,5},{2,6},{3,7}};
    for (int e = 0; e < 12; e++) {
        dd_vec3 a = pts[edges[e][0]], b = pts[edges[e][1]];
        dd_vec3 ab = scale3(norm3(sub3(b, a)), 0.5f);
        dd__push(a, add3(a, ab));
        dd__push(b, sub3(b, ab));
    }
}

// --- Camera body ---
// Helper: transform a local point by position + orientation
static dd_vec3 dd__cam_pt(dd_vec3 local, dd_vec3 pos, dd_vec3 fwd, dd_vec3 rt, dd_vec3 up3) {
    return add3(pos,
        add3(add3(scale3(rt,  local.x), scale3(up3, local.y)), scale3(fwd, local.z)));
}

void dd_camera(dd_vec3 pos, dd_vec3 forward, dd_vec3 up3) {
    // original artwork by Alexandri Zavodny
    // - rlyeh, public domain

    dd_vec3 fwd = norm3(forward);
    dd_vec3 rt  = norm3(cross3(fwd, up3));
    dd_vec3 u   = norm3(cross3(rt, fwd));

    #define CP(lx,ly,lz) dd__cam_pt(dd3(lx,ly,lz), pos, fwd, rt, u)

    // Body box
    dd_vec3 p[8] = {
        CP( .5f, .5f, 1.f), CP( .5f, .5f,-1.f),
        CP(-.5f, .5f,-1.f), CP(-.5f, .5f, 1.f),
        CP( .5f,-.5f, 1.f), CP( .5f,-.5f,-1.f),
        CP(-.5f,-.5f,-1.f), CP(-.5f,-.5f, 1.f),
    };
    for (int i = 0; i < 4; i++) {
        dd__push(p[i],   p[(i+1)%4]);
        dd__push(p[i+4], p[(i+1)%4+4]);
        dd__push(p[i],   p[i+4]);
    }

    // Two half-reels: centred at y=0.5, z=-0.5 and z=+0.5 (not at end faces).
    // Arc sweeps in the local YZ plane: y goes up, z goes out then back.
    // Matches the original dd_camera reference geometry.
    float reel_r = 0.25f;
    int   reel_det = 32;
    for (int reel = 0; reel < 2; reel++) {
        float rz0 = -0.5f + 1.0f * (float)reel;  // centre z: -0.5 or +0.5
        dd_vec3 centre = CP(0.f, 0.5f, rz0);
        dd_vec3 from = centre;
        for (int i = 0; i <= reel_det / 2; i++) {
            float theta = (float)i / (float)(reel_det / 2) * DD_PI;
            float ex  = -SDL_cosf(theta);   // z direction
            float why =  SDL_sinf(theta);   // y direction
            dd_vec3 to = CP(0.f, 0.5f + why * reel_r * 2.0f, rz0  + ex  * reel_r * 2.0f);
            dd__push(from, to);
            from = to;
        }
        // Close back to centre
        dd__push(from, centre);
    }

    // Lens shield (trapezoid protruding forward)
    float lo = 0.2f, loz = 0.4f, loff = 0.3f;
    dd_vec3 v0=CP(.5f,.5f,1.f), v1=CP(-.5f,.5f,1.f);
    dd_vec3 v2=CP(-.5f,-.5f,1.f), v3_=CP(.5f,-.5f,1.f);
    dd_vec3 l0=CP(.5f+lo,.5f+loff,1.f+loz);
    dd_vec3 l1=CP(-.5f-lo,.5f+loff,1.f+loz);
    dd_vec3 l2=CP(-.5f-lo,-.5f-loff,1.f+loz);
    dd_vec3 l3=CP(.5f+lo,-.5f-loff,1.f+loz);
    dd__push(l0,l1); dd__push(l1,l2);
    dd__push(l2,l3); dd__push(l3,l0);
    dd__push(v0,l0); dd__push(v1,l1);
    dd__push(v2,l2); dd__push(v3_,l3);
    #undef CP
}

// --- Position rings ---
static void dd__oriented_ring(dd_vec3 center, dd_vec3 dir, float radius) {
    // Circle whose normal is `dir` with a small opening (29/32 of ring)
    dd_vec3 n   = norm3(dir);
    dd_vec3 ref = (SDL_fabsf(n.x) < 0.9f) ? dd3(1,0,0) : dd3(0,1,0);
    dd_vec3 rt  = norm3(cross3(ref, n));
    dd_vec3 up2 = cross3(n, rt);
    int segs = 32, skip = 3;
    dd_vec3 prev3;
    bool first = true;
    for (int i = 0; i <= segs - skip; i++) {
        float t = (float)(i + skip/2) / (float)segs * DD_TAU;
        dd_vec3 cur = add3(center,
            add3(scale3(rt, SDL_cosf(t)*radius), scale3(up2, SDL_sinf(t)*radius)));
        if (!first) dd__push(prev3, cur);
        first = false;
        prev3 = cur;
    }
}

void dd_positionDir(dd_vec3 pos, dd_vec3 dir, float radius) {
    dd_pushColor(pos.y > 0 ? DD_CYAN : DD_MAGENTA);

    dd_point(pos,1);

    dd_vec3 ground = dd3(pos.x, 0.f, pos.z);
    dd__push(ground, pos);   // vertical line

    float h = SDL_fabsf(pos.y);
    int rings = (int)h + 1;
    if (rings > 10) rings = 10;
    // Fix: rings lie in the XZ plane, so tangent axes are X and Z (not X and Y)
    for (int i = 0; i < rings; i++) {
        float r = radius;
        if (i < 2 && len3(dir) > 1e-4f)
            dd__oriented_ring(ground, dir, r);
        else
            dd__ring(ground, dd3(1,0,0), dd3(0,0,1), r, 32);
        radius *= 0.9f;
    }

    dd_popColors(1);
}

void dd_position(dd_vec3 pos, float radius) {
    dd_positionDir(pos, dd3(0,1,0), radius);
}

// --- Boid ---
void dd_boid(dd_vec3 pos, dd_vec3 dir) {
    dd_vec3 fwd = norm3(dir);
    // Use world up to derive right -> boid lies flat in XZ plane by default,
    // correct for both 3D games and 2D top-down games
    dd_vec3 world_up = dd3(0, 1, 0);
    dd_vec3 rt = norm3(cross3(fwd, world_up));
    // Degenerate: dir is straight up/down, fall back to Z
    if (len3(rt) < 0.001f)
        rt = norm3(cross3(fwd, dd3(0, 0, 1)));

    dd_vec3 front  = add3(pos, scale3(fwd,  1.0f));
    dd_vec3 back   = add3(pos, scale3(fwd, -0.25f));
    dd_vec3 right2 = add3(back, scale3(rt,  0.5f));
    dd_vec3 left2  = add3(back, scale3(rt, -0.5f));

    dd__push(front,  left2);
    dd__push(left2,  pos);
    dd__push(pos,    right2);
    dd__push(right2, front);
}

// --- Diamond ---
void dd_diamond(dd_vec3 center, dd_vec3 up3, float size) {
    dd_vec3 u   = norm3(up3);
    dd_vec3 ref = (SDL_fabsf(u.x) < 0.9f) ? dd3(1,0,0) : dd3(0,1,0);
    dd_vec3 fwd = scale3(norm3(cross3(ref, u)), size);
    dd_vec3 rt  = scale3(norm3(cross3(u, fwd)), size);
    dd_vec3 top = add3(center, scale3(u,  size));
    dd_vec3 bot = add3(center, scale3(u, -size));
    dd_vec3 a   = add3(center, fwd);
    dd_vec3 b   = sub3(center, fwd);
    dd_vec3 cr  = add3(center, rt);
    dd_vec3 d   = sub3(center, rt);
    // Equator
    dd__push(a,cr); dd__push(cr,b);
    dd__push(b,d); dd__push(d, a);
    // Top/bot spikes
    dd__push(a,top); dd__push(cr,top);
    dd__push(b,top); dd__push(d, top);
    dd__push(a,bot); dd__push(cr,bot);
    dd__push(b,bot); dd__push(d, bot);
}

// TEXT 3D  (Hershey simplex vector font + camera-facing billboard)
//
// Glyph data: Paul Bourke / Hershey (public domain).
// Each entry: [num_verts][x_advance][x0][y0][x1][y1]...
// -----------------------------------------------------------------------------
// Pen-up encoded as (-1,-1). Coords are Hershey units (range ~-8..+8).

static const char *dd__hershey[] = { // simplex font
"AQ","IKFVFH@@FCEBFAGBFC","FQEVEO@@MVMO","LVLZE:@@RZK:@@EMSM@@DGRG","[UIZI=@@MZ"
"M=@@RSPUMVIVFUDSDQEOFNHMNKPJQIRGRDPBMAIAFBDD","`YVVDA@@IVKTKRJPHOFODQDSEUGVIVK"
"UNTQTTUVV@@RHPGOEOCQASAUBVDVFTHRH","c[XMXNWOVOUNTLRGPDNBLAHAFBECDEDGEIFJMNNOOQ"
"OSNULVJUISIQJNLKQDSBUAWAXBXC","HKFTEUFVGUGSFQEP","KOLZJXHUFQELEHFCH?J<L:","KOD"
"ZFXHUJQKLKHJCH?F<D:","IQIVIJ@@DSNM@@NSDM","F[NSNA@@EJWJ","IKGBFAEBFCGBG@F>E=",\
"C[EJWJ","FKFCEBFAGBFC","CWUZC:","RUJVGUERDMDJEEGBJALAOBQERJRMQROULVJV","EUGRIS"
"LVLA","OUEQERFTGUIVMVOUPTQRQPPNNKDARA","PUFVQVKNNNPMQLRIRGQDOBLAIAFBECDE","GUN"
"VDHSH@@NVNA","RUPVFVEMFNIOLOONQLRIRGQDOBLAIAFBECDE","XUQSPUMVKVHUFREMEHFDHBKAL"
"AOBQDRGRHQKOMLNKNHMFKEH","FURVHA@@DVRV","^UIVFUESEQFOHNLMOLQJRHREQCPBMAIAFBECD"
"EDHEJGLJMNNPOQQQSPUMVIV","XUQOPLNJKIJIGJELDODPESGUJVKVNUPSQOQJPENBKAIAFBED","L"
"KFOENFMGNFO@@FCEBFAGBFC","OKFOENFMGNFO@@GBFAEBFCGBG@F>E=","DYUSEJUA","F[EMWM@@"
"EGWG","DYESUJEA","USDQDRETFUHVLVNUOTPRPPONNMJKJH@@JCIBJAKBJC","x\\SNRPPQMQKPJO"
"ILIIJGLFOFQGRI@@MQKOJLJIKGLF@@SQRIRGTFVFXHYKYMXPWRUTSUPVMVJUHTFREPDMDJEGFEHCJB"
"MAPASBUCVD@@TQSISGTF","ISJVBA@@JVRA@@EHOH","XVEVEA@@EVNVQURTSRSPRNQMNL@@ELNLQK"
"RJSHSERCQBNAEA","SVSQRSPUNVJVHUFSEQDNDIEFFDHBJANAPBRDSF","PVEVEA@@EVLVOUQSRQSN"
"SIRFQDOBLAEA","LTEVEA@@EVRV@@ELML@@EARA","ISEVEA@@EVRV@@ELML","WVSQRSPUNVJVHUF"
"SEQDNDIEFFDHBJANAPBRDSFSI@@NISI","IWEVEA@@SVSA@@ELSL","CIEVEA","KQMVMFLCKBIAGA"
"EBDCCFCH","IVEVEA@@SVEH@@JMSA","FREVEA@@EAQA","LYEVEA@@EVMA@@UVMA@@UVUA","IWEV"
"EA@@EVSA@@SVSA","VWJVHUFSEQDNDIEFFDHBJANAPBRDSFTITNSQRSPUNVJV","NVEVEA@@EVNVQU"
"RTSRSORMQLNKEK","YWJVHUFSEQDNDIEFFDHBJANAPBRDSFTITNSQRSPUNVJV@@MES?","QVEVEA@@"
"EVNVQURTSRSPRNQMNLEL@@LLSA","UURSPUMVIVFUDSDQEOFNHMNKPJQIRGRDPBMAIAFBDD","FQIV"
"IA@@BVPV","KWEVEGFDHBKAMAPBRDSGSV","FSBVJA@@RVJA","LYCVHA@@MVHA@@MVRA@@WVRA",""
"FUDVRA@@RVDA","GSBVJLJA@@RVJL","IURVDA@@DVRV@@DARA","LOEZE:@@FZF:@@EZLZ@@E:L:",
"COAVO>","LOJZJ:@@KZK:@@DZKZ@@D:K:","KQGPISKP@@DMIRNM@@IRIA","CQA?Q?","HKGVFUES"
"EQFPGQFR","RTPOPA@@PLNNLOIOGNELDIDGEDGBIALANBPD","RTEVEA@@ELGNIOLONNPLQIQGPDNB"
"LAIAGBED","OSPLNNLOIOGNELDIDGEDGBIALANBPD","RTPVPA@@PLNNLOIOGNELDIDGEDGBIALANB"
"PD","RSDIPIPKOMNNLOIOGNELDIDGEDGBIALANBPD","IMKVIVGUFRFA@@COJO","WTPOP?O<N;L:I"
":G;@@PLNNLOIOGNELDIDGEDGBIALANBPD","KTEVEA@@EKHNJOMOONPKPA","IIDVEUFVEWDV@@EOE"
"A","LKFVGUHVGWFV@@GOG>F;D:B:","IREVEA@@OOEE@@IIPA","CIEVEA","S_EOEA@@EKHNJOMOO"
"NPKPA@@PKSNUOXOZN[K[A","KTEOEA@@EKHNJOMOONPKPA","RTIOGNELDIDGEDGBIALANBPDQGQIP"
"LNNLOIO","RTEOE:@@ELGNIOLONNPLQIQGPDNBLAIAGBED","RTPOP:@@PLNNLOIOGNELDIDGEDGBI"
"ALANBPD","INEOEA@@EIFLHNJOMO","RROLNNKOHOENDLEJGILHNGOEODNBKAHAEBDD","IMFVFEGB"
"IAKA@@COJO","KTEOEEFBHAKAMBPE@@POPA","FQCOIA@@OOIA","LWDOHA@@LOHA@@LOPA@@TOPA",
"FRDOOA@@OODA","JQCOIA@@OOIAG=E;C:B:","IROODA@@DOOO@@DAOA","hOJZHYGXFVFTGRHQIOI"
"MGK@@HYGWGUHSIRJPJNILEJIHJFJDIBHAG?G=H;@@GIIGIEHCGBF@F>G<H;J:","CIEZE:","hOFZH"
"YIXJVJTIRHQGOGMIK@@HYIWIUHSGRFPFNGLKJGHFFFDGBHAI?I=H;@@IIGGGEHCIBJ@J>I<H;F:",""
"XYDGDIELGMIMKLOIQHSHUIVK@@DIEKGLILKKOHQGSGUHVKVM" };

// Render a Hershey glyph as 3D line segments on a camera-facing billboard.
// `origin` : world-space anchor (bottom-left of glyph)
// `right_w`: world-space right direction scaled by one Hershey unit
// `up_w`   : world-space up direction scaled by one Hershey unit
static float dd__hershey_glyph(dd_vec3 origin, dd_vec3 right_w, dd_vec3 up_w, char ch) {
    if (ch < 32 || ch > 126) return 0.0f;
    const char *g = dd__hershey[ch - 32];
    int nverts   = (int)(g[0] - 65);
    float advance = (float)(g[1] - 65);

    bool pen_down = false;
    dd_vec3 prev3 = {0};
    for (int i = 0; i < nverts; i++) {
        int gx = (int)(g[2 + i*2 + 0] - 65);
        int gy = (int)(g[2 + i*2 + 1] - 65);
        if (gx == -1 && gy == -1) { pen_down = false; continue; }
        dd_vec3 cur = add3(origin,
            add3(scale3(right_w, (float)gx), scale3(up_w, (float)gy)));
        if (pen_down) dd__push(prev3, cur);
        pen_down = true;
        prev3 = cur;
    }
    return advance;
}

void dd_label3d(dd_vec3 world_pos, float scale, const char *fmt, ...) {
    char buf[256];
    va_list ap; va_start(ap, fmt);
    SDL_vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    // Build camera-facing billboard basis
    dd_vec3 cam_fwd = norm3(sub3(dd_ctx->cam.target, dd_ctx->cam.position));
    dd_vec3 cam_rt  = norm3(cross3(cam_fwd, dd_ctx->cam.up));
    dd_vec3 cam_up  = cross3(cam_rt, cam_fwd);

    // Scale: `scale` is world units per Hershey unit.
    // Typical Hershey glyph height ~14 units -> scale=0.01 gives ~0.14 wu tall
    dd_vec3 right_w = scale3(cam_rt, scale);
    dd_vec3 up_w    = scale3(cam_up, scale);

    // Hershey 'W' glyph [1]-65 gives the line height
    float line_h = (float)(dd__hershey['W' - 32][1] - 65) * 1.35f;

    dd_vec3 line_start = world_pos;
    dd_vec3 cursor     = line_start;

    for (const char *p = buf; *p; p++) {
        if (*p == '\n') {
            // Move down one line
            line_start = add3(line_start, scale3(up_w, -line_h));
            cursor = line_start;
            continue;
        }
        float adv = dd__hershey_glyph(cursor, right_w, up_w, *p);
        cursor = add3(cursor, scale3(right_w, adv));
    }
}

void dd_label3dTimed(dd_vec3 pos, float scale, float lt, const char *fmt, ...) {
    // Render into frame buffer then migrate to timed buffer
    char buf[256];
    va_list ap; va_start(ap, fmt);
    SDL_vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    DD__PUSH_TIMED_BEGIN(lt)
    dd_label3d(pos, scale, "%s", buf);
    DD__PUSH_TIMED_END()
}
// dd_gizmo.c  -  Translation / Rotation / Scale gizmo visuals + interaction

// -----------------------------------------------------------------------------
// GEOMETRY CONSTANTS  (all as fractions of world-scale `s`)

#define SHAFT_F      0.82f   // shaft end as fraction of s
#define CONE_R_F     0.07f   // cone base radius fraction 
#define CONE_H_F     0.18f   // cone height fraction      
#define PLANE_OFF_F  0.22f   // plane handle offset       
#define PLANE_SZ_F   0.20f   // plane handle side length  
#define CUBE_H_F     0.060f  // scale cube half-extent    
#define RING_R_F     0.50f   // rotation ring radius      
#define OUTER_R_F    0.54f   // outer (view-plane) ring   
#define CENTER_R_F   0.12f   // centre sphere pick radius 

// -----------------------------------------------------------------------------
// INTERNAL PICK STATE  (static: one active gizmo drag at a time)

typedef struct {
    bool         dragging;
    bool         claimed;
    int          gizmo_type; // 0=translate, 1=translate2d, 2=rotate, 3=rotate2d, 4=scale
    dd_gizmoAxis part;

    // Axis/plane drag: store the constraint
    dd_vec3      drag_origin;   // gizmo origin at drag start
    dd_vec3      drag_axis;     // unit constraint axis (axis drag)
    dd_vec3      drag_normal;   // plane normal (plane drag)
    dd_vec3      drag_hit;      // world-space ray hit at drag start

    // Rotate drag
    dd_vec3      rot_axis;      // rotation axis (world)
    float        rot_start_angle; // atan2 of first hit on ring plane

    // Scale drag
    float        scale_start_mouse; // 1D mouse position at drag start

    // 2D modes: screen-space anchor
    float        anchor_mx, anchor_my;

    // View-plane rotate: projected gizmo center in screen pixels
    float        proj_cx, proj_cy;
    float        rot2d_start_angle;
} GizmoPickState;

static GizmoPickState g_pick; // single global: one drag at a time

// -----------------------------------------------------------------------------
// MATH HELPERS

static float gizmo__world_scale(dd_vec3 origin, float screen_size) {
    const dd_camera_t *cam = dd_getCamera();
    int vp_w, vp_h;
    dd_getViewport(&vp_w, &vp_h);
    (void)vp_w;
    dd_vec3 to = sub3(origin, cam->position);
    float d = len3(to);
    if (d < 0.001f) d = 0.001f;
    float tan_half = tanf(cam->fov_y * (DD_PI / 180.0f) * 0.5f);
    float ppu = (float)vp_h / (2.0f * d * tan_half);
    return screen_size / ppu;
}

// Build a world-space ray from a pixel coordinate.
// Subtracts the viewport offset (inp->vp_x, inp->vp_y) before converting
// to NDC so picking works even when the renderer viewport is offset inside
// the OS window.
static void gizmo__mouse_ray(const dd_gizmoInput *inp, dd_vec3 *ray_o, dd_vec3 *ray_d) {
    const dd_camera_t *cam = dd_getCamera();
    int vp_w, vp_h;
    dd_getViewport(&vp_w, &vp_h);

    // Subtract viewport offset so coords are relative to the render area
    float lx = inp->mouse_x - inp->vp_x;
    float ly = inp->mouse_y - inp->vp_y;

    // NDC [-1,1]
    float ndcx = (lx / (float)vp_w) * 2.0f - 1.0f;
    float ndcy = 1.0f - (ly / (float)vp_h) * 2.0f;

    float tan_half = tanf(cam->fov_y * (DD_PI / 180.0f) * 0.5f);
    float aspect   = (float)vp_w / (float)vp_h;

    // Camera basis
    dd_vec3 fwd   = norm3(sub3(cam->target, cam->position));
    dd_vec3 right = norm3(cross3(fwd, cam->up));
    dd_vec3 up    = cross3(right, fwd);

    *ray_o = cam->position;
    *ray_d = norm3(
        add3(fwd,
        add3(scale3(right, ndcx * aspect * tan_half),
               scale3(up,    ndcy * tan_half))));
}

// Ray vs infinite plane: returns t (distance along ray), or -1 if parallel
static float ray_plane(dd_vec3 ro, dd_vec3 rd, dd_vec3 plane_pt, dd_vec3 plane_n) {
    float denom = dot3(rd, plane_n);
    if (SDL_fabsf(denom) < 1e-6f) return -1.0f;
    return dot3(sub3(plane_pt, ro), plane_n) / denom;
}

// Ray vs infinite cylinder (axis through origin along dir, radius r).
// Returns smallest positive t, or -1.
static float ray_cylinder(dd_vec3 ro, dd_vec3 rd, dd_vec3 axis_origin, dd_vec3 axis_dir, float r) {
    dd_vec3 oc = sub3(ro, axis_origin);
    float dd = dot3(rd, axis_dir);
    float oo = dot3(oc, axis_dir);
    dd_vec3 perp_d = sub3(rd, scale3(axis_dir, dd));
    dd_vec3 perp_o = sub3(oc, scale3(axis_dir, oo));
    float a = dot3(perp_d, perp_d);
    if (a < 1e-8f) return -1.0f;
    float b = 2.0f * dot3(perp_o, perp_d);
    float c = dot3(perp_o, perp_o) - r * r;
    float disc = b*b - 4.0f*a*c;
    if (disc < 0.0f) return -1.0f;
    float sq = SDL_sqrtf(disc);
    float t0 = (-b - sq) / (2.0f * a);
    float t1 = (-b + sq) / (2.0f * a);
    if (t0 > 0.001f) return t0;
    if (t1 > 0.001f) return t1;
    return -1.0f;
}

// Project a world point to screen pixels (including vp offset). Returns false if behind camera.
static bool world_to_screen(const dd_vec3 p, const dd_gizmoInput *inp, float *sx, float *sy) {
    const dd_camera_t *cam = dd_getCamera();
    int vp_w, vp_h;
    dd_getViewport(&vp_w, &vp_h);
    dd_vec3 fwd   = norm3(sub3(cam->target, cam->position));
    dd_vec3 right = norm3(cross3(fwd, cam->up));
    dd_vec3 up    = cross3(right, fwd);
    dd_vec3 d     = sub3(p, cam->position);
    float z    = dot3(d, fwd);
    if (z < 0.001f) return false;
    float tan_half = tanf(cam->fov_y * (DD_PI / 180.0f) * 0.5f);
    float aspect   = (float)vp_w / (float)vp_h;
    float px = dot3(d, right) / (z * aspect * tan_half);
    float py = dot3(d, up)    / (z * tan_half);
    // Add viewport offset so result is in the same space as mouse coords
    *sx = ( px * 0.5f + 0.5f) * (float)vp_w + inp->vp_x;
    *sy = (-py * 0.5f + 0.5f) * (float)vp_h + inp->vp_y;
    return true;
}

// Closest point along a ray to a given line (for axis drag)
static float ray_closest_t_to_line(dd_vec3 ro, dd_vec3 rd, dd_vec3 lo, dd_vec3 ld) {
    dd_vec3 r = sub3(ro, lo);
    float a = dot3(rd, rd);
    float b = dot3(rd, ld);
    float c = dot3(ld, ld);
    float d = dot3(rd, r);
    float e = dot3(ld, r);
    float denom = a*c - b*b;
    if (SDL_fabsf(denom) < 1e-8f) return 0.0f;
    return (b*e - c*d) / denom;  // t along ray
}

// Parameter along constraint axis of closest point on axis to a ray
static float ray_axis_param(dd_vec3 ro, dd_vec3 rd, dd_vec3 axis_o, dd_vec3 axis_d) {
    dd_vec3 r = sub3(ro, axis_o);
    float a = dot3(rd, rd);
    float b = dot3(rd, axis_d);
    float c = dot3(axis_d, axis_d);
    float d = dot3(rd, r);
    float e = dot3(axis_d, r);
    float denom = a*c - b*b;
    if (SDL_fabsf(denom) < 1e-8f) return 0.0f;
    return (a*e - b*d) / denom;  // t along axis
}

// Signed angle on a plane defined by (right, up) of a point relative to center
static float angle_on_plane(dd_vec3 p, dd_vec3 center, dd_vec3 right, dd_vec3 up) {
    dd_vec3 d = sub3(p, center);
    return SDL_atan2f(dot3(d, up), dot3(d, right));
}

// -----------------------------------------------------------------------------
// COLOR

static dd_rgba gizmo__col(dd_gizmoAxis which, dd_gizmoAxis hot, bool dim) {
    dd_rgba base;
    switch (which) {
    case DD_GIZMO_AXIS_X:    base = DD_RED; break;
    case DD_GIZMO_AXIS_Y:    base = DD_GREEN; break;
    case DD_GIZMO_AXIS_Z:    base = DD_BLUE; break;
    case DD_GIZMO_AXIS_XY:   base = DD_YELLOW; break;
    case DD_GIZMO_AXIS_XZ:   base = DD_MAGENTA; break;
    case DD_GIZMO_AXIS_YZ:   base = DD_CYAN; break;
    case DD_GIZMO_AXIS_ALL:  base = DD_WHITE2; break;
    case DD_GIZMO_AXIS_VIEW: base = DD_ORANGE; break;
    default:                 base = DD_ORANGE; break;
    }
    bool is_hot = (hot != DD_GIZMO_AXIS_NONE) && (which == hot);
    if (is_hot) {
        base.r = (Uint8)(base.r + (255 - base.r) * 0.55f);
        base.g = (Uint8)(base.g + (255 - base.g) * 0.55f);
        base.b = (Uint8)(base.b + (255 - base.b) * 0.55f);
    }
    if (dim) base.a = 90;
    return base;
}

// -----------------------------------------------------------------------------
// DRAW HELPERS  (same as before)

static void gizmo__arrow(dd_vec3 origin, dd_vec3 dir, float s, float shaft_t, float tip_r, float tip_h) {
    dd_vec3 shaft_end = add3(origin, scale3(dir, s * shaft_t));
    dd_vec3 tip_end   = add3(origin, scale3(dir, s * (shaft_t + tip_h)));
    dd_line(origin, shaft_end);
    dd_cone(tip_end, shaft_end, s * tip_r, 8);
}

static void gizmo__plane_handle(dd_vec3 origin, dd_vec3 a, dd_vec3 b, float offset, float side) {
    dd_vec3 o  = add3(origin, add3(scale3(a, offset), scale3(b, offset)));
    dd_vec3 p0 = o;
    dd_vec3 p1 = add3(o, scale3(a, side));
    dd_vec3 p2 = add3(o, add3(scale3(a, side), scale3(b, side)));
    dd_vec3 p3 = add3(o, scale3(b, side));
    dd_line(p0, p1);
    dd_line(p1, p2);
    dd_line(p2, p3);
    dd_line(p3, p0);
}

static void gizmo__cube(dd_vec3 center, dd_vec3 ax, dd_vec3 ay, dd_vec3 az, float h) {
    dd_obb(center, scale3(ax, h), scale3(ay, h), scale3(az, h));
}

static void gizmo__thick_line(dd_vec3 from, dd_vec3 to, dd_vec3 dir, float s) {
    dd_vec3 perp_ref = (SDL_fabsf(dir.x) < 0.9f) ? dd3(1,0,0) : dd3(0,1,0);
    dd_vec3 perp = norm3(cross3(dir, perp_ref));
    float thk = s * 0.008f;
    dd_line(add3(from, scale3(perp,  thk)), add3(to, scale3(perp,  thk)));
    dd_line(add3(from, scale3(perp, -thk)), add3(to, scale3(perp, -thk)));
}

static void gizmo__rot_ring(dd_vec3 origin, dd_vec3 normal, float radius, int segs, dd_gizmoAxis which, dd_gizmoAxis hot) {
    dd_vec3 ref   = SDL_fabsf(normal.x) < 0.9f ? dd3(1,0,0) : dd3(0,1,0);
    dd_vec3 right = norm3(cross3(ref, normal));
    dd_vec3 up    = cross3(normal, right);
    const dd_camera_t *cam = dd_getCamera();
    dd_vec3 cam_fwd = norm3(sub3(origin, cam->position));
    dd_rgba col_front = gizmo__col(which, hot, false);
    dd_rgba col_back  = gizmo__col(which, hot, true);
    bool is_hot = (hot != DD_GIZMO_AXIS_NONE) && (which == hot);

    dd_vec3 prev = add3(origin, scale3(right, radius));
    dd_rgba bak = *dd_ctx->dd_color_inuse;
    for (int i = 1; i <= segs; i++) {
        float t   = (float)i / (float)segs * DD_TAU;
        dd_vec3 cur = add3(origin, add3(scale3(right, SDL_cosf(t) * radius), scale3(up, SDL_sinf(t) * radius)));
        dd_vec3 mid     = dd3(0.5f*(prev.x+cur.x), 0.5f*(prev.y+cur.y), 0.5f*(prev.z+cur.z));
        dd_vec3 mid_dir = norm3(sub3(mid, origin));
        bool front = dot3(mid_dir, cam_fwd) < 0.0f;
        dd_color(front ? col_front : col_back);
        dd_line(prev, cur);
        if (is_hot && front) {
            float ri = radius * 0.91f;
            float tp = t - DD_TAU / (float)segs;
            dd_vec3 pi = add3(origin, add3(scale3(right, SDL_cosf(tp)*ri), scale3(up, SDL_sinf(tp)*ri)));
            dd_vec3 ci = add3(origin, add3(scale3(right, SDL_cosf(t)*ri),  scale3(up, SDL_sinf(t)*ri)));
            dd_line(pi, ci);
        }
        prev = cur;
    }
    dd_color(bak);
}

// -----------------------------------------------------------------------------
// HOVER TESTS  (no drag yet: just "is the cursor near this part?")

// Hit-test a gizmo arrow shaft + cone tip.
// Returns the ray-t of the hit, or -1.
// We test the shaft as a thick cylinder (radius = pick_r) and
// the cone tip as a sphere of the same radius.
static float pick_arrow(dd_vec3 ro, dd_vec3 rd, dd_vec3 origin, dd_vec3 dir, float s) {
    float shaft_end = s * (SHAFT_F + CONE_H_F);
    float pick_r    = s * 0.055f;   // generous pick cylinder

    float t = ray_cylinder(ro, rd, origin, dir, pick_r);
    if (t > 0.001f) {
        // Check the hit is within [0, shaft_end] along the axis
        dd_vec3 hit = add3(ro, scale3(rd, t));
        float along = dot3(sub3(hit, origin), dir);
        if (along >= 0.0f && along <= shaft_end)
            return t;
    }
    return -1.0f;
}

// Hit-test a plane quad handle.
// Returns the ray-t of the hit inside the quad, or -1.
static float pick_plane_quad(dd_vec3 ro, dd_vec3 rd, dd_vec3 origin, dd_vec3 a, dd_vec3 b, float offset, float side) {
    // Normal of the plane = cross(a,b)
    dd_vec3 normal = norm3(cross3(a, b));
    dd_vec3 corner = add3(origin, add3(scale3(a, offset), scale3(b, offset)));
    float t = ray_plane(ro, rd, corner, normal);
    if (t < 0.001f) return -1.0f;
    dd_vec3 hit  = add3(ro, scale3(rd, t));
    dd_vec3 diff = sub3(hit, corner);
    float ua = dot3(diff, a);
    float ub = dot3(diff, b);
    if (ua >= 0.0f && ua <= side && ub >= 0.0f && ub <= side)
        return t;
    return -1.0f;
}

// Hit-test a rotation ring (torus approximated as a flat annulus).
// Returns the ray-t, or -1.
static float pick_ring(dd_vec3 ro, dd_vec3 rd, dd_vec3 origin, dd_vec3 normal, float radius, float s) {
    float t = ray_plane(ro, rd, origin, normal);
    if (t < 0.001f) return -1.0f;
    dd_vec3 hit  = add3(ro, scale3(rd, t));
    float dist = len3(sub3(hit, origin));
    float band = s * 0.06f;   // pick band around ring
    if (dist >= radius - band && dist <= radius + band)
        return t;
    return -1.0f;
}

// Hit-test the gizmo centre (small sphere).
static float pick_center(dd_vec3 ro, dd_vec3 rd, dd_vec3 origin, float s) {
    dd_vec3 oc = sub3(ro, origin);
    float r = s * CENTER_R_F;
    float b = dot3(oc, rd);
    float c = dot3(oc, oc) - r * r;
    float disc = b*b - c;
    if (disc < 0.0f) return -1.0f;
    float t = -b - SDL_sqrtf(disc);
    return t > 0.001f ? t : -1.0f;
}

// -----------------------------------------------------------------------------
// UPDATE: TRANSLATE 3D

void dd_gizmoUpdateTranslate(dd_gizmoDesc *g, const dd_gizmoInput *inp, dd_gizmoDelta *delta) {
    SDL_memset(delta, 0, sizeof(*delta));
    delta->scale = 1.0f;

    dd_vec3 ro, rd;
    gizmo__mouse_ray(inp, &ro, &rd);
    float s = gizmo__world_scale(g->origin, g->screen_size);

    // --- DRAG in progress ---
    if (g_pick.dragging && g_pick.gizmo_type == 0) {

        if (inp->mouse_released) {
            g_pick.dragging = false; g_pick.claimed = false;
            g->hot_axis = DD_GIZMO_AXIS_NONE;
            return;
        }

        delta->active = true;
        delta->part   = g_pick.part;
        g->hot_axis   = g_pick.part;

        if (g_pick.part == DD_GIZMO_AXIS_ALL) {
            // 2D screen-plane drag: pixel delta -> world via camera right/up
            const dd_camera_t *cam = dd_getCamera();
            dd_vec3 fwd   = norm3(sub3(cam->target, cam->position));
            dd_vec3 right = norm3(cross3(fwd, cam->up));
            dd_vec3 up    = cross3(right, fwd);
            float wpp = s / g->screen_size;
            float dmx = inp->mouse_x - g_pick.anchor_mx;
            float dmy = inp->mouse_y - g_pick.anchor_my;
            g_pick.anchor_mx = inp->mouse_x;
            g_pick.anchor_my = inp->mouse_y;
            delta->translate = add3(scale3(right,  dmx * wpp), scale3(up, -dmy * wpp));
        } else if (g_pick.part == DD_GIZMO_AXIS_X ||
                   g_pick.part == DD_GIZMO_AXIS_Y ||
                   g_pick.part == DD_GIZMO_AXIS_Z) {
            float cur_t = ray_axis_param(ro, rd, g->origin, g_pick.drag_axis);
            dd_vec3 cur_hit = add3(g->origin, scale3(g_pick.drag_axis, cur_t));
            delta->translate = sub3(cur_hit, g_pick.drag_hit);
            g_pick.drag_hit = cur_hit;
        } else {
            // Plane drag
            float t = ray_plane(ro, rd, g->origin, g_pick.drag_normal);
            if (t > 0.001f) {
                dd_vec3 cur_hit = add3(ro, scale3(rd, t));
                delta->translate = sub3(cur_hit, g_pick.drag_hit);
                g_pick.drag_hit  = cur_hit;
            }
        }
        return;
    }

    // --- HOVER ---
    if (!inp->mouse_down) {
        float best_t = 1e30f;
        dd_gizmoAxis best = DD_GIZMO_AXIS_NONE;

        // Centre sphere -> 2D screen pan
        float t_ctr = pick_center(ro, rd, g->origin, s);
        if (t_ctr > 0.001f) { best_t = t_ctr; best = DD_GIZMO_AXIS_ALL; }

        // Arrows
        struct { dd_vec3 dir; dd_gizmoAxis id; } axes[3] = {
            {g->axis_x, DD_GIZMO_AXIS_X},
            {g->axis_y, DD_GIZMO_AXIS_Y},
            {g->axis_z, DD_GIZMO_AXIS_Z},
        };
        for (int i = 0; i < 3; i++) {
            float t = pick_arrow(ro, rd, g->origin, axes[i].dir, s);
            if (t > 0.001f && t < best_t) { best_t = t; best = axes[i].id; }
        }

        // Plane quads
        struct { dd_vec3 a, b; dd_gizmoAxis id; } planes[3] = {
            {g->axis_x, g->axis_y, DD_GIZMO_AXIS_XY},
            {g->axis_x, g->axis_z, DD_GIZMO_AXIS_XZ},
            {g->axis_y, g->axis_z, DD_GIZMO_AXIS_YZ},
        };
        float po = s * PLANE_OFF_F, ps = s * PLANE_SZ_F;
        for (int i = 0; i < 3; i++) {
            float t = pick_plane_quad(ro, rd, g->origin, planes[i].a, planes[i].b, po, ps);
            if (t > 0.001f && t < best_t) { best_t = t; best = planes[i].id; }
        }

        g->hot_axis = best;
    }

    // --- PRESS: begin drag ---
    if (inp->mouse_pressed && g->hot_axis != DD_GIZMO_AXIS_NONE) {
        g_pick.dragging    = true; g_pick.claimed = true;
        g_pick.gizmo_type  = 0;
        g_pick.part        = g->hot_axis;

        if (g->hot_axis == DD_GIZMO_AXIS_ALL) {
            // 2D pan: anchor mouse position
            g_pick.anchor_mx = inp->mouse_x;
            g_pick.anchor_my = inp->mouse_y;
        } else if (g->hot_axis == DD_GIZMO_AXIS_X ||
                   g->hot_axis == DD_GIZMO_AXIS_Y ||
                   g->hot_axis == DD_GIZMO_AXIS_Z) {
            g_pick.drag_axis = (g->hot_axis == DD_GIZMO_AXIS_X) ? g->axis_x :
                               (g->hot_axis == DD_GIZMO_AXIS_Y) ? g->axis_y : g->axis_z;
            float t0 = ray_axis_param(ro, rd, g->origin, g_pick.drag_axis);
            g_pick.drag_hit = add3(g->origin, scale3(g_pick.drag_axis, t0));
        } else {
            const dd_camera_t *cam = dd_getCamera();
            dd_vec3 plane_n;
            if (g->hot_axis == DD_GIZMO_AXIS_XY)      plane_n = g->axis_z;
            else if (g->hot_axis == DD_GIZMO_AXIS_XZ)  plane_n = g->axis_y;
            else                                          plane_n = g->axis_x;
            if (dot3(plane_n, sub3(cam->position, g->origin)) < 0.0f)
                plane_n = scale3(plane_n, -1.0f);
            g_pick.drag_normal = plane_n;
            float t0 = ray_plane(ro, rd, g->origin, plane_n);
            g_pick.drag_hit = t0 > 0.001f ? add3(ro, scale3(rd, t0)) : g->origin;
        }
    }
}

// -----------------------------------------------------------------------------
// UPDATE: TRANSLATE 2D  (screen-plane drag via centre square)

void dd_gizmoUpdateTranslate2D(dd_gizmoDesc *g, const dd_gizmoInput *inp, dd_gizmoDelta *delta) {
    SDL_memset(delta, 0, sizeof(*delta));
    delta->scale = 1.0f;

    float s = gizmo__world_scale(g->origin, g->screen_size);

    // Project gizmo origin to screen to decide if cursor is in center region
    float sx, sy;
    bool visible = world_to_screen(g->origin, inp, &sx, &sy);

    // Pick radius for the centre handle in pixels
    float pick_px = g->screen_size * PLANE_OFF_F * 1.4f;

    // --- DRAG ---
    if (g_pick.dragging && g_pick.gizmo_type == 1 && g_pick.part == DD_GIZMO_AXIS_XY) {
        if (inp->mouse_released) {
            g_pick.dragging = false; g_pick.claimed = false;
            g->hot_axis = DD_GIZMO_AXIS_NONE;
            return;
        }

        delta->active = true;
        delta->part   = DD_GIZMO_AXIS_XY;
        g->hot_axis   = DD_GIZMO_AXIS_XY;

        // Convert pixel delta to world-space displacement on camera plane
        const dd_camera_t *cam = dd_getCamera();
        dd_vec3 fwd   = norm3(sub3(cam->target, cam->position));
        dd_vec3 right = norm3(cross3(fwd, cam->up));
        dd_vec3 up    = cross3(right, fwd);

        // world_units_per_pixel = world_scale / screen_size
        float wpp = s / g->screen_size;

        float dmx = inp->mouse_x - g_pick.anchor_mx;
        float dmy = inp->mouse_y - g_pick.anchor_my;
        g_pick.anchor_mx = inp->mouse_x;
        g_pick.anchor_my = inp->mouse_y;

        delta->translate = add3(scale3(right,  dmx * wpp), scale3(up, -dmy * wpp));
        return;
    }

    // --- HOVER ---
    if (!inp->mouse_down) {
        float dx = inp->mouse_x - sx;
        float dy = inp->mouse_y - sy;
        g->hot_axis = (visible && SDL_sqrtf(dx*dx + dy*dy) < pick_px) ? DD_GIZMO_AXIS_XY : DD_GIZMO_AXIS_NONE;
    }

    // --- PRESS ---
    if (inp->mouse_pressed && g->hot_axis == DD_GIZMO_AXIS_XY) {
        g_pick.dragging   = true; g_pick.claimed = true;
        g_pick.gizmo_type = 1;
        g_pick.part       = DD_GIZMO_AXIS_XY;
        g_pick.anchor_mx  = inp->mouse_x;
        g_pick.anchor_my  = inp->mouse_y;
    }
}

// -----------------------------------------------------------------------------
// UPDATE: ROTATE 3D

void dd_gizmoUpdateRotate(dd_gizmoDesc *g, const dd_gizmoInput *inp, dd_gizmoDelta *delta) {
    SDL_memset(delta, 0, sizeof(*delta));
    delta->scale = 1.0f;

    dd_vec3 ro, rd;
    gizmo__mouse_ray(inp, &ro, &rd);
    float s = gizmo__world_scale(g->origin, g->screen_size);
    float ring_r = s * RING_R_F;

    // --- DRAG ---
    if (g_pick.dragging && g_pick.gizmo_type == 2) {

        if (inp->mouse_released) {
            g_pick.dragging = false; g_pick.claimed = false;
            g->hot_axis = DD_GIZMO_AXIS_NONE;
            return;
        }

        delta->active = true;
        delta->part   = g_pick.part;
        g->hot_axis   = g_pick.part;

        if (g_pick.part == DD_GIZMO_AXIS_VIEW) {
            // View-plane rotate: angle from projected gizmo centre
            float cur_angle = SDL_atan2f(-(inp->mouse_y - g_pick.proj_cy), inp->mouse_x - g_pick.proj_cx);
            delta->rotate = cur_angle - g_pick.rot_start_angle;
            while (delta->rotate >  DD_PI) delta->rotate -= DD_TAU;
            while (delta->rotate < -DD_PI) delta->rotate += DD_TAU;
            g_pick.rot_start_angle = cur_angle;
        } else {
            // 3D axis rotate
            float t = ray_plane(ro, rd, g->origin, g_pick.rot_axis);
            if (t > 0.001f) {
                dd_vec3 hit = add3(ro, scale3(rd, t));
                dd_vec3 ref   = SDL_fabsf(g_pick.rot_axis.x) < 0.9f ? dd3(1,0,0) : dd3(0,1,0);
                dd_vec3 right = norm3(cross3(ref, g_pick.rot_axis));
                dd_vec3 up    = cross3(g_pick.rot_axis, right);
                float cur_angle = angle_on_plane(hit, g->origin, right, up);
                delta->rotate = cur_angle - g_pick.rot_start_angle;
                while (delta->rotate >  DD_PI) delta->rotate -= DD_TAU;
                while (delta->rotate < -DD_PI) delta->rotate += DD_TAU;
                g_pick.rot_start_angle = cur_angle;
            }
        }
        return;
    }

    // --- HOVER ---
    if (!inp->mouse_down) {
        float best_t = 1e30f;
        dd_gizmoAxis best = DD_GIZMO_AXIS_NONE;

        // Outer view-plane ring: project a point on the ring to get true pixel radius
        float cx, cy;
        if (world_to_screen(g->origin, inp, &cx, &cy)) {
            // Project one point on the ring edge
            const dd_camera_t *cam2 = dd_getCamera();
            dd_vec3 view_n2 = norm3(sub3(g->origin, cam2->position));
            dd_vec3 ref2    = SDL_fabsf(view_n2.x) < 0.9f ? dd3(1,0,0) : dd3(0,1,0);
            dd_vec3 right2  = norm3(cross3(ref2, view_n2));
            dd_vec3 ring_pt = add3(g->origin, scale3(right2, s * OUTER_R_F));
            float rx, ry;
            if (world_to_screen(ring_pt, inp, &rx, &ry)) {
                float ring_px  = SDL_sqrtf((rx-cx)*(rx-cx) + (ry-cy)*(ry-cy));
                float band_px  = ring_px * 0.12f; // 12% band
                float dx = inp->mouse_x - cx, dy = inp->mouse_y - cy;
                float dist_px = SDL_sqrtf(dx*dx + dy*dy);
                if (SDL_fabsf(dist_px - ring_px) < band_px) {
                    best_t = 0.0f;
                    best = DD_GIZMO_AXIS_VIEW;
                }
            }
        }

        // 3D axis rings
        struct { dd_vec3 ax; dd_gizmoAxis id; } rings[3] = {
            {g->axis_x, DD_GIZMO_AXIS_X},
            {g->axis_y, DD_GIZMO_AXIS_Y},
            {g->axis_z, DD_GIZMO_AXIS_Z},
        };
        for (int i = 0; i < 3; i++) {
            float t = pick_ring(ro, rd, g->origin, rings[i].ax, ring_r, s);
            if (t > 0.001f && t < best_t) { best_t = t; best = rings[i].id; }
        }
        g->hot_axis = best;
    }

    // --- PRESS ---
    if (inp->mouse_pressed && g->hot_axis != DD_GIZMO_AXIS_NONE) {
        g_pick.dragging   = true; g_pick.claimed = true;
        g_pick.gizmo_type = 2;
        g_pick.part       = g->hot_axis;

        if (g->hot_axis == DD_GIZMO_AXIS_VIEW) {
            // Store projected centre for angle computation
            float cx, cy;
            world_to_screen(g->origin, inp, &cx, &cy);
            g_pick.proj_cx = cx; g_pick.proj_cy = cy;
            g_pick.rot_start_angle = SDL_atan2f(-(inp->mouse_y - cy), inp->mouse_x - cx);
        } else {
            g_pick.rot_axis = (g->hot_axis == DD_GIZMO_AXIS_X) ? g->axis_x :
                              (g->hot_axis == DD_GIZMO_AXIS_Y) ? g->axis_y : g->axis_z;
            float t0 = ray_plane(ro, rd, g->origin, g_pick.rot_axis);
            dd_vec3 ref   = SDL_fabsf(g_pick.rot_axis.x) < 0.9f ? dd3(1,0,0) : dd3(0,1,0);
            dd_vec3 right = norm3(cross3(ref, g_pick.rot_axis));
            dd_vec3 up    = cross3(g_pick.rot_axis, right);
            if (t0 > 0.001f) {
                dd_vec3 hit = add3(ro, scale3(rd, t0));
                g_pick.rot_start_angle = angle_on_plane(hit, g->origin, right, up);
            }
        }
    }
}

// -----------------------------------------------------------------------------
// UPDATE: ROTATE 2D  (view-plane, outer ring)

void dd_gizmoUpdateRotate2D(dd_gizmoDesc *g, const dd_gizmoInput *inp, dd_gizmoDelta *delta) {
    SDL_memset(delta, 0, sizeof(*delta));
    delta->scale = 1.0f;

    float s = gizmo__world_scale(g->origin, g->screen_size);

    // Project gizmo center to screen
    float cx, cy;
    bool visible = world_to_screen(g->origin, inp, &cx, &cy);
    if (!visible) { g->hot_axis = DD_GIZMO_AXIS_NONE; return; }

    // Pick band in pixels matching outer ring size
    float ring_px   = g->screen_size * OUTER_R_F;
    float band_px   = g->screen_size * 0.06f;

    float dx = inp->mouse_x - cx;
    float dy = inp->mouse_y - cy;
    float dist_px = SDL_sqrtf(dx*dx + dy*dy);

    // --- DRAG ---
    if (g_pick.dragging && g_pick.gizmo_type == 3 && g_pick.part == DD_GIZMO_AXIS_VIEW) {
        if (inp->mouse_released) {
            g_pick.dragging = false; g_pick.claimed = false;
            g->hot_axis = DD_GIZMO_AXIS_NONE;
            return;
        }

        delta->active = true;
        delta->part   = DD_GIZMO_AXIS_VIEW;
        g->hot_axis   = DD_GIZMO_AXIS_VIEW;

        // Compute signed angle delta in screen space.
        // atan2(dy,dx) gives the angle of the mouse from the projected center.
        // We negate dy because screen Y is flipped.
        
        float cur_angle = SDL_atan2f(-(inp->mouse_y - g_pick.proj_cy), inp->mouse_x - g_pick.proj_cx);
        delta->rotate = cur_angle - g_pick.rot2d_start_angle;
        while (delta->rotate >  DD_PI) delta->rotate -= DD_TAU;
        while (delta->rotate < -DD_PI) delta->rotate += DD_TAU;
        g_pick.rot2d_start_angle = cur_angle;

        (void)s;
        return;
    }

    // --- HOVER ---
    if (!inp->mouse_down) {
        bool on_ring = SDL_fabsf(dist_px - ring_px) < band_px;
        g->hot_axis = on_ring ? DD_GIZMO_AXIS_VIEW : DD_GIZMO_AXIS_NONE;
    }

    // --- PRESS ---
    if (inp->mouse_pressed && g->hot_axis == DD_GIZMO_AXIS_VIEW) {
        g_pick.dragging          = true; g_pick.claimed = true;
        g_pick.gizmo_type        = 3;
        g_pick.part              = DD_GIZMO_AXIS_VIEW;
        g_pick.proj_cx          = cx;
        g_pick.proj_cy          = cy;
        g_pick.rot2d_start_angle = SDL_atan2f(-(inp->mouse_y - cy), inp->mouse_x - cx);
    }
}

// -----------------------------------------------------------------------------
// UPDATE: SCALE 3D

void dd_gizmoUpdateScale(dd_gizmoDesc *g, const dd_gizmoInput *inp, dd_gizmoDelta *delta) {
    SDL_memset(delta, 0, sizeof(*delta));
    delta->scale = 1.0f;

    dd_vec3 ro, rd;
    gizmo__mouse_ray(inp, &ro, &rd);
    float s = gizmo__world_scale(g->origin, g->screen_size);

    // --- DRAG ---
    if (g_pick.dragging && g_pick.gizmo_type == 4) {

        if (inp->mouse_released) {
            g_pick.dragging = false; g_pick.claimed = false;
            g->hot_axis = DD_GIZMO_AXIS_NONE;
            return;
        }

        delta->active = true;
        delta->part   = g_pick.part;
        g->hot_axis   = g_pick.part;

        if (g_pick.part == DD_GIZMO_AXIS_ALL) {
            // Uniform scale: use vertical mouse movement.
            // Moving up (decreasing Y) = scale up.
            float dy = g_pick.scale_start_mouse - inp->mouse_y;
            g_pick.scale_start_mouse = inp->mouse_y;
            delta->scale = 1.0f + dy * 0.005f;
        } else {
            // Axis scale: project mouse onto shaft axis, measure how far
            // the cursor has moved along it. Convert to a scale factor.
            float cur_t = ray_axis_param(ro, rd, g->origin, g_pick.drag_axis);
            float shaft_len = s * SHAFT_F;
            float prev_t    = g_pick.scale_start_mouse; // reused for axis t
            g_pick.scale_start_mouse = cur_t;
            float dt = cur_t - prev_t;
            delta->scale = 1.0f + dt / shaft_len;
        }
        return;
    }

    // --- HOVER ---
    if (!inp->mouse_down) {
        float sc  = g->scale > 0.0f ? g->scale   : 1.0f;
        float sca[3] = {
            g->scale_x > 0.0f ? g->scale_x : 1.0f,
            g->scale_y > 0.0f ? g->scale_y : 1.0f,
            g->scale_z > 0.0f ? g->scale_z : 1.0f,
        };
        float best_t = 1e30f;
        dd_gizmoAxis best = DD_GIZMO_AXIS_NONE;
        struct { dd_vec3 dir; dd_gizmoAxis id; int i; } axes[3] = {
            {g->axis_x, DD_GIZMO_AXIS_X, 0},
            {g->axis_y, DD_GIZMO_AXIS_Y, 1},
            {g->axis_z, DD_GIZMO_AXIS_Z, 2},
        };
        for (int i = 0; i < 3; i++) {
            float shaft_len = s * (SHAFT_F + CONE_H_F) * sc * sca[i];
            float pick_r    = s * 0.055f;
            float t = ray_cylinder(ro, rd, g->origin, axes[i].dir, pick_r);
            if (t > 0.001f) {
                dd_vec3 hit = add3(ro, scale3(rd, t));
                float along = dot3(sub3(hit, g->origin), axes[i].dir);
                if (along >= 0.0f && along <= shaft_len && t < best_t)
                    { best_t = t; best = axes[i].id; }
            }
        }
        float t_ctr = pick_center(ro, rd, g->origin, s);
        if (t_ctr > 0.001f && t_ctr < best_t) best = DD_GIZMO_AXIS_ALL;
        g->hot_axis = best;
    }

    // --- PRESS ---
    if (inp->mouse_pressed && g->hot_axis != DD_GIZMO_AXIS_NONE) {
        g_pick.dragging   = true; g_pick.claimed = true;
        g_pick.gizmo_type = 4;
        g_pick.part       = g->hot_axis;

        if (g->hot_axis == DD_GIZMO_AXIS_ALL) {
            g_pick.scale_start_mouse = inp->mouse_y;
        } else {
            g_pick.drag_axis = (g->hot_axis == DD_GIZMO_AXIS_X) ? g->axis_x :
                               (g->hot_axis == DD_GIZMO_AXIS_Y) ? g->axis_y : g->axis_z;
            g_pick.scale_start_mouse = ray_axis_param(ro, rd, g->origin, g_pick.drag_axis);
        }
    }
}

// -----------------------------------------------------------------------------
// DRAW

void dd_gizmoTranslate(const dd_gizmoDesc *g) {
    float s = gizmo__world_scale(g->origin, g->screen_size);

    dd_rgba bak = *dd_ctx->dd_color_inuse;
    dd_rgba cx  = gizmo__col(DD_GIZMO_AXIS_X,  g->hot_axis, false);
    dd_rgba cy  = gizmo__col(DD_GIZMO_AXIS_Y,  g->hot_axis, false);
    dd_rgba cz  = gizmo__col(DD_GIZMO_AXIS_Z,  g->hot_axis, false);
    dd_rgba cxy = gizmo__col(DD_GIZMO_AXIS_XY, g->hot_axis, false);
    dd_rgba cxz = gizmo__col(DD_GIZMO_AXIS_XZ, g->hot_axis, false);
    dd_rgba cyz = gizmo__col(DD_GIZMO_AXIS_YZ, g->hot_axis, false);

    dd_color(cx); gizmo__arrow(g->origin, g->axis_x, s, SHAFT_F, CONE_R_F, CONE_H_F);
    dd_color(cy); gizmo__arrow(g->origin, g->axis_y, s, SHAFT_F, CONE_R_F, CONE_H_F);
    dd_color(cz); gizmo__arrow(g->origin, g->axis_z, s, SHAFT_F, CONE_R_F, CONE_H_F);

    float po = s * PLANE_OFF_F, ps = s * PLANE_SZ_F;
    dd_color(cxy); gizmo__plane_handle(g->origin, g->axis_x, g->axis_y, po, ps);
    dd_color(cxz); gizmo__plane_handle(g->origin, g->axis_x, g->axis_z, po, ps);
    dd_color(cyz); gizmo__plane_handle(g->origin, g->axis_y, g->axis_z, po, ps);

    // Centre sphere: 2D screen-pan handle: small yellow sphere
    {
        dd_rgba call = (g->hot_axis == DD_GIZMO_AXIS_ALL)
                       ? DD_RGBA(255, 255,  80, 255)  // hot: bright yellow
                       : DD_RGBA(220, 200,  40, 200); // idle: dim yellow
        dd_color(call);
        dd_sphereEx(g->origin, s * 0.045f, 10);
    }

    // Hot axis thickness
    dd_gizmoAxis h = g->hot_axis;
    if (h == DD_GIZMO_AXIS_X || h == DD_GIZMO_AXIS_Y || h == DD_GIZMO_AXIS_Z) {
        dd_vec3 dir = (h == DD_GIZMO_AXIS_X) ? g->axis_x : (h == DD_GIZMO_AXIS_Y) ? g->axis_y : g->axis_z;
        dd_rgba ch = gizmo__col(h, h, false);
        dd_vec3 end = add3(g->origin, scale3(dir, s * SHAFT_F));
        dd_color(ch);
        gizmo__thick_line(g->origin, end, dir, s);
    }

    *dd_ctx->dd_color_inuse = bak;
}

void dd_gizmoRotate(const dd_gizmoDesc *g) {
    float s    = gizmo__world_scale(g->origin, g->screen_size);
    int   segs = 48;
    float ring_r = s * RING_R_F;

    gizmo__rot_ring(g->origin, g->axis_x, ring_r, segs, DD_GIZMO_AXIS_X, g->hot_axis);
    gizmo__rot_ring(g->origin, g->axis_y, ring_r, segs, DD_GIZMO_AXIS_Y, g->hot_axis);
    gizmo__rot_ring(g->origin, g->axis_z, ring_r, segs, DD_GIZMO_AXIS_Z, g->hot_axis);

    // Outer view-plane ring: yellow, brighter when hot
    const dd_camera_t *cam = dd_getCamera();
    dd_vec3 view_n = norm3(sub3(g->origin, cam->position));
    dd_rgba cview  = (g->hot_axis == DD_GIZMO_AXIS_VIEW)
                     ? DD_RGBA(255, 230,  50, 255)   // hot: bright yellow
                     : DD_RGBA(200, 180,  40, 160);  // idle: dim yellow

    dd_pushColor(cview);
    dd_circle(g->origin, view_n, s * OUTER_R_F, segs);
    dd_popColors(1);

    // Hot ring extra thickness handled inside gizmo__rot_ring
}

void dd_gizmoScale(const dd_gizmoDesc *g) {
    float s  = gizmo__world_scale(g->origin, g->screen_size);
    float sc  = g->scale   > 0.0f ? g->scale   : 1.0f;
    float scx = g->scale_x > 0.0f ? g->scale_x : 1.0f;
    float scy = g->scale_y > 0.0f ? g->scale_y : 1.0f;
    float scz = g->scale_z > 0.0f ? g->scale_z : 1.0f;

    dd_rgba cx   = gizmo__col(DD_GIZMO_AXIS_X,   g->hot_axis, false);
    dd_rgba cy   = gizmo__col(DD_GIZMO_AXIS_Y,   g->hot_axis, false);
    dd_rgba cz   = gizmo__col(DD_GIZMO_AXIS_Z,   g->hot_axis, false);
    dd_rgba call = (g->hot_axis == DD_GIZMO_AXIS_ALL)
                    ? gizmo__col(DD_GIZMO_AXIS_ALL, g->hot_axis, false)
                    : DD_RGBA(255, 220, 50, 255);

    float shaft = SHAFT_F;
    float ch    = CUBE_H_F;

    // Each axis end cube moves by its own per-axis scale x uniform scale
    dd_vec3 ex = add3(g->origin, scale3(g->axis_x, s * shaft * sc * scx));
    dd_vec3 ey = add3(g->origin, scale3(g->axis_y, s * shaft * sc * scy));
    dd_vec3 ez = add3(g->origin, scale3(g->axis_z, s * shaft * sc * scz));

    dd_pushColor(cx); gizmo__cube(ex, g->axis_x, g->axis_y, g->axis_z, s * ch * sc * scx); dd_line(g->origin, ex);
    dd_color(cy);     gizmo__cube(ey, g->axis_x, g->axis_y, g->axis_z, s * ch * sc * scy); dd_line(g->origin, ey);
    dd_color(cz);     gizmo__cube(ez, g->axis_x, g->axis_y, g->axis_z, s * ch * sc * scz); dd_line(g->origin, ez);
    dd_color(call);   gizmo__cube(g->origin, g->axis_x, g->axis_y, g->axis_z, s * ch * 0.9f);

    dd_gizmoAxis h = g->hot_axis;
    if (h == DD_GIZMO_AXIS_X || h == DD_GIZMO_AXIS_Y || h == DD_GIZMO_AXIS_Z) {
        dd_vec3 dir  = (h == DD_GIZMO_AXIS_X) ? g->axis_x : (h == DD_GIZMO_AXIS_Y) ? g->axis_y : g->axis_z;
        float   hsc  = (h == DD_GIZMO_AXIS_X) ? scx : (h == DD_GIZMO_AXIS_Y) ? scy : scz;
        dd_vec3 end  = add3(g->origin, scale3(dir, s * shaft * sc * hsc));
        dd_color(gizmo__col(h, h, false)); gizmo__thick_line(g->origin, end, dir, s);
    }

    dd_popColors(1);
}

bool dd_gizmoIsDragging(void) { return g_pick.claimed; }

#endif // KIT_CODE

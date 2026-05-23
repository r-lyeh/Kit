/*
 * demo.c  –  Interactive demo for dd3d
 *
 * Controls:
 *   Mouse drag (LMB)  –  orbit camera
 *   Scroll wheel      –  zoom
 *   R                 –  reset camera
 *   ESC / Q           –  quit
 */

#include "kit.h"
const char *hints;

// ----------------------------------------------------------------------------
// demo

// ----------------------------------------------------------------------------
// Orbit camera state

typedef struct {
    float yaw;       // horizontal angle in degrees
    float pitch;     // vertical angle in degrees
    float distance;
    dd_vec3 target;
} orbital_camera_t;

static void orbital_to_dd(const orbital_camera_t *oc, dd_camera_t *cam) {
    float y   = oc->yaw   * (DD_PI / 180.0f);
    float p   = oc->pitch * (DD_PI / 180.0f);
    float d   = oc->distance;
    cam->position.x = oc->target.x + d * SDL_cosf(p) * SDL_sinf(y);
    cam->position.y = oc->target.y + d * SDL_sinf(p);
    cam->position.z = oc->target.z + d * SDL_cosf(p) * SDL_cosf(y);
    cam->target  = oc->target;
    cam->up      = dd3(0, 1, 0);
    cam->fov_y   = 60.0f;
    cam->near_z  = 0.1f;
    cam->far_z   = 500.0f;
}

/* -------------------------------------------------------------------------
 * Scene drawing
 * ---------------------------------------------------------------------- */

static void draw_scene(float t,
                       int gizmo_mode,
                       dd_gizmoDesc *gw,
                       const dd_gizmoInput *inp,
                       dd_gizmoDelta *delta) {
    /* ---- Ground grid ---- */
    /* Two-pass grid: major lines every 1 unit, 4 minor subdivisions.
       Axes are drawn automatically at grid extent, on top of the grid. */
    dd_grid(dd3(0,0,0), 1.0f, 20, DD_RGBA(100,100,100,255), DD_RGBA(50, 50, 50, 255), 4);

    /* ---- Static AABB ---- */
    dd_color(DD_CYAN), dd_aabb(dd3(-5,0,-2), dd3(-3,2,0));

    /* ---- v5: Group 1 = "physics" — toggle with P key ---- */
    dd_setGroup(1);
    dd_label(dd3(-5, 2.5f, -2), "[physics group]");
    dd_setGroup(0);

    /* ---- Animated sphere ---- */
    dd_vec3 sphere_pos = dd3(
        2.0f * SDL_cosf(t * 0.8f),
        1.0f + 0.5f * SDL_sinf(t * 1.3f),
        2.0f * SDL_sinf(t * 0.8f));
    dd_color(DD_YELLOW), dd_sphere(sphere_pos, 0.6f); 
    dd_color(DD_WHITE),  dd_point( sphere_pos, 0.15f); 

    /* v5: label on the sphere */
    dd_color(DD_YELLOW), dd_label(add3(sphere_pos, dd3(0, 0.8f, 0)), "sphere (%.1f, %.1f, %.1f)", sphere_pos.x, sphere_pos.y, sphere_pos.z);

    /* ---- Capsule ---- */
    dd_color(DD_LIME), dd_capsule(dd3(3,-0.5f,3), dd3(3,1.5f,3), 0.4f), dd_label(dd3(3, 2.1f, 3), "capsule");

    /* ---- OBB (rotates) ---- */
    dd_color(DD_ORANGE);
    float a = t * 0.5f;
    dd_vec3 obb_ax = dd3( SDL_cosf(a), 0, -SDL_sinf(a));
    dd_vec3 obb_ay = dd3( 0,       1,  0);
    dd_vec3 obb_az = dd3( SDL_sinf(a), 0,  SDL_cosf(a));
    dd_obb(dd3(-2, 1, 3),
                 scale3(obb_ax, 1.0f),
                 scale3(obb_ay, 0.5f),
                 scale3(obb_az, 0.7f));

    /* ---- Cone / ray ---- */
    dd_color(DD_RGBA(180,255,80,255)), dd_cone(dd3(0, 3, 0), dd3(0, 0, 0), 1.5f, 16);
    /* v5: timed label at cone apex — stays 2 seconds, re-triggered each frame */
    dd_color(DD_RGBA(180,255,80,255)), dd_label(dd3(0, 3.3f, 0), "cone apex");

    dd_vec3 ray_dir = dd3(SDL_cosf(t * 0.4f), 0.4f, SDL_sinf(t * 0.4f));
    dd_color(DD_RED), dd_ray(dd3(0, 0.05f, 0), ray_dir, 4.0f);

    /* ---- Triangle ---- */
    dd_color(DD_RGBA(255, 180, 80, 255));
    dd_triangle(
        dd3(-1, 0, -3),
        dd3( 1, 0, -3),
        dd3( 0, 2, -3));

    /* ---- Circle in XZ plane ---- */
    dd_color(DD_CYAN);
    dd_circle(dd3(5, 0.01f, -2), dd3(0,1,0), 1.2f, 32);

    /* ---- Bouncing box for fun ---- */
    dd_color(DD_GREEN);
    float bh = 0.5f + 0.5f * SDL_fabsf(SDL_sinf(t * 1.8f));
    dd_box(dd3(-3, bh, 4), dd3(0.4f, bh, 0.4f));

    /* ---- Frustum of a secondary camera ---- */
    dd_camera_t sec_cam = {
        .position = dd3(8, 5, -4),
        .target   = dd3(0, 0, 0),
        .up       = dd3(0, 1, 0),
        .fov_y    = 45.0f,
        .near_z   = 0.5f,
        .far_z    = 15.0f
    };
    dd_mat4 sv = m4_lookat(sec_cam.position, sec_cam.target, sec_cam.up);
    dd_mat4 sp = m4_perspective(sec_cam.fov_y, 1.777f, sec_cam.near_z, sec_cam.far_z);
    dd_mat4 svp = m4_mul(&sp, &sv);
    /* Invert by brute-force transpose trick — OK for debug camera */
    /* Real inversion would be needed for arbitrary matrices.
       For this demo, we'll pass a "pre-inverted" approach using inverse_vp.
       Here we use a simple 4x4 inversion (Gauss-Jordan): */

    /* Gauss-Jordan 4x4 inversion */
    float src[4][8];
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) src[r][c] = svp.m[c][r]; /* transpose col→row */
        for (int c = 4; c < 8; c++) src[r][c] = (c-4 == r) ? 1.0f : 0.0f;
    }
    for (int col = 0; col < 4; col++) {
        /* pivot */
        int pivot = col;
        for (int r = col+1; r < 4; r++)
            if (SDL_fabsf(src[r][col]) > SDL_fabsf(src[pivot][col])) pivot = r;
        float tmp[8];
        SDL_memcpy(tmp, src[col], sizeof(tmp));
        SDL_memcpy(src[col], src[pivot], sizeof(tmp));
        SDL_memcpy(src[pivot], tmp, sizeof(tmp));
        float inv = 1.0f / src[col][col];
        for (int c = 0; c < 8; c++) src[col][c] *= inv;
        for (int r = 0; r < 4; r++) {
            if (r == col) continue;
            float f = src[r][col];
            for (int c = 0; c < 8; c++) src[r][c] -= f * src[col][c];
        }
    }
    dd_mat4 inv_svp;
    for (int r = 0; r < 4; r++)
        for (int c = 0; c < 4; c++)
            inv_svp.m[c][r] = src[r][c+4]; /* put back into col-major */

    dd_color(DD_RGBA(200,200,50,255));
    dd_frustum(&inv_svp);

    /* ---- v5.1 new primitives ---- */
    /* Camera body */
    dd_color(DD_RGBA(200,200,50,255));
    dd_camera(dd3(8, 5, -4), dd3(-8,-5,4), dd3(0,1,0));

    /* AABB corner ticks */
    dd_color(DD_ORANGE);
    dd_aabbCorners(dd3(5,0,3), dd3(7,2,5));
    dd_label(dd3(6, 2.3f, 4), "corners");

    /* Diamond marker */
    dd_color(DD_CYAN);
    dd_diamond(dd3(-4, 1, -4), dd3(0,1,0), 0.4f);
    dd_label(dd3(-4, 1.8f, -4), "diamond");

    /* Boid */
    dd_color(DD_MAGENTA);
    dd_boid(dd3(6, 0.1f, 0), dd3(-1, 0, -0.5f));

    /* Position marker */
    dd_color(DD_RGBA(255,160,80,200));
    dd_position(dd3(-6, 3, 2), 0.6f);
    dd_position(dd3(-8, -3, 2), 0.6f);

    /* Prism variants */
    dd_color(DD_PURPLE); dd_prism(dd3(4,0,-4), 0.8f,  0.f,  dd3(0,1,0), 5); // pentagon
    dd_color(DD_CYAN);   dd_prism(dd3(6,0,-4), 0.8f,  0.f,  dd3(0,1,0), 6); // hexagon
    dd_color(DD_GREEN);  dd_prism(dd3(5,0,-6), 0.6f, -1.5f, dd3(0,1,0), 6); // hex prism

    /* 3-D world text */
    dd_color(DD_RGBA(180,255,80,255));
    dd_label3d(dd3(-1, 4.2f, 0), 0.012f, "hello\ndd3d v5");
    dd_label3d(dd3(-4, 0.1f, -6), 0.010f, "text3d = world space");

    /* ---- v6: thickness + stipple showcase ---- */
    dd_setStyle((dd_style){ 3.f, DD_STIPPLE_SOLID,    16, 0.f });
    dd_color(DD_RGBA(255,100,100,255)); dd_box(dd3(6,1,-8), dd3(1,1,1));
    dd_color(DD_WHITE); dd_label(dd3(6,2.5f,-8), "thick");

    dd_setStyle((dd_style){ 1.f, DD_STIPPLE_DASH,     16, 0.f });
    dd_color(DD_RGBA(100,200,255,255)); dd_box(dd3(9,1,-8), dd3(1,1,1));
    dd_color(DD_WHITE); dd_label(dd3(9,2.5f,-8), "dash");

    dd_setStyle((dd_style){ 1.f, DD_STIPPLE_DOT,      12, 0.f });
    dd_color(DD_RGBA(100,255,150,255)); dd_sphere(dd3(12,1,-8), 1.f);
    dd_color(DD_WHITE); dd_label(dd3(12,2.5f,-8), "dot");

    /* Animated: marching ants — speed=8 means one full rotation/sec */
    dd_setStyle((dd_style){ 2.f, DD_STIPPLE_DASH,     16, 8.f });
    dd_color(DD_YELLOW); dd_box(dd3(15,1,-8), dd3(1,1,1));
    dd_color(DD_WHITE);  dd_label(dd3(15,2.5f,-8), "marching");

    /* Animated thick dash-dot */
    dd_setStyle((dd_style){ 4.f, DD_STIPPLE_DASH_DOT, 20, 4.f });
    dd_color(DD_MAGENTA); dd_circle(dd3(18,1,-8), dd3(0,1,0), 1.f, 32);
    dd_color(DD_WHITE);   dd_label(dd3(18,2.5f,-8), "anim+thick");

    /* Reverse direction */
    dd_setStyle((dd_style){ 1.f, DD_STIPPLE_DASH,     16, -6.f });
    dd_color(DD_CYAN);    dd_circle(dd3(21,1,-8), dd3(0,1,0), 1.f, 32);
    dd_color(DD_WHITE);   dd_label(dd3(21,2.5f,-8), "reverse");

    dd_resetStyle();
    /* Run update BEFORE draw so hot_axis is set for the draw call */
    if (gizmo_mode == 0) {
        dd_gizmoUpdateTranslate(gw, inp, delta);
        dd_gizmoTranslate(gw);
    } else if (gizmo_mode == 1) {
        dd_gizmoUpdateRotate(gw, inp, delta);
        dd_gizmoRotate(gw);
    } else {
        dd_gizmoUpdateScale(gw, inp, delta);
        dd_gizmoScale(gw);
    }

    /* ---- Static visual-only gizmo on the rotating OBB ---- */
    float a2 = t * 0.5f;
    dd_gizmoDesc go = {
        .origin      = dd3(-2, 1, 3),
        .axis_x      = dd3( SDL_cosf(a2), 0, SDL_sinf(a2)),
        .axis_y      = dd3(0, 1, 0),
        .axis_z      = dd3(-SDL_sinf(a2), 0, SDL_cosf(a2)),
        .screen_size = 80.0f,
        .hot_axis    = DD_GIZMO_AXIS_NONE,
    };
    if (gizmo_mode == 0)      dd_gizmoTranslate(&go);
    else if (gizmo_mode == 1) dd_gizmoRotate(&go);
    else                      dd_gizmoScale(&go);
}

/* -------------------------------------------------------------------------
 * Main
 * ---------------------------------------------------------------------- */

void main(event ev) {

    int W = 1280, H = 720;
    static dd_context *ctx;
    static orbital_camera_t orbit = { .yaw = 35.0f, .pitch = 25.0f, .distance = 18.0f, .target = {0,0,0} };
    static bool running       = true;
    static bool mouse_down    = false;
    static bool mouse_pressed = false, mouse_released = false;
    static float mouse_x = 0, mouse_y = 0;
    static float mouse_prev_x = 0, mouse_prev_y = 0;
    static float orbit_dx_accum = 0, orbit_dy_accum = 0;  /* accumulated between ticks */
    static uint64_t t0;
    static int  gizmo_mode = 0;   /* 0=translate  1=rotate  2=scale */
    /* Interactive gizmo state — caller owns the transform */
    static dd_gizmoDesc gw = {
        .origin      = {-6, 0, -5},
        .axis_x      = {1,0,0},
        .axis_y      = {0,1,0},
        .axis_z      = {0,0,1},
        .screen_size = 110.0f,
        .scale       = 1.0f,
        .scale_x     = 1.0f,
        .scale_y     = 1.0f,
        .scale_z     = 1.0f,
        .hot_axis    = DD_GIZMO_AXIS_NONE,
    };
    static float gw_angle = 0.0f;   /* accumulated rotation angle (radians) */
    static float gw_scale = 1.0f;   /* accumulated scale factor */
    static const char *mode_names[] = { "TRANSLATE", "ROTATE", "SCALE" };

    if( ev.init ) {
        render.open(W, H, 0.85, 0);
    
        SDL_SetRenderDrawBlendMode(render.handle, SDL_BLENDMODE_BLEND);

        ctx = dd_createContext(render.handle);

        /* v5: enable depth fade */
        dd_depthFade fade = { .enabled=true, .fade_near=4.f, .fade_far=60.f, .min_alpha=25 };
        dd_setDepthFade(fade);

        t0 = SDL_GetTicks();
    }

    if( ev.emit ) {
        SDL_Event *event = ev.emit;
        //SDL_ConvertEventToRenderCoordinates(render.handle, event);

        switch (event->type) {
            case SDL_EVENT_QUIT:
                app.quit(0);
                break;
            case SDL_EVENT_KEY_DOWN:
                if (event->key.key == SDLK_ESCAPE || event->key.key == SDLK_Q)
                    app.quit(0);
                if (event->key.key == SDLK_R) {
                    orbit.yaw = 35.0f; orbit.pitch = 25.0f;
                    orbit.distance = 18.0f;
                }
                /* G cycles gizmo mode */
                if (event->key.key == SDLK_SPACE)
                    gizmo_mode = (gizmo_mode + 1) % 3;
                /* P toggles physics group (group 1) */
                if (event->key.key == SDLK_P) {
                    static bool phys_visible = true;
                    phys_visible = !phys_visible;
                    dd_setGroupVisible(1, phys_visible);
                }
                break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                if (event->button.button == SDL_BUTTON_LEFT) {
                    /* Only start orbit if gizmo didn't consume the press.
                       We set mouse_pressed here; the gizmo update runs first
                       in the render block and sets g_pick.dragging if it hit.
                       We delay orbit start until we know the gizmo didn't grab. */
                    mouse_pressed = true;
                    mouse_down    = true;
                    mouse_x = event->button.x;
                    mouse_y = event->button.y;
                    mouse_prev_x = mouse_x;
                    mouse_prev_y = mouse_y;
                }
                break;
            case SDL_EVENT_MOUSE_BUTTON_UP:
                if (event->button.button == SDL_BUTTON_LEFT) {
                    mouse_released = true;
                    mouse_down     = false;
                }
                break;
            case SDL_EVENT_MOUSE_MOTION:
                mouse_x = event->motion.x;
                mouse_y = event->motion.y;
                if (mouse_down) {
                    /* Accumulate — do NOT apply here. Orbit is applied after
                       dd_gizmoUpdate* so we can check dd_gizmoIsDragging(). */
                    orbit_dx_accum += mouse_x - mouse_prev_x;
                    orbit_dy_accum += mouse_y - mouse_prev_y;
                    mouse_prev_x = mouse_x;
                    mouse_prev_y = mouse_y;
                }
                break;
            case SDL_EVENT_MOUSE_WHEEL:
                orbit.distance -= event->wheel.y * 0.8f;
                if (orbit.distance < 1.0f) orbit.distance = 1.0f;
                break;
            case SDL_EVENT_WINDOW_RESIZED:
                W = event->window.data1;
                H = event->window.data2;
                dd_setViewport(W, H);
                break;
            default:
                break;
        }
    }

    if(ev.tick) {
        float t = (float)(SDL_GetTicks() - t0) / 1000.0f;

        /* Update camera */
        dd_camera_t cam;
        orbital_to_dd(&orbit, &cam);
        dd_setCamera(&cam);

        /* Build input snapshot BEFORE clearing edge flags */
        dd_gizmoInput inp = {
            .mouse_x        = mouse_x,
            .mouse_y        = mouse_y,
            .mouse_down     = mouse_down,
            .mouse_pressed  = mouse_pressed,
            .mouse_released = mouse_released,
        };

        /* Clear edge-triggered flags for next tick */
        mouse_pressed  = false;
        mouse_released = false;

        /* Render */
        SDL_SetRenderDrawColor(render.handle, 18, 18, 22, 255);
        SDL_RenderClear(render.handle);

        dd_gizmoDelta delta = {0};
        delta.scale = 1.0f;

        dd_beginFrame((double)t, W, H);
        draw_scene(t, gizmo_mode, &gw, &inp, &delta);
        dd_endFrame();

        /* Apply orbit only if gizmo didn't claim the drag this tick */
        if (!dd_gizmoIsDragging()) {
            orbit.yaw   -= orbit_dx_accum * 0.4f;
            orbit.pitch -= orbit_dy_accum * 0.4f;
            if (orbit.pitch >  89.0f) orbit.pitch =  89.0f;
            if (orbit.pitch < -89.0f) orbit.pitch = -89.0f;
        }
        orbit_dx_accum = 0;
        orbit_dy_accum = 0;

        /* Apply delta to the gizmo's own transform */
        if (delta.active) {
            /* Translation */
            gw.origin = add3(gw.origin, delta.translate);

            /* Rotation: Rodrigues around the dragged axis */
            if (delta.rotate != 0.0f) {
                /* For VIEW part, rotate around camera-forward */
                dd_vec3 k =
                    (delta.part == DD_GIZMO_AXIS_X)    ? gw.axis_x :
                    (delta.part == DD_GIZMO_AXIS_Y)    ? gw.axis_y :
                    (delta.part == DD_GIZMO_AXIS_Z)    ? gw.axis_z :
                    norm3(sub3(gw.origin, cam.position)); /* VIEW */
                float c  = SDL_cosf(delta.rotate);
                float ss = SDL_sinf(delta.rotate);
                #define ROD(v) add3(add3(scale3(v,c), scale3(cross3(k,v),ss)), scale3(k,dot3(k,v)*(1.0f-c)))
                gw.axis_x = ROD(gw.axis_x);
                gw.axis_y = ROD(gw.axis_y);
                gw.axis_z = ROD(gw.axis_z);
                #undef ROD
                gw_angle += delta.rotate;
            }

            /* Scale: apply to the dragged axis only, or all if ALL */
            if (delta.scale != 1.0f) {
                if (delta.part == DD_GIZMO_AXIS_ALL) {
                    gw_scale *= delta.scale;
                    if (gw_scale < 0.05f) gw_scale = 0.05f;
                    if (gw_scale > 20.0f) gw_scale = 20.0f;
                    gw.scale = gw_scale;
                } else if (delta.part == DD_GIZMO_AXIS_X) {
                    gw.scale_x *= delta.scale;
                    if (gw.scale_x < 0.05f) gw.scale_x = 0.05f;
                    if (gw.scale_x > 20.0f) gw.scale_x = 20.0f;
                } else if (delta.part == DD_GIZMO_AXIS_Y) {
                    gw.scale_y *= delta.scale;
                    if (gw.scale_y < 0.05f) gw.scale_y = 0.05f;
                    if (gw.scale_y > 20.0f) gw.scale_y = 20.0f;
                } else if (delta.part == DD_GIZMO_AXIS_Z) {
                    gw.scale_z *= delta.scale;
                    if (gw.scale_z < 0.05f) gw.scale_z = 0.05f;
                    if (gw.scale_z > 20.0f) gw.scale_z = 20.0f;
                }
            }
        }

        /* HUD */
        const dd_stats *st = dd_getStats();
        char hud[320];
        SDL_snprintf(hud, sizeof(hud),
            "dd3d v5 - lines:%d/%d timed:%d labels:%d - SPC=[%s]  P=physics-group  LMB=drag/orbit  Scroll=zoom  R=reset",
            st->lines_drawn, st->lines_submitted,
            st->timed_lines_active, st->labels_drawn,
            mode_names[gizmo_mode]);
        SDL_SetWindowTitle(window.handle, hud);

        SDL_RenderPresent(render.handle);
        SDL_Delay(8); /* ~120 fps cap */
    }

    if( ev.quit )
        dd_destroyContext(ctx);
}

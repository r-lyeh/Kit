/*
 * camera.c  –  Simple orbit camera
 *
 * Controls:
 *   LMB drag → orbit
 *   Scroll   → zoom
 *   WASD     → pan (world-space)
 */

#include "camera.h"
#include <SDL3/SDL.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

/* ─── Math helpers ──────────────────────────────────────────────────────────── */

static void mat4_identity(float m[16])
{
    memset(m, 0, 64);
    m[0] = m[5] = m[10] = m[15] = 1.f;
}

static void mat4_mul(float out[16], const float a[16], const float b[16])
{
    float tmp[16] = {0};
    for (int row = 0; row < 4; row++)
        for (int col = 0; col < 4; col++)
            for (int k = 0; k < 4; k++)
                tmp[col*4+row] += a[k*4+row] * b[col*4+k];
    memcpy(out, tmp, 64);
}

/*
 * Column-major perspective for SDL_gpu.
 *
 * SDL_gpu normalises clip space across backends so we do NOT manually flip Y.
 * glTF uses right-handed +Y-up: keeping +t here means models load the right
 * way up without any coordinate-space fixup.
 */
static void mat4_perspective(float m[16], float fov, float aspect, float n, float f)
{
    memset(m, 0, 64);
    float t = 1.f / tanf(fov * 0.5f);
    m[0]  =  t / aspect;
    m[5]  =  t;                        /* no Y-flip – glTF is +Y-up, SDL_gpu is consistent */
    m[10] =  f / (n - f);
    m[11] = -1.f;
    m[14] =  (n * f) / (n - f);
}

static void mat4_look_at(float m[16],
                         const float eye[3],
                         const float center[3],
                         const float up[3])
{
    float f[3], s[3], u[3];

    /* forward = normalize(center - eye) */
    for (int i=0;i<3;i++) f[i] = center[i] - eye[i];
    float fl = sqrtf(f[0]*f[0]+f[1]*f[1]+f[2]*f[2]);
    for (int i=0;i<3;i++) f[i] /= fl;

    /* right = normalize(forward × up) */
    s[0]=f[1]*up[2]-f[2]*up[1]; s[1]=f[2]*up[0]-f[0]*up[2]; s[2]=f[0]*up[1]-f[1]*up[0];
    float sl=sqrtf(s[0]*s[0]+s[1]*s[1]+s[2]*s[2]);
    for (int i=0;i<3;i++) s[i]/=sl;

    /* up' = right × forward */
    u[0]=s[1]*f[2]-s[2]*f[1]; u[1]=s[2]*f[0]-s[0]*f[2]; u[2]=s[0]*f[1]-s[1]*f[0];

    mat4_identity(m);
    m[0]=s[0]; m[4]=s[1]; m[8] =s[2];
    m[1]=u[0]; m[5]=u[1]; m[9] =u[2];
    m[2]=-f[0]; m[6]=-f[1]; m[10]=-f[2];
    m[12]=-(s[0]*eye[0]+s[1]*eye[1]+s[2]*eye[2]);
    m[13]=-(u[0]*eye[0]+u[1]*eye[1]+u[2]*eye[2]);
    m[14]= (f[0]*eye[0]+f[1]*eye[1]+f[2]*eye[2]);
}

/* ─── Camera struct ─────────────────────────────────────────────────────────── */

struct Camera {
    /* Orbit parameters */
    float  target[3];
    float  yaw, pitch, radius;

    /* Projection parameters */
    float  fov, aspect, near, far;

    float  eye[3];         /* world-space camera position */
    /* Cached matrices */
    float  view[16];
    float  proj[16];
    float  viewproj[16];
    bool   dirty;

    /* Input state */
    bool   lmb_down;
    float  last_mx, last_my;

    /* Keyboard pan */
    bool   key_w, key_a, key_s, key_d;
};

/* ─── Impl ──────────────────────────────────────────────────────────────────── */

Camera *camera_create(void)
{
    Camera *c = (Camera*)calloc(1, sizeof(Camera));
    c->radius = 5.f;
    c->yaw    = 0.f;
    c->pitch  = 0.2f;
    c->dirty  = true;
    return c;
}

void camera_destroy(Camera *cam) { free(cam); }

void camera_set_perspective(Camera *cam, float fov_rad, float aspect, float n, float f)
{
    cam->fov    = fov_rad;
    cam->aspect = aspect;
    cam->near   = n;
    cam->far    = f;
    cam->dirty  = true;
}

void camera_set_aspect(Camera *cam, float aspect)
{
    cam->aspect = aspect;
    cam->dirty  = true;
}

void camera_look_at(Camera *cam, const float eye[3], const float center[3], const float up[3])
{
    (void)up;
    for (int i=0;i<3;i++) cam->target[i] = center[i];
    float dx = eye[0]-center[0], dy = eye[1]-center[1], dz = eye[2]-center[2];
    cam->radius = sqrtf(dx*dx+dy*dy+dz*dz);
    cam->yaw    = atan2f(dx, dz);
    cam->pitch  = atan2f(dy, sqrtf(dx*dx+dz*dz));
    cam->dirty  = true;
}

void camera_handle_event(Camera *cam, const SDL_Event *e)
{
    switch (e->type) {
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
        if (e->button.button == SDL_BUTTON_LEFT) {
            cam->lmb_down = true;
            cam->last_mx  = e->button.x;
            cam->last_my  = e->button.y;
        }
        break;
    case SDL_EVENT_MOUSE_BUTTON_UP:
        if (e->button.button == SDL_BUTTON_LEFT) cam->lmb_down = false;
        break;
    case SDL_EVENT_MOUSE_MOTION:
        if (cam->lmb_down) {
            float dx = e->motion.x - cam->last_mx;
            float dy = e->motion.y - cam->last_my;
            dy *= -1.0f;   /* invert Y: drag up = orbit up */
            cam->yaw   -= dx * 0.005f;
            cam->pitch  = SDL_clamp(cam->pitch - dy * 0.005f,
                                     -SDL_PI_F * 0.49f,
                                      SDL_PI_F * 0.49f);
            cam->last_mx = e->motion.x;
            cam->last_my = e->motion.y;
            cam->dirty   = true;
        }
        break;
    case SDL_EVENT_MOUSE_WHEEL:
        cam->radius = SDL_max(0.5f, cam->radius - e->wheel.y * 0.3f);
        cam->dirty  = true;
        break;
    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_KEY_UP: {
        bool down = (e->type == SDL_EVENT_KEY_DOWN);
        switch (e->key.key) {
        case SDLK_W: cam->key_w = down; break;
        case SDLK_A: cam->key_a = down; break;
        case SDLK_S: cam->key_s = down; break;
        case SDLK_D: cam->key_d = down; break;
        default: break;
        }
        break;
    }
    default: break;
    }
}

void camera_update(Camera *cam, float dt)
{
    /* Pan target with WASD */
    float speed = cam->radius * 0.5f * dt;
    float sy = sinf(cam->yaw), cy = cosf(cam->yaw);
    if (cam->key_w) { cam->target[0] -= sy*speed; cam->target[2] -= cy*speed; cam->dirty=true; }
    if (cam->key_s) { cam->target[0] += sy*speed; cam->target[2] += cy*speed; cam->dirty=true; }
    if (cam->key_a) { cam->target[0] -= cy*speed; cam->target[2] += sy*speed; cam->dirty=true; }
    if (cam->key_d) { cam->target[0] += cy*speed; cam->target[2] -= sy*speed; cam->dirty=true; }

    if (!cam->dirty) return;

    /* Recompute eye position from spherical coordinates */
    float cp = cosf(cam->pitch), sp = sinf(cam->pitch);
    float eye[3] = {
        cam->target[0] + cam->radius * cp * sinf(cam->yaw),
        cam->target[1] + cam->radius * sp,
        cam->target[2] + cam->radius * cp * cosf(cam->yaw),
    };
    float up[3] = {0.f, 1.f, 0.f};

    memcpy(cam->eye, eye, sizeof(eye));
    mat4_look_at(cam->view, eye, cam->target, up);
    mat4_perspective(cam->proj, cam->fov, cam->aspect, cam->near, cam->far);
    mat4_mul(cam->viewproj, cam->proj, cam->view);
    cam->dirty = false;
}

void camera_get_viewproj(const Camera *cam, float out[16]) { memcpy(out, cam->viewproj, 64); }
void camera_get_view    (const Camera *cam, float out[16]) { memcpy(out, cam->view,     64); }
void camera_get_proj    (const Camera *cam, float out[16]) { memcpy(out, cam->proj,     64); }
void camera_get_eye(const Camera *cam, float out[3]) { memcpy(out, cam->eye, 12); }

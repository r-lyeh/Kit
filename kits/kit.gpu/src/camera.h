#pragma once
/*
 * camera.h  –  Orbit camera
 */

#include <SDL3/SDL.h>
#include <stdbool.h>

typedef struct Camera Camera;

Camera *camera_create(void);
void    camera_destroy(Camera *cam);

void camera_set_perspective(Camera *cam, float fov_rad, float aspect, float near, float far);
void camera_set_aspect(Camera *cam, float aspect);
void camera_look_at(Camera *cam, const float eye[3], const float center[3], const float up[3]);

void camera_handle_event(Camera *cam, const SDL_Event *e);
void camera_update(Camera *cam, float dt);

/* Matrices */
void camera_get_viewproj(const Camera *cam, float out[16]);
void camera_get_view(const Camera *cam, float out[16]);
void camera_get_proj(const Camera *cam, float out[16]);

/* World-space eye position – needed by fragment shader for specular */
void camera_get_eye(const Camera *cam, float out[3]);

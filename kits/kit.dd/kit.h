#include "dd3d.h"

#ifndef KIT_DD_H
#define KIT_DD_H "0.0.0"

extern struct debugdraw {
void        (*aabb)(dd_vec3 mn, dd_vec3 mx);
void        (*aabbCorners)(dd_vec3 mn, dd_vec3 mx);
void        (*axes)(dd_vec3 origin, dd_vec3 axis_x, dd_vec3 axis_y, dd_vec3 axis_z, float length); // dd_axes: XYZ cross at origin.
void        (*boid)(dd_vec3 position, dd_vec3 dir); // cheap directional triangle indicator (front/back).
void        (*box)(dd_vec3 center, dd_vec3 half);
void        (*camera)(dd_vec3 position, dd_vec3 forward, dd_vec3 up); // film-camera body: rectangular body, two half-reels, and a trapezoidal lens shield. `forward` and `up` define camera orientation.
void        (*capsule)(dd_vec3 base, dd_vec3 tip, float radius);
void        (*circle)(dd_vec3 center, dd_vec3 normal, float radius, int segments);
void        (*cone)(dd_vec3 apex, dd_vec3 base_center, float radius, int segments);
void        (*diamond)(dd_vec3 center, dd_vec3 up, float size); // 6-point octahedron / diamond shape. Good as a position marker with optional orientation.
void        (*frustum)(const dd_mat4 *inv_view_proj);
void        (*grid)(dd_vec3 center, float cell_size, int cells, dd_rgba major_color, dd_rgba minor_color, int subdivisions); // dd_grid: two-pass grid (major + minor lines).
void        (*line)(dd_vec3 a, dd_vec3 b);
void        (*lineDashed)(dd_vec3 a, dd_vec3 b);
void        (*normal)(dd_vec3 pos, dd_vec3 normal);
void        (*obb)(dd_vec3 center, dd_vec3 axis_x, dd_vec3 axis_y, dd_vec3 axis_z);
void        (*point)(dd_vec3 p, float size);
void        (*position)(dd_vec3 pos, float radius); // stacked circle rings with a vertical line to the ground (Y=0). Ring count and radius shrink with height. Useful to locate "where is this actor" quickly.
void        (*positionDir)(dd_vec3 pos, dd_vec3 dir, float radius); // same but the bottom ring is oriented toward `dir` (shows facing direction).
void        (*prism)(dd_vec3 center, float radius, float height, dd_vec3 normal, int segments);
void        (*ray)(dd_vec3 origin, dd_vec3 direction, float length);
void        (*sphere)(dd_vec3 center, float radius);
void        (*sphereEx)(dd_vec3 center, float radius, int segments);
void        (*triangle)(dd_vec3 a, dd_vec3 b, dd_vec3 c);
} debugdraw;

#elif KIT_CODE
#pragma once

struct debugdraw debugdraw = {
    .aabb = dd_aabb,
    .aabbCorners = dd_aabbCorners,
    .axes = dd_axes,
    .boid = dd_boid,
    .box = dd_box,
    .camera = dd_camera,
    .capsule = dd_capsule,
    .circle = dd_circle,
    .cone = dd_cone,
    .diamond = dd_diamond,
    .frustum = dd_frustum,
    .grid = dd_grid,
    .line = dd_line,
    .lineDashed = dd_lineDashed,
    .normal = dd_normal,
    .obb = dd_obb,
    .point = dd_point,
    .position = dd_position,
    .positionDir = dd_positionDir,
    .prism = dd_prism,
    .ray = dd_ray,
    .sphere = dd_sphere,
    .sphereEx = dd_sphereEx,
    .triangle = dd_triangle,
};

#endif

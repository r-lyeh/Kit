#pragma once
/*
 * mesh.h  –  GPU mesh, material, and scene management
 *
 * Vertex layout (matches shader attribute locations):
 *   Location 0 : position  (vec3)
 *   Location 1 : normal    (vec3)
 *   Location 2 : texcoord  (vec2)
 *   Location 3 : tangent   (vec4, w = bitangent sign)
 */

#include <SDL3/SDL.h>
#include "texture_cache.h"
#include "gpu_pipeline.h"
#include <stdbool.h>
#include <stdint.h>

/* ─── Vertex ─────────────────────────────────────────────────────────────────── */
typedef struct Vertex {
    float position[4];
    float normal[4];
    float texcoord[4];
    float tangent[4];    /* xyz = tangent, w = bitangent sign (-1 or +1) */
} Vertex;

/* ─── Material ───────────────────────────────────────────────────────────────── */
typedef struct Material {
    /* PBR base colour */
    GpuTex  albedo;
    float   base_color[4];       /* linear RGBA factor */

    /* Metallic-roughness */
    GpuTex  metallic_roughness;
    float   metallic_factor;
    float   roughness_factor;

    /* Normal map */
    GpuTex  normal_map;
    float   normal_scale;

    /* Occlusion */
    GpuTex  occlusion;
    float   occlusion_strength;

    /* Emissive */
    GpuTex  emissive;
    float   emissive_factor[3];

    float     alpha_cutoff;
    float     normal_y_sign;
    BlendMode blend_mode;   /* OPAQUE, MASK, or BLEND */   /* +1.0 = OpenGL/glTF, -1.0 = DX (flip green channel) */
    float   mr_swizzle;      /* +1.0 = spec (G=rough,B=metal), -1.0 = legacy (R=metal,G=rough) */
    bool    double_sided;
} Material;

/* ─── Mesh ───────────────────────────────────────────────────────────────────── */
typedef struct Mesh {
    SDL_GPUBuffer *vertex_buf;
    SDL_GPUBuffer *index_buf;
    uint32_t       index_count;
    uint32_t       vertex_count;
    float          transform[16];
    Material       material;
} Mesh;

/* ─── Scene ──────────────────────────────────────────────────────────────────── */
#define SCENE_MAX_MESHES 256

typedef struct Scene {
    Mesh          *meshes[SCENE_MAX_MESHES];
    uint32_t       mesh_count;
    TextureCache  *tcache;
    PipelineCache *pipeline_cache;
} Scene;

/* ─── API ────────────────────────────────────────────────────────────────────── */

Mesh  *mesh_create(SDL_GPUDevice  *gpu,
                   const Vertex   *vertices, uint32_t vertex_count,
                   const uint32_t *indices,  uint32_t index_count);
void   mesh_destroy(SDL_GPUDevice *gpu, Mesh *mesh);
void   mesh_draw(Mesh *mesh, SDL_GPURenderPass *pass, SDL_GPUCommandBuffer *cmd);

Scene *scene_create(SDL_GPUDevice *gpu, SDL_Window *window);
void   scene_destroy(SDL_GPUDevice *gpu, Scene *scene);
bool   scene_add_mesh(Scene *scene, Mesh *mesh);
void   scene_draw_opaque(Scene *scene, SDL_GPURenderPass *pass, SDL_GPUCommandBuffer *cmd);
void   scene_draw_blend (Scene *scene, SDL_GPURenderPass *pass, SDL_GPUCommandBuffer *cmd,
                         const float cam_pos[3]);
void   scene_add_placeholder(SDL_GPUDevice *gpu, Scene *scene);

#pragma once
/*
 * gltf_loader.h  –  Minimal glTF / GLB loader built on cgltf
 *
 * Loads meshes from a .gltf or .glb file and populates a Scene.
 * Textures and materials are skipped in this starter; extend as needed.
 */

#include <SDL3/SDL.h>
#include <stdbool.h>

typedef struct Scene Scene;

/*
 * Load all meshes from `path` into `scene`.
 * Returns false on any error (logs via SDL_Log).
 */
bool gltf_load_into_scene(SDL_GPUDevice *gpu,
                           const char    *path,
                           Scene         *scene);

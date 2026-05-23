# assets/

Drop any `.gltf` or `.glb` file here.

The app looks for `assets/Box.glb` by default (set in `src/app.c`).
If the file is not found it falls back to a placeholder triangle.

## Getting sample models

The official glTF sample model repository has many ready-to-use files:
  https://github.com/KhronosGroup/glTF-Sample-Models

Quick picks that work well as a first test:
  - Box.glb              (simplest possible mesh)
  - DamagedHelmet.glb    (PBR materials, ~2 MB)
  - Sponza.glb           (large scene, stress-test)

Download any `.glb` into this folder, then change `gltf_path` in `src/app.c`.

## Environment map (IBL)

The renderer looks for `assets/env.hdr` at startup.
If not found it falls back to a neutral grey environment (IBL still works,
just without interesting reflections).

Free HDR environment maps:
  https://polyhaven.com/hdris   (CC0 – recommended)
  https://hdrihaven.com

Recommended for testing:
  - venice_sunset_1k.hdr   (warm outdoor, good for metallic helmets)
  - studio_small_08_1k.hdr (neutral studio, good for material validation)

Download any .hdr and save it as assets/env.hdr.
1K resolution is sufficient; 2K gives sharper specular reflections.

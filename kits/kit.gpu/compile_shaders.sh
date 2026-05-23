#!/usr/bin/env bash
# compile_shaders.sh:  Compile all GLSL shaders to SPIR-V
set -e

OUT="$(dirname "$0")/shaders"

if ! command -v glslc &>/dev/null; then
    echo "ERROR: glslc not found. Install the Vulkan SDK: https://vulkan.lunarg.com/"
    exit 1
fi

echo "Compiling graphics shaders..."
glslc -fshader-stage=vert  shaders/mesh.vert.glsl        -o "$OUT/mesh.vert.spv"
glslc -fshader-stage=frag  shaders/mesh.frag.glsl        -o "$OUT/mesh.frag.spv"

echo "Compiling IBL compute shaders..."
glslc -fshader-stage=comp  shaders/equirect_to_cube.comp.glsl  -o "$OUT/equirect_to_cube.comp.spv"
glslc -fshader-stage=comp  shaders/irradiance_conv.comp.glsl   -o "$OUT/irradiance_conv.comp.spv"
glslc -fshader-stage=comp  shaders/prefilter_env.comp.glsl     -o "$OUT/prefilter_env.comp.spv"
glslc -fshader-stage=comp  shaders/brdf_lut.comp.glsl          -o "$OUT/brdf_lut.comp.spv"
glslc -fshader-stage=comp  shaders/mip_gen.comp.glsl           -o "$OUT/mip_gen.comp.spv"

echo "All shaders compiled to $OUT/"

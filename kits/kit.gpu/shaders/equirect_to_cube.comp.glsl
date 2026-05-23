#version 450
/*
 * equirect_to_cube.comp  –  one dispatch per face
 *
 * SDL3 GPU binds each cubemap face as a plain 2D storage image.
 * We use image2D (not imageCube/image2DArray) and receive the face
 * index via uniform buffer so the shader knows which direction to sample.
 *
 * Uniform (set 2, binding 0): uint face_index  (0=+X … 5=-Z)
 * Dispatched: (face_size/8, face_size/8, 1)  × 6 times (once per face)
 */

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

layout(set = 0, binding = 0)          uniform sampler2D u_equirect;
layout(set = 1, binding = 0, rgba16f) uniform writeonly image2D u_face;

layout(set = 2, binding = 0) uniform FaceUBO { uint face_index; };

const float TWO_PI = 6.28318530718;
const float PI     = 3.14159265359;

vec3 cube_dir(uint face, vec2 uv)
{
    switch (face) {
    case 0: return normalize(vec3( 1.0,  uv.y, -uv.x));
    case 1: return normalize(vec3(-1.0,  uv.y,  uv.x));
    case 2: return normalize(vec3( uv.x,  1.0, -uv.y));
    case 3: return normalize(vec3( uv.x, -1.0,  uv.y));
    case 4: return normalize(vec3( uv.x,  uv.y,  1.0));
    default:return normalize(vec3(-uv.x,  uv.y, -1.0));
    }
}

vec2 dir_to_equirect(vec3 d)
{
    return vec2(atan(d.z, d.x) / TWO_PI + 0.5,
                asin(clamp(d.y, -1.0, 1.0)) / PI + 0.5);
}

void main()
{
    ivec2 coord = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size  = imageSize(u_face);
    if (coord.x >= size.x || coord.y >= size.y) return;

    vec2 uv  = (vec2(coord) + 0.5) / vec2(size) * 2.0 - 1.0;
    uv.y    = -uv.y;

    vec3 dir = cube_dir(face_index, uv);
    vec2 eq  = dir_to_equirect(dir);
    vec4 col = texture(u_equirect, eq);

    imageStore(u_face, coord, col);
}

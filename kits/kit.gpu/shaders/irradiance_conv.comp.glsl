#version 450
/*
 * irradiance_conv.comp  –  one dispatch per face
 *
 * Uniform (set 2, binding 0): uint face_index
 * Output:  image2D u_face  (single face, bound per-dispatch)
 */

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

layout(set = 0, binding = 0)          uniform samplerCube u_env;
layout(set = 1, binding = 0, rgba16f) uniform writeonly image2D u_face;

layout(set = 2, binding = 0) uniform FaceUBO { uint face_index; };

const float PI     = 3.14159265359;
const float TWO_PI = 6.28318530718;

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

void main()
{
    ivec2 coord = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size  = imageSize(u_face);
    if (coord.x >= size.x || coord.y >= size.y) return;

    vec2 uv = (vec2(coord) + 0.5) / vec2(size) * 2.0 - 1.0;
    uv.y   = -uv.y;
    vec3 N  = cube_dir(face_index, uv);

    vec3 up    = abs(N.y) < 0.999 ? vec3(0,1,0) : vec3(1,0,0);
    vec3 right = normalize(cross(up, N));
    up         = cross(N, right);

    vec3  irr     = vec3(0.0);
    float samples = 0.0;
    float d_phi   = TWO_PI / 128.0;
    float d_theta = (PI * 0.5) / 64.0;

    for (float phi = 0.0; phi < TWO_PI; phi += d_phi) {
        for (float theta = 0.0; theta < PI * 0.5; theta += d_theta) {
            float sp = sin(phi), cp = cos(phi), st = sin(theta), ct = cos(theta);
            vec3  s  = st*cp*right + st*sp*up + ct*N;
            irr     += texture(u_env, s).rgb * ct * st;
            samples += 1.0;
        }
    }
    irr = PI * irr / samples;

    imageStore(u_face, coord, vec4(irr, 1.0));
}

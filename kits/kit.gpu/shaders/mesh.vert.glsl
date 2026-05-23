#version 450
/*
 * mesh.vert.glsl  –  glTF vertex shader v5
 *
 * Passes tangent.w through to the fragment shader as v_tangent_w so the
 * fragment shader can detect the "no tangents" sentinel (w == 0.0) and
 * switch to screen-space derivative TBN automatically.
 *
 * When tangents ARE present (w == +1 or -1):
 *   TBN is computed in object space then transformed by the normal matrix.
 *   This is the correct procedure per glTF spec §3.7.2.1.
 *
 * When tangents are absent (w == 0, sentinel):
 *   v_T and v_B are zero vectors; the fragment shader uses dFdx/dFdy.
 */

/* All attributes are vec4. Unused components carry reserved data.
 * position.w = 1.0 (reserved)
 * normal.w   = 0.0 (reserved)
 * texcoord.zw = uv1 (reserved, currently 0)
 * tangent.w  = bitangent sign (-1 or +1), 0 = no tangents (dFdx fallback) */
layout(location = 0) in vec4 a_position;
layout(location = 1) in vec4 a_normal;
layout(location = 2) in vec4 a_texcoord;
layout(location = 3) in vec4 a_tangent;

layout(set = 1, binding = 0) uniform VP    { mat4 view_proj; };
layout(set = 1, binding = 1) uniform Model { mat4 model;     };

layout(location = 0) out vec3 v_world_pos;
layout(location = 1) out vec2 v_texcoord;
layout(location = 2) out vec3 v_T;
layout(location = 3) out vec3 v_B;
layout(location = 4) out vec3 v_N;
layout(location = 5) out float v_tangent_w;   /* +1/-1 = valid, 0 = absent */

void main()
{
    vec4 world_pos = model * vec4(a_position.xyz, 1.0);
    v_world_pos    = world_pos.xyz;
    v_texcoord     = a_texcoord.xy;
    v_tangent_w    = a_tangent.w;

    mat3 nm = transpose(inverse(mat3(model)));
    vec3 N  = normalize(a_normal.xyz);
    v_N     = nm * N;

    if (abs(a_tangent.w) > 0.5) {
        /* Valid tangent: compute B in object space then transform */
        vec3 T  = normalize(a_tangent.xyz);
        vec3 B  = cross(N, T) * a_tangent.w;
        v_T = nm * T;
        v_B = nm * B;
    } else {
        /* Sentinel – zero out; fragment shader will derive from dFdx/dFdy */
        v_T = vec3(0.0);
        v_B = vec3(0.0);
    }

    gl_Position = view_proj * world_pos;
}

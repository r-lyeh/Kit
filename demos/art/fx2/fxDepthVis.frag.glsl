// fxDepthVis.glsl — Depth buffer visualization (false color)
// Ported from r-lyeh/v2 demos/fx/fxDepthVis.glsl
// Compile: glslc -fshader-stage=frag fxDepthVis.glsl -o fxDepthVis.spv
//
// u[0].x = near       (default: 0.1)
// u[0].y = far        (default: 1000.0)
// u[0].z = min_depth  (default: 0.0)
// u[0].w = max_depth  (default: 200.0)

#version 450
layout(location=0) in  vec2 v_uv;
layout(location=0) out vec4 out_color;
layout(set=2, binding=0) uniform sampler2D u_tex;
layout(set=3, binding=0) uniform Params { vec4 u[8]; };

vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0/3.0, 1.0/3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main() {
    float near      = u[0].x != 0.0 ? u[0].x :    0.1;
    float far       = u[0].y != 0.0 ? u[0].y : 1000.0;
    float min_depth = u[0].z;
    float max_depth = u[0].w != 0.0 ? u[0].w :  200.0;
    float depth  = texture(u_tex, v_uv).r;
    float linear_depth = (2.0 * near * far) / (far + depth * (near - far));
    float nd = clamp((linear_depth - min_depth) / (max_depth - min_depth), 0.0, 1.0);
    out_color = vec4(hsv2rgb(vec3(0.66 * (1.0 - nd), 1.0, 1.0)), 1.0);
}

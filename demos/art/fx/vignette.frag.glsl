#version 450

// ---------------------------------------------------------------------------

// shaders/postfx_vignette.glsl
// u[0].x = radius (default 0.75)
// u[0].y = softness (default 0.45)
// u[0].z = strength (default 0.8)

layout(location=0) in  vec2 v_uv;
layout(location=0) out vec4 out_color;

layout(set=2, binding=0) uniform sampler2D u_tex;

layout(set=3, binding=0) uniform Params {
    vec4 u[8];
};

void main() {
    vec4 c = texture(u_tex, v_uv);
    vec2 uv = v_uv - 0.5;
    float radius   = u[0].x > 0.0 ? u[0].x : 0.75;
    float softness = u[0].y > 0.0 ? u[0].y : 0.45;
    float strength = u[0].z > 0.0 ? u[0].z : 0.80;
    float d = length(uv);
    float vignette = smoothstep(radius, radius - softness, d);
    out_color = vec4(c.rgb * mix(1.0, vignette, strength), c.a);
}

// fxAberration.glsl — Chromatic aberration
// Ported from r-lyeh/v2 demos/fx/fxAberration.glsl
// Compile: glslc -fshader-stage=frag fxAberration.glsl -o fxAberration.spv
//
// u[0].x = separation  (default: 0.003, range: -0.10..0.10)
// u[0].y = angle       (default: 0.0,   range: 0..6.28)

#version 450
layout(location=0) in  vec2 v_uv;
layout(location=0) out vec4 out_color;
layout(set=2, binding=0) uniform sampler2D u_tex;
layout(set=3, binding=0) uniform Params { vec4 u[8]; };

void main() {
    float separation = u[0].x != 0.0 ? u[0].x : 0.003;
    float angle      = u[0].y;
    vec2 offset = separation * vec2(cos(angle), sin(angle));
    vec4 color  = texture(u_tex, v_uv);
    color.r     = texture(u_tex, v_uv + offset).r;
    color.b     = texture(u_tex, v_uv - offset).b;
    out_color   = color;
}

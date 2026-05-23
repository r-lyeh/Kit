// fxLetterbox.glsl — Cinematic letterbox bars
// Ported from r-lyeh/v2 demos/fx/fxLetterbox.glsl
// Compile: glslc -fshader-stage=frag fxLetterbox.glsl -o fxLetterbox.spv
//
// u[0].x = amount  (default: 0.10, range: 0..0.5 — fraction of screen height per bar)

#version 450
layout(location=0) in  vec2 v_uv;
layout(location=0) out vec4 out_color;
layout(set=2, binding=0) uniform sampler2D u_tex;
layout(set=3, binding=0) uniform Params { vec4 u[8]; };

void main() {
    float amount = u[0].x != 0.0 ? u[0].x : 0.10;
    if (v_uv.y < amount || (1.0 - v_uv.y) < amount) {
        out_color = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }
    out_color = texture(u_tex, v_uv);
}

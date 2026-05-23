// fxQuantize.glsl — Color quantization / posterization
// Ported from r-lyeh/v2 demos/fx/fxQuantize.glsl
// Compile: glslc -fshader-stage=frag fxQuantize.glsl -o fxQuantize.spv
//
// u[0].x = factor  (default: 3.0, range: 1..255 — number of color steps)

#version 450
layout(location=0) in  vec2 v_uv;
layout(location=0) out vec4 out_color;
layout(set=2, binding=0) uniform sampler2D u_tex;
layout(set=3, binding=0) uniform Params { vec4 u[8]; };

void main() {
    float factor = u[0].x >= 1.0 ? u[0].x : 3.0;
    vec4  src    = texture(u_tex, v_uv);
    out_color = vec4(floor(src.rgb * factor + 0.5) / factor, src.a);
}

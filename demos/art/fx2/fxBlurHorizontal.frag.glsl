// fxBlurHorizontal.glsl — Efficient Gaussian blur (horizontal pass)
// Ref: https://rastergrid.com/blog/2010/09/efficient-gaussian-blur-with-linear-sampling/
// Ported from r-lyeh/v2 demos/fx/fxBlurHorizontal.glsl
// Compile: glslc -fshader-stage=frag fxBlurHorizontal.glsl -o fxBlurHorizontal.spv
//
// u[0].x = intensity   (default: 4.0, pixels)
// u[0].xy = 1/resolution (set this every frame)

#version 450
layout(location=0) in  vec2 v_uv;
layout(location=0) out vec4 out_color;
layout(set=2, binding=0) uniform sampler2D u_tex;
layout(set=3, binding=0) uniform Params { vec4 u[8]; };

void main() {
    float intensity  = u[0].x != 0.0 ? u[0].x : 4.0;
    vec2  inv_res    = u[0].yz; // 1/w, 1/h in .y and .z, or set u[0].xy = 1/res and intensity separately
    // Note: pack as u[0] = {intensity, 1/width, 1/height, 0}
    vec2  offset     = vec2(intensity * u[0].y, 0.0);
    vec4  base       = texture(u_tex, v_uv);
    vec4  color      = base * 0.30;
    color += texture(u_tex, v_uv + offset) * 0.35;
    color += texture(u_tex, v_uv - offset) * 0.35;
    out_color = vec4(color.rgb, base.a);
}

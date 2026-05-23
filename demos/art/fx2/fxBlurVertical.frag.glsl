// fxBlurVertical.glsl — Efficient Gaussian blur (vertical pass)
// Ref: https://rastergrid.com/blog/2010/09/efficient-gaussian-blur-with-linear-sampling/
// Ported from r-lyeh/v2 demos/fx/fxBlurVertical.glsl
// Compile: glslc -fshader-stage=frag fxBlurVertical.glsl -o fxBlurVertical.spv
//
// u[0] = {intensity, 1/width, 1/height, 0}

#version 450
layout(location=0) in  vec2 v_uv;
layout(location=0) out vec4 out_color;
layout(set=2, binding=0) uniform sampler2D u_tex;
layout(set=3, binding=0) uniform Params { vec4 u[8]; };

void main() {
    float intensity = u[0].x != 0.0 ? u[0].x : 4.0;
    vec2  offset    = vec2(0.0, intensity * u[0].z);
    vec4  base      = texture(u_tex, v_uv);
    vec4  color     = base * 0.30;
    color += texture(u_tex, v_uv + offset) * 0.35;
    color += texture(u_tex, v_uv - offset) * 0.35;
    out_color = vec4(color.rgb, base.a);
}

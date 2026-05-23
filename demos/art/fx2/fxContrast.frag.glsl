// fxContrast.glsl — Contrast & brightness
// Ported from r-lyeh/v2 demos/fx/fxContrast.glsl
// Compile: glslc -fshader-stage=frag fxContrast.glsl -o fxContrast.spv
//
// u[0].x = contrast    (default: 1.5; <1 bleach, >1 saturate)
// u[0].y = brightness  (default: 0.0; range 0..2)

#version 450
layout(location=0) in  vec2 v_uv;
layout(location=0) out vec4 out_color;
layout(set=2, binding=0) uniform sampler2D u_tex;
layout(set=3, binding=0) uniform Params { vec4 u[8]; };

void main() {
    float contrast   = u[0].x != 0.0 ? u[0].x : 1.5;
    float brightness = u[0].y;
    vec4 c = texture(u_tex, v_uv);
    if (c.a > 0.0) c.rgb /= c.a;
    c.rgb  = ((c.rgb - 0.5) * max(contrast, 0.0)) + 0.5;
    c.rgb += brightness;
    c.rgb *= c.a;
    out_color = c;
}

// fxGrain.glsl — Film grain
// Ported from r-lyeh/v2 demos/fx/fxGrain.glsl
// Compile: glslc -fshader-stage=frag fxGrain.glsl -o fxGrain.spv
//
// u[0].x = intensity  (default: 16.0, range: 0..32)
// u[0].y = time       (seconds, update per frame)

#version 450
layout(location=0) in  vec2 v_uv;
layout(location=0) out vec4 out_color;
layout(set=2, binding=0) uniform sampler2D u_tex;
layout(set=3, binding=0) uniform Params { vec4 u[8]; };

void main() {
    float intensity = u[0].x != 0.0 ? u[0].x : 16.0;
    float t         = u[0].y;
    vec4  color     = texture(u_tex, v_uv);
    float x         = (v_uv.x + 4.0) * (v_uv.y + 4.0) * (t * 10.0);
    vec4  grain     = vec4(mod((mod(x, 13.0)+1.0)*(mod(x, 123.0)+1.0), 0.01) - 0.005) * intensity;
    out_color = vec4((color + grain).rgb, color.a);
}

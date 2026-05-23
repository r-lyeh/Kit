// fxSharpen.glsl — Unsharp mask sharpening (3x3 kernel)
// Ported from r-lyeh/v2 demos/fx/fxSharpen.glsl
// Compile: glslc -fshader-stage=frag fxSharpen.glsl -o fxSharpen.spv
//
// u[0].x  = intensity   (default: 0.25, range: 0..2)
// u[0].yz = 1/resolution (update per frame)

#version 450
layout(location=0) in  vec2 v_uv;
layout(location=0) out vec4 out_color;
layout(set=2, binding=0) uniform sampler2D u_tex;
layout(set=3, binding=0) uniform Params { vec4 u[8]; };

void main() {
    float intensity = u[0].x != 0.0 ? u[0].x : 0.25;
    vec2  p         = u[0].yz; // 1/width, 1/height
    vec4  src       = texture(u_tex, v_uv);
    vec3  kernel    = src.rgb * 9.0
        - texture(u_tex, v_uv+vec2(-p.x,-p.y)).rgb
        - texture(u_tex, v_uv+vec2( 0.0,-p.y)).rgb
        - texture(u_tex, v_uv+vec2( p.x,-p.y)).rgb
        - texture(u_tex, v_uv+vec2(-p.x, 0.0)).rgb
        - texture(u_tex, v_uv+vec2( p.x, 0.0)).rgb
        - texture(u_tex, v_uv+vec2(-p.x, p.y)).rgb
        - texture(u_tex, v_uv+vec2( 0.0, p.y)).rgb
        - texture(u_tex, v_uv+vec2( p.x, p.y)).rgb;
    out_color = vec4(mix(src.rgb, kernel, intensity), src.a);
}

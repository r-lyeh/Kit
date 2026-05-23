// fxOutline.glsl — Alpha-based outline / edge detection
// Ported from r-lyeh/v2 demos/fx/fxOutline.glsl
// Tip: ensure color buffer alpha is cleared before this pass.
// Compile: glslc -fshader-stage=frag fxOutline.glsl -o fxOutline.spv
//
// u[0].x    = thickness      (default: 2, in pixels)
// u[0].yzw  = border_color   (default: 1,1,0,1) — also u[1].x for alpha
// u[1].xy   = 1.0/resolution (update per frame)

#version 450
layout(location=0) in  vec2 v_uv;
layout(location=0) out vec4 out_color;
layout(set=2, binding=0) uniform sampler2D u_tex;
layout(set=3, binding=0) uniform Params { vec4 u[8]; };

void main() {
    int   thickness    = int(u[0].x != 0.0 ? u[0].x : 2.0);
    vec4  border_color = vec4(u[0].yzw, u[1].x);
    if (border_color == vec4(0.0)) border_color = vec4(1.0, 1.0, 0.0, 1.0);
    vec2  inv_res      = u[1].yz; // pack 1/w, 1/h in u[1].yz

    vec4  texel   = texture(u_tex, v_uv);
    float outline = 0.0;
    if (texel.a < 0.01) {
        for (int x = -thickness; x <= thickness; x++)
        for (int y = -thickness; y <= thickness; y++) {
            if (texture(u_tex, v_uv + vec2(x,y)*inv_res).a > 0.0)
                outline = 1.0;
        }
    }
    out_color = mix(texel, border_color, outline * border_color.a);
}

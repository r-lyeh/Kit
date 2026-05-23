// fxPixelate.glsl — Pixelation / mosaic
// Ported from r-lyeh/v2 demos/fx/fxPixelate.glsl
// Compile: glslc -fshader-stage=frag fxPixelate.glsl -o fxPixelate.spv
//
// u[0].x = cell_size   (default: 2.5, range: 1..16 pixels)
// u[0].yz = resolution (width, height — update per frame)

#version 450
layout(location=0) in  vec2 v_uv;
layout(location=0) out vec4 out_color;
layout(set=2, binding=0) uniform sampler2D u_tex;
layout(set=3, binding=0) uniform Params { vec4 u[8]; };

void main() {
    float cell = u[0].x != 0.0 ? u[0].x : 2.5;
    vec2  res  = u[0].yz; // pack as {cell_size, width, height, 0}
    if (res.x < 1.0) res = vec2(1280.0, 720.0); // fallback
    float xp = res.x / cell, yp = res.y / cell;
    vec2  uv = vec2(floor(v_uv.s * xp)/xp, floor(v_uv.t * yp)/yp);
    out_color = texture(u_tex, uv);
}

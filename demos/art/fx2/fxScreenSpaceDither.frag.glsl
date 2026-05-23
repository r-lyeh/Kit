// fxScreenSpaceDither.glsl — Screen-space RGB dither (Valve / Portal 2 method)
// Ref: http://alex.vlachos.com/graphics/Alex_Vlachos_Advanced_VR_Rendering_GDC2015.pdf
// Ported from r-lyeh/v2 demos/fx/fxScreenSpaceDither.glsl
// Compile: glslc -fshader-stage=frag fxScreenSpaceDither.glsl -o fxScreenSpaceDither.spv
//
// u[0].x = intensity   (default: 250.0, range: 245..255; higher = less dither)
// u[0].y = time        (seconds, update per frame)

#version 450
layout(location=0) in  vec2 v_uv;
layout(location=0) out vec4 out_color;
layout(set=2, binding=0) uniform sampler2D u_tex;
layout(set=3, binding=0) uniform Params { vec4 u[8]; };

vec3 screenSpaceDither(vec2 screenPos, float colorDepth, float t) {
    vec3 d = vec3(dot(vec2(131.0, 312.0), screenPos + t));
    d.rgb  = fract(d.rgb / vec3(103.0, 71.0, 97.0)) - 0.5;
    return (d / colorDepth) * 0.375;
}

void main() {
    float intensity = u[0].x >= 1.0 ? u[0].x : 250.0;
    float t         = u[0].y;
    vec4  color     = texture(u_tex, v_uv);
    color.rgb      += screenSpaceDither(gl_FragCoord.xy, 255.0 - intensity, t);
    out_color       = color;
}

// fxHSV.glsl — HSV color adjustment
// Ported from r-lyeh/v2 demos/fx/fxHSV.glsl
// Compile: glslc -fshader-stage=frag fxHSV.glsl -o fxHSV.spv
//
// u[0].x = h  hue shift     (default: 1.0)
// u[0].y = s  saturation    (default: 1.0; <1 desaturate, >1 saturate)
// u[0].z = v  value/bright  (default: 1.0; <1 darken, >1 brighten)

#version 450
layout(location=0) in  vec2 v_uv;
layout(location=0) out vec4 out_color;
layout(set=2, binding=0) uniform sampler2D u_tex;
layout(set=3, binding=0) uniform Params { vec4 u[8]; };

vec3 rgb2hsv(vec3 c) {
    vec4 K = vec4(0.0,-1.0/3.0,2.0/3.0,-1.0);
    vec4 p = mix(vec4(c.bg,K.wz),vec4(c.gb,K.xy),step(c.b,c.g));
    vec4 q = mix(vec4(p.xyw,c.r),vec4(c.r,p.yzx),step(p.x,c.r));
    float d=q.x-min(q.w,q.y), e=1e-10;
    return vec3(abs(q.z+(q.w-q.y)/(6.0*d+e)),d/(q.x+e),q.x);
}
vec3 hsv2rgb(vec3 c) {
    return mix(vec3(1.),clamp(abs(fract(c.r+vec3(3.,2.,1.)/3.)*6.-3.)-1.,0.,1.),c.g)*c.b;
}

void main() {
    float h = u[0].x != 0.0 ? u[0].x : 1.0;
    float s = u[0].y != 0.0 ? u[0].y : 1.0;
    float v = u[0].z != 0.0 ? u[0].z : 1.0;
    vec4 src = texture(u_tex, v_uv);
    vec3 hsv = rgb2hsv(src.rgb);
    out_color = vec4(hsv2rgb(hsv * vec3(h, s, v)), src.a);
}

// fxTonemapUchimura.glsl — Uchimura HDR tonemapping
// Ref: "HDR theory and practice" – Uchimura 2017
// Ported from r-lyeh/v2 demos/fx/fxTonemapUchimura.glsl
// Compile: glslc -fshader-stage=frag fxTonemapUchimura.glsl -o fxTonemapUchimura.spv
//
// No per-frame uniforms needed (all constants baked in).

#version 450
layout(location=0) in  vec2 v_uv;
layout(location=0) out vec4 out_color;
layout(set=2, binding=0) uniform sampler2D u_tex;
layout(set=3, binding=0) uniform Params { vec4 u[8]; };

vec3 uchimura(vec3 x, float P, float a, float m, float l, float c, float b) {
    float l0 = ((P-m)*l)/a, L0=m-m/a, L1=m+(1.0-m)/a;
    float S0=m+l0, S1=m+a*l0, C2=(a*P)/(P-S1), CP=-C2/P;
    vec3 w0=vec3(1.0-smoothstep(0.0,m,x));
    vec3 w2=vec3(step(m+l0,x));
    vec3 w1=vec3(1.0-w0-w2);
    vec3 T=vec3(m*pow(x/m,vec3(c))+b);
    vec3 S=vec3(P-(P-S1)*exp(CP*(x-S0)));
    vec3 L=vec3(m+a*(x-m));
    return T*w0+L*w1+S*w2;
}
vec3 uchimura(vec3 x) {
    return uchimura(x, 1.0, 1.0, 0.22, 0.4, 1.33, 0.0);
}

void main() {
    vec4 src  = texture(u_tex, v_uv);
    out_color = vec4(uchimura(src.rgb), src.a);
}

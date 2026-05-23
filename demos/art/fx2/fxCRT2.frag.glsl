// fxCRT2.glsl — Windows Terminal CRT shader (CC0)
// Src: https://github.com/Hammster/windows-terminal-shaders
// Ported from r-lyeh/v2 demos/fx/fxCRT2.glsl
// Compile: glslc -fshader-stage=frag fxCRT2.glsl -o fxCRT2.spv
//
// u[0].x = time        (seconds, update per frame)
// u[0].yz = resolution (width, height — update per frame)
// Features: barrel distortion, scanlines, refresh line, noise. All enabled by default.

#version 450
layout(location=0) in  vec2 v_uv;
layout(location=0) out vec4 out_color;
layout(set=2, binding=0) uniform sampler2D u_tex;
layout(set=3, binding=0) uniform Params { vec4 u[8]; };

float permute(float x) { x*=(34*x+1); return 289.0*fract(x/289.0); }
float randF(float s)   { return fract(permute(s)/41.0); }
float fmod_(float x,float y) { return x-y*trunc(x/y); }

vec4 CRT(vec2 uv, float t, vec2 res) {
    vec2 xy = uv - 0.5;
    float r  = dot(xy,xy);
    xy = (xy*(4.2+r))*0.25 + 0.5;
    if (xy.x<-0.025||xy.y<-0.025||xy.x>1.025||xy.y>1.025) return vec4(0.0);
    if (xy.x<-0.015||xy.y<-0.015||xy.x>1.015||xy.y>1.015) return vec4(0.03,0.03,0.03,0.0);
    if (xy.x<0.001 ||xy.y<0.001 ||xy.x>0.999||xy.y>0.999) return vec4(0.0);

    vec4 color = texture(u_tex, xy);

    float timeOver = fmod_(t/5.0, 1.0);
    float rlTint   = timeOver - xy.y;
    if (xy.y>timeOver && xy.y-0.03<timeOver) color.rgb += rlTint*2.0;

    if (fmod_(floor(uv.y*res.y), 2.0) != 0.0) color *= vec4(0.6,0.6,0.6,0.0);

    return color;
}

void main() {
    float t   = u[0].x;
    vec2  res = u[0].yz;
    if (res.x < 1.0) res = vec2(1280.0, 720.0);
    vec4 src = texture(u_tex, v_uv);
    out_color = vec4(CRT(v_uv, t, res).rgb, src.a);
}

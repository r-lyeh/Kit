// fxSSAO.glsl — Screen Space Ambient Occlusion
// Based on code by arkano22. Ref: http://www.gamedev.net/forums/topic/550699-ssao-no-halo-artifacts/
// Originally: rlyeh, public domain
// Ported from r-lyeh/v2 demos/fx/fxSSAO.glsl
// Compile: glslc -fshader-stage=frag fxSSAO.glsl -o fxSSAO.spv
//
// u[0].xy = camera_near, camera_far  (default: 150, 850)
// u[0].z  = strength                 (default: 16, range: 1..64)
// u[0].w  = time                     (seconds, for jitter)
// u[1].xy = 1/resolution             (update per frame)
//
// Note: u_tex .rgb = color, .a (or a second depth texture in iChannel1).
// For a full SSAO you would pass the depth in alpha. This port reads
// depth from the alpha channel of u_tex (pack depth into alpha upstream).

#version 450
layout(location=0) in  vec2 v_uv;
layout(location=0) out vec4 out_color;
layout(set=2, binding=0) uniform sampler2D u_tex;
layout(set=3, binding=0) uniform Params { vec4 u[8]; };

#define MOD3 vec3(.1031,.11369,.13787)

vec2 hash22(vec2 p) {
    vec3 p3 = fract(vec3(p.xyx) * MOD3);
    p3 += dot(p3, p3.yzx+19.19);
    return fract(vec2((p3.x+p3.y)*p3.z, (p3.x+p3.z)*p3.y));
}

vec2 getRandom(vec2 uv) {
    return normalize(hash22(uv*126.1231)*2.0-1.0);
}

float readDepth(vec2 coord, float near, float far) {
    if (coord.x<0.0||coord.y<0.0||coord.x>1.0||coord.y>1.0) return 1.0;
    float posZ = texture(u_tex, coord).a; // depth stored in alpha
    return (2.0 * near) / (near + far - posZ * (far - near));
}

float compareDepths(float d1, float d2, inout int far) {
    float diff = (d1 - d2) * 100.0;
    float gdisplace = 0.2, garea = 2.0;
    if (diff < gdisplace) garea = 0.1; else far = 1;
    return exp2(-2.0*(diff-gdisplace)*(diff-gdisplace)/(garea*garea));
}

float calcAO(float depth, vec2 uv, float dw, float dh, float near, float far) {
    float coordw  = uv.x + dw/depth, coordh  = uv.y + dh/depth;
    float coordw2 = uv.x - dw/depth, coordh2 = uv.y - dh/depth;
    if (coordw<1.0&&coordw>0.0&&coordh<1.0&&coordh>0.0) {
        int farFlag = 0;
        float temp  = compareDepths(depth, readDepth(vec2(coordw,coordh),near,far), farFlag);
        if (farFlag > 0) temp += (1.0-temp)*compareDepths(readDepth(vec2(coordw2,coordh2),near,far), depth, farFlag);
        return temp;
    }
    return 0.0;
}

void main() {
    float near     = u[0].x != 0.0 ? u[0].x : 150.0;
    float far      = u[0].y != 0.0 ? u[0].y : 850.0;
    float strength = u[0].z != 0.0 ? u[0].z :  16.0;
    float t        = u[0].w;
    vec2  inv_res  = u[1].xy;
    if (inv_res.x == 0.0) inv_res = vec2(1.0/1280.0, 1.0/720.0);

    vec2  random = getRandom(v_uv + vec2(t));
    float depth  = readDepth(v_uv, near, far);
    float pw = inv_res.x * 0.5, ph = inv_res.y * 0.5;
    float ao = 0.0;
    for (int i = 0; i < 4; i++) {
        ao += calcAO(depth,v_uv, pw,  ph, near,far);
        ao += calcAO(depth,v_uv, pw, -ph, near,far);
        ao += calcAO(depth,v_uv,-pw,  ph, near,far);
        ao += calcAO(depth,v_uv,-pw, -ph, near,far);
        ao += calcAO(depth,v_uv, pw*1.2, 0.0, near,far);
        ao += calcAO(depth,v_uv,-pw*1.2, 0.0, near,far);
        ao += calcAO(depth,v_uv, 0.0,  ph*1.2, near,far);
        ao += calcAO(depth,v_uv, 0.0, -ph*1.2, near,far);
        pw += random.x*0.0007; ph += random.y*0.0007;
        pw *= 1.7; ph *= 1.7;
    }
    vec4  texel   = texture(u_tex, v_uv);
    float finalAO = 1.0 - (ao/strength);
    finalAO = 0.5 + finalAO * 0.5;
    out_color = vec4(texel.rgb * finalAO, texel.a);
}

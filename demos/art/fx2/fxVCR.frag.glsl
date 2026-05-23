// fxVCR.glsl — VHS/VCR tape effect
// (c) 2021 FMS_Cat – MIT License
// Original: https://www.shadertoy.com/view/MdffD7
// Ported from r-lyeh/v2 demos/fx/fxVCR.glsl
// Compile: glslc -fshader-stage=frag fxVCR.glsl -o fxVCR.spv
//
// u[0].x  = time        (seconds, update per frame)
// u[0].yz = resolution  (width, height — update per frame)
// u[0].w  = scale       (default: 1.0)
// u[1]    = background color (default: 0,0,0,1)

#version 450
layout(location=0) in  vec2 v_uv;
layout(location=0) out vec4 out_color;
layout(set=2, binding=0) uniform sampler2D u_tex;
layout(set=3, binding=0) uniform Params { vec4 u[8]; };

const int   SAMPLES          = 6;
const float COLOR_NOISE_AMP  = 0.1;
const vec3  YIQ_OFFSET       = vec3(-0.1,-0.1, 0.0);
const vec3  YIQ_AMP          = vec3( 1.2, 1.1, 1.5);
const float PI               = 3.14159265;

float fs(float s)           { return fract(sin(s*114.514)*1919.810); }
float fs2(vec2 s)           { return fs(s.x + fs(s.y)); }
bool  validuv(vec2 uv)      { return uv.x>0.0&&uv.x<1.0&&uv.y>0.0&&uv.y<1.0; }
vec2  yflip(vec2 uv)        { return vec2(uv.x, 1.0-uv.y); }

float v2Random(vec2 v) {
    vec2 vf=fract(v*256.0), vi=floor(v*256.0)/256.0;
    vec2 d=vec2(0.0,1.0/256.0);
    return mix(mix(fs2(vi+d.xx),fs2(vi+d.yx),vf.x),
               mix(fs2(vi+d.xy),fs2(vi+d.yy),vf.x),vf.y);
}

vec3 rgb2yiq(vec3 c) { return mat3(0.299,0.596,0.211,0.587,-0.274,-0.523,0.114,-0.322,0.312)*c; }
vec3 yiq2rgb(vec3 c) { return mat3(1.0,1.0,1.0,0.956,-0.272,-1.106,0.621,-0.647,1.703)*c; }

vec3 vhsTex2D(vec2 uv, vec2 inv_res, vec4 bg) {
    if (!validuv(uv)) return vec3(0.1);
    vec3 yiq = vec3(0.0);
    for (int i=0;i<SAMPLES;i++) {
        vec2 uvt = uv - vec2(float(i),0.0)*inv_res;
        if (validuv(uvt)) {
            vec4  tex = texture(u_tex, uvt);
            yiq += (rgb2yiq(mix(bg.rgb, tex.rgb, tex.a)) *
                    vec2(float(i),float(SAMPLES-1-i)).yxx / float(SAMPLES-1))
                    / float(SAMPLES) * 2.0;
        }
    }
    return yiq2rgb(yiq);
}

void main() {
    float t       = u[0].x;
    vec2  res     = u[0].yz;
    if (res.x < 1.0) res = vec2(1280.0, 720.0);
    vec2  inv_res = 1.0 / res;
    vec4  bg      = u[1];
    if (bg == vec4(0.0)) bg = vec4(0.0,0.0,0.0,1.0);

    vec2 uvt = yflip(v_uv);

    // tape wave
    uvt.x += (v2Random(vec2(uvt.y/10.0, t/10.0))-0.5)*inv_res.x;
    uvt.x += (v2Random(vec2(uvt.y, t*10.0))-0.5)*inv_res.x;

    // tape crease
    float tcPhase = smoothstep(0.9,0.96,sin(uvt.y*8.0-(t+0.14*v2Random(t*vec2(0.67,0.59)))*PI*1.2));
    float tcNoise = smoothstep(0.3,1.0,v2Random(vec2(uvt.y*4.77,t)));
    float tc      = tcPhase * tcNoise;
    uvt.x        -= tc * inv_res.x * 8.0;

    // switching noise
    float snPhase = smoothstep(6.0*inv_res.y, 0.0, uvt.y);
    uvt.y += snPhase * 0.3;
    uvt.x += snPhase * (v2Random(vec2(v_uv.y*100.0, t*10.0))-0.5)*inv_res.x*24.0;

    vec4  tex   = texture(u_tex, v_uv);
    vec3  color = vhsTex2D(yflip(uvt), inv_res, bg);
    color = pow(color, vec3(0.4545));

    // crease noise
    float cn = tcNoise * (0.3 + 0.7*tcPhase);
    if (cn > 0.29) {
        vec2  uvtt = (uvt + vec2(1.0,0.0)*v2Random(vec2(uvt.y,t)))*vec2(0.1,1.0);
        float n0 = v2Random(uvtt), n1 = v2Random(uvtt+vec2(1.0,0.0)*inv_res.x);
        if (n1 < n0) color = mix(color, vec3(2.0), pow(n0,10.0));
    }

    // ac beat
    color *= 1.0 + 0.1*smoothstep(0.4,0.6,v2Random(vec2(0.0, 0.1*(v_uv.y+t*0.2))/10.0));

    // color noise
    vec2 noiseuv = uvt + vec2(fs(t), fs(t/0.7));
    vec3 noise   = vec3(v2Random(noiseuv), v2Random(noiseuv+0.7), v2Random(noiseuv+1.4));
    color = clamp(color, 0.0, 1.0);
    color = rgb2yiq(color);
    color += COLOR_NOISE_AMP * (noise - 0.5);
    color = YIQ_OFFSET + YIQ_AMP * color;
    color = yiq2rgb(color);
    color = pow(color, vec3(2.2));

    out_color = vec4(color, tex.a);
}

// fxCRT.glsl — CRT scan-line shader (Timothy Lottes, public domain)
// Ported from r-lyeh/v2 demos/fx/fxCRT.glsl
// Compile: glslc -fshader-stage=frag fxCRT.glsl -o fxCRT.spv
//
// u[0].x = hardScan   (default: -8.0;  -8=soft, -16=medium)
// u[0].y = hardPix    (default: -3.0;  -2=soft, -4=hard)
// u[0].zw = resolution (width, height — update per frame)

#version 450
layout(location=0) in  vec2 v_uv;
layout(location=0) out vec4 out_color;
layout(set=2, binding=0) uniform sampler2D u_tex;
layout(set=3, binding=0) uniform Params { vec4 u[8]; };

float ToLinear1(float c) { return c<=0.04045?c/12.92:pow((c+0.055)/1.055,2.4); }
vec3  ToLinear(vec3 c)   { return vec3(ToLinear1(c.r),ToLinear1(c.g),ToLinear1(c.b)); }
float ToSrgb1(float c)   { return c<0.0031308?c*12.92:1.055*pow(c,0.41666)-0.055; }
vec3  ToSrgb(vec3 c)     { return vec3(ToSrgb1(c.r),ToSrgb1(c.g),ToSrgb1(c.b)); }

vec3 Fetch(vec2 pos, vec2 off, vec2 res) {
    pos = floor(pos*res+off)/res;
    if (max(abs(pos.x-0.5),abs(pos.y-0.5))>0.5) return vec3(0.0);
    return ToLinear(textureLod(u_tex, pos, -16.0).rgb);
}
vec2 Dist(vec2 pos, vec2 res) { pos=pos*res; return -((pos-floor(pos))-vec2(0.5)); }
float Gaus(float pos, float scale) { return exp2(scale*pos*pos); }

vec3 Horz3(vec2 pos,float off,float hardPix,vec2 res){
    vec3 b=Fetch(pos,vec2(-1,off),res), c=Fetch(pos,vec2(0,off),res), d=Fetch(pos,vec2(1,off),res);
    float dst=Dist(pos,res).x;
    float wb=Gaus(dst-1,hardPix),wc=Gaus(dst,hardPix),wd=Gaus(dst+1,hardPix);
    return (b*wb+c*wc+d*wd)/(wb+wc+wd);
}
vec3 Horz5(vec2 pos,float off,float hardPix,vec2 res){
    vec3 a=Fetch(pos,vec2(-2,off),res),b=Fetch(pos,vec2(-1,off),res),c=Fetch(pos,vec2(0,off),res);
    vec3 d=Fetch(pos,vec2(1,off),res),e=Fetch(pos,vec2(2,off),res);
    float dst=Dist(pos,res).x;
    float wa=Gaus(dst-2,hardPix),wb=Gaus(dst-1,hardPix),wc=Gaus(dst,hardPix),wd=Gaus(dst+1,hardPix),we=Gaus(dst+2,hardPix);
    return (a*wa+b*wb+c*wc+d*wd+e*we)/(wa+wb+wc+wd+we);
}
float Scan(vec2 pos,float off,float hardScan,vec2 res){return Gaus(Dist(pos,res).y+off,hardScan);}

vec3 Tri(vec2 pos,float hardScan,float hardPix,vec2 res){
    vec3 a=Horz3(pos,-1,hardPix,res),b=Horz5(pos,0,hardPix,res),c=Horz3(pos,1,hardPix,res);
    return a*Scan(pos,-1,hardScan,res)+b*Scan(pos,0,hardScan,res)+c*Scan(pos,1,hardScan,res);
}

vec2 Warp(vec2 pos) {
    vec2 warp=vec2(1.0/32.0,1.0/24.0);
    pos=pos*2.0-1.0;
    pos*=vec2(1.0+(pos.y*pos.y)*warp.x, 1.0+(pos.x*pos.x)*warp.y);
    return pos*0.5+0.5;
}

vec3 Mask(vec2 pos) {
    float maskDark=0.5,maskLight=1.5;
    pos.x+=pos.y*3.0;
    vec3 mask=vec3(maskDark);
    pos.x=fract(pos.x/6.0);
    if(pos.x<0.333)mask.r=maskLight;
    else if(pos.x<0.666)mask.g=maskLight;
    else mask.b=maskLight;
    return mask;
}

void main() {
    float hardScan = u[0].x != 0.0 ? u[0].x : -8.0;
    float hardPix  = u[0].y != 0.0 ? u[0].y : -3.0;
    vec2  res      = u[0].zw;
    if (res.x < 1.0) res = vec2(1280.0, 720.0);
    vec2 emuRes = res / 6.0;

    vec2 pos   = Warp(gl_FragCoord.xy / res);
    vec3 color = Tri(pos, hardScan, hardPix, emuRes) * Mask(gl_FragCoord.xy);
    float alpha = textureLod(u_tex, gl_FragCoord.xy/res, 0.0).a;
    out_color   = vec4(ToSrgb(color), alpha);
}

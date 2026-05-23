#ifndef EASE_H
#define EASE_H

extern struct ease_api {
float       (*ping)(float t01, const char *fn); // `/` 0-to-1 shape
float       (*pong)(float t01, const char *fn); // `\` 1-to-0 shape
float       (*pingpong)(float t01, const char *fn1, const char *fn2); // `/\` 0-to-1-to-0 shape
float       (*pongping)(float t01, const char *fn1, const char *fn2); // `\/` 1-to-0-to-1 shape
} ease;

#elif KIT_CODE
#pragma once

// ----------------------------------------------------------------------------
// ease

#ifndef C_PI
#define C_PI 3.1415926535897932384626433832795f
#endif

float ease_zero(float t) { return 0; }
float ease_one(float t) { return 1; }
float ease_linear(float t) { return t; }

float ease_out_sine(float t) { return sinf(t*(C_PI*0.5f)); }
float ease_out_quad(float t) { return -(t*(t-2)); }
float ease_out_cubic(float t) { float f=t-1; return f*f*f+1; }
float ease_out_quart(float t) { float f=t-1; return f*f*f*(1-t)+1; }
float ease_out_quint(float t) { float f=(t-1); return f*f*f*f*f+1; }
float ease_out_expo(float t) { return (t >= 1) ? t : 1-powf(2,-10*t); }
float ease_out_circ(float t) { return sqrtf((2-t)*t); }
float ease_out_back(float t) { float f=1-t; return 1-(f*f*f-f*sinf(f*C_PI)); }
float ease_out_elastic(float t) { return sinf(-13*(C_PI*0.5f)*(t+1))*powf(2,-10*t)+1; }
float ease_out_bounce(float t) { return (t < 4.f/11) ? (121.f*t*t)/16 : (t < 8.f/11) ? (363.f/40*t*t)-(99.f/10*t)+17.f/5 : (t < 9.f/10) ? (4356.f/361*t*t)-(35442.f/1805*t)+16061.f/1805 : (54.f/5*t*t)-(513.f/25*t)+268.f/25; }

float ease_in_sine(float t) { return 1+sinf((t-1)*(C_PI*0.5f)); }
float ease_in_quad(float t) { return t*t; }
float ease_in_cubic(float t) { return t*t*t; }
float ease_in_quart(float t) { return t*t*t*t; }
float ease_in_quint(float t) { return t*t*t*t*t; }
float ease_in_expo(float t) { return (t <= 0) ? t : powf(2,10*(t-1)); }
float ease_in_circ(float t) { return 1-sqrtf(1-(t*t)); }
float ease_in_back(float t) { return t*t*t-t*sinf(t*C_PI); }
float ease_in_elastic(float t) { return sinf(13*(C_PI*0.5f)*t)*powf(2,10*(t-1)); }
float ease_in_bounce(float t) { return 1-ease_out_bounce(1-t); }

float ease_inout_sine(float t) { return 0.5f*(1-cosf(t*C_PI)); }
float ease_inout_quad(float t) { return (t < 0.5f) ? 2*t*t : (-2*t*t)+(4*t)-1; }
float ease_inout_cubic(float t) { float f; return (t < 0.5f) ? 4*t*t*t : (f=(2*t)-2,0.5f*f*f*f+1); }
float ease_inout_quart(float t) { float f; return (t < 0.5f) ? 8*t*t*t*t : (f=(t-1),-8*f*f*f*f+1); }
float ease_inout_quint(float t) { float f; return (t < 0.5f) ? 16*t*t*t*t*t : (f=((2*t)-2),0.5f*f*f*f*f*f+1); }
float ease_inout_expo(float t) { return (t <= 0 || t >= 1) ? t : t < 0.5f ? 0.5f*powf(2,(20*t)-10) : -0.5f*powf(2,(-20*t)+10)+1; }
float ease_inout_circ(float t) { return t < 0.5f ? 0.5f*(1-sqrtf(1-4*(t*t))) : 0.5f*(sqrtf(-((2*t)-3)*((2*t)-1))+1); }
float ease_inout_back(float t) { float f; return t < 0.5f ? (f=2*t,0.5f*(f*f*f-f*sinf(f*C_PI))) : (f=(1-(2*t-1)),0.5f*(1-(f*f*f-f*sinf(f*C_PI)))+0.5f); }
float ease_inout_elastic(float t) { return t < 0.5f ? 0.5f*sinf(13*(C_PI*0.5f)*(2*t))*powf(2,10*((2*t)-1)) : 0.5f*(sinf(-13*(C_PI*0.5f)*((2*t-1)+1))*powf(2,-10*(2*t-1))+2); }
float ease_inout_bounce(float t) { return t < 0.5f ? 0.5f*ease_in_bounce(t*2) : 0.5f*ease_out_bounce(t*2-1)+0.5f; }

float ease_inout_perlin(float t) { float t3=t*t*t,t4=t3*t,t5=t4*t; return 6*t5-15*t4+10*t3; }

#if 0
// arch: parabolic peak at t=0.5. great for squash/stretch, impact flashes
float ease_arch(float t) { return 4*t*(1.f-t); }

// gain: pushes values toward 0 or 1 extremes. k>1 = contrast, k<1 = soften
float ease_gain(float t, float k) { return t < 0.5f ? 0.5f*powf(2*t, k) : 1 - 0.5f*powf(2 - 2*t, k); }

// bias: skews the curve toward 0 (k<0.5) or 1 (k>0.5)
float ease_bias(float t, float k) { return powf(t, logf(k) / logf(0.5f)); }

// spring: critically damped, very popular in game cameras and UI
// stiffness: how fast it settles (6-12 typical), damping: oscillation (1=no oscillation)
float ease_spring(float t, float stiffness, float damping) { return 1 - expf(-stiffness*t) * cosf(damping*t); }

// pulse: Gaussian bell centered at c with width w. good for glows, shockwaves
float ease_pulse(float t, float c, float w) { float x = (t - c) / w; return expf(-x*x); }

// power: generalized n-th power. ease_power(t,2)=quad, (t,3)=cubic etc. 
// useful when driven by a config value rather than hardcoded curve name
float ease_out_power(float t, float p) { return 1 - powf(1-t, p); }
float ease_in_power (float t, float p) { return powf(t, p); }

// bezier: CSS cubic-bezier(x1,y1,x2,y2) compatible. essential for animation tools
// needs a Newton-Raphson solver for t given x: more complex, but widely expected
float ease_bezier(float t, float x1, float y1, float x2, float y2);
#endif

float ease_ping(float t, const char *name) {
    struct { const char *name; float(*fn)(float); } map[] = {
        {"zero",         ease_zero}, // [0]
        {"one",          ease_one},
        {"linear",       ease_linear},
        {NULL},
        {"out.sine",     ease_out_sine}, // [4]
        {"out.quad",     ease_out_quad},
        {"out.cubic",    ease_out_cubic},
        {"out.quart",    ease_out_quart},
        {"out.quint",    ease_out_quint},
        {"out.expo",     ease_out_expo},
        {"out.circ",     ease_out_circ},
        {"out.back",     ease_out_back},
        {"out.elastic",  ease_out_elastic},
        {"out.bounce",   ease_out_bounce},
        {NULL},
        {"in.sine",      ease_in_sine}, // [15]
        {"in.quad",      ease_in_quad},
        {"in.cubic",     ease_in_cubic},
        {"in.quart",     ease_in_quart},
        {"in.quint",     ease_in_quint},
        {"in.expo",      ease_in_expo},
        {"in.circ",      ease_in_circ},
        {"in.back",      ease_in_back},
        {"in.elastic",   ease_in_elastic},
        {"in.bounce",    ease_in_bounce},
        {NULL},
        {"inout.sine",   ease_inout_sine}, // [26]
        {"inout.quad",   ease_inout_quad},
        {"inout.cubic",  ease_inout_cubic},
        {"inout.quart",  ease_inout_quart},
        {"inout.quint",  ease_inout_quint},
        {"inout.expo",   ease_inout_expo},
        {"inout.circ",   ease_inout_circ},
        {"inout.back",   ease_inout_back},
        {"inout.elastic",ease_inout_elastic},
        {"inout.bounce", ease_inout_bounce},
        {"inout.perlin", ease_inout_perlin},
        {NULL}
    };

    int i = 0;
    /**/ if( name[0] == 'o' && name[1] == 'u' ) i = 4;
    else if( name[0] == 'i' ) i = name[2] == 'o' ? 26 : 15;

    for (; map[i].name; i++)
        if (!strcmp(map[i].name, name)) return map[i].fn(t < 0 ? 0 : t > 1 ? 1 : t);
    return t; // unknown, linear fallback
}
float ease_pong    (float t, const char *fn) { return 1 - ease_ping(t, fn); }
float ease_pingpong(float t, const char *fn1, const char *fn2) { return t < 0.5 ? ease_ping(t*2, fn1) : ease_ping(1-(t-0.5f)*2, fn2); }
float ease_pongping(float t, const char *fn1, const char *fn2) { return 1 - ease_pingpong(t, fn1, fn2); }

struct ease_api ease = { .ping=ease_ping, .pong=ease_pong, .pingpong=ease_pingpong, .pongping=ease_pongping };

#endif

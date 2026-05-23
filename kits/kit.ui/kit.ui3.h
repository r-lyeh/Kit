#if 0

int ui_label3_(const char *txt, float v[3]) {
    return ui_label(va("%s: %f %f %f", txt, v[0], v[1], v[2]));
}
int ui_label4_(const char *txt, float v[4]) {
    return ui_label(va("%s: %f %f %f %f", txt, v[0], v[1], v[2], v[3]));
}
int ui_float2_(const char *txt, float v[2], float lo, float hi) {
    int mod = 0;
    mod |= ui_label(txt);
    mod |= ui_float("x", v+0, lo, hi);
    mod |= ui_float("y", v+1, lo, hi);
    return mod;
}
int ui_float3_(const char *txt, float v[3], float lo, float hi) {
    int mod = 0;
    mod |= ui_label(txt);
    mod |= ui_float("x", v+0, lo, hi);
    mod |= ui_float("y", v+1, lo, hi);
    mod |= ui_float("z", v+2, lo, hi);
    return mod;
}
int ui_float4_(const char *txt, float v[4], float lo, float hi) {
    int mod = 0;
    mod |= ui_label(txt);
    mod |= ui_float("x", v+0, lo, hi);
    mod |= ui_float("y", v+1, lo, hi);
    mod |= ui_float("z", v+2, lo, hi);
    mod |= ui_float("w", v+3, lo, hi);
    return mod;
}

int (*ui_label3)(const char *, float [3]) = ui_label3_;
int (*ui_label4)(const char *, float [4]) = ui_label4_;
int (*ui_float2)(const char *, float [2], float, float) = ui_float2_;
int (*ui_float3)(const char *, float [3], float, float) = ui_float3_;
int (*ui_float4)(const char *, float [4], float, float) = ui_float4_;

int (*ui_texture)(void *) = ui_texture_;

#endif

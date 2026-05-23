#ifndef OBJ_H
#define OBJ_H

// objhash(p,h)

#define objsave(p,fp,...)  objsave(p, fp, "" __VA_ARGS__)
#define objload(p,i,...)   objload(p, i, "" __VA_ARGS__)
#define objecho(p,...)     objecho(p, "" __VA_ARGS__)
#define objdiff(o1,o2,...) objdiff(o1, o2, "" __VA_ARGS__)

void (objsave)(void *p, FILE *fp, const char *type_);
void (objload)(void *p, const char *inidata, const char *type_);
void (objecho)(void *p, const char *type_);
void (objdiff)(void *p, void *q, const char *type_);

#elif KIT_CODE
#pragma once

void (objdiff)(void *p, void *q, const char *type_) {
    FILE *fp = stdout;
    if(!type_ || !type_[0]) type_ = type(p); // assert( type(q) == type_ );
    for( rtti *R = reflected(type_); R; R = 0 ) {
        static const char *stable_int = 0; if(!stable_int) stable_int = stable("int");
        static const char *stable_bool = 0; if(!stable_bool) stable_bool = stable("bool");
        static const char *stable_char = 0; if(!stable_char) stable_char = stable("char");
        static const char *stable_float = 0; if(!stable_float) stable_float = stable("float");
        static const char *stable_double = 0; if(!stable_double) stable_double = stable("double");
        static const char *stable_unsigned = 0; if(!stable_unsigned) stable_unsigned = stable("unsigned");

//        fprintf(fp,"(%s:", R->name);
        int c = 0;
        for( int i = 0, ii = array_count(R->child); i < ii; ++i ) {
            rtti *C = &R->child[i];
            /**/ if(C->type == stable_int)      { if( memcmp((char*)p + C->offset, (char*)q + C->offset, C->size) ) fprintf(fp,!(c++)+",%s=%i", C->name, *(int*)((char*)q + C->offset)); }
            else if(C->type == stable_bool)     { if( memcmp((char*)p + C->offset, (char*)q + C->offset, C->size) ) fprintf(fp,!(c++)+",%s=%i", C->name, *(bool*)((char*)q + C->offset)); }
            else if(C->type == stable_float)    { if( memcmp((char*)p + C->offset, (char*)q + C->offset, C->size) ) fprintf(fp,!(c++)+",%s=%.9g", C->name, *(float*)((char*)q + C->offset)); } // safe float  ->  9 digits (float  has ~24 bits of mantissa (~ 7.22 decimal digits))
            else if(C->type == stable_double)   { if( memcmp((char*)p + C->offset, (char*)q + C->offset, C->size) ) fprintf(fp,!(c++)+",%s=%.17g", C->name, *(double*)((char*)q + C->offset)); } // safe double -> 17 digits (double has ~53 bits of mantissa (~15.95 decimal digits))
            else if(C->type == stable_unsigned) { if( memcmp((char*)p + C->offset, (char*)q + C->offset, C->size) ) fprintf(fp,!(c++)+",%s=%u", C->name, *(unsigned*)((char*)q + C->offset)); }
            else if(C->type == stable_char && C->is_ptr) { if( memcmp((char*)p + C->offset, (char*)q + C->offset, C->size) ) fprintf(fp,!(c++)+",%s=\"%s\"", C->name, *(const char* *)((char*)q + C->offset)); }
            else fprintf(fp, "%s?", C->type);
        }
//        fprintf(fp,")\n");
    }
}
void (objsave)(void *p, FILE *fp, const char *type_) {
    if(!type_ || !type_[0]) type_ = type(p);
    for( rtti *R = reflected(type_); R; R = 0 ) {
        static const char *stable_int = 0; if(!stable_int) stable_int = stable("int");
        static const char *stable_bool = 0; if(!stable_bool) stable_bool = stable("bool");
        static const char *stable_char = 0; if(!stable_char) stable_char = stable("char");
        static const char *stable_float = 0; if(!stable_float) stable_float = stable("float");
        static const char *stable_double = 0; if(!stable_double) stable_double = stable("double");
        static const char *stable_unsigned = 0; if(!stable_unsigned) stable_unsigned = stable("unsigned");

        int c = 0;
        fprintf(fp,"(%s:", R->name);
        for( int i = 0, ii = array_count(R->child); i < ii; ++i ) {
            rtti *C = &R->child[i];
            /**/ if(C->type == stable_int)      fprintf(fp,!(c++)+",%s=%i", C->name, *(int*)((char*)p + C->offset));
            else if(C->type == stable_bool)     fprintf(fp,!(c++)+",%s=%i", C->name, *(bool*)((char*)p + C->offset));
            else if(C->type == stable_float)    fprintf(fp,!(c++)+",%s=%.9g", C->name, *(float*)((char*)p + C->offset));   // safe float  ->  9 digits (float  has ~24 bits of mantissa (~ 7.22 decimal digits))
            else if(C->type == stable_double)   fprintf(fp,!(c++)+",%s=%.17g", C->name, *(double*)((char*)p + C->offset)); // safe double -> 17 digits (double has ~53 bits of mantissa (~15.95 decimal digits))
            else if(C->type == stable_unsigned) fprintf(fp,!(c++)+",%s=%u", C->name, *(unsigned*)((char*)p + C->offset));
            else if(C->type == stable_char && C->is_ptr) fprintf(fp,!(c++)+",%s=\"%s\"", C->name, *(const char* *)((char*)p + C->offset)); // @todo
            else fprintf(fp, "%s?", C->type);
        }
        fprintf(fp,")\n");
    }
}
void (objload)(void *p, const char *inidata, const char *type_) {
    int numpairs; const char **pairs = ini(inidata, &numpairs);

    if(!type_ || !type_[0]) type_ = type(p);
    for( rtti *R = reflected(type_); R; R = 0 ) {
        static const char *stable_int = 0; if(!stable_int) stable_int = stable("int");
        static const char *stable_bool = 0; if(!stable_bool) stable_bool = stable("bool");
        static const char *stable_char = 0; if(!stable_char) stable_char = stable("char");
        static const char *stable_float = 0; if(!stable_float) stable_float = stable("float");
        static const char *stable_double = 0; if(!stable_double) stable_double = stable("double");
        static const char *stable_unsigned = 0; if(!stable_unsigned) stable_unsigned = stable("unsigned");

        memset(p, 0, R->size);

        int tl = strlen(type_);
        for( int j = 0, jj = numpairs; j < jj; ++j ) {
            const char *k = pairs[j*2+0];
            const char *v = pairs[j*2+1];

            const char *clean = strchr(k, '.'); // support both `x` and `vec3.x` key types
            if( clean ) k = clean + 1;

            for( int i = 0, ii = array_count(R->child); i < ii; ++i ) {
                rtti *C = &R->child[i];

                if( strcmp(k, C->name) ) continue;
                printf("found %s\n", C->name);

                union {
                    int i;
                    bool b;
                    char *s;
                    float f;
                    double d;
                    unsigned u;
                    uintptr_t p;
                } u;
                /**/ if(C->type == stable_int)      u.i = atoi(v), memcpy((char*)p + C->offset, &u.i, C->size);
                else if(C->type == stable_bool)     u.b = atoi(v), memcpy((char*)p + C->offset, &u.b, C->size);
                else if(C->type == stable_float)    u.f = atof(v), memcpy((char*)p + C->offset, &u.f, C->size);
                else if(C->type == stable_double)   u.d = atof(v), memcpy((char*)p + C->offset, &u.d, C->size);
                else if(C->type == stable_unsigned) u.u = atoi(v), memcpy((char*)p + C->offset, &u.u, C->size);
                else if(C->type == stable_char && C->is_ptr) u.s = strdup(v+1), u.s[strlen(u.s)-1] = '\0', memcpy((char*)p + C->offset, &u.p, sizeof(u.p)); // \"quotes\" excluded
                else fprintf(stdout, "%s?", C->type);
            }
        }
    }
}
void (objecho)(void *p, const char *type_) {
    (objsave)(p, stdout, type_);
}

#if defined TEST || defined TESTS

REFLECT(
    typedef struct fvec3 {
        float x,y,z;
    } fvec3;
)

AUTOTEST {
    fvec3 *q = make(fvec3, 1);
    objsave(q, stdout);             // (fvec3:0,0,0)
    objload(q, "z=30\ny=20\nx=10");
    objsave(q, stdout);             // (fvec3:10,20,30)
    drop(&q);

    fvec3 p = {1,2,3};
    objsave(&p, stdout, "fvec3");    // (fvec3:1,2,3)
    objload(&p, "y=22\n\nx=11\n  z= 33", "fvec3");
    objsave(&p, stdout, "fvec3");    // (fvec3:11,22,33)
}

#endif // TEST

#endif // KIT_CODE

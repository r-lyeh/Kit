#ifndef SERIAL_H
#define SERIAL_H

// serial ---------------------------------------------------------------------
// we push the whole struct contents (fixed datas), then we save the variable-length members separately.
// during loading, we load the whole struct contents, then patch all the variable-length members separately.
// note: big endian not supported (any machine around at all?)
// note: pragma pack(push,1) may be needed on some architectures. use everywhere?
// note: we do not clean inter-padded gaps that could contain uninitialized datas. should we?

bool save_struct(const void *ptr, unsigned len, FILE *fp);
bool load_struct(void **ptr, unsigned len, FILE *fp);
bool save_string(const char *s, FILE *fp);
bool load_string(char **s, FILE *fp);

// savegame -------------------------------------------------------------------

FILE *      savegame();
FILE *      loadgame();
const char *savename();
void        cancel_savegame(void);

#elif KIT_CODE
#pragma once

bool save_struct(const void *ptr, unsigned len, FILE *fp) {
    if( fwrite(ptr, len, 1, fp) != 1 ) return 0;
    return 1;
}
bool load_struct(void **ptr, unsigned len, FILE *fp) {
    if( fread(*ptr, len, 1, fp) != 1 ) return memset(*ptr, 0, len), 0;
    return 1;
}
bool save_string(const char *s, FILE *fp) {
    if( s ) {
        unsigned len = strlen(s) + 1; // include NUL
        if( fwrite(&len, sizeof(len), 1, fp) != 1 ) return 0;
        if( fwrite(s, len, 1, fp) != 1 )            return 0;
    } else {
        unsigned len = 0;
        if( fwrite(&len, sizeof(len), 1, fp) != 1 ) return 0;
    }
    return 1;
}
bool load_string(char **s, FILE *fp) {
    unsigned len;

    if( fread(&len, sizeof(len), 1, fp) != 1 ) return *s = NULL, 0;
    if( len == 0 ) return *s = NULL, 1;
    if( NULL == (*s = malloc(len)) ) return *s = NULL, 0; // die?
    if( fread(*s, len, 1, fp) != 1 ) return free(*s), *s = NULL, 0;

    return 1;
}

// savegame -------------------------------------------------------------------

FILE *savefp;
FILE *loadfp;

void cancel_savegame(void) {
    if(savefp) fclose(savefp), savefp = 0;
    if(loadfp) fclose(loadfp), loadfp = 0;
}
const char *savename() {
    return ".save";
}
FILE *savegame() {
    ONCE atexit(cancel_savegame);

    if( loadfp ) fclose(loadfp), loadfp = 0;
    if( !savefp ) savefp = fopen(savename(), "wb");
    return savefp;
}
FILE *loadgame() {
    if( savefp ) fclose(savefp), savefp = 0;
    if( !loadfp ) loadfp = fopen(savename(), "rb");
    return loadfp;
}    

#if 0
// ----------------------------------------------------------------------------
// demo

//#pragma pack(push,1)
typedef struct {
    int16_t    id;
    const char *name;   // the pointer we need to patch
    unsigned char age;
    float  height;
} Person;
//#pragma pack(pop)

// SERIALIZE
bool save_Person(const Person *p, FILE *fp) {
    // write whole struct
    if( !save_struct(p, sizeof(*p), fp) ) return 0;

    // write all variable-length members now (pointers, arrays, maps...)
    if( !save_string(p->name, fp) ) return 0;

    return 1;
}

// DESERIALIZE + PATCH
bool load_Person(Person *p, FILE *fp) {
    // read whole struct
    if( !load_struct(&p, sizeof(*p), fp) ) return 0;

    // The pointer p->name is garbage right now -> we will patch it

    // read all variable-length members now (pointers, arrays, maps...)
    if( !load_string(&p->name, fp)) return 0;

    return 1;
}

#endif

#endif

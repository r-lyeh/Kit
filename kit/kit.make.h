// @todo: ensure C old pointers can be passed to new pointers api. eg, count(malloc(1)); use canary at [-1]

#define      make(type, num) make3((num), sizeof(type), #type)
void*        make3(unsigned num, unsigned sz, const char *type);
void          resize(void *vptr, unsigned num);
unsigned       bytes(void *ptr);
unsigned       count(void *ptr);
const char*    type(void *ptr);
void         drop(void *vptr);

#if KIT_CODE
#pragma once

#pragma pack(push,1)
typedef struct objhdr {
    uintptr_t num : 32, sz: 32;
    union {
    uintptr_t    typeid;
    const char * type;
    };
} objhdr;
typedef int objhdr_check[ sizeof(objhdr) == 16 ? 1 : -1];
#pragma pack(pop)

void* make3(unsigned num, unsigned sz, const char *type) { num += !num;
    objhdr *ptr = zrealloc(0, sizeof(objhdr) + num * sz);
    ptr->num = num;
    ptr->sz = sz;
    ptr->type = stable(type);
    return (char*)ptr + sizeof(objhdr);
}
void resize(void *vptr, unsigned num) { void **ptr = vptr; // note: silenced gcc warnings: void **ptr > void *vptr
    if(!ptr || !*ptr) return;
    objhdr *hdr = (objhdr*)*ptr - 1;
    hdr = zrealloc(hdr, sizeof(objhdr) + hdr->sz * (hdr->num = num));
    *ptr = (char*)hdr + sizeof(objhdr);
}
void drop(void *vptr) { void **ptr = vptr; // note: silenced gcc warnings: void **ptr > void *vptr
    if(!ptr || !*ptr) return;
    objhdr *hdr = (objhdr*)*ptr - 1;
    *ptr = zrealloc(hdr, 0);
}
unsigned bytes(void *ptr) {
    if(!ptr) return 0;
    objhdr *hdr = (objhdr*)ptr - 1;
    return hdr->num * hdr->sz;
}
unsigned count(void *ptr) {
    if(!ptr) return 0;
    objhdr *hdr = (objhdr*)ptr - 1;
    return hdr->num;
}
const char* type(void *ptr) {
    if(!ptr) { static const char *stable_null = 0; if(!stable_null) stable_null = stable("null"); return stable_null; }
    objhdr *hdr = (objhdr*)ptr - 1;
    return hdr->type;
}

AUTOTEST {
    char *str = make(wchar_t, 16);
    test(str);
    test(str[0] == 0);
    strcpy(str, "hello");
    test(!strcmp(str,"hello"));
    test(!strcmp(type(str),"wchar_t"));
    test(count(str)==16);
    test(bytes(str)==16*sizeof(wchar_t));
    resize(&str,100);
    test(count(str)==100);
    test(bytes(str)==100*sizeof(wchar_t));
    drop(&str);
    test(str == 0);
}

#endif

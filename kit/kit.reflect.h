// @todo: append rtti -> gametree (registry)
// @todo: union {a,b,c,d;}  > should only load/save one. verify me
// @todo: META() int a,b,c; > a+b+c should use same meta. verify me

#ifndef REFLECT_H
#define REFLECT_H

// api: inscribe

#define META(...)
#define REFLECT(...) __VA_ARGS__; AUTORUN { void reflect(const char*); reflect(#__VA_ARGS__); }

// api: describe

unsigned    size_of(const char *type);
unsigned  offset_of(const char *type);
const char *name_of(const char *type);
const char *type_of(const char *type);
unsigned    members_of(const char *type);
struct rtti* member_at(const char *type, unsigned index);

// api: iterate

typedef struct rtti {
    const char *file;
    const char *name;
    const char *type;
    const char *meta;
    const char *value;
    const char *text; char *copy;
    array_(struct rtti) child;
    unsigned size;
    unsigned offset:23, bits:5, is_const:1, is_union:1, is_ptr:1, is_vla:1; //< @todo: bits,vla
    unsigned line; //< @todo
} rtti;

#define each_member(type, member) \
    (rtti *R_ = reflected(type);R_;R_=0) \
        for(int i_ = 0, ii_ = array_count(R_->child); i_ < ii_; ++i_ ) \
            for(rtti *Q_ = &R_->child[i_], member = *Q_;Q_;Q_=0)

// internal utils

rtti* reflected(const char *type);
char* reflected_datas(const char *type);

#elif KIT_CODE
#pragma once

// ----------------------------------------------------------------------------
// reflection impl

static map_(const char*, char*) g_reflect = {less,hash};

void reflect(const char *doc) {
    char *simpler_typedef(const char *);
    char *c = strdup(simpler_typedef(doc));
    char name[64]; sscanf(c,"%63s",name);
    for( const char *label = stable(name); label && !map_find(g_reflect,label); label = 0 ) {
        map_add(g_reflect,label,c);
        reflected(name); // force a init
    }
}
char *reflected_datas(const char *type) {
    char** found = map_find(g_reflect, stable(type));
    return found ? *found : NULL;
}

// ----------------------------------------------------------------------------
// C parser impl

// transform c typedef union/structs/enums grammar into something else, which is simpler to parse
// the new output syntax has some interesting properties:
// - structs, unions and enums are all defined similarly
// - struct is declared by reading lines until final ; is found
// - num lines == num members
// - each line is formatted as `[*]name[: [bits]] [union|struct|enum] [type...] [META(...)] \n`
// - each word can be read with a `%s` scanf because inner spaces are `@`s: `META("i@am@meta")`, `array_(const@char@*)`, etc
// - one-word lines are members that share properties from previous lines
// - pointer asterisks `*` are glued to variable names, never to types
// - bitfield separators `:` are glued to variable names, then number of bits follow
const char *findopen(const char *s, const char *digits) { // find digit within string. returns EOS if no digits are found
    return s+strcspn(s, digits);
}
const char *findclose(const char *s) { // find closing digit within string. ignores new occurences of other [{(enclosing)}] sequences, if any. returns EOS if no closing patterns are found
    int cnt = 0, tbl[] = {['(']=1,[')']=-1,['{']=3,['}']=-3,['[']=5,[']']=-5};
    do {
        cnt += tbl[*s]; // printf("\n%d %s\n", cnt, s);
        if(!cnt) break;
        s = findopen(s+1, "{}()[]");
    } while( cnt && *s );
    return s;
}
char *simpler_typedef(const char *PP_struct_) {
    char *s = va("%s", PP_struct_);
    
    // - trim typedefs
    strswap(s,"typedef ","        ");
    // - commas as semi-colons
    strswap(s,",",";");
    // - colons as spaces
    strswap(s,":"," ");
    // - *+space as space+*
    // - space+colon as colon+space
    for( int i = 0; s[i]; ++i ) {
        if(s[i] == '*' && s[i+1] == ' ' ) s[i]=' ',s[i+1]='*';
        else
        if(s[i] == ' ' && s[i+1] == ':' ) s[i]=':',s[i+1]=' ';
    }
    // - initial `{}` scope as `; `
    {
        const char *open = findopen(s, "{"), *close = findclose(open);
        if(open[0] && close[0]) {

            *(char*)open = ';', *(char*)close = ' ';

            // handle TYPEDEF STRUCT { ... } NAME; case. changes to TYPEDEF STRUCT NAME { ... };
            // TYPEDEF STRUCT NAME { ... } NAME > TYPEDEF STRUCT NAME NAME { ... }; case is handled at exit
            char *copy = va("%s", close+1);
            int l = strlen(copy);
            memmove((char*)open+l,(char*)open,strlen(open)-l);
            memcpy((char*)open, (char*)copy, l);
        }
    }
    // - `{}` as `  `, unless there are other inner {} blocks within
    {
        repeat:;
        const char *open = findopen(s, "{"), *close = findclose(open);
        if( open[0] && close[0] ) {
            const char *next = strchr(open+1, '{');
            if( !next || next > close ) { *(char*)open = *(char*)close = ' '; goto repeat; }
        }
    }
    // - ` ` as `@`, within META() and array_() strings
    {
        const char *z = s;
        repeat2:;
        const char *open = findopen(z, "("), *close = findclose(open);
        if( open[0] && close[0] ) {
            while( open++ < close ) if(open[-1]==' ') ((char*)open)[-1] = '@';
            z = close;
            goto repeat2;
        }
    }

    static array_(char) out = 0; array_resize(out, 0);

    // final transforms and prettify
    for each_string_ex(line, s, ";") {
        line += strspn(line, " ");
        if(!line[0]) continue;
        char *is_enum = strchr(line, '='); if(is_enum) is_enum[0] = ' '; // remove '=' in enums
        static array_(const char *) words = 0; array_resize(words, 0);
        for each_string_ex(token, line, " ") array_push(words, token);
#if 1
        if( !is_enum ) { // if !enum line (union/struct line)
        // move typename to home (TYPEDEF STRUCT NAME > NAME TYPEDEF STRUCT)
        const char *end = *array_back(words);
        for( int i = array_count(words); i > 1; --i ) words[i-1] = words[i-2]; words[0] = end;
        // remove redundant typenames (NAME TYPEDEF STRUCT NAME > NAME TYPEDEF STRUCT)
        if( array_count(words) > 1 && !strcmp(words[0], *array_back(words)) ) array_pop(words);
        // repeat if bitfield ((TYPE FIELD: NUM >) NUM TYPE FIELD: > FIELD: NUM TYPE)
        if( atoi(words[0]) ) { end = *array_back(words);
        for( int i = array_count(words); i > 1; --i ) words[i-1] = words[i-2]; words[0] = end; }
        }
#endif
        // move METAs to end
        if( strstr(line, "META(") )
        for( int i = 0, j = array_count(words); i < j; ++i ) {
            if( strncmp(words[i], "META(", 5) ) continue;
            const char *meta = words[i];
            while( ++i < j ) words[i-1] = words[i]; *array_back(words) = meta;
            break;
        }
        // save result as string
        for( int i = 0, ii = array_count(words); i < ii; ) { 
            array_concat(out, words[i]);
            int trim_space = strcmp(words[i], "signed") && strcmp(words[i], "unsigned") && strcmp(words[i], "long");
            if(++i < ii) array_concat(out, trim_space ? " " : "@");
        }
        array_concat(out, "\n");
    }
    array_concat(out, ";\n");

    // puts(PP_struct_); puts(""); puts(out);
    return out;
}

// ----------------------------------------------------------------------------
// rtti impl

static map_(const char*, rtti) g_rtti = {less,hash}; //< rename to registry?

static rtti* R_search(const char *name, rtti *R) {
    if( !R ) {
        for( int i = 0, ii = array_count(g_rtti.values); i < ii; ++i ) {
            R = R_search(name, &g_rtti.values[i]);
            if( R ) return R;
        }
        return NULL;
    }
    unsigned span = strcspn(name, ":.->/@");
    if( !strncmp(R->name, name, span) && R->name[span] == 0 ) {
        if( name[span] == 0 )
            return R;

        name = name + span + strspn(name + span, ":.->/@");

        for( int i = 0, ii = array_count(R->child); i < ii; ++i ) {
            rtti *S = R_search(name, &R->child[i]);
            if( S ) return S;
        }
    }
    return NULL;
}
static void R_printf(rtti *r) { //  @todo: value, bits
    printf("[%3d +%3d] %s%s%30s%c %10s; // %s\n", r->offset, r->size, r->is_const?"const":"     ", r->is_union?"union":"     ", r->type, " *"[r->is_ptr], r->name, r->meta);
    for( int i = 0; i < array_count(r->child); ++i )
        R_printf(&r->child[i]);
}

const char *name_of(const char *type) { rtti *R = reflected(type); return R ? R->name : ""; }
const char *type_of(const char *type) { rtti *R = reflected(type); return R ? R->type : ""; }
unsigned    size_of(const char *type) { rtti *R = reflected(type); return R ? R->size : 0; }
unsigned  offset_of(const char *type) { rtti *R = reflected(type); return R ? R->offset : 0; }
unsigned members_of(const char *type) { rtti *R = reflected(type); return R ? array_count(R->child) : 0; }
rtti* member_at(const char *type, unsigned at) { rtti *R = reflected(type); return R ? &R->child[at] : NULL; }
rtti* reflected(const char *type) {
    type = stable(type);
    rtti* found = 0;
    if(!found) { // quicksearch by hash
        found = map_find(g_rtti, type);
    }
    if(!found) { // recursive type->type->type search
        found = R_search(type, NULL);
    }
    if(!found) { // never registered? try to register type from reflected data
        char *refl = reflected_datas(type);
        if(!refl) return 0;

        rtti R = {"","","","","","",""}, P = R;
        R.text = refl;
        R.copy = strdup(refl);

        int loaded = 0;
        for each_line(line, refl) {
            if( line[0] == ';' ) break;

            rtti C = {"","","","","","",""};
            if(!loaded) C = R;

            char *found = strstr(line, "META(");
            if(found) found[-1] = '\0', C.meta = (char*)stable(found);

            int is_enum = 0;
            static array_(const char*) tokens = 0; array_resize(tokens, 0);
            for each_word(token, line) {
                if(!C.bits ) if( 0 != (C.bits = atoi(token))) continue;
                /**/ if(!strcmp(token, "union")) C.is_union = 1;
                else if(!strcmp(token, "const")) C.is_const = 1;
                else if(!strcmp(token, "enum"))    is_enum = 1;
                else array_push(tokens, token);
            }

            int num = array_count(tokens);
            if( num ) {
                C.is_ptr = tokens[0][0] == '*';
                tokens[0] += strspn(tokens[0], "*");

                if( num == 1 ) C = P, C.name = stable(tokens[0]);
                if( num == 2 ) C.name = stable(tokens[0]), C.type = stable(tokens[1]), P = C;
                if( num  > 2 ) C.name = stable(tokens[0]), C.type = stable(tokens[1]), P = C;
            }
            
            if( !loaded++ ) R = C;
            else {
                // try to guess sizes. this is hacky, but we use very few types
                /**/ if( C.is_ptr ) C.size = sizeof(void*);
                else if( !C.type ) ; // assert?
                else if( strstr(C.type, "array_(") ) C.size = sizeof(void*);
                // else if( strstr(C.type, "map_(") ) C.size = sizeof(void*); // nope :(
                else if( strstr(C.type, "64") ) C.size = 8;
                else if( strstr(C.type, "32") ) C.size = 4;
                else if( strstr(C.type, "16") ) C.size = 2;
                else if( strstr(C.type, "8")  ) C.size = 1;
                else if( strstr(C.type, "double") ) C.size = 8;
                else if( strstr(C.type, "float")  ) C.size = 4;
                else if( strstr(C.type, "short")  ) C.size = 2;
                else if( strstr(C.type, "char")   ) C.size = 1;
                else if( strstr(C.type, "long long") ) C.size = sizeof(long long);
                else if( strstr(C.type, "unsigned") )  C.size = 4;
                else if( strstr(C.type, "int") )       C.size = 4;
                else if( strstr(C.type, "_Bool") )     C.size = sizeof(_Bool);
                else if( strstr(C.type, "bool") )      C.size = sizeof(bool);
                else ; // assert?

                if( array_count(R.child) )
                {
                    C.offset = array_back(R.child)->offset + array_back(R.child)->size;


                    unsigned o = C.offset, s = C.size;
                    if( strstr(R.meta, "pack") ? 0 : 1 ) //< ignore padded fields if pragma pack, 1
                    if( (o % (s+!s)) ) C.offset += s - (o % (s+!s));
                }

                array_push(R.child, C);
            }
        }

        rtti *L = array_back(R.child);
        if( L ) R.size = L->offset + L->size;

        found = map_find_or_add(g_rtti, type, R);
    }
    return found;
}

// ----------------------------------------------------------------------------
// parser tests

AUTOTEST {
    #define TEST_PARSER(output, ...) do { if(!test(0 == strcmp(output,simpler_typedef(#__VA_ARGS__)))) hexdump(output),puts(""),hexdump(simpler_typedef(#__VA_ARGS__)),puts("\n\n"); } while(0)

    // define same struct in different ways

    TEST_PARSER("empty struct\ndummy int16_t\n;\n",
        struct empty {
            int16_t dummy;
        };
    );

    TEST_PARSER("empty struct\ndummy int16_t\n;\n",
        typedef struct {
            int16_t dummy;
        } empty;
    );

    TEST_PARSER("empty struct\ndummy int16_t\n;\n",
        typedef struct empty {
            int16_t dummy;
        } empty;
    );

    TEST_PARSER("empty struct\ndummy int16_t\n;\n",
        typedef struct empty {
            int16_t dummy;
        };
    );

    TEST_PARSER("a struct empty\ndummy int16_t\n;\n", //< @fixme: beware, empty/a order swapped
        struct empty {
            int16_t dummy;
        } a;
    );

    TEST_PARSER("a struct empty\nb\ndummy int16_t\n;\n", //< @fixme: beware, empty/a order swapped
        struct empty {
            int16_t dummy;
        } a,b;
    );

    // enum inference

    TEST_PARSER("enum\nONE 1\nZERO ONE-ONE\nTWO ZERO+1+ONE META(\"two@better@than@one\")\nTHREE\n;\n",
        enum {
            ONE = 1,
            ZERO= ONE-ONE,
            META("two better than one")
            TWO =ZERO+1+ONE,
            THREE
        };
    );

    // typedef struct sample with annotations

    TEST_PARSER("Person struct META(\"packed@struct\")\n"
        "id int16_t META(\"my@national@\"\"id\"\"@number\")\n"
        "*name const char META(\"person's@name\")\n"
        "age unsigned@char META(\"age\")\n"
        "height float META(\"height@(meters)\")\n"
        "children array_(struct@Person) META(\"descendants\")\n"
        ";\n",
        META("packed struct")
        typedef struct {
            META("my national ""id"" number")
            int16_t id;

            META("person's name")
            const char *name;

            META("age")
            unsigned char age;

            META("height (meters)")
            float height;

            META("descendants")
            array_(struct Person) children;
        } Person;
    );

    // complex case

    TEST_PARSER(
        "options_t struct META()\n"
        "bitfield 13 int\n"
        "other 1\n"
        "*closetotype const char\n"
        "*closetovar\n"
        "*separated\n"
        "w union int\n"
        "width\n"
        "h union int\n"
        "height\n"
        "s union float META(\"scale@test\")\n"
        "scale\n"
        "a union float\n"
        "aspect\n"
        "multi union char\n"
        "line\n"
        "visible bool\n"
        "minimized\n"
        "rotated\n"
        "transparent bool\n"
        "resizable\n"
        "decorated\n"
        "debug bool\n"
        "silent\n"
        "aa union unsigned\n"
        "msaa\n"
        "samples\n;\n",
        META()
        typedef struct  options_t{
            int bitfield : 13, other : 1;
            const char*   closetotype, *closetovar, * separated;
            union { int w, width; } ;
            union { int h, height; };
            META("scale test")
            union { float s, scale; };
            union { float a, aspect; };
            union { char multi,
                    line};
            bool visible,minimized,rotated;
            bool transparent, resizable, decorated;
            bool debug, silent; // gl: debug context and whether to report errors or not
            union { unsigned aa, msaa, samples; }; //gl
            // icon, icon_small
            // good ideas? fullscreen, depth:32/24, stencil:8, srgb
        }
        options_t;
    );

    #undef TEST_PARSER
}

// ----------------------------------------------------------------------------
// rtti tests

#if defined TEST || defined TESTS

// enum sample without annotations

REFLECT(
    enum {
        ONE = 1,
        ZERO = ONE-ONE,
        TWO = ZERO+2,
        THREE
    };
)

// struct sample without annotations

REFLECT(
    typedef struct {
        int16_t id;
        const char *name;
        unsigned char age;
        float height;
        array_(struct Person) children;
    } Person;
)

// sample with pragma pack and annotations

#pragma pack(push,1)

REFLECT(
    META("packed struct")
    typedef struct {
        META("my national ""id"" number")
        int16_t id;

        META("person's name")
        const char *name;

        META("age")
        unsigned char age;

        META("height (meters)")
        float height;

        META("descendants")
        array_(struct Person) children;
    } Person2;
)

REFLECT(
typedef struct  window_t {
    unsigned bitfield:13, other: 10;
    const char *title, *extra;
    union { int w, width; };
    union { int h, height; };
    META("scale test")
    union { float s, scale; };
    union { float a, aspect; };
    bool visible, minimized, rotated;
    bool transparent, resizable, decorated;
    bool debug, silent; // gl: debug context and whether to report errors or not
    union { unsigned aa, msaa, samples; }; //gl
    // icon, icon_small
    // good ideas? fullscreen, depth:32/24, stencil:8, srgb
} window_t;
);

#pragma pack(pop)

AUTOTEST {
    Person2 p = {123456, "Alice, the serializer", 34, 1.66};
    //Person2 *p2 = make("(Person2){123456,\"Alice, the Serializer\",34,1.66}"); // @todo

    test( sizeof(Person) == size_of("Person") );
    test( sizeof(Person2) == size_of("Person2") );
    test( sizeof(p.age) == size_of("Person2.age") );
    test( sizeof(p.age) == size_of("Person2/age") );
    test( sizeof(p.age) == size_of("Person2@age") );
    test( sizeof(p.age) == size_of("Person2::age") );
    test( sizeof(p.age) == size_of("Person2->age") );
    test( p.age == ((char*)&p)[offset_of("Person2.age")] );

    for each_member("Person2", R) {
        printf("[%2d+%2d] %25s%c %10s; // %s \n", R.offset,R.size, R.type, " *"[R.is_ptr], R.name, R.meta);
    }
}

#if 0 // @fixme
REFLECT (
    union flags {
        struct {
            bool option1;
            bool option2;
        };
        unsigned mask;
    };
)
#endif

#endif // TESTS

#endif // KIT_CODE

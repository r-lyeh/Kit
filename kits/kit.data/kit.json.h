#include "3rd_json5.h"

typedef union json_t { char* s; double f; int64_t i; uintptr_t p; array_(union json_t) arr; } json_t;

static THREAD array_(json5) roots;
static THREAD array_(char*) sources;

bool json_push(const char *source) {
    char *source_rw = SDL_strdup(source);
    json5 root = {0};
    char *error = json5_parse(&root, source_rw, 0);
    if( error ) {
        SDL_free(source_rw);
        return false;
    } else {
        array_push(sources, source_rw);
        array_push(roots, root);
        return true;
    }
}
bool json_pushf(const char *pathfile) {
    return json_push(file.read(pathfile, NULL));
}
bool json_pop(void) {
    if( array_count(roots) > 0 ) {
        SDL_free(*array_back(sources));
        array_pop(sources);

        json5_free(array_back(roots));
        array_pop(roots);
        return true;
    }
    return false;
}

void* json_node(const char *pathkey) {
    json5 *j = array_back(roots), *r = j;
    for each_string( key, pathkey, "/[.]" ) {
        r = 0;
        /**/ if( j->type == JSON5_ARRAY ) r = j = &j->array[atoi(key)];
        else if( j->type == JSON5_OBJECT && SDL_isdigit(key[0]) )
        for( int i = 0, seq = SDL_atoi(key); !r && i < j->count; ++i ) {
            if( i == seq ) {
                r = j = &j->nodes[i];
                break;
            }
        }
        else if( j->type == JSON5_OBJECT )
        for( int i = 0; !r && i < j->count; ++i ) {
            if( j->nodes[i].name && !SDL_strcmp(j->nodes[i].name, key) ) {
                r = j = &j->nodes[i];
                break;
            }
        }
        if( !j ) break;
    }
    return r;
}
const char *json_key(const char *pathkey) {
    json5 *j = (json5*)json_node(pathkey);
    if( !j ) return "";
    if( !j->name ) return "";
    return j->name;
}

static
json_t json_get(char type, const char *pathkey) {
    json5 *j = (json5*)json_node(pathkey);

    json_t v = {0};
    v.i = j ? j->integer : 0;
    if(type == 's' && (!v.p || j->type == JSON5_NULL)) v.s = ""; // if_null_string
    if(type == 'f' && j && j->type == JSON5_INTEGER) v.f = j->integer;
    return v;
}
int json_integer(const char *pathkey) {
    return json_get('i', pathkey).i;
}
double json_floating(const char *pathkey) {
    return json_get('f', pathkey).f;
}
char *json_string(const char *pathkey) {
    return json_get('s', pathkey).s;
}
int json_count(const char *pathkey) {
    json5* j = (json5*)json_node(pathkey);
    return j ? j->count : 0;
}

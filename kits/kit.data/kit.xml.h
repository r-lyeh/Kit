// xml impl

#include "3rd_xml.h"

static THREAD array_(char *) xml_sources;
static THREAD array_(struct xml *) xml_docs;

int xml_push(const char *xml_source) {
    if( xml_source ) {
        char *src = SDL_strdup(xml_source), *error = 0;
        for( struct xml *doc = xml_parse(src, 0, &error); doc && !error; ) {
            array_push(xml_docs, doc);
            array_push(xml_sources, src);
            return 1;
        }
        if( error ) fprintf(stderr, "%s\n", error);
        SDL_free(src);
    }
    return 0;
}
int xml_pushf(const char *pathfile) {
    return xml_push(file.read(pathfile, NULL));
}

void xml_pop() {
    if( array_count(xml_docs) ) {
        xml_free( *array_back(xml_docs) );
        array_pop(xml_docs);
        SDL_free( *array_back(xml_sources) );
        array_pop(xml_sources);
    }
}

static void *xml_path(struct xml *node, const char *path, int down) {
    if( !path || !path[0] ) return node;
    if( node ) {

        char type = path[0];
        if( type == '/' ) {
            int sep = /*SDL_*/strcspn(++path, "/[@$");
            if( !sep ) type = path[0];
            else
            if( 1 ) { // path[ sep ] ) {
                char tag[32]; SDL_snprintf(tag, 32, "%.*s", sep, path);
                // Find the first sibling with the given tag name (may be the same node)
                struct xml *next = down ? xml_find_down(node, tag) : xml_find(node, tag);
                return xml_path(next, &path[ sep ], 1);
            }
        }
        if( type == '$' ) {
            return (void*)( node->down ? xml_text( node->down ) : xml_tag( node ) );
        }
        if( type == '@' ) {
            return (void*)xml_att(node, ++path);
        }
        if( type == '[' ) {
            for( int i = 0, end = atoi(++path); i < end; ++i ) { node = xml_find_next(node, xml_tag(node)); if(!node) return NULL; }
            while( SDL_isdigit(path[0]) ) ++path;
            return xml_path(node, ++path, 1);
        }
    }
    return NULL;
}

unsigned xml_count(const char *key) {
    struct xml *node = xml_path(*array_back(xml_docs), key, 0);
    if( !node ) return 0;
    const char *tag = xml_tag(node);
    unsigned count = 1;
    while( (node = xml_find_next(node, tag)) != 0) ++count;
    return count;
}
const char *xml_string(const char *key) {
    struct xml *node = xml_path(*array_back(xml_docs), key, 0);
    if( node && SDL_strchr(key, '@') ) return (const char *)node;
    if( node && SDL_strchr(key, '$') ) return (const char *)node;
    return "";
}
int xml_integer(const char *key) {
    return SDL_atoi(xml_string(key));
}
double xml_floating(const char *key) {
    return SDL_atof(xml_string(key));
}
#if 0
array_(char) (xml_base64)(char *key) { // base64 blob
    struct xml *node = xml_path(*array_back(xml_docs), key, 0);
    if( !node ) return 0;
    if( !SDL_strchr(key, '$') ) return 0;
    const char *data = (const char*)node;
    array_(char) out = base64_decode(data, strlen(data)); // either array of chars (ok) or null (error)
    return out;
}
#endif

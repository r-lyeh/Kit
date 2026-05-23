#ifndef KIT_DATA_H
#define KIT_DATA_H "0.0.0"

// ----------------------------------------------------------------------------
// json api

extern struct json_api {
bool        (*push)(const char *source);
bool        (*pushf)(const char *pathfile);
int             (*integer)(const char *pathkey);
double          (*floating)(const char *pathkey);
char*           (*string)(const char *pathkey);
int             (*count)(const char *pathkey);
void*           (*node)(const char *pathkey);
bool        (*pop)(void);
} json;

// -----------------------------------------------------------------------------
// xml api

extern struct xml_api {
int         (*push)(const char *xml_contents);
int         (*pushf)(const char *xml_pathfile);
int             (*integer)(const char *key);
double          (*floating)(const char *key);
const char*     (*string)(const char *key);
unsigned        (*count)(const char *key);
//array_(char)  (*base64)(char *key);
void        (*pop)();
} xml;

#elif KIT_CODE
#pragma once

#include "kit.json.h"
#include "kit.xml.h"

struct json_api json = {
.push = json_push,
.pushf = json_pushf,
.integer = json_integer,
.floating = json_floating,
.string = json_string,
.count = json_count,
.node = json_node,
.pop = json_pop,
};

struct xml_api xml = {
.push = xml_push,
.pushf = xml_pushf,
.integer = xml_integer,
.floating = xml_floating,
.string = xml_string,
.count = xml_count,
//.base64 = xml_base64,
.pop = xml_pop,
};

#if TEST

AUTOTEST {
    const char json5doc[] =
    "  /* json5 */ // comment\n"
    "  abc: 42.67, def: true, integer:0x100 \n"
    "  huge: 2.2239333e5, \n"
    "  hello: 'world /*comment in string*/ //again', \n"
    "  children : { a: 1, b: 2, c: 3 },\n"
    "  array: [+1,2,-3,4,5],    \n"
    "  invalids : [ nan, NaN, -nan, -NaN, inf, Infinity, -inf, -Infinity ],";
    if( json.push(json5doc) ) {
        test( json.floating("/abc") == 42.67 );
        test( json.integer("/def") == 1 );
        test( json.integer("/integer") == 0x100 );
        test( json.floating("/huge") > 2.22e5 );
        test( strlen(json.string("/hello")) == 35 );
        test( json.integer("/children/a") == 1 );
        test( json.integer("/children.b") == 2 );
        test( json.integer("/children[c]") == 3 );
        test( json.integer(va("/array[%d]", 2)) == -3 );
        test( json.count("/invalids") == 8 );
        test( isnan(json.floating("/invalids[0]")) );
        test( !json.node("/non_existing") );
        json.pop();
    }
}

AUTOTEST {
    const char *xmldoc =
    "<!-- XML representation of a person record -->"
    "<person created=\"2006-11-11T19:23\" modified=\"2006-12-31T23:59\">"
    "    <firstName>Robert</firstName>"
    "    <lastName>Smith</lastName>"
    "    <address type=\"home\">"
    "        <street>12345 Sixth Ave</street>"
    "        <city>Anytown</city>"
    "        <state>CA</state>"
    "        <postalCode>98765-4321</postalCode>"
    "    </address>"
    "</person>";
    if( xml.push(xmldoc) ) {
        test( strcmp("Robert", xml.string("/person/firstName/$")) == 0 );
        test( strcmp("Smith", xml.string("/person/lastName/$")) == 0 );
        test( strcmp("home", xml.string("/person/address/@type")) == 0 );
        xml.pop();
    }
}

#endif // TEST
#endif // CODE

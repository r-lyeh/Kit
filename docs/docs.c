// docgen.c - API doc generator for kit.h
// - rlyeh, public domain
//
// usage:
// cl docgen.c && docgen < ../kit.h > kit.md.html
// gcc docgen.c -o docgen && ./docgen < ../kit.h > kit.md.html

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// ---------------------------------------------------------------------------
// helpers

static char *trim(char *s) {
    while (*s && isspace((unsigned char)*s)) s++;
    char *e = s + strlen(s);
    while (e > s && isspace((unsigned char)e[-1])) *--e = 0;
    return s;
}

static int startswith(const char *s, const char *prefix) {
    return strncmp(s, prefix, strlen(prefix)) == 0;
}

// strip leading // or /// from a comment-only line; returns ptr into s or NULL
static const char *strip_comment(const char *s) {
    s = s + strspn(s, " \t");
    if (startswith(s, "///")) return s + 3 + (s[3] == ' ');
    if (startswith(s, "//"))  return s + 2 + (s[2] == ' ');
    return NULL;
}

// extract inline trailing comment after the last //
static const char *trailing_comment(const char *line) {
    const char *p = strstr(line, "//");
    if (!p) return "";
    p += 2;
    while (*p == ' ') p++;
    // strip trailing newline
    static char buf[512];
    snprintf(buf, sizeof buf, "%s", p);
    char *e = buf + strlen(buf);
    while (e > buf && (*e == 0 || *e == '\n' || *e == '\r')) *e-- = 0;
    return buf;
}

// extract function pointer member name from "rettype (*name)(args);"
static int extract_fn_name(const char *line, char *out, size_t outsz) {
    const char *p = strstr(line, "(*");
    if (!p) return 0;
    p += 2;
    const char *end = p;
    while (*end && (isalnum((unsigned char)*end) || *end == '_')) end++;
    size_t len = end - p;
    if (!len || len >= outsz) return 0;
    memcpy(out, p, len);
    out[len] = 0;
    return 1;
}

// extract plain field name (last word before ';', stripping '*')
static int extract_field_name(const char *line, char *out, size_t outsz) {
    char buf[512];
    snprintf(buf, sizeof buf, "%s", line);
    char *sc = strrchr(buf, ';'); if (sc) *sc = 0;
    char *cm = strstr(buf, "//"); if (cm) *cm = 0;
    char *t  = trim(buf);
    char *sp = strrchr(t, ' ');
    char *fn = sp ? sp + 1 : t;
    while (*fn == '*') fn++;
    size_t len = strlen(fn);
    if (!len || len >= outsz) return 0;
    memcpy(out, fn, len + 1);
    return 1;
}

// ---------------------------------------------------------------------------
// html escaping

static const char *html_escape(const char *s) {
    static char buf[1024];
    char *d = buf;
    char *end = buf + sizeof buf - 1;
    while (*s && d < end) {
        if (*s == '<' && d + 4 <= end) {
            memcpy(d, "&lt;", 4); d += 4;
        } else if (*s == '>' && d + 4 <= end) {
            memcpy(d, "&gt;", 4); d += 4;
        } else if (*s == '&' && d + 5 <= end) {
            memcpy(d, "&amp;", 5); d += 5;
        } else {
            *d++ = *s;
        }
        s++;
    }
    *d = 0;
    return buf;
}

// ---------------------------------------------------------------------------
// markdeep boilerplate

static void emit_header(void) {
//  puts("<meta charset='utf-8' emacsmode='-*- markdown -*-'>");
//  puts("<link rel='stylesheet' href='https://casual-effects.com/markdeep/latest/apidoc.css?'>");
//  puts("");
}

static void emit_footer(void) {
    puts("");
    puts("<script src='https://casual-effects.com/markdeep/latest/markdeep.min.js' charset='utf-8'></script>");
}

// ---------------------------------------------------------------------------
// main parser

#define MAX_LINES 8192
#define MAX_LEN   1024

int main(void) {
    static char lines[MAX_LINES][MAX_LEN];
    int nlines = 0;
    while (nlines < MAX_LINES && fgets(lines[nlines], MAX_LEN, stdin))
        nlines++;

    emit_header();
    printf("**kit** - API reference\n\n");
    printf("Auto-generated from `kit.h`.\n\n");

    int  in_module   = 0;
    int  first       = 1;
    char modname[64] = {0};

    for (int i = 0; i < nlines; i++) {
        char line[MAX_LEN];
        snprintf(line, sizeof line, "%s", lines[i]);
        char *t = trim(line);

        // ----------------------------------------------------------------
        // outside a module: detect "extern struct <name> {"
        if (!in_module) {
            if (startswith(t, "extern struct ")) {
                const char *p = t + strlen("extern struct ");
                char tmp[64] = {0};
                int j = 0;
                while (*p && (isalnum((unsigned char)*p) || *p == '_') && j < 63)
                    tmp[j++] = *p++;
                while (*p && isspace((unsigned char)*p)) p++;
                if (*p != '{') continue;

                strncpy(modname, tmp, sizeof modname - 1);
                in_module = 1;

                // gather preceding comment block (up to 12 lines back)
                char block[2048] = {0};
                for (int k = i - 1; k >= 0 && k >= i - 12; k--) {
                    char prev[MAX_LEN];
                    snprintf(prev, sizeof prev, "%s", lines[k]);
                    const char *cm = strip_comment(trim(prev));
                    if (!cm) break;
                    // prepend this line to block
                    char tmp2[2048];
                    size_t cml = strlen(cm), bl = strlen(block);
                    if (cml + 1 + bl < sizeof tmp2) {
                        memcpy(tmp2, cm, cml);
                        tmp2[cml] = '\n';
                        memcpy(tmp2 + cml + 1, block, bl + 1);
                        memcpy(block, tmp2, cml + 1 + bl + 1);
                    }
                }

                if (!first) puts("");
                first = 0;

                printf("# `%s`\n\n", modname);

                char *bt = trim(block);
                if (bt[0]) printf("%s\n\n", html_escape(bt));

                printf("Function | Description\n");
                printf("---------|------------\n");
            }
            continue;
        }

        // ----------------------------------------------------------------
        // inside a module

        // closing brace
        if (t[0] == '}') {
            in_module = 0;
            modname[0] = 0;
            puts("");
            continue;
        }

        if (!t[0]) continue;

        // comment-only line → emit as italicised note spanning both columns
        {
            const char *cm = strip_comment(t);
            if (cm) {
                char note[512];
                snprintf(note, sizeof note, "%s", cm);
                char *nt = trim(note);
                if (nt[0])
                    printf("  | *%s*\n", html_escape(nt));
                continue;
            }
        }

        // function pointer member
        char fname[128] = {0};
        if (extract_fn_name(t, fname, sizeof fname)) {
            const char *desc = html_escape(trailing_comment(t));
            printf("`%s.%s` | %s\n", modname, fname, desc);
        }
        // plain field (e.g. SDL_Renderer *handle)
        else if (strchr(t, ';')) {
            char fname2[128] = {0};
            if (extract_field_name(t, fname2, sizeof fname2)) {
                const char *desc = html_escape(trailing_comment(t));
                printf("`%s.%s` | %s\n", modname, fname2, desc);
            }
        }
    }

    emit_footer();
    return 0;
}

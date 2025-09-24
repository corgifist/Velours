#ifndef VELOURS_UTF8_H
#define VELOURS_UTF8_H

#include <stddef.h>
#include <stdint.h>

#define UTF8_IS_ASCII(b)      (((b) & 0x80) == 0)
#define UTF8_IS_LEAD_2(b)     (((b) & 0xE0) == 0xC0)
#define UTF8_IS_LEAD_3(b)     (((b) & 0xF0) == 0xE0)
#define UTF8_IS_LEAD_4(b)     (((b) & 0xF8) == 0xF0)
#define UTF8_IS_CONT(b)       (((b) & 0xC0) == 0x80)

static uint32_t utf8_decode(const char **p, int *out_len) {
    const unsigned char* s = (const unsigned char*)*p;
    uint32_t cp;
    int len = 1;

    if (UTF8_IS_ASCII(s[0])) {
        cp = s[0];
        len = 1;
    } else if (UTF8_IS_LEAD_2(s[0]) && UTF8_IS_CONT(s[1])) {
        cp = ((s[0] & 0x1F) << 6) |
            (s[1] & 0x3F);
        len = 2;
        if (cp < 0x80) cp = 0xFFFD;
    } else if (UTF8_IS_LEAD_3(s[0]) &&
        UTF8_IS_CONT(s[1]) && UTF8_IS_CONT(s[2])) {
        cp = ((s[0] & 0x0F) << 12) |
            ((s[1] & 0x3F) << 6) |
            (s[2] & 0x3F);
        len = 3;
        if (cp < 0x800) cp = 0xFFFD;
        else if (cp >= 0xD800 && cp <= 0xDFFF)
            cp = 0xFFFD;
    } else if (UTF8_IS_LEAD_4(s[0]) &&
        UTF8_IS_CONT(s[1]) && UTF8_IS_CONT(s[2]) && UTF8_IS_CONT(s[3])) {
        cp = ((s[0] & 0x07) << 18) |
            ((s[1] & 0x3F) << 12) |
            ((s[2] & 0x3F) << 6) |
            (s[3] & 0x3F);
        len = 4;
        if (cp < 0x10000 || cp > 0x10FFFF) cp = 0xFFFD;
    } else {
        cp = 0xFFFD;
        len = 1;
    }

    *p = (const char*)(s + len);
    if (out_len) *out_len = len;
    return cp;
}


// important note:
// out must be unsigned char[4] so under any circumstances we would be able to fit in a UTF-8 character
static int utf8_encode(uint32_t cp, unsigned char *out) {
    if (cp > 0x10FFFF ||
        (cp >= 0xD800 && cp <= 0xDFFF) ||
        cp == 0xFFFE || cp == 0xFFFF) {
        cp = 0xFFFD;
    }

    if (cp < 0x80) {
        if (out) out[0] = (unsigned char)cp;
        return 1;
    } else if (cp < 0x800) {
        if (out) {
            out[0] = (unsigned char)(0xC0 | (cp >> 6));
            out[1] = (unsigned char)(0x80 | (cp & 0x3F));
        }
        return 2;
    } else if (cp < 0x10000) {
        if (out) {
            out[0] = (unsigned char)(0xE0 | (cp >> 12));
            out[1] = (unsigned char)(0x80 | ((cp >> 6) & 0x3F));
            out[2] = (unsigned char)(0x80 | (cp & 0x3F));
        }
        return 3;
    } else {
        if (out) {
            out[0] = (unsigned char)(0xF0 | (cp >> 18));
            out[1] = (unsigned char)(0x80 | ((cp >> 12) & 0x3F));
            out[2] = (unsigned char)(0x80 | ((cp >> 6) & 0x3F));
            out[3] = (unsigned char)(0x80 | (0x3F & cp));
        }
        return 4;
    }
}

size_t utf8_strlen(const char* s) {
    size_t count = 0;
    const unsigned char* p = (const unsigned char*)s;

    while (*p) {
        if (UTF8_IS_ASCII(*p)) {
            ++p;
        } else if (UTF8_IS_LEAD_2(*p) &&
            UTF8_IS_CONT(p[1])) {
            p += 2;
        } else if (UTF8_IS_LEAD_3(*p) &&
            UTF8_IS_CONT(p[1]) && UTF8_IS_CONT(p[2])) {
            p += 3;
        } else if (UTF8_IS_LEAD_4(*p) &&
            UTF8_IS_CONT(p[1]) && UTF8_IS_CONT(p[2]) && UTF8_IS_CONT(p[3])) {
            p += 4;
        } else {
            ++p;
        }
        ++count;
    }
    return count;
}

// advances given utf-8 encoded string by given amount of characters
// do not use if you care about performance!
// usage:
//     const char *s = u8"Привет, мир!";
//     printf("%s\n", s); | => "Привет, мир!"
//     utf8_advance(&s, 3);
//     printf("%s\n", s); | => "вет, мир!"
static size_t utf8_advance(const char **s, size_t count) {
    size_t skipped = 0;
    while (count > skipped) {
        utf8_decode(s, NULL);
        skipped++;
    }
    return skipped;
}

static void utf8_dump_codepoint(uint32_t cp) {
    if (cp > 0x10FFFF) cp = 0xFFFD;

    if (cp <= 0xFFFF) printf("U-%04X", cp);
    else printf("U-%06X", cp);
}

#endif

#ifndef VELOURS_UTF8_H
#define VELOURS_UTF8_H

#include <stddef.h>
#include <stdint.h>

#define UTF8_IS_ASCII(b)      (((b) & 0x80) == 0)
#define UTF8_IS_LEAD_2(b)     (((b) & 0xE0) == 0xC0)
#define UTF8_IS_LEAD_3(b)     (((b) & 0xF0) == 0xE0)
#define UTF8_IS_LEAD_4(b)     (((b) & 0xF8) == 0xF0)
#define UTF8_IS_CONT(b)       (((b) & 0xC0) == 0x80)

static uint32_t utf8_decode(const char** p, int *out_len)
{
    const unsigned char* s = (const unsigned char*)*p;
    uint32_t cp;
    int len = 1;

    if (UTF8_IS_ASCII(s[0])) {
        cp = s[0];
        len = 1;
    }
    else if (UTF8_IS_LEAD_2(s[0]) && UTF8_IS_CONT(s[1])) {
        cp = ((s[0] & 0x1F) << 6) |
            (s[1] & 0x3F);
        len = 2;
        if (cp < 0x80) cp = 0xFFFD;
    }
    else if (UTF8_IS_LEAD_3(s[0]) &&
        UTF8_IS_CONT(s[1]) && UTF8_IS_CONT(s[2])) {
        cp = ((s[0] & 0x0F) << 12) |
            ((s[1] & 0x3F) << 6) |
            (s[2] & 0x3F);
        len = 3;
        if (cp < 0x800) cp = 0xFFFD;
        else if (cp >= 0xD800 && cp <= 0xDFFF)
            cp = 0xFFFD;
    }
    else if (UTF8_IS_LEAD_4(s[0]) &&
        UTF8_IS_CONT(s[1]) && UTF8_IS_CONT(s[2]) && UTF8_IS_CONT(s[3])) {
        cp = ((s[0] & 0x07) << 18) |
            ((s[1] & 0x3F) << 12) |
            ((s[2] & 0x3F) << 6) |
            (s[3] & 0x3F);
        len = 4;
        if (cp < 0x10000 || cp > 0x10FFFF) cp = 0xFFFD;
    }
    else {
        cp = 0xFFFD;
        len = 1;
    }

    *p = (const char*)(s + len);
    if (out_len) *out_len = len;
    return cp;
}


size_t utf8_strlen(const char* s)
{
    size_t count = 0;
    const unsigned char* p = (const unsigned char*)s;

    while (*p) {
        if (UTF8_IS_ASCII(*p)) {
            /* 1‑byte (ASCII) */
            ++p;
        }
        else if (UTF8_IS_LEAD_2(*p) &&
            UTF8_IS_CONT(p[1])) {
            /* 2‑byte sequence */
            p += 2;
        }
        else if (UTF8_IS_LEAD_3(*p) &&
            UTF8_IS_CONT(p[1]) && UTF8_IS_CONT(p[2])) {
            /* 3‑byte sequence */
            p += 3;
        }
        else if (UTF8_IS_LEAD_4(*p) &&
            UTF8_IS_CONT(p[1]) && UTF8_IS_CONT(p[2]) && UTF8_IS_CONT(p[3])) {
            /* 4‑byte sequence */
            p += 4;
        }
        else {
            /* Invalid start byte or missing continuation.
               Treat it as a single replacement character. */
            ++p;               /* skip the bad byte */
        }
        ++count;               /* one Unicode scalar value counted */
    }
    return count;
}

#endif

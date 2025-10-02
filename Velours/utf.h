/*
	utf.h - utilities for encoding / decoding utf-8 and utf-16 strings
*/

#ifndef VELOURS_UTF_H
#define VELOURS_UTF_H

#include <stdint.h>

#include "da.h"

typedef char  u8;
typedef uint16_t u16;
typedef uint32_t u32;

#define UTF8_IS_ASCII(b)      (((b) & 0x80) == 0)
#define UTF8_IS_LEAD_2(b)     (((b) & 0xE0) == 0xC0)
#define UTF8_IS_LEAD_3(b)     (((b) & 0xF0) == 0xE0)
#define UTF8_IS_LEAD_4(b)     (((b) & 0xF8) == 0xF0)
#define UTF8_IS_CONT(b)       (((b) & 0xC0) == 0x80)

// decodes one codepoint from utf-8 string
// usage:
//     const u8 *s = u8"Привет!";
//     int len;
//     u32 cp = utf8_decode(&s, &len);
//     printf("%.*s\n", len, s - len); | => "П" 
 static uint32_t utf8_decode(u8** p, int* out_len) {
	u8* s = (u8*)*p;
	u32 u32;
	int len = 1;

	if ((((s[0]) & 0x80) == 0)) {
		u32 = s[0];
		len = 1;
	}
	else if ((((s[0]) & 0xE0) == 0xC0) && (((s[1]) & 0xC0) == 0x80)) {
		u32 = ((s[0] & 0x1F) << 6) |
			(s[1] & 0x3F);
		len = 2;
		if (u32 < 0x80) u32 = 0xFFFD;
	}
	else if ((((s[0]) & 0xF0) == 0xE0) &&
		(((s[1]) & 0xC0) == 0x80) && (((s[2]) & 0xC0) == 0x80)) {
		u32 = ((s[0] & 0x0F) << 12) |
			((s[1] & 0x3F) << 6) |
			(s[2] & 0x3F);
		len = 3;
		if (u32 < 0x800) u32 = 0xFFFD;
		else if (u32 >= 0xD800 && u32 <= 0xDFFF)
			u32 = 0xFFFD;
	}
	else if ((((s[0]) & 0xF8) == 0xF0) &&
		(((s[1]) & 0xC0) == 0x80) && (((s[2]) & 0xC0) == 0x80) && (((s[3]) & 0xC0) == 0x80)) {
		u32 = ((s[0] & 0x07) << 18) |
			((s[1] & 0x3F) << 12) |
			((s[2] & 0x3F) << 6) |
			(s[3] & 0x3F);
		len = 4;
		if (u32 < 0x10000 || u32 > 0x10FFFF) u32 = 0xFFFD;
	}
	else {
		u32 = 0xFFFD;
		len = 1;
	}

	*p = (u8*)(s + len);
	if (out_len) *out_len = len;
	return u32;
}


// important note:
// out must be unsigned char[4] so under any circumstances we would be able to fit in a UTF-8 character
static int utf8_encode(u32 u32, u8 out[4]) {
	if (u32 > 0x10FFFF ||
		(u32 >= 0xD800 && u32 <= 0xDFFF) ||
		u32 == 0xFFFE || u32 == 0xFFFF) {
		u32 = 0xFFFD;
	}

	if (u32 < 0x80) {
		if (out) out[0] = (u8)u32;
		return 1;
	}
	else if (u32 < 0x800) {
		if (out) {
			out[0] = (u8)(0xC0 | (u32 >> 6));
			out[1] = (u8)(0x80 | (u32 & 0x3F));
		}
		return 2;
	}
	else if (u32 < 0x10000) {
		if (out) {
			out[0] = (u8)(0xE0 | (u32 >> 12));
			out[1] = (u8)(0x80 | ((u32 >> 6) & 0x3F));
			out[2] = (u8)(0x80 | (u32 & 0x3F));
		}
		return 3;
	}
	else {
		if (out) {
			out[0] = (u8)(0xF0 | (u32 >> 18));
			out[1] = (u8)(0x80 | ((u32 >> 12) & 0x3F));
			out[2] = (u8)(0x80 | ((u32 >> 6) & 0x3F));
			out[3] = (u8)(0x80 | (0x3F & u32));
		}
		return 4;
	}
}

static size_t utf8_strlen(const u8* s) {
	if (!s) return 0;
	size_t count = 0;
	u8* p = (u8*)s;

	while (*p) {
		if ((((*p) & 0x80) == 0)) {
			++p;
		}
		else if ((((*p) & 0xE0) == 0xC0) &&
			UTF8_IS_CONT(p[1])) {
			p += 2;
		}
		else if ((((*p) & 0xF0) == 0xE0) &&
			(((p[1]) & 0xC0) == 0x80) && (((p[2]) & 0xC0) == 0x80)) {
			p += 3;
		}
		else if ((((*p) & 0xF8) == 0xF0) &&
			(((p[1]) & 0xC0) == 0x80) && (((p[2]) & 0xC0) == 0x80) && (((p[3]) & 0xC0) == 0x80)) {
			p += 4;
		}
		else {
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
static size_t utf8_advance(u8** s, size_t count) {
	size_t skipped = 0;
	while (count > skipped) {
		utf8_decode(s, NULL);
		skipped++;
	}
	return skipped;
}

// just a small helper function that will conveniently dump current codepoint
static void utf8_dump_codepoint(u32 u32) {
	if (u32 > 0x10FFFF) u32 = 0xFFFD;

	if (u32 <= 0xFFFF) printf("U-%04X", u32);
	else printf("U-%06X", u32);
}

static uint32_t utf16_decode(const u16** p)
{
	u16 w1 = **p;
	(*p)++;

	if (w1 < 0xD800 || w1 > 0xDFFF) {
		return w1;
	}

	/* high surrogates must be followed by a low surrogate */
	if (w1 >= 0xD800 && w1 <= 0xDBFF) {
		uint16_t w2 = **p;
		if (w2 >= 0xDC00 && w2 <= 0xDFFF) {
			(*p)++;
			uint32_t high = w1 - 0xD800;
			uint32_t low = w2 - 0xDC00;
			return (high << 10) + low + 0x10000;
		}
		VL_LOG_ERROR("isolated high surrogate 0x%04X\n", w1);
		return 0xFFFD;
	}

	VL_LOG_ERROR("isolated low surrogate 0x%04X\n", w1);
	return 0xFFFD;
}

static int utf16_encode(u32 codepoint, u16 out[2]) {
	if (codepoint > 0x10FFFF ||
		(codepoint >= 0xD800 && codepoint <= 0xDFFF))
		return 0;

	if (codepoint <= 0xFFFF) {
		out[0] = (u16)codepoint;
		return 1;
	}

	codepoint -= 0x10000;
	u16 high = (u16)((codepoint >> 10) + 0xD800);
	u16 low = (u16)((codepoint & 0x3FF) + 0xDC00);
	out[0] = high;
	out[1] = low;
	return 2;
}

static VL_DA(u16) utf8_to_utf16(const u8* s) {
	VL_DA(u16) result;
	VL_DA_NEW(result, u16);
	const u8* p = s;
	u32 u32 = 0;
	while ((u32 = utf8_decode(&p, NULL))) {
		u16 enc[2];
		int len = utf16_encode(u32, enc);
		VL_DA_APPEND(result, enc[0]);
		if (len >= 2) VL_DA_APPEND(result, enc[1]);
	}
	VL_DA_RESIZE(result, VL_DA_HEADER(result)->count + 1);
	result[VL_DA_HEADER(result)->count - 1] = 0;
	return result;
}

#endif
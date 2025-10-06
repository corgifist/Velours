#include "file.h"
#include "memory.h"

VL_API VlFile *vl_file_new( const u8* path, const u8* mode) {
	VlFile* file = VL_MALLOC(sizeof(VlFile));
	if (!file) return NULL;
	memset(file, 0, sizeof(VlFile));
	FILE* f = fopen(path, mode);
	if (!f) {
		printf("failed to open file %s with mode %s, errno: %i\n", path, mode, errno);
		return NULL;
	}
	file->f = f;
	file->path = path;
	file->mode = mode;

	// reading BOM from file
	unsigned char bom[4];
	size_t n = fread(bom, 1, 4, f);

	fseek(f, 0, SEEK_END);
	file->size = ftell(f);
	fseek(f, 0, SEEK_SET);

	file->bom = VL_BOM_UTF8;                                                              // fallback to UTF-8 in case we can't read the BOM
	if (n >= 3 && bom[0] == 0xEF && bom[1] == 0xBB && bom[2] == 0xBF) {                   // UTF‑8
		file->bom = VL_BOM_UTF8;
		fseek(f, 3, SEEK_SET);
	}
	if (n >= 2 && bom[0] == 0xFF && bom[1] == 0xFE) {                                     // UTF‑16 LE (or UTF‑32 LE)
		file->bom = VL_BOM_UTF16_LE;
		fseek(f, 2, SEEK_SET);
	}

	if (n >= 2 && bom[0] == 0xFE && bom[1] == 0xFF) {                                     // UTF‑16 BE (or UTF‑32 BE)
		file->bom = VL_BOM_UTF16_BE;                                     
		fseek(f, 2, SEEK_SET);
	}
	if (n == 4 && bom[0] == 0xFF && bom[1] == 0xFE && bom[2] == 0x00 && bom[3] == 0x00) { // UTF‑32 LE
		file->bom = VL_BOM_UTF32_LE; 
		fseek(f, 4, SEEK_SET);
	}
	if (n == 4 && bom[0] == 0x00 && bom[1] == 0x00 && bom[2] == 0xFE && bom[3] == 0xFF) { // UTF‑32 BE
		file->bom = VL_BOM_UTF32_BE; 
		fseek(f, 4, SEEK_SET);
	}

	return file;
}

VL_API VlResult vl_file_read_whole(VlFile *file, VL_DA(char)* output) {
	if (!file || !file->f || !output) return VL_ERROR;
	FILE *f = file->f;

	fseek(f, 0, SEEK_END);
	size_t file_size = ftell(f);
	fseek(f, 0, SEEK_SET);

	VL_DA_INDIRECT(output,
		VL_DA_NEW_WITH_ELEMENT_SIZE_AND_ALLOCATOR_AND_CAPACITY(INDIRECT, sizeof(char), VL_MALLOC, file_size + 1)
	);
	char *mem = *output;

	char c;
	uint64_t offset = 0;
	while ((c = (char) fgetc(f)) != EOF) {
		mem[offset++] = c;
	}
	mem[offset] = 0;

	return VL_SUCCESS;
}

VL_API u32 vl_file_read_codepoint(VlFile* file) {
	if (!file) return 0;
	if (file->bom != VL_BOM_UTF8) {
		VL_LOG_ERROR("vl_file_read_codepoint does not support non-utf-8 encodings for now");
		return 0;
	}

	u32 u32 = 0;
	int c;
	u8 s[4];

	c = fgetc(file->f);
	if (c == EOF) {
		return 0;
	}
	s[0] = (u8) c;

	if ((((s[0]) & 0x80) == 0)) {
		u32 = s[0];
		return u32;
	}

	c = fgetc(file->f);
	if (c == EOF) {
		return 0;
	}
	s[1] = (u8)c;

	if ((((s[0]) & 0xE0) == 0xC0) && (((s[1]) & 0xC0) == 0x80)) {
		u32 = ((s[0] & 0x1F) << 6) |
			(s[1] & 0x3F);
		if (u32 < 0x80) u32 = 0xFFFD;
		return u32;
	}

	c = fgetc(file->f);
	if (c == EOF) {
		return 0;
	}
	s[2] = (u8) c;

	if ((((s[0]) & 0xF0) == 0xE0) &&
		(((s[1]) & 0xC0) == 0x80) && (((s[2]) & 0xC0) == 0x80)) {
		u32 = ((s[0] & 0x0F) << 12) |
			((s[1] & 0x3F) << 6) |
			(s[2] & 0x3F);
		if (u32 < 0x800) u32 = 0xFFFD;
		else if (u32 >= 0xD800 && u32 <= 0xDFFF)
			u32 = 0xFFFD;
		return u32;
	}

	c = fgetc(file->f);
	if (c == EOF) {
		return 0;
	}
	s[3] = (u8)c;

	if ((((s[0]) & 0xF8) == 0xF0) &&
		(((s[1]) & 0xC0) == 0x80) && (((s[2]) & 0xC0) == 0x80) && (((s[3]) & 0xC0) == 0x80)) {
		u32 = ((s[0] & 0x07) << 18) |
			((s[1] & 0x3F) << 12) |
			((s[2] & 0x3F) << 6) |
			(s[3] & 0x3F);
		if (u32 < 0x10000 || u32 > 0x10FFFF) u32 = 0xFFFD;

		return u32;
	}
	
	return 0;
}

VL_API VlResult vl_file_free(VlFile *file) {
	if (!file) return VL_ERROR;
	if (file->f) fclose(file->f);
	VL_FREE(file);
	return VL_SUCCESS;
}
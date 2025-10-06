/*
	file.h - different file utilities
	provides ways to read, write, get size of files
*/
#ifndef VELOURS_FILE_H
#define VELOURS_FILE_H

#include <file.h>

#include "velours.h"
#include "da.h"

// encoding that file uses, for now only utf encodings are supported
// if file has no BOM or reading BOM has failed, then we default to utf-8
// and hope for the best
typedef enum {
	VL_BOM_UTF8 = 0,
	VL_BOM_UTF16_LE = 1,
	VL_BOM_UTF16_BE = 2,
	VL_BOM_UTF32_LE = 3,
	VL_BOM_UTF32_BE = 4
} VlBOM;

typedef struct {
	FILE *f;
	const char *path;
	const char *mode;
	uint64_t size;
	VlBOM bom;
} VlFile;

VL_API VlFile *vl_file_new(const u8* path, const u8 *mode);
VL_API VlResult vl_file_read_whole(VlFile *file, VL_DA(char)* output);
VL_API u32 vl_file_read_codepoint(VlFile* file);
VL_API VlResult vl_file_free(VlFile *file);

#endif // VELOURS_FILE_H
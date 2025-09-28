#include "file.h"

#include <file.h>

VL_API VlResult vl_file_read(const char *path, VL_DA(char) *output) {
	FILE* f = fopen(path, "r");
	if (!f) return VL_ERROR;

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

	fclose(f);
	return VL_SUCCESS;
}
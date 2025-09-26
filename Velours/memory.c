#include "memory.h"

#include <stdio.h>

#ifdef VELOURS_MANAGED_MALLOC

static size_t s_allocated = 0;

typedef struct {
	size_t size;
} VlAllocHeader;

VL_API void *vl_malloc(size_t size) {
	if (!size) return NULL;
	void *res = malloc(size + sizeof(VlAllocHeader));
	if (res) s_allocated += size;
	if (res) ((VlAllocHeader*) res)->size = size;
	printf("vl_malloc(%zu)\n", size);
	return res ? (char*) res + sizeof(VlAllocHeader) : NULL;
}

VL_API void *vl_calloc(size_t count, size_t size) {
	if (size * count == 0) return NULL;
	void *res = malloc(size * count + sizeof(VlAllocHeader));
	if (res) s_allocated += size * count;
	if (res) memset(res, 0, size * count + sizeof(VlAllocHeader));
	if (res) ((VlAllocHeader*) res)->size = size;
	printf("vl_callloc(%zu, %zu)\n", count, size);
	return res ? (char*) res + sizeof(VlAllocHeader) : NULL;
}

VL_API void *vl_realloc(void *mem, size_t new_size) {
	if (!mem) return NULL;
	size_t old_size = ((VlAllocHeader*) ((char*) mem - sizeof(VlAllocHeader)))->size;
	void* res = realloc((char*) mem - sizeof(VlAllocHeader), new_size + sizeof(VlAllocHeader));
	printf("vl_realloc(%p, %zu), old size: %zu\n", mem, new_size, old_size);
	if (res) ((VlAllocHeader*) res)->size = new_size;
	if (res) s_allocated += new_size - old_size;
	return res ? (char*) res + sizeof(VlAllocHeader) : NULL;
}

VL_API void vl_free(void* mem) {
	if (!mem) return;
	size_t size = ((VlAllocHeader*) ((char*) mem - sizeof(VlAllocHeader)))->size;
	printf("vl_free(%p), alloc size: %zu\n", mem, size);
	free(((char*) mem - sizeof(VlAllocHeader)));
	s_allocated -= size;
}

VL_API size_t vl_get_memory_usage(void) {
	return s_allocated;
}

#else

VL_API size_t vl_get_memory_usage(void) {
	return 0;
}

#endif// VELOURS_MANAGED_MALLOC
#include "memory.h"
#include "ht.h"

#include <stdio.h>

#ifdef VELOURS_MANAGED_MALLOC

static size_t s_allocated = 0;

typedef struct {
	size_t size;
} VlAllocHeader;

typedef struct {
	void *ptr;
	size_t size;
	const char *file;
	size_t line;
} VlAllocInfo;

VL_HT_HASH(VlAllocInfo) {
	return (uint64_t) ((VlAllocInfo*) p)->ptr;
}

static VL_HT(void*, VlAllocInfo) s_allocs;
static int s_level = VL_MEMORY_ONLY_ERRORS;

#define VL_MEMORY_DEFAULT_CAPACITY 1024

// do NOT use managed malloc when using dynamic arrays here
// so we don't get stuck in a loop where vl_malloc calls vl_malloc and so on
#define CHECK_ALLOCS() \
	if (!s_allocs) VL_HT_NEW_WITH_ALLOCATOR_AND_SIZE_AND_HASH_FUNCTION_AND_CAPACITY(s_allocs, sizeof(void*), sizeof(VlAllocInfo), malloc, vl_ht_hash_VlAllocInfo, VL_MEMORY_DEFAULT_CAPACITY);

VL_API void *vl_malloc(const char *file, size_t line, size_t size) {
	if (!size) return NULL;
	CHECK_ALLOCS();
	if (s_level >= VL_MEMORY_ALL) printf("%s:%zu: vl_malloc(%zu)\n", file, line, size);
	void *res = malloc(size + sizeof(VlAllocHeader));
	if (res) s_allocated += size;
	if (res) {
		((VlAllocHeader*) res)->size = size;

		VlAllocInfo info;
		info.ptr = (char*) res + sizeof(VlAllocHeader);
		info.size = size;
		info.file = file;
		info.line = line;

		VL_HT_PUT_WITH_ALLOCATOR(s_allocs, info.ptr, info, realloc);
	}
	return res ? (char*) res + sizeof(VlAllocHeader) : NULL;
}

VL_API void *vl_calloc(const char *file, size_t line, size_t count, size_t size) {
	if (size * count == 0) return NULL;
	CHECK_ALLOCS();
	if (s_level >= VL_MEMORY_ALL) printf("%s:%zu: vl_callloc(%zu, %zu)\n", file, line, count, size);
	void *res = malloc(size * count + sizeof(VlAllocHeader));
	if (res) s_allocated += size * count;
	if (res) memset(res, 0, size * count + sizeof(VlAllocHeader));
	if (res) {
		((VlAllocHeader*) res)->size = size * count;

		VlAllocInfo info;
		info.ptr = (char*) res + sizeof(VlAllocHeader);
		info.size = size * count;
		info.file = file;
		info.line = line;

		VL_HT_PUT_WITH_ALLOCATOR(s_allocs, info.ptr, info, realloc);
	}
	return res ? (char*) res + sizeof(VlAllocHeader) : NULL;
}

VL_API void *vl_realloc(const char *file, size_t line, void *mem, size_t new_size) {
	if (!mem) return NULL;
	CHECK_ALLOCS();
	size_t old_size = ((VlAllocHeader*) ((char*) mem - sizeof(VlAllocHeader)))->size;
	if (s_level >= VL_MEMORY_ALL) printf("%s:%zu: vl_realloc(%p, %zu), old size: %zu\n", file, line, mem, new_size, old_size);
	void* res = realloc((char*) mem - sizeof(VlAllocHeader), new_size + sizeof(VlAllocHeader));
	if (res) {
		((VlAllocHeader*) res)->size = new_size;

		char success = 0;
		VL_HT_DELETE(s_allocs, mem, success);
		if (!success && s_level >= VL_MEMORY_ONLY_ERRORS) printf("failed to remove allocation %p from s_allocs\n", mem);
		if (!success) exit(VL_ERROR);

		VlAllocInfo info;
		info.ptr = (char*) res + sizeof(VlAllocHeader);
		info.size = new_size;
		info.file = file;
		info.line = line;

		VL_HT_PUT_WITH_ALLOCATOR(s_allocs, info.ptr, info, realloc);
	}
	if (res) s_allocated += new_size - old_size;
	return res ? (char*) res + sizeof(VlAllocHeader) : NULL;
}

VL_API void vl_free(const char *file, size_t line, void *mem) {
	if (!mem) return;
	CHECK_ALLOCS();
	size_t size = ((VlAllocHeader*) ((char*) mem - sizeof(VlAllocHeader)))->size;
	if (s_level >= VL_MEMORY_ALL) printf("%s:%zu: vl_free(%p), alloc size: %zu\n", file, line, mem, size);
	char success = 0;
	VL_HT_DELETE(s_allocs, mem, success);
	if (!success && s_level >= VL_MEMORY_ONLY_ERRORS) printf("failed to remove allocation %p from s_allocs\n", mem);
	if (!success) exit(VL_ERROR);
	free(((char*) mem - sizeof(VlAllocHeader)));
	s_allocated -= size;
}

VL_API void vl_memory_set_logging_level(int level) {
	s_level = level;
}

VL_API size_t vl_get_memory_usage(void) {
	return s_allocated;
}

VL_API void vl_dump_all_allocations(void) {
	VL_HT(void*, VlAllocInfo) iterator_pos = s_allocs;
	VlHTEntry entry;
	printf("%zu allocation(s):\n", VL_HT_HEADER(s_allocs)->count);
	while (vl_ht_iterate(s_allocs, &iterator_pos, &entry)) {
		VlAllocInfo *alloc = (VlAllocInfo*) entry.value;
		printf("%s:%zu: %zu bytes, %p\n", alloc->file, alloc->line, alloc->size, alloc->ptr);
	}
}

#else

VL_API void vl_memory_set_logging_level(int level) {
	VL_UNUSED(level);
}

VL_API size_t vl_get_memory_usage(void) {
	return 0;
}

VL_API void vl_dump_all_allocations(void) {}

#endif// VELOURS_MANAGED_MALLOC
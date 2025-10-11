#include "memory.h"
#include "ht.h"

#include <stdio.h>

#ifdef VELOURS_MANAGED_MALLOC

static size_t s_allocated = 0;
static size_t s_allocations = 0;

struct VlAllocInfo {
	void *ptr;
	size_t size;
	const char *file;
	size_t line;
	struct VlAllocInfo *prev, *next;
};

#define VL_ALLOC_INFO(PTR, SIZE, FILE, LINE) \
	((struct VlAllocInfo) {.ptr = (void*) ((char*) (PTR) + sizeof(struct VlAllocInfo)), .size = (SIZE), .file = (FILE), .line = (LINE), .prev = NULL, .next = NULL})

static int s_level = VL_MEMORY_ONLY_ERRORS;

static struct VlAllocInfo s_root = { 0 };
static struct VlAllocInfo *s_first = &s_root;
static struct VlAllocInfo *s_last = &s_root;

VL_API void *vl_malloc(const char *file, size_t line, size_t size) {
	if (!size) return NULL;
	if (s_level >= VL_MEMORY_ALL) printf("%s(%zu): vl_malloc(%zu)", file, line, size);
	struct VlAllocInfo* res = malloc(size + sizeof(struct VlAllocInfo));
	if (s_level >= VL_MEMORY_ALL) printf(", %p\n", res);
	if (res) {
		s_allocated += size;
		s_allocations++;

		*res = VL_ALLOC_INFO(res, size, file, line);
		
		res->next = NULL;
		res->prev = s_last;
		s_last->next = res;
		s_last = res;
	}
	return res ? res->ptr : NULL;
}

VL_API void *vl_realloc(const char *file, size_t line, void *mem, size_t new_size) {
	if (!mem) return vl_malloc(file, line, new_size);
	struct VlAllocInfo *old_info = (struct VlAllocInfo*) ((char*) mem - sizeof(struct VlAllocInfo));
	struct VlAllocInfo* prev = old_info->prev;
	struct VlAllocInfo* next = old_info->next;
	size_t old_size = old_info->size;
	if (s_level >= VL_MEMORY_ALL) printf("%s(%zu): vl_realloc(%p, %zu), old size: %zu", file, line, mem, new_size, old_size);
	if (!old_info->ptr && s_level >= VL_MEMORY_ONLY_ERRORS) {
		printf("\npossible realloc of NULL pointer: %s(%zu)\n", file, line);
		exit(1);
	}
	struct VlAllocInfo* res = realloc(old_info, new_size + sizeof(struct VlAllocInfo));
	if (s_level >= VL_MEMORY_ALL) printf(", %p\n", res);
	if (res) {
		s_allocated += new_size - old_size;

		*res = VL_ALLOC_INFO(res, new_size, file, line);
		res->prev = prev;
		res->next = next;

		if (res->prev) res->prev->next = res;
		if (res->next) {
			res->next->prev = res;
		} else {
			if (s_last->prev) s_last->prev->next = res;
			s_last = res;
		}
	}
	return res ? res->ptr : NULL;
}

VL_API void vl_free(const char *file, size_t line, void *mem) {
	if (!mem) return;
	size_t size = ((struct VlAllocInfo*) ((char*) mem - sizeof(struct VlAllocInfo)))->size;
	if (s_level >= VL_MEMORY_ALL) printf("%s(%zu): vl_free(%p), alloc size: %zu\n", file, line, mem, size);
	struct VlAllocInfo *info = (struct VlAllocInfo*) ((char*) mem - sizeof(struct VlAllocInfo));
	if ((!info->ptr || !info->prev) && s_level >= VL_MEMORY_ONLY_ERRORS) {
		printf("possible double free: %s(%zu)\n", file, line);
		exit(1);
		return;
	}
	if (info->prev) info->prev->next = info->next;
	if (info->next) info->next->prev = info->prev;
	else {
		s_last->next = info->prev;
		s_last = info->prev;
	}
	info->ptr = NULL;
	free(info);
	s_allocated -= size;
	s_allocations--;
}

VL_API void vl_memory_set_logging_level(int level) {
	s_level = level;
}

VL_API size_t vl_memory_get_usage(void) {
	return s_allocated;
}

VL_API void vl_memory_dump(void) {
	struct VlAllocInfo* p = s_first;
	printf("%zu allocation(s):\n", s_allocations);
	size_t check = 0;
	while ((p = p->next)) {
		check++;
		printf("%s(%zu) %zu bytes, %p\n", p->file, p->line, p->size, p->ptr);
	}
	if (check != s_allocations) {
		printf("managed malloc corruption\n");
		exit(1);
	}
}

#else

VL_API void vl_memory_set_logging_level(int level) {
	VL_UNUSED(level);
}

VL_API size_t vl_memory_get_usage(void) {
	return 0;
}

VL_API void vl_memory_dump(void) {}

#endif// VELOURS_MANAGED_MALLOC
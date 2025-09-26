#ifndef VELOURS_MEMORY_H
#define VELOURS_MEMORY_H

#include "velours.h"

#include <stdlib.h>

#ifndef VELOURS_MANAGED_MALLOC
#define VL_MALLOC malloc
#define VL_CALLOC calloc
#define VL_REALLOC realloc
#define VL_FREE free
#else
VL_API void* vl_malloc(size_t size);
VL_API void* vl_calloc(size_t num, size_t size);
VL_API void* vl_realloc(void* mem, size_t new_size);
VL_API void vl_free(void* mem);

#define VL_MALLOC vl_malloc
#define VL_CALLOC vl_calloc
#define VL_REALLOC vl_realloc
#define VL_FREE vl_free
#endif

// returns an APPROXIMATE memory usage in bytes
VL_API size_t vl_get_memory_usage(void);

#endif // VELOURS_MEMORY_H
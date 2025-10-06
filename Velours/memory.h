/* 
	memory.h - provides macros like VL_MALLOC, VL_REALLOC etc. 

	if at compile time VELOURS_MANAGED_MALLOC macro was defined,
	managed allocation functions like vl_malloc, vl_realloc would be used

	key difference between vl_malloc, vl_realloc etc. and malloc, realloc etc.
	is that vl_xxx functions stores additional information alongside the allocated memory
	additional memory includes: 
		* source code location from which the memory was allocated
		* the size of allocated
	managed malloc comes in hand when detecting various memory leaks and profiling
*/

#ifndef VELOURS_MEMORY_H
#define VELOURS_MEMORY_H

#include "velours.h"

#include <stdlib.h>

#define VL_MEMORY_NO_LOGGING 0
#define VL_MEMORY_ONLY_ERRORS 1
#define VL_MEMORY_ALL 2

#ifndef VELOURS_MANAGED_MALLOC
#define VL_MALLOC malloc
#define VL_REALLOC realloc
#define VL_FREE free
#else
VL_API void* vl_malloc(const char* file, size_t line, size_t size);
VL_API void* vl_realloc(const char* file, size_t line, void* mem, size_t new_size);
VL_API void vl_free(const char* file, size_t line, void* mem);

#define VL_MALLOC(...) \
	vl_malloc(__FILE__, __LINE__, __VA_ARGS__)
#define VL_REALLOC(...) \
	vl_realloc(__FILE__, __LINE__, __VA_ARGS__)
#define VL_FREE(...) \
	vl_free(__FILE__, __LINE__, __VA_ARGS__)
#endif // VELOURS_MANAGED_MALLOC

VL_API void vl_memory_set_logging_level(int level);

// returns an APPROXIMATE memory usage in bytes
VL_API size_t vl_memory_get_usage(void);

// dumps every not freed malloc call to stdout
VL_API void vl_memory_dump(void);

#endif // VELOURS_MEMORY_H
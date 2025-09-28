/* 
	da.h - velour's implementation of dynamic arrays in C 
	automatically grows and shinrks according to the user needs
*/

#ifndef VELOURS_DA_H
#define VELOURS_DA_H

#include "memory.h"

#include <stdint.h>
#include <stdio.h>

typedef struct {
	size_t element_size, count, cap;
} VlDAHeader;

// velours' dynamic array memory layout:
// .........[VlDAHeader][ELEMENT1][ELEMENT2][ELEMENT3]...........
//                      ^
//          VL_DA_NEW returns pointer to here

#pragma warning(push)
#pragma warning( disable : 6011 )
#pragma warning( disable : 6001 )
#pragma warning( disable : 6308 )

// VL_BASE macro is necessary!
// without it we wouldn't be able to chain different macros together

// Preprocessor macro that does nothing, but makes it clear that the type is DYNAMIC ARRAY, e.g: 
// typedef struct {
//     int *pointer_to_some_int;
//     VL_DA(int) dynamic_array_of_ints;
// } SomeStruct;
#define VL_DA(T) T*

#define VL_DA_DEFAULT_CAPACITY 8

#define VL_DA_HEADER(VAR) ((VlDAHeader*) (((char*) VAR) - sizeof(VlDAHeader)))

#define VL_DA_LENGTH(VAR) (VAR ? VL_DA_HEADER(VAR)->count : 0)

#define VL_DA_NEW_WITH_ELEMENT_SIZE_AND_ALLOCATOR_AND_CAPACITY(VAR, SIZE, MALLOC, CAPACITY) \
	do { \
		VAR = MALLOC(SIZE * (CAPACITY) + sizeof(VlDAHeader)); \
		if (!VAR) { \
			printf("malloc from VL_DA_NEW_WITH_ELEMENT_SIZE(%s, %zu) has returned NULL", #VAR, SIZE); \
			break; \
		} \
		((VlDAHeader*) VAR)->element_size = SIZE; \
		((VlDAHeader*) VAR)->count = 0; \
		((VlDAHeader*) VAR)->cap = CAPACITY; \
		VAR = (void*) ((char*) VAR + sizeof(VlDAHeader)); \
	} while (0)

#define VL_DA_NEW_WITH_ELEMENT_SIZE_AND_ALLOCATOR(VAR, SIZE, MALLOC) \
	VL_BASE(VL_DA_NEW_WITH_ELEMENT_SIZE_AND_ALLOCATOR_AND_CAPACITY(VAR, SIZE, MALLOC, VL_DA_DEFAULT_CAPACITY))
#define VL_DA_NEW_WITH_ALLOCATOR(VAR, T, MALLOC) VL_BASE(VL_DA_NEW_WITH_ELEMENT_SIZE_AND_ALLOCATOR(VAR, sizeof(T), MALLOC))
#define VL_DA_NEW(VAR, T) VL_BASE(VL_DA_NEW_WITH_ALLOCATOR(VAR, T, VL_MALLOC))

#define VL_DA_APPEND_WITH_ALLOCATOR(VAR, ELEMENT, REALLOC) \
	do { \
		VlDAHeader *header = VL_DA_HEADER(VAR); \
		if (header->count + 1 >= header->cap) { \
			header->cap *= 2; \
			VAR = (void*) ((char*) REALLOC(header, header->element_size * header->cap + sizeof(VlDAHeader)) + sizeof(VlDAHeader)); \
			if (!VAR) { \
				printf("realloc from VL_DA_APPEND(%s, %s) has returned NULL", #VAR, #ELEMENT); \
				break; \
			} \
			header = VL_DA_HEADER(VAR); \
		} \
		memcpy((char*) VAR + header->element_size * header->count, &(ELEMENT), header->element_size); \
		header->count++; \
	} while (0)

#define VL_DA_APPEND(VAR, ELEMENT) VL_BASE(VL_DA_APPEND_WITH_ALLOCATOR(VAR, ELEMENT, VL_REALLOC))

#define VL_DA_APPEND_CONST_WITH_ALLOCATOR(VAR, T, ELEMENT, REALLOC) \
	do { \
		T t = ELEMENT; \
		VL_DA_APPEND_WITH_ALLOCATOR(VAR, t, REALLOC); \
	} while (0)

#define VL_DA_APPEND_CONST(VAR, T, ELEMENT) VL_BASE(VL_DA_APPEND_CONST_WITH_ALLOCATOR(VAR, T, ELEMENT, VL_REALLOC))

#define VL_DA_DELETE_WITH_ALLOCATOR(VAR, I, REALLOC) \
	do { \
		VlDAHeader *header = VL_DA_HEADER(VAR); \
		size_t index = (I); \
		if (header->count == 0 || index >= header->count) { \
			printf("invalid index in VL_DA_DELETE(%s, %s), ignoring", #VAR, #I); \
			break; \
		} \
		header->count--; \
		for (size_t i = index; i < header->count; i++) { \
			memcpy(VAR + i, VAR + i + 1, header->element_size); \
		} \
		if (header->count <= header->cap / 2 && header->cap / 2 > VL_DA_DEFAULT_CAPACITY) { \
			VAR = REALLOC((char*) VAR - sizeof(VlDAHeader), header->cap / 2 * header->element_size + sizeof(VlDAHeader)); \
			if (!VAR) { \
				printf("realloc in VL_DA_DELETE(%s, %s) has returned NULL", #VAR, #I); \
				break; \
			} \
			header = (VlDAHeader*) VAR; \
			header->cap /= 2; \
			VAR = (void*) ((char*) VAR + sizeof(VlDAHeader)); \
		} \
	} while (0)

#define VL_DA_DELETE(VAR, I) VL_BASE(VL_DA_DELETE_WITH_ALLOCATOR(VAR, I, VL_REALLOC))

#define VL_DA_FREE_WITH_ALLOCATOR(VAR, FREE) \
	do { \
		if (!VAR) break; \
		FREE(VL_DA_HEADER(VAR)); \
		VAR = NULL; \
	} while (0)

#define VL_DA_FREE(VAR) VL_BASE(VL_DA_FREE_WITH_ALLOCATOR(VAR, VL_FREE))

#define VL_DA_RESET_WITH_ALLOCATOR(VAR, MALLOC, FREE) \
	do { \
		size_t element_size = VL_DA_HEADER(VAR)->element_size; \
		VL_DA_FREE_WITH_ALLOCATOR(VAR, FREE); \
		VL_DA_NEW_WITH_ELEMENT_SIZE_AND_ALLOCATOR(VAR, element_size, MALLOC); \
	} while (0)

#define VL_DA_RESET(VAR) VL_BASE(VL_DA_RESET_WITH_ALLOCATOR(VAR, VL_MALLOC, VL_FREE))

#define VL_DA_FOREACH(VAR, I) \
	for (size_t I = 0; I < VL_DA_LENGTH(VAR); I++)

#define VL_DA_INDIRECT(PTR, ACTION) \
	do { \
		if (!PTR) break; \
		void *INDIRECT = *PTR; \
		ACTION; \
		*PTR = INDIRECT; \
	} while (0)

#define VL_DA_DUMP_HEADER(DA) \
	do { \
		if (!DA) break; \
		VlDAHeader *header = VL_DA_HEADER(DA); \
		printf("VL_DA_DUMP_HEADER(%s):\n", #DA); \
		printf("\telement_size: %zu\n", header->element_size); \
		printf("\tcount: %zu\n", header->count); \
		printf("\tcap: %zu\n", header->cap); \
	} while (0)

#pragma warning(pop)
#endif // VELOURS_DA_H
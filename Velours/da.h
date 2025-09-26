/* 
	da.h - velour's implementation of dynamic arrays in C 
	automatically grows and shinrks according to the user needs
*/

#ifndef VELOURS_DA_H
#define VELOURS_DA_H

#include <stdlib.h>
#include <stdio.h>

#include "memory.h"

typedef struct {
	size_t element_size, count, cap;
} VlDAHeader;

#pragma warning(push)
#pragma warning( disable : 6011 )
#pragma warning( disable : 6001 )
#pragma warning( disable : 6308 )

// Preprocessor macro that does nothing, but makes it clear that the type is DYNAMIC ARRAY, e.g: 
// typedef struct {
//     int *pointer_to_some_int;
//     VL_DA(int) dynamic_array_of_ints;
// } SomeStruct;
#define VL_DA(T) T*

#define VL_DA_DEFAULT_CAPACITY 8

#define VL_DA_HEADER(VAR) ((VlDAHeader*) (((char*) VAR) - sizeof(VlDAHeader)))

#define VL_DA_LENGTH(VAR) (VAR ? VL_DA_HEADER(VAR)->count : 0)

#define VL_DA_NEW_WITH_ELEMENT_SIZE(VAR, SIZE) \
	do { \
		VAR = VL_MALLOC(SIZE * (VL_DA_DEFAULT_CAPACITY) + sizeof(VlDAHeader)); \
		if (!VAR) { \
			printf("malloc from VL_DA_NEW_WITH_ELEMENT_SIZE(%s, %zu) has returned NULL", #VAR, SIZE); \
			break; \
		} \
		((VlDAHeader*) VAR)->element_size = SIZE; \
		((VlDAHeader*) VAR)->count = 0; \
		((VlDAHeader*) VAR)->cap = VL_DA_DEFAULT_CAPACITY; \
		VAR = (void*) ((char*) VAR + sizeof(VlDAHeader)); \
	} while (0)

#define VL_DA_NEW(VAR, T) VL_DA_NEW_WITH_ELEMENT_SIZE(VAR, sizeof(T))

#define VL_DA_APPEND(VAR, ELEMENT) \
	do { \
		VlDAHeader *header = VL_DA_HEADER(VAR); \
		if (header->count + 1 > header->cap) { \
			header->cap *= 2; \
			VAR = (void*) ((char*) VL_REALLOC(header, header->element_size * header->cap + sizeof(VlDAHeader)) + sizeof(VlDAHeader)); \
			if (!VAR) { \
				printf("realloc from VL_DA_APPEND(%s, %s) has returned NULL", #VAR, #ELEMENT); \
				break; \
			} \
			header = VL_DA_HEADER(VAR); \
		} \
		memcpy((char*) VAR + header->element_size * header->count, &(ELEMENT), header->element_size); \
		header->count++; \
	} while (0)

#define VL_DA_APPEND_CONST(VAR, T, ELEMENT) \
	do { \
		T t = ELEMENT; \
		VL_DA_APPEND(VAR, t); \
	} while (0)

#define VL_DA_DELETE(VAR, I) \
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
		if (header->count <= header->cap / 2) { \
			VAR = VL_REALLOC((char*) VAR - sizeof(VlDAHeader), header->cap / 2 * header->element_size + sizeof(VlDAHeader)); \
			if (!VAR) { \
				printf("realloc in VL_DA_DELETE(%s, %s) has returned NULL", #VAR, #I); \
				break; \
			} \
			header = (VlDAHeader*) VAR; \
			header->cap /= 2; \
			VAR = (void*) ((char*) VAR + sizeof(VlDAHeader)); \
		} \
	} while (0)

#define VL_DA_FREE(VAR) \
	do { \
		if (!VAR) break; \
		VL_FREE(VL_DA_HEADER(VAR)); \
		VAR = NULL; \
	} while (0)

#define VL_DA_RESET(VAR) \
	do { \
		size_t element_size = VL_DA_HEADER(VAR)->element_size; \
		VL_DA_FREE(VAR); \
		VL_DA_NEW_WITH_ELEMENT_SIZE(VAR, element_size); \
	} while (0)

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
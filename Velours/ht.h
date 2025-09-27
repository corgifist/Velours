/*
	ht.h - velours' hash table implementation
	hash table allows you to store values in key-value manner
*/

#ifndef VELOURS_HT_H
#define VELOURS_HT_H

#include "velours.h"
#include "memory.h"

#include <stdint.h>

#define VL_HT_DEFAULT_ENTRIES_CAPACITY 8

// macro that explicitly states that a type is a HASH TABLE
// usage:
//     struct SomeStruct {
//         int *some_pointer;
//         VL_HT(int) some_hash_table;
//     };
#define VL_HT(KEY_T, VALUE_T) VALUE_T*

typedef struct {
	size_t key_size, value_size, count, cap;
	uint64_t (*hash_key)(void*);
} VlHTHeader;

// used for iterating over hash table using vl_ht_iterate
typedef struct {
	void *key, *value;
} VlHTEntry;

// how velours' hash table looks in memory:
// ........[VlHTHeader][uint64_t KEY HASH][char OCCUPIED][KEY][VALUE][uint64_t KEY_HASH][char occupied][KEY][VALUE].........
//                     ^
//         VL_HT_NEW returns pointer to here

#define VL_HT_HEADER(VAR) ((VlHTHeader*) ((char*) (VAR) - sizeof(VlHTHeader)))

#define VL_HT_NEW_WITH_ALLOCATOR_AND_SIZE_AND_HASH_FUNCTION(VAR, KEY_SIZE, VALUE_SIZE, MALLOC, HASH_FUNCTION) \
	do { \
		VAR = MALLOC((sizeof(uint64_t) + sizeof(char) + (KEY_SIZE) + VALUE_SIZE) * VL_HT_DEFAULT_ENTRIES_CAPACITY + sizeof(VlHTHeader)); \
		if (!VAR) { \
			printf("malloc in VL_HT_ALLOC_WITH_ELEMENT_SIZE_AND_ALLOCATOR(%s, %zu, %zu, %s, %s) has returned NULL\n", #VAR, (KEY_SIZE), (VALUE_SIZE), #MALLOC, #HASH_FUNCTION); \
			break; \
		} \
		memset(VAR, 0, (sizeof(uint64_t) + sizeof(char) + (KEY_SIZE) + (VALUE_SIZE)) * VL_HT_DEFAULT_ENTRIES_CAPACITY + sizeof(VlHTHeader)); \
		((VlHTHeader*) VAR)->key_size = (KEY_SIZE); \
		((VlHTHeader*) VAR)->value_size = (VALUE_SIZE); \
		((VlHTHeader*) VAR)->count = 0; \
		((VlHTHeader*) VAR)->cap = VL_HT_DEFAULT_ENTRIES_CAPACITY; \
		((VlHTHeader*) VAR)->hash_key = HASH_FUNCTION; \
		VAR = (void*) ((char*) VAR + sizeof(VlHTHeader)); \
	} while (0)

#define VL_HT_NEW_WITH_ALLOCATOR(VAR, KEY_T, VALUE_T, MALLOC) \
	VL_BASE(VL_HT_NEW_WITH_ALLOCATOR_AND_SIZE_AND_HASH_FUNCTION(VAR, sizeof(KEY_T), sizeof(VALUE_T), MALLOC, vl_ht_hash_##KEY_T))

#define VL_HT_NEW(VAR, KEY_T, VALUE_T) \
	VL_BASE(VL_HT_NEW_WITH_ALLOCATOR(VAR, KEY_T, VALUE_T, VL_MALLOC))

#define VL_HT_GROW_WITH_ALLOCATOR(VAR, REALLOC) \
	do { \
		VlHTHeader *grow_header = VL_HT_HEADER(VAR); \
		uint64_t entry_size = (sizeof(uint64_t) + sizeof(char) + header->key_size + header->value_size); \
		size_t old_size = entry_size * grow_header->cap + sizeof(VlHTHeader); \
		grow_header->cap *= 2; \
		size_t new_size = entry_size * grow_header->cap + sizeof(VlHTHeader); \
		grow_header = REALLOC(grow_header, new_size); \
		if (!grow_header) { \
			printf("realloc in VL_HT_GROW(%s, %s) has returned NULL\n", #VAR, #REALLOC); \
			break; \
		} \
		memset((char*) grow_header + old_size, 0, new_size - old_size); \
		VAR = (void*) ((char*) grow_header + sizeof(VlHTHeader)); \
	} while (0)

#define VL_HT_GROW(VAR) \
	VL_BASE(VL_HT_GROW_WITH_ALLOCATOR(VAR, VL_REALLOC))

#define VL_HT_PUT_WITH_ALLOCATOR(VAR, KEY, VALUE, REALLOC) \
	do { \
		VlHTHeader *header = VL_HT_HEADER(VAR); \
		if (header->count + 1 >= header->cap) { \
			VL_HT_GROW_WITH_ALLOCATOR(VAR, REALLOC); \
			header = VL_HT_HEADER(VAR); \
		} \
		uint64_t hash = header->hash_key(&(KEY)); \
		size_t index = hash & (header->cap - 1); \
		while (index < header->cap) { \
			uint64_t *entry_hash = (uint64_t*) ((char*) VAR + (sizeof(uint64_t) + sizeof(char) + header->key_size + header->value_size) * index); \
			char *entry_occupied = (char*) entry_hash + sizeof(uint64_t); \
			void *entry_key = (char*) entry_occupied + sizeof(char); \
			void *entry_value = (char*) entry_key + header->key_size; \
			if (!*entry_occupied) { \
				memcpy(entry_key, &(KEY), header->key_size); \
				memcpy(entry_value, &(VALUE), header->value_size); \
				header->count++; \
				*entry_hash = hash; \
				*entry_occupied = 1; \
				break; \
			} \
			/* updating already existing hash table entry */ \
			if (*entry_hash == hash && *entry_occupied) { \
				memcpy(entry_value, &(VALUE), header->value_size); \
				break; \
			} \
			/* wrap around the hash table so we can try to find suitable entry one more time */ \
			if (index + 1 >= header->cap) index = 0; \
			else index++; \
		} \
	} while (0)

#define VL_HT_PUT(VAR, KEY, VALUE) \
	VL_BASE(VL_HT_PUT_WITH_ALLOCATOR(VAR, KEY, VALUE, VL_REALLOC))

#define VL_HT_PUT_CONST_WITH_ALLOCATOR(VAR, KEY_T, KEY, VALUE_T, VALUE, REALLOC) \
	do { \
		KEY_T k = KEY; \
		VALUE_T v = VALUE; \
		VL_HT_PUT_WITH_ALLOCATOR(VAR, k, v, REALLOC); \
	} while (0)

#define VL_HT_PUT_CONST(VAR, KEY_T, KEY, VALUE_T, VALUE) \
	VL_BASE(VL_HT_PUT_CONST_WITH_ALLOCATOR(VAR, KEY_T, KEY, VALUE_T, VALUE, VL_REALLOC))

#define VL_HT_GET(VAR, KEY, VALUE, FOUND) \
	do { \
		VlHTHeader *header = VL_HT_HEADER(VAR); \
		uint64_t hash = header->hash_key(&(KEY)); \
		size_t index = hash & (header->cap - 1); \
		char second_leap = 0; \
		FOUND = 0; \
		while (index < header->cap) { \
			uint64_t *entry_hash = (uint64_t*) ((char*) VAR + (sizeof(uint64_t) + sizeof(char) + header->key_size + header->value_size) * index); \
			char *entry_occupied = (char*) entry_hash + sizeof(uint64_t); \
			void *entry_key = (char*) entry_occupied + sizeof(char); \
			void *entry_value = (char*) entry_key + header->key_size; \
			/* updating already existing hash table entry */ \
			if (*entry_hash == hash && *entry_occupied) { \
				memcpy(&(VALUE), entry_value, header->value_size); \
				FOUND = 1; \
				break; \
			} \
			/* wrap around the hash table so we can try to find a suitable entry one more time */ \
			if (++index >= header->cap) { \
				if (!second_leap) { \
					index = 0; \
					second_leap = 1; \
				} else { \
					break; \
				} \
			} \
		} \
	} while (0)

#define VL_HT_GET_CONST(VAR, KEY_T, KEY, VALUE, FOUND) \
	do { \
		KEY_T __ht_get_const_k = (KEY); \
		VL_HT_GET(VAR, __ht_get_const_k, VALUE, FOUND); \
	} while (0)

#define VL_HT_FREE_WITH_ALLOCATOR(VAR, FREE) \
	do { \
		if (!(VAR)) break; \
		FREE(VL_HT_HEADER(VAR)); \
		VAR = NULL; \
	} while (0)

#define VL_HT_FREE(VAR) \
	VL_BASE(VL_HT_FREE_WITH_ALLOCATOR(VAR, VL_FREE))

#define VL_HT_RESET_WITH_ALLOCATOR(VAR, MALLOC, FREE) \
	do { \
		if (!(VAR)) break; \
		VlHTHeader reserved_header = *VL_HT_HEADER(VAR); \
		VL_HT_FREE_WITH_ALLOCATOR(VAR, FREE); \
		VL_HT_NEW_WITH_ALLOCATOR_AND_SIZE_AND_HASH_FUNCTION(VAR, reserved_header.key_size, reserved_header.value_size, MALLOC, reserved_header.hash_key); \
	} while (0) \

#define VL_HT_RESET(VAR) \
	VL_BASE(VL_HT_RESET_WITH_ALLOCATOR(VAR, VL_MALLOC, VL_FREE))

#define VL_HT_HASH(T) \
	VL_API uint64_t vl_ht_hash_##T(void* p)

// types like void*, const char* etc. cannot be used in hash tables directly
// instead of defining hash table like this:
//     VL_HT(const char*, void*) some_table;
// you should do the folowwing:
//     VL_HT(VlString, VlPtr) some_table;
typedef void* VlPtr;
typedef const char* VlString;

VL_HT_HASH(int);
VL_HT_HASH(char);
VL_HT_HASH(VlPtr);

VL_API char vl_ht_iterate(void *ht, void **pos, VlHTEntry *entry);

#endif // VELOURS_HT_H
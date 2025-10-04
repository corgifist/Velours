#include "ht.h"

#define FNV_OFFSET_BASIS 0xcbf29ce484222325ULL
#define FNV_PRIME 0x100000001b3ULL

VL_HT_HASH(int) {
	if (!p) return 0;
	return *((int*) p);
}

VL_HT_HASH(char) {
	if (!p) return 0;
	return *((char*) p);
}

VL_HT_HASH(VlPtr) {
	uint64_t hash = FNV_OFFSET_BASIS;

	const unsigned char* bytes = (const unsigned char*)p;
	size_t i;
	for (i = 0; i < sizeof(void*); ++i) {
		hash ^= (uint64_t)bytes[i];
		hash *= FNV_PRIME;
	}
	return hash;
}

VL_HT_HASH(VlString) {
	const u8* s = *((const u8**)p);

    uint64_t hash = FNV_OFFSET_BASIS;
    while (s && *s) {
		hash ^= (uint64_t)(unsigned char)*s;
        hash *= FNV_PRIME;
        ++s;
    }
    return hash;
}

VL_API char vl_ht_iterate(void *table, void **pos, VlHTEntry *entry) {
	if (!*pos) 
		*pos = table;
	VlHTHeader* header = VL_HT_HEADER(table);
	size_t entry_size = (sizeof(uint64_t) + sizeof(char) + header->key_size + header->value_size);
	size_t size = header->cap * entry_size;
	size_t offset = (size_t) *pos - (size_t) table;
	while (offset < size) {
		uint64_t *hash = (uint64_t*) ((char*) table + offset);
		char *is_occupied = (char*)((char*)hash + sizeof(uint64_t));
		void *key = (char*) is_occupied + sizeof(char);
		void *value = (char*) key + header->key_size;
		*pos = (char*) *pos + entry_size;
		offset = (size_t) *pos - (size_t) table;
		if (*is_occupied) {
			entry->key = key;
			entry->value = value;
			return 1;
		}
	}
	return 0;
}
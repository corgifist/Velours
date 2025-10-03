#include "ht.h"

VL_HT_HASH(int) {
	if (!p) return 0;
	return *((int*) p);
}

VL_HT_HASH(char) {
	if (!p) return 0;
	return *((char*) p);
}

VL_HT_HASH(VlPtr) {
	return (uint64_t) (size_t) p;
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
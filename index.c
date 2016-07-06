#include "index.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

bool pack_index_parse(char * indexData, size_t indexSize, struct pack_index * index)
{
	assert(index);

	struct pack_index_entry ** entries = NULL;
	size_t allocSize = 0;
	size_t numEntries = 0;

	size_t iter = 0;
	while(iter < indexSize) {
		struct pack_index_entry * entry = malloc(sizeof(struct pack_index_entry));

		// read the name string
		size_t nameStringLen = strlen(indexData+iter);

		// make sure the string is sane
		if(nameStringLen+1+iter > indexSize) {
			free(entry);
			goto error;
		}

		entry->name = strdup(indexData+iter);
		iter += nameStringLen+1; // skip null

		uint32_t * targets[] = {
			&entry->unk1, &entry->offset, &entry->unk3,
			&entry->compressedSize, &entry->decompressedSize, &entry->crc};

		size_t i;
		for(i = 0; i < sizeof(targets)/sizeof(*targets); i++) {
			if(iter+4 > indexSize) {
				free(entry->name);
				free(entry);
				goto error;
			}

			memcpy(targets[i], indexData+iter, 4);
			iter += 4;
		}

		// more space please!
		if(numEntries >= allocSize) {
			allocSize += 20;
			entries = realloc(entries, sizeof(struct pack_index_entry *)*allocSize);
		}

		entries[numEntries] = entry;
		numEntries++;
	}

	// remove excess
	if(allocSize > numEntries) {
		allocSize = numEntries;
		entries = realloc(entries, sizeof(struct pack_index_entry *)*allocSize);
	}

	index->numEntries = numEntries;
	index->index = entries;

	return true;
error:
	if(entries) {
		free(entries);

		struct pack_index idx;
		idx.numEntries = numEntries;
		idx.index = entries;

		pack_index_free(&idx);
	}

	return false;
}

void pack_index_free(struct pack_index * index)
{
	assert(index);

	size_t i;
	for(i = 0; i < index->numEntries; i++) {
		struct pack_index_entry * e = index->index[i];

		free(e->name);
		free(e);
	}

	free(index->index);
}

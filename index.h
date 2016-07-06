#ifndef PSPACK_INDEX_H
#define PSPACK_INDEX_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

struct pack_index_entry {
  char * name;
  uint32_t unk1; // always zero
  uint32_t offset;
  uint32_t unk3; // always zero
  uint32_t unk4; // deprecated @shaql2 patch 1
  uint32_t unk5; // deprecated @shaql2 patch 1
  uint32_t compressedSize;
  uint32_t decompressedSize;
  uint32_t crc;
};

struct pack_index {
  size_t numEntries;
  struct pack_index_entry ** index;
};

bool pack_index_parse(char * indexData, size_t indexSize, struct pack_index * index);
void pack_index_free(struct pack_index * index);

#endif

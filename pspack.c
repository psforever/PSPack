#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <stdarg.h>
#include <stdbool.h>

#include "asprintf.h"
#include "minilzo.h"
#include "index.h"
#include "fs.h"
#include "util.h"

const char PACK_MAGIC[4] = {'P', 'A', 'C', 'K'};
const char LZO1_MAGIC[4] = {'L', 'Z', 'O', '1'};

bool carve_lzo(FILE * fp, uint32_t compressedSize, char ** decompressed, size_t * decompressedSize);
bool carve_lzo_to_file(FILE * fp, const char * name, uint32_t compressedSize);

struct pack_header
{
  uint8_t magic[4];
  uint32_t version;
  uint32_t compressed_index_size;
  uint32_t decompressed_index_size;
  uint32_t num_files;
  uint32_t unk1;
  uint32_t unk2;
};

int main(int argc, char *argv[])
{
  if(argc < 2) {
    fatal("PACK file argument required");
  }

  if (lzo_init() != LZO_E_OK) {
    fatal("lzo_init() failed\n");
  }

  char * packFileName = argv[1];
  FILE * pFile = fopen(packFileName, "rb");

  if(!pFile) {
    fatal("could not open '%s' for reading", packFileName);
  }

  struct pack_header header;

  if(fread(&header, sizeof(header), 1, pFile) != 1) {
    fatal("PACK file too small (not enough bytes for the complete header)");
  }


  if(memcmp(header.magic, PACK_MAGIC, sizeof(PACK_MAGIC))) {
    fatal("PACK has invalid magic");
  }

  char * indexData = NULL;
  size_t indexDataSize = 0;

  if(!carve_lzo(pFile, header.compressed_index_size, &indexData, &indexDataSize)) {
    fatal("failed to decompress PACK index");
  }

  size_t startOfEntries = ftell(pFile);

  struct pack_index index;
  if(!pack_index_parse(indexData, indexDataSize, &index)) {
    fatal("failed to parse PACK index");
  }

  char * dirName = NULL;
  char *packBaseName = basename(packFileName, false);
  asprintf(&dirName, "%s-out/", packBaseName);

  if(!create_dir(dirName)) {
    fatal("failed to create output directory");
  }

  printf("Outputing files to %s\n", dirName);
  printf("Index listing:\n");

  int i;
  for(i = 0; i < index.numEntries; i++) {
    struct pack_index_entry * e = index.index[i];
    printf("{%d} %30s (compressed size %u -> %u, offset %6u, CRC-32 0x%08x, U1 %u, U3 %u)\n",
        i+1, e->name, e->compressedSize, e->decompressedSize,
        e->offset,
        e->crc,
        e->unk1,
        e->unk3);

    fseek(pFile, e->offset+startOfEntries, SEEK_SET);

    char *outName = NULL;
    asprintf(&outName, "./%s%s", dirName, e->name);

    //printf("Writting %s...\n", outName);

    if(e->compressedSize > 0) {
      if(!carve_lzo_to_file(pFile, outName, e->compressedSize)) {
        fatal("failed to unpack file %s", e->name);
      }
    } else {
      FILE * fp = fopen(outName, "wb"); // create a blank file
      fclose(fp);
    }
  }


  /*uint32_t sizeGood = 0x1df9-compressedSize-8-0x1c-0x7da;
    printf("Size good %u (0x%x)\n", sizeGood, sizeGood);

    carve_lzo(pFile, "file1", sizeGood);
    printf("\n");

  // what is the size for this guy?
  carve_lzo(pFile, "file2", compressedSize);
  printf("\n");*/

  return 0;
}

bool carve_lzo(FILE * fp, uint32_t compressedSize, char ** decompressed, size_t * decompressedSize)
{
  uint32_t lzoCRC = 0;
  char lzoMagic[4] = {0};

  // nothing to decompress, just skip the entry and signal an error
  if(compressedSize == 0)
  {
    *decompressed = NULL;
    *decompressedSize = 0;
    return false;
  }

  size_t start = ftell(fp);
  uint32_t localDecompressedSize;
  if(fread(&localDecompressedSize, 4, 1, fp) != 1) { // decompressed size
    return false;
  }

  fread(&lzoCRC, 4, 1, fp); // CRC
  fread(&lzoMagic, 1, 4, fp); // LZO magic

  if(memcmp(lzoMagic, LZO1_MAGIC, 4) != 0) {
    fatal("LZO magic mismatch");
  }

  size_t adj = 4 + 8;
  if(compressedSize < adj) {
    fatal("compressed size too small, ds %zu", localDecompressedSize);
  }

  compressedSize -= adj;

  bool disableDecompression = false;

  // The MSB of the decompressed size is set when no compression actually took place
  if(localDecompressedSize & 0x80000000) {
    localDecompressedSize &= ~0x80000000;
    disableDecompression = true;
  }

  //printf("Extracting LZO object at 0x%x (compressed %zu+12, decompressed %zu %x)\n",
      //start, compressedSize, localDecompressedSize, localDecompressedSize);

  const char * compressed = malloc(compressedSize);

  if(!compressed) {
    fatal("failed to allocate compressed memory");
  }

  if(fread(compressed, sizeof(char), compressedSize, fp) != compressedSize) {
    fatal("ran out of bytes when reading compressed data");
  }

  if(disableDecompression) {
    // Note: no need to free compressed as we are handing it off to the caller
    *decompressed = compressed;
    *decompressedSize = localDecompressedSize;

    return true;
  } else {
    uint32_t decompressedNewSize = localDecompressedSize;
    char * localDecompressed = malloc(localDecompressedSize);

    if(!localDecompressed) {
      fatal("failed to allocate decompressed memory");
    }

    int r = lzo1x_decompress(compressed, compressedSize, localDecompressed, &decompressedNewSize, NULL);

    // free compressed as we are done with that memory
    free(compressed);

    if (r == LZO_E_OK) {
      //printf("decompressed %u bytes\n", decompressedNewSize);

      *decompressed = localDecompressed;
      *decompressedSize = decompressedNewSize;

      return true;
    }
    else
    {
      printf("internal error - decompression failed: %d\n", r);
      printf("newSize %u, oldSize %u\n", decompressedNewSize, localDecompressedSize);

      free(localDecompressed);
      return false;
    }
  }
}

bool carve_lzo_to_file(FILE * fp, const char * name, uint32_t compressedSize)
{
  char * decompressed = NULL;
  size_t decompressedSize = 0;

  if(!carve_lzo(fp, compressedSize, &decompressed, &decompressedSize))
    return false;

  FILE * outFile = fopen(name, "wb");

  if(!outFile)
    fatal("failed to open output file for writing");

  if(fwrite(decompressed, sizeof(char), decompressedSize, outFile) != decompressedSize) {
    fatal("failed to write all bytes to output file");
  }

  fclose(outFile);

  free(decompressed);

  return true;
}

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

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

const char PACK_MAGIC[4] = {'P', 'A', 'C', 'K'};
const char LZO1_MAGIC[4] = {'L', 'Z', 'O', '1'};

bool carve_lzo(FILE * fp, uint32_t compressedSize, char ** decompressed, size_t * decompressedSize);
bool carve_lzo_to_file(FILE * fp, const char * name, uint32_t compressedSize);

int main(int argc, char *argv[])
{
	char packMethod[2];
	char * packFileName;
	char * packFolder;

	if (argc < 2) {
		printf("Would you like to "	ANSI_COLOR_GREEN	"extract"	ANSI_COLOR_RESET
					" or "	ANSI_COLOR_CYAN	"pack"	ANSI_COLOR_RESET	" a set? [E/P] ");

		scanf("%1c", &packMethod);
	} else if (argv[1] != 'E' || argv[1] != 'e' || argv[1] != 'P' || argv[1] != 'p') {
		packMethod[0] = 'E';
		strcpy(packFileName, argv[1]);
	} else {
		strcpy(packMethod, argv[1]);
	}

	switch (*packMethod) {
		case 'e':
		case 'E':
			if (!packFileName) {
				if (argc < 3) {
					printf("Please provide the path of the pack (.PAK) file: ");

					scanf("%s", &packFileName);
				} else {
					strcpy(packFileName, argv[2]);
				}
			}

			FILE * pFile = fopen(packFileName, "rb");

			uint32_t readData;
			fread(&readData, 4, 1, pFile); // PACK

			if(memcmp(&readData, PACK_MAGIC, sizeof(PACK_MAGIC))) {
				fatal("PACK has invalid magic");
			}

			uint32_t compressedSize = 0;

			// PACK header
			fread(&readData, 4, 1, pFile); // unk

			fread(&compressedSize, 4, 1, pFile); // compressed index size

			fread(&readData, 4, 1, pFile); // unk
			fread(&readData, 4, 1, pFile); // unk
			fread(&readData, 4, 1, pFile); // unk
			fread(&readData, 4, 1, pFile); // unk
			//printf("0x%08x (%ud)\n", readData, readData);


			char * indexData = NULL;
			size_t indexDataSize = 0;

			if(!carve_lzo(pFile, compressedSize, &indexData, &indexDataSize)) {
				fatal("failed to read PACK index");
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
				printf("{%d} %30s (compressed size %6d, offset %6u, U3 %u, U4 %u, U5 %u)\n",
					i+1, e->name, e->compressedSize,
					e->offset,
					e->unk3,
					e->unk4,
					e->unk5);

				fseek(pFile, e->offset+startOfEntries, SEEK_SET);

				char *outName = NULL;
				asprintf(&outName, "./%s%s", dirName, e->name);
				//printf("Writting %s...\n", outName);
				carve_lzo_to_file(pFile, outName, e->compressedSize);
			}


			/*uint32_t sizeGood = 0x1df9-compressedSize-8-0x1c-0x7da;
				printf("Size good %u (0x%x)\n", sizeGood, sizeGood);

				carve_lzo(pFile, "file1", sizeGood);
				printf("\n");

			// what is the size for this guy?
			carve_lzo(pFile, "file2", compressedSize);
			printf("\n");*/
			break;
		case 'p':
		case 'P':
			fatal(ANSI_COLOR_RED	"We cannot yet pack a set."	ANSI_COLOR_RESET);
			break;
		default:
			fatal(ANSI_COLOR_RED	"You must select either E (Extract) or P (Pack) to continue."	ANSI_COLOR_RESET);
		break;
	}

	return 0;
}

bool carve_lzo(FILE * fp, uint32_t compressedSize, char ** decompressed, size_t * decompressedSize)
{
  uint32_t lzoCRC = 0;
  char lzoMagic[4] = {0};

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
    fatal("compressed size too small");
  }

  compressedSize -= adj;

  bool disableDecompression = false;

  // looks like a decompression error
  if(localDecompressedSize & 0x80000000) {
    printf("warning: encountered file that wasn't compressed\n");
    //localDecompressedSize &= ~0x80000000;
    disableDecompression = true;
  }

  printf("Extracting LZO object at 0x%x (compressed %zu+12, decompressed %zu %x)\n",
      start, compressedSize, localDecompressedSize, localDecompressedSize);


  const char * compressed = malloc(compressedSize);

  if(!compressed) {
    fatal("failed to allocate compressed memory");
  }

  if(fread(compressed, sizeof(char), compressedSize, fp) != compressedSize) {
    fatal("ran out of bytes when reading compressed data");
  }

  if(disableDecompression) {
    *decompressed = compressed;
    *decompressedSize = compressedSize;

    return true;
  } else {
    uint32_t decompressedNewSize = localDecompressedSize;
    char * localDecompressed = malloc(localDecompressedSize);

    if(!localDecompressed) {
      fatal("failed to allocate decompressed memory");
    }

    int r = lzo1x_decompress(compressed, compressedSize, localDecompressed, &decompressedNewSize, NULL);

    free(compressed);

    if (r == LZO_E_OK) {
      printf("decompressed %u bytes\n", decompressedNewSize);

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
    fatal("failed to write all byte to output file");
  }

  fclose(outFile);

  free(decompressed);

  return true;
}

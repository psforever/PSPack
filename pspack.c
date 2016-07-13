// Include standard libraries.
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <stdarg.h>
#include <stdbool.h>
#include <unistd.h>

// Include local libraries.
#include "asprintf.h"
#include "minilzo.h"
#include "index.h"
#include "fs.h"
#include "util.h"
#include "prompt.h"

//////////// GLOBALS
// Define various pack versions for their respectable repositories.
#define VERSION_MAJOR 1
#define VERSION_MINOR 0
#define VERSION_REVISION 0

// Set the magic constants used for packaging.
const char PACK_MAGIC[4] = {'P', 'A', 'C', 'K'};
const char LZO1_MAGIC[4] = {'L', 'Z', 'O', '1'};
int g_verbose = 0;
int g_debug = 0;

//////////// FUNCTIONS
void banner();
bool extractPack(char * path);
bool carve_lzo(FILE * fp, uint32_t compressedSize, char ** decompressed, size_t * decompressedSize);
bool carve_lzo_to_file(FILE * fp, const char * name, uint32_t compressedSize);

//////////// TYPES
// Set the pack header structure/object values.
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

enum pack_method
{
  METHOD_NONE,
  METHOD_EXTRACT,
  METHOD_CREATE
};

int main(int argc, char ** argv)
{
	// Define the variables needed to handle getopt.
	int	args;
	enum pack_method method = METHOD_NONE;
	char * arguments = NULL;

	if(!is_terminal(stdout))
	{
		colors_disable();
	}
        else
        {
                // When running under cygwin, stdout appears to be a pipe, 
                // which means stdout is fully buffered (bad)
                // Fix this by enabling line buffering
                if(is_cygwin())
                      setvbuf(stdout, NULL, _IOLBF, 4096);
        }

	banner();

	// While there are arguments passed into the system.
	while ((args = getopt(argc, argv, ":dvc:x:")) != -1)
	{
		switch (args)
		{
		// case 'create':
		case 'c':
			// Assign the pack method.
			method = METHOD_CREATE;

			// Assign additional arguments.
			arguments = strdup(optarg);

			break;
		// case 'extract':
		case 'x':
			// Assign the pack method.
			method = METHOD_EXTRACT;

			// Assign additional arguments.
			arguments = strdup(optarg);

			break;
		case 'v':
			g_verbose++;
			break;
		case 'd':
			g_debug++;
			break;
		case '?':
			fatal("Unknown option '%c'", optopt);
			break;
		case ':':
			fatal("Missing required argument for '%c'", optopt);
			break;
		}
	}

	// If there is no pack method defined.
	if (method == METHOD_NONE)
	{
		if(!is_terminal(stdin))
		{
			fatal("Can only be interactive when reading from a terminal");
		}

		char * prompt = NULL;

		// Prompt the user for input.
		asprintf(&prompt, "Would you like to e%sx%stract or %sc%sreate a pack? [%sx%s/%sc%s/%sq%s] ",
		    AC_CYAN, AC_RESET,
		    AC_YELLOW, AC_RESET,
		    AC_CYAN, AC_RESET,
		    AC_YELLOW, AC_RESET,
		    AC_RED, AC_RESET);

		prompt_choice_t choices[] = {
		  {"c", "create", 'c'},
		  {"x", "extract", 'x'},
		  {"q", "quit", 'q'},
		  {0, 0, 0}
		};

		int choice = prompt_choice(prompt, choices, false, true);

		switch (choice)
		{
		case 'c': case 'C':
			method = METHOD_CREATE;
			break;
		case 'x': case 'X':
			method = METHOD_EXTRACT;
			break;
		case 'q': case 'Q':
			method = METHOD_NONE;
			break;
		default:
			fatal("%d You must choose to extract or create a pack to continue.", choice);
			break;
		}
	}

	if(method == METHOD_NONE)
	{
		// Warn the user that they've decided to quit.
		printf("%sQuitting.%s\n", AC_YELLOW, AC_RESET);
	}
	else if(method == METHOD_EXTRACT)
	{
		// Run the extractor.
		extractPack(arguments);
	}
	else if(method == METHOD_CREATE)
	{
		fatal("We cannot yet create a pack.");
	}

	return 0;
}

// Output the banner in ASCII format using ANSI colors.
void banner()
{
	printf("\n");
	printf("   _______/\\\n");
	printf("  /      /\\ \\       %s   ___  _______           __  \n%s", AC_BLUE, AC_RESET);
	printf(" / .PAK /, \\/_/\\    %s  / _ \\/ __/ _ \\___ _____/ /__\n%s", AC_BLUE, AC_RESET);
	printf("/______/. '/ / /     %s/ ___/\\ \\/ ___/ _ `/ __/  '_/\n%s", AC_MAGENTA, AC_RESET);
	printf("\\     /\\ \"/ / /     %s/_/  /___/_/   \\_,_/\\__/_/\\_\\ \n%s", AC_RED, AC_RESET);
	printf(" \\____\\ \\/__\\/    PSPack v%d.%d.%d - PSForever Project\n", VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION);
	printf("       \\/\n\n");
}

bool extractPack(char * path)
{
	// Define variables necessary to compute pack extraction.
	char * packFileName = NULL;

	// If there is no path.
	if (!path)
	{
		// Prompt the user for input.
		packFileName = prompt_string("Please provide the path of the pack (.PAK) file: ");
		if(!packFileName)
		{
		  fatal("failed to read PAK path");
		}
	} else {
		packFileName = path;
	}

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

	if(g_debug >= 1)
	{
		printf("PACK v.%"PRIu32", 0x%"PRIx32", 0x%"PRIx32"\n",
		    header.version, header.unk1, header.unk2);
	}

	char * indexData = NULL;
	size_t indexDataSize = 0;

	if(!carve_lzo(pFile, header.compressed_index_size, &indexData, &indexDataSize)) {
		fatal("failed to decompress PACK index");
	}

	if(indexDataSize != header.decompressed_index_size) {
		warning("PACK header's decompressed index size (%"PRIu32") != actual index size (%"PRIuSZT")",
		    header.decompressed_index_size, indexDataSize);
	}

	size_t startOfEntries = ftell(pFile);

	struct pack_index index;
	if(!pack_index_parse(indexData, indexDataSize, &index)) {
		fatal("failed to parse PACK index");
	}

	if(index.numEntries != header.num_files)
	{
	  warning("index has %"PRIuSZT" files, but pack header says we should have %"PRIu32,
	      index.numEntries, header.num_files);
	}

	char * dirName = NULL;
	char *packBaseName = basename(packFileName, false);
	asprintf(&dirName, "%s-out/", packBaseName);

	printf("Extracting %"PRIu32" files to %s\n",
	    header.num_files, dirName);

	if(!create_dir(dirName)) {
		fatal("failed to create output directory");
	}

	int i;
	for(i = 0; i < index.numEntries; i++) {
		struct pack_index_entry * e = index.index[i];

		if(g_verbose >= 1)
		{
			printf("{%d} %30s (compressed size %u -> %u, offset %6u, CRC-32 0x%08x, U1 %u, U3 %u)\n",
				i+1, e->name, e->compressedSize, e->decompressedSize,
				e->offset,
				e->crc,
				e->unk1,
				e->unk3);
		}

		fseek(pFile, e->offset+startOfEntries, SEEK_SET);

		char *outName = NULL;
		asprintf(&outName, "./%s%s", dirName, e->name);

		if(e->compressedSize > 0) {
			if(!carve_lzo_to_file(pFile, outName, e->compressedSize)) {
				fatal("failed to unpack file %s", e->name);
			}
		} else {
			FILE * fp = fopen(outName, "wb"); // create a blank file
			fclose(fp);
		}
	}

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

  long start = ftell(fp);

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
    fatal("compressed size too small, ds %"PRIu32, localDecompressedSize);
  }

  compressedSize -= adj;

  bool disableDecompression = false;

  // The MSB of the decompressed size is set when no compression actually took place
  if(localDecompressedSize & 0x80000000) {
    localDecompressedSize &= ~0x80000000;
    disableDecompression = true;
  }

  if(g_debug >= 1)
  {
	    printf("Extracting LZO object at 0x%ld (compressed %"PRIu32"+12, decompressed %"PRIu32" %"PRIx32")\n",
		start, compressedSize, localDecompressedSize, localDecompressedSize);
  }

  char * compressed = malloc(compressedSize);

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

    int r = lzo1x_decompress((const unsigned char *)compressed, compressedSize, (unsigned char *)localDecompressed, &decompressedNewSize, NULL);

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
      printf("LZO: internal error - decompression failed: %d\n", r);

      if(g_debug >= 1)
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
